// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "Async/Async.h"

/**
 * Retry configuration with exponential backoff
 */
struct RIFTBORNAI_API FRetryConfig
{
	/** Maximum number of retry attempts */
	int32 MaxRetries = 3;
	
	/** Initial delay between retries (seconds) */
	float InitialDelaySeconds = 1.0f;
	
	/** Maximum delay between retries (seconds) */
	float MaxDelaySeconds = 30.0f;
	
	/** Exponential backoff multiplier (2.0 = double delay each retry) */
	float BackoffMultiplier = 2.0f;
	
	/** Add random jitter to prevent thundering herd (0.0-1.0) */
	float JitterFactor = 0.1f;
	
	/** HTTP status codes that should trigger retry (e.g., 429, 500, 502, 503, 504) */
	TArray<int32> RetryableStatusCodes = {429, 500, 502, 503, 504};
	
	/** Whether to retry on timeout */
	bool bRetryOnTimeout = true;
	
	/** Whether to retry on connection failure */
	bool bRetryOnConnectionFailure = true;
	
	/** Default config for Claude API */
	static FRetryConfig ForClaude()
	{
		FRetryConfig Config;
		Config.MaxRetries = 3;
		Config.InitialDelaySeconds = 1.0f;
		Config.MaxDelaySeconds = 60.0f;  // Claude rate limits can be long
		Config.RetryableStatusCodes = {429, 500, 502, 503, 504, 529};  // 529 = overloaded
		return Config;
	}
	
	/** Default config for OpenAI API */
	static FRetryConfig ForOpenAI()
	{
		FRetryConfig Config;
		Config.MaxRetries = 1;  // One retry only — avoid cascading 120s timeouts
		Config.InitialDelaySeconds = 2.0f;
		Config.MaxDelaySeconds = 30.0f;
		Config.RetryableStatusCodes = {429, 500, 502, 503, 504};
		Config.bRetryOnTimeout = false;  // Don't retry timeouts — they cascade
		return Config;
	}
	
	/** Default config for local Ollama */
	static FRetryConfig ForOllama()
	{
		FRetryConfig Config;
		Config.MaxRetries = 2;  // Fewer retries for local
		Config.InitialDelaySeconds = 0.5f;
		Config.MaxDelaySeconds = 10.0f;
		return Config;
	}
	
	/** Aggressive config for high-reliability scenarios */
	static FRetryConfig Aggressive()
	{
		FRetryConfig Config;
		Config.MaxRetries = 5;
		Config.InitialDelaySeconds = 0.5f;
		Config.MaxDelaySeconds = 120.0f;
		Config.BackoffMultiplier = 2.0f;
		return Config;
	}
};

/**
 * Result of a retry attempt
 */
struct RIFTBORNAI_API FRetryResult
{
	bool bSuccess = false;
	FString Response;
	FString ErrorMessage;
	int32 AttemptsMade = 0;
	float TotalTimeSeconds = 0.0f;
	int32 LastStatusCode = 0;
	TArray<FString> AttemptErrors;
};

/**
 * Retry policy with exponential backoff for LLM API calls
 * 
 * Usage:
 *   FRetryPolicy Retry(FRetryConfig::ForClaude());
 *   Retry.ExecuteWithRetry(
 *       [](auto OnDone) { MakeHttpRequest(OnDone); },
 *       [](bool bOk, FString Resp) { HandleResult(bOk, Resp); }
 *   );
 */
class RIFTBORNAI_API FRetryPolicy
{
public:
	explicit FRetryPolicy(const FRetryConfig& InConfig = FRetryConfig())
		: Config(InConfig)
	{
	}
	
	/**
	 * Calculate delay for a specific attempt number (0-indexed)
	 */
	float CalculateDelay(int32 AttemptNumber) const
	{
		float Delay = Config.InitialDelaySeconds * FMath::Pow(Config.BackoffMultiplier, static_cast<float>(AttemptNumber));
		Delay = FMath::Min(Delay, Config.MaxDelaySeconds);
		
		// Add jitter
		if (Config.JitterFactor > 0.0f)
		{
			float Jitter = FMath::FRandRange(-Config.JitterFactor, Config.JitterFactor) * Delay;
			Delay += Jitter;
		}
		
		return FMath::Max(Delay, 0.0f);
	}
	
	/**
	 * Check if an HTTP status code is retryable
	 */
	bool IsRetryableStatusCode(int32 StatusCode) const
	{
		return Config.RetryableStatusCodes.Contains(StatusCode);
	}
	
	/**
	 * Check if an error should trigger retry
	 */
	bool ShouldRetry(bool bConnectionSuccess, int32 StatusCode, const FString& ErrorMessage) const
	{
		// Connection failure
		if (!bConnectionSuccess && Config.bRetryOnConnectionFailure)
		{
			return true;
		}
		
		// Timeout
		if (ErrorMessage.Contains(TEXT("timeout"), ESearchCase::IgnoreCase) && Config.bRetryOnTimeout)
		{
			return true;
		}
		
		// Retryable status code
		if (IsRetryableStatusCode(StatusCode))
		{
			return true;
		}
		
		return false;
	}
	
	/**
	 * Execute an async operation with retry logic
	 * 
	 * @param Operation - Function that takes a completion callback
	 * @param OnComplete - Final completion callback after all retries
	 */
	void ExecuteWithRetry(
		TFunction<void(TFunction<void(bool bSuccess, int32 StatusCode, const FString& Response)>)> Operation,
		TFunction<void(const FRetryResult& Result)> OnComplete)
	{
		// Capture state for the retry loop
		TSharedPtr<int32> AttemptCount = MakeShared<int32>(0);
		TSharedPtr<float> StartTime = MakeShared<float>(static_cast<float>(FPlatformTime::Seconds()));
		TSharedPtr<TArray<FString>> Errors = MakeShared<TArray<FString>>();
		TSharedPtr<FRetryResult> FinalResult = MakeShared<FRetryResult>();
		
		// The retry loop lambda
		TSharedPtr<TFunction<void()>> RetryLoop = MakeShared<TFunction<void()>>();
		
		*RetryLoop = [this, Operation, OnComplete, AttemptCount, StartTime, Errors, FinalResult, RetryLoop]()
		{
			(*AttemptCount)++;
			
			Operation([this, OnComplete, AttemptCount, StartTime, Errors, FinalResult, RetryLoop]
				(bool bSuccess, int32 StatusCode, const FString& Response)
			{
				FinalResult->LastStatusCode = StatusCode;
				FinalResult->AttemptsMade = *AttemptCount;
				FinalResult->TotalTimeSeconds = static_cast<float>(FPlatformTime::Seconds()) - *StartTime;
				
				if (bSuccess && (StatusCode >= 200 && StatusCode < 300))
				{
					// Success!
					FinalResult->bSuccess = true;
					FinalResult->Response = Response;
					FinalResult->AttemptErrors = *Errors;
					OnComplete(*FinalResult);
					return;
				}
				
				// Record this error
				FString ErrorMsg = FString::Printf(TEXT("Attempt %d: HTTP %d - %s"), 
					*AttemptCount, StatusCode, *Response.Left(200));
				Errors->Add(ErrorMsg);
				
				// Check if we should retry
				bool bShouldRetry = ShouldRetry(bSuccess, StatusCode, Response);
				bool bHasRetriesLeft = *AttemptCount < Config.MaxRetries;
				
				if (bShouldRetry && bHasRetriesLeft)
				{
					// Calculate delay
					float DelaySeconds = CalculateDelay(*AttemptCount - 1);
					
					UE_LOG(LogTemp, Warning, TEXT("Retry %d/%d in %.1fs: %s"),
						*AttemptCount, Config.MaxRetries, DelaySeconds, *ErrorMsg);
					
					// Schedule retry
					FTimerHandle TimerHandle;
					if (GEngine)
					{
						// Use async delay that works without world context
						AsyncTask(ENamedThreads::GameThread, [DelaySeconds, RetryLoop]()
						{
							// Use FTSTicker for delayed execution without requiring a world
							FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([RetryLoop](float DeltaTime) -> bool
							{
								if (RetryLoop.IsValid() && *RetryLoop)
								{
									(*RetryLoop)();
								}
								return false; // Don't repeat
							}), DelaySeconds);
						});
					}
					else
					{
						// Fallback: immediate retry (for non-game contexts)
						(*RetryLoop)();
					}
				}
				else
				{
					// No more retries - return failure
					FinalResult->bSuccess = false;
					FinalResult->ErrorMessage = FString::Printf(
						TEXT("Failed after %d attempts. Last error: %s"),
						*AttemptCount, *Response.Left(500));
					FinalResult->AttemptErrors = *Errors;
					OnComplete(*FinalResult);
				}
			});
		};
		
		// Start the first attempt
		(*RetryLoop)();
	}
	
	/**
	 * Get current configuration
	 */
	const FRetryConfig& GetConfig() const { return Config; }
	
private:
	FRetryConfig Config;
};

/**
 * Helper macro for adding retry to existing provider calls
 */
#define RIFTBORN_WITH_RETRY(Config, Operation, OnComplete) \
	do { \
		FRetryPolicy _RetryPolicy(Config); \
		_RetryPolicy.ExecuteWithRetry(Operation, OnComplete); \
	} while(0)
