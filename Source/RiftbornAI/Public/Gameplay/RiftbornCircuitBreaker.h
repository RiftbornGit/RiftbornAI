// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"

/**
 * Circuit breaker states
 */
enum class ECircuitState : uint8
{
	Closed,     // Normal operation - requests allowed
	Open,       // Failing - requests blocked
	HalfOpen    // Testing - limited requests allowed
};

/**
 * Circuit breaker configuration
 */
struct FCircuitBreakerConfig
{
	/** Number of failures before opening circuit */
	int32 FailureThreshold = 5;
	
	/** Optional friendly name for logging/compatibility helpers */
	FString Name = TEXT("Default");

	/** Time to wait before attempting recovery (seconds) */
	double RecoveryTimeoutSeconds = 60.0;
	
	/** Number of successes needed to close circuit from half-open */
	int32 SuccessThreshold = 2;
	
	/** Timeout for individual requests (seconds) */
	double RequestTimeoutSeconds = 30.0;

	static FCircuitBreakerConfig ForOllama()
	{
		FCircuitBreakerConfig Config;
		Config.Name = TEXT("Ollama");
		Config.FailureThreshold = 3;
		Config.RecoveryTimeoutSeconds = 60.0;
		Config.SuccessThreshold = 1;
		return Config;
	}

	static FCircuitBreakerConfig ForTool()
	{
		FCircuitBreakerConfig Config;
		Config.Name = TEXT("Tool");
		Config.FailureThreshold = 10;
		Config.RecoveryTimeoutSeconds = 10.0;
		Config.SuccessThreshold = 2;
		return Config;
	}
};

/**
 * Circuit breaker for protecting against cascading failures
 * 
 * Use this to wrap external API calls (Claude, OpenAI, etc.)
 * When too many failures occur, the circuit opens and fails fast
 * without making actual requests.
 * 
 * Thread-safe for use from any thread.
 */
class RIFTBORNAI_API FCircuitBreaker
{
public:
	FCircuitBreaker(const FString& InName, const FCircuitBreakerConfig& InConfig = FCircuitBreakerConfig());

	/**
	 * Check if a request is allowed
	 * @return true if the circuit is closed or half-open
	 */
	bool IsRequestAllowed();

	/** Backward-compatible alias for legacy callers. */
	bool CanExecute() { return IsRequestAllowed(); }
	
	/**
	 * Record a successful request
	 */
	void RecordSuccess();
	
	/**
	 * Record a failed request
	 */
	void RecordFailure();

	/** Backward-compatible helper for legacy callers. */
	void RecordResult(bool bSuccess) { bSuccess ? RecordSuccess() : RecordFailure(); }
	
	/**
	 * Get current circuit state
	 */
	ECircuitState GetState() const { return State; }
	
	/**
	 * Get state as string for logging
	 */
	FString GetStateString() const;
	
	/**
	 * Get number of consecutive failures
	 */
	int32 GetConsecutiveFailures() const { return ConsecutiveFailures; }

	/**
	 * Get configured failure threshold for status reporting
	 */
	int32 GetFailureThreshold() const { return Config.FailureThreshold; }
	
	/**
	 * Reset the circuit breaker to closed state
	 */
	void Reset();
	
	/**
	 * Get circuit breaker name
	 */
	const FString& GetName() const { return Name; }
	
	/**
	 * Execute a function with circuit breaker protection
	 * @param Func - Function to execute
	 * @param OutResult - Result from function
	 * @return true if function was executed, false if circuit was open
	 */
	template<typename TResult>
	bool Execute(TFunction<TResult()> Func, TResult& OutResult)
	{
		if (!IsRequestAllowed())
		{
			return false;
		}
		
		try
		{
			OutResult = Func();
			RecordSuccess();
			return true;
		}
		catch (...)
		{
			RecordFailure();
			throw;
		}
	}

private:
	void TransitionToState(ECircuitState NewState);
	
	FString Name;
	FCircuitBreakerConfig Config;
	
	ECircuitState State = ECircuitState::Closed;
	int32 ConsecutiveFailures = 0;
	int32 ConsecutiveSuccesses = 0;
	double LastFailureTime = 0.0;
	double LastStateChangeTime = 0.0;
	
	mutable FCriticalSection Mutex;
};

/**
 * Manager for multiple circuit breakers
 */
class RIFTBORNAI_API FCircuitBreakerManager
{
public:
	static FCircuitBreakerManager& Get();
	
	/**
	 * Get or create a circuit breaker by name
	 */
	TSharedPtr<FCircuitBreaker> GetCircuitBreaker(const FString& Name, const FCircuitBreakerConfig& Config = FCircuitBreakerConfig());
	
	/**
	 * Get all circuit breaker statuses for monitoring
	 */
	TMap<FString, ECircuitState> GetAllStates() const;
	
	/**
	 * Reset all circuit breakers
	 */
	void ResetAll();

private:
	FCircuitBreakerManager() = default;
	
	TMap<FString, TSharedPtr<FCircuitBreaker>> CircuitBreakers;
	mutable FCriticalSection Mutex;
};
