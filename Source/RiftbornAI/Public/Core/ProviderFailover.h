// Copyright RiftbornAI. All Rights Reserved.
// ProviderFailover - Automatic cross-provider failover when primary provider is down
// Gap #9: Multi-provider auto-failover

#pragma once

#include "CoreMinimal.h"
#include "IAIProvider.h"
#include "RiftbornRetryPolicy.h"

/**
 * Health status of a single provider
 */
struct RIFTBORNAI_API FProviderHealth
{
	FString ProviderName;
	
	/** Is the provider currently considered healthy? */
	bool bHealthy = true;
	
	/** Consecutive failures (reset on success) */
	int32 ConsecutiveFailures = 0;
	
	/** Total requests since last health check */
	int32 TotalRequests = 0;
	
	/** Total successes since last health check */
	int32 TotalSuccesses = 0;
	
	/** When the provider was marked unhealthy */
	double UnhealthyTimestamp = 0.0;
	
	/** Cooldown before trying this provider again (seconds) */
	float CooldownSeconds = 60.0f;
	
	/** Average latency (TTFT) in seconds */
	float AverageLatencySeconds = 0.0f;
	
	/** Last error message */
	FString LastError;
	
	/** Number of consecutive failures before marking unhealthy */
	static constexpr int32 UNHEALTHY_THRESHOLD = 3;
	
	/** Time to wait before retrying an unhealthy provider (seconds) */
	static constexpr float DEFAULT_COOLDOWN = 60.0f;
	
	/** Is the cooldown expired? (i.e., should we try this provider again?) */
	bool IsCooldownExpired() const
	{
		if (bHealthy) return true;
		return (FPlatformTime::Seconds() - UnhealthyTimestamp) > CooldownSeconds;
	}
	
	/** Record a successful request */
	void RecordSuccess(float LatencySeconds)
	{
		TotalRequests++;
		TotalSuccesses++;
		ConsecutiveFailures = 0;
		bHealthy = true;
		
		// Update rolling average
		AverageLatencySeconds = (AverageLatencySeconds * (TotalSuccesses - 1) + LatencySeconds) / TotalSuccesses;
	}
	
	/** Record a failed request */
	void RecordFailure(const FString& Error)
	{
		TotalRequests++;
		ConsecutiveFailures++;
		LastError = Error;
		
		if (ConsecutiveFailures >= UNHEALTHY_THRESHOLD)
		{
			bHealthy = false;
			UnhealthyTimestamp = FPlatformTime::Seconds();
		}
	}
	
	/** Get success rate (0.0 to 1.0) */
	float GetSuccessRate() const
	{
		return TotalRequests > 0 ? static_cast<float>(TotalSuccesses) / TotalRequests : 1.0f;
	}
};

/**
 * Result of a failover attempt
 */
struct RIFTBORNAI_API FFailoverResult
{
	bool bSuccess = false;
	FString ProviderUsed;
	int32 ProvidersAttempted = 0;
	TArray<FString> AttemptLog;
	FString Response;
	FString ErrorMessage;
	float TotalTimeSeconds = 0.0f;
};

/**
 * Provider Failover Manager
 * 
 * Wraps IAIProvider with automatic cross-provider failover.
 * If the primary provider (e.g., Claude) fails after retries,
 * automatically tries backup providers (e.g., OpenAI → Ollama).
 * 
 * Features:
 *   - Health tracking per provider (circuit breaker pattern)
 *   - Automatic failover chain with priority ordering
 *   - Cooldown for unhealthy providers (don't keep hammering a down service)
 *   - Transparent to callers — same IAIProvider interface
 *   - Configurable failover order
 *   - Metrics: which providers succeeded, latency per provider
 * 
 * This is the missing piece that makes RiftbornAI reliable in production:
 * users should NEVER see "Claude is down" — they should see their request
 * handled seamlessly by a backup provider.
 */
class RIFTBORNAI_API FProviderFailover
{
public:
	static FProviderFailover& Get();
	
	/**
	 * Send a message with automatic failover across providers.
	 * Tries providers in priority order until one succeeds.
	 * 
	 * @param Message The user message to send
	 * @param OnComplete Callback with the result
	 * @param PreferredProvider Optional: try this provider first (empty = use default order)
	 */
	void SendWithFailover(
		const FString& Message,
		TFunction<void(const FFailoverResult& Result)> OnComplete,
		const FString& PreferredProvider = TEXT(""));
	
	/**
	 * Send a message with tools, with automatic failover.
	 * The system prompt and tools must be set before calling.
	 * 
	 * @param Message The user message
	 * @param RequestId Request ID for correlation
	 * @param OnComplete Called on success or after all providers exhausted
	 * @param OnProgress Optional progress updates
	 * @param PreferredProvider Optional: try this first
	 */
	void SendWithToolsFailover(
		const FString& Message,
		const FString& RequestId,
		TFunction<void(bool bSuccess, const FString& Response, const FString& RequestId, const FString& ProviderUsed)> OnComplete,
		TFunction<void(const FString& Status)> OnProgress = nullptr,
		const FString& PreferredProvider = TEXT(""));
	
	/**
	 * Get health status for all tracked providers.
	 */
	TMap<FString, FProviderHealth> GetAllProviderHealth() const;
	
	/**
	 * Get health for a specific provider.
	 */
	FProviderHealth GetProviderHealth(const FString& ProviderName) const;
	
	/**
	 * Get the ordered failover chain (healthy providers first, then cooled-down unhealthy).
	 */
	TArray<FString> GetFailoverChain(const FString& PreferredProvider = TEXT("")) const;
	
	/**
	 * Manually mark a provider as healthy or unhealthy.
	 */
	void SetProviderHealthy(const FString& ProviderName, bool bHealthy);
	
	/**
	 * Set the failover priority order.
	 * Default: Claude → OpenAI → Ollama
	 */
	void SetFailoverOrder(const TArray<FString>& Order);
	
	/**
	 * Get a summary of provider health for display.
	 */
	FString GetHealthSummary() const;
	
	/**
	 * Reset all health tracking (e.g., after configuration change).
	 */
	void ResetHealth();
	
	/**
	 * Report a successful request for a provider.
	 * Updates health tracking.
	 */
	void ReportSuccess(const FString& ProviderName, float LatencySeconds = 0.0f);
	
	/**
	 * Report a failed request for a provider.
	 * Updates health tracking. If threshold exceeded, marks provider unhealthy.
	 */
	void ReportFailure(const FString& ProviderName, const FString& ErrorMessage);
	
	/**
	 * Get the next healthy provider to use after the given one has failed.
	 * Creates and configures the provider via FAIProviderFactory.
	 * Returns nullptr if no healthy alternatives exist.
	 */
	TSharedPtr<IAIProvider> GetNextHealthyProvider(const FString& FailedProviderName);

private:
	FProviderFailover();
	
	/** Try a single provider, with internal retry. Returns immediately via callback. */
	void TryProvider(
		const FString& ProviderName,
		const FString& Message,
		TFunction<void(bool bSuccess, const FString& Response, float LatencySeconds)> OnResult);
	
	/** Try a single provider with tools. */
	void TryProviderWithTools(
		const FString& ProviderName,
		const FString& Message,
		const FString& RequestId,
		TFunction<void(bool bSuccess, const FString& Response, float LatencySeconds)> OnResult,
		TFunction<void(const FString& Status)> OnProgress);
	
	/** Provider health tracking */
	TMap<FString, FProviderHealth> ProviderHealthMap;
	
	/** Failover priority order */
	TArray<FString> FailoverOrder;
	
	/** Cached provider instances — avoids re-creating on every failover attempt (LOW-2).
	 *  Invalidated when a provider is marked unhealthy. */
	TMap<FString, TSharedPtr<IAIProvider>> ProviderCache;
	
	/** Get or create a provider, returning cached instance if still healthy. */
	TSharedPtr<IAIProvider> GetOrCreateProvider(const FString& ProviderName);
	
	/** Thread safety */
	mutable FCriticalSection HealthLock;
};
