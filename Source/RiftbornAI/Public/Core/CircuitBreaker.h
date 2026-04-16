// Copyright RiftbornAI. All Rights Reserved.
// CircuitBreaker.h - Resilience pattern for external service calls

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "Misc/ScopeLock.h"

/**
 * Circuit Breaker States
 */
enum class ECircuitBreakerState : uint8
{
    Closed,    // Normal operation - requests pass through
    Open,      // Tripped - requests fail fast
    HalfOpen   // Testing - allow one request to probe
};

/**
 * Circuit Breaker Configuration
 */
struct FCircuitBreakerConfig
{
    // Number of consecutive failures before opening circuit
    int32 FailureThreshold = 5;
    
    // Time in seconds before attempting to close circuit (move to half-open)
    float ResetTimeoutSeconds = 30.0f;
    
    // Number of successful calls in half-open state to close circuit
    int32 SuccessThreshold = 2;
    
    // Name for logging
    FString Name = TEXT("Default");
    
    static FCircuitBreakerConfig ForOllama()
    {
        FCircuitBreakerConfig Config;
        Config.Name = TEXT("Ollama");
        Config.FailureThreshold = 3;          // Open after 3 failures
        Config.ResetTimeoutSeconds = 60.0f;   // Try again after 60 seconds
        Config.SuccessThreshold = 1;          // One success to close
        return Config;
    }
    
    static FCircuitBreakerConfig ForTool()
    {
        FCircuitBreakerConfig Config;
        Config.Name = TEXT("Tool");
        Config.FailureThreshold = 10;         // More lenient for tools
        Config.ResetTimeoutSeconds = 10.0f;   // Quick recovery
        Config.SuccessThreshold = 2;
        return Config;
    }
};

/**
 * Circuit Breaker - Prevents cascading failures by failing fast
 * 
 * Usage:
 *   if (!CircuitBreaker.CanExecute())
 *   {
 *       return Error("Service unavailable - circuit open");
 *   }
 *   
 *   bool bSuccess = DoOperation();
 *   CircuitBreaker.RecordResult(bSuccess);
 */
class FCircuitBreaker
{
public:
    FCircuitBreaker(const FCircuitBreakerConfig& InConfig = FCircuitBreakerConfig())
        : Config(InConfig)
        , State(ECircuitBreakerState::Closed)
        , ConsecutiveFailures(0)
        , ConsecutiveSuccesses(0)
        , LastFailureTime(0)
        , TotalFailures(0)
        , TotalSuccesses(0)
    {
    }
    
    /**
     * Check if a request should be allowed
     * Returns true if request can proceed, false if circuit is open
     */
    bool CanExecute()
    {
        FScopeLock Lock(&StateLock);
        
        switch (State)
        {
            case ECircuitBreakerState::Closed:
                return true;
                
            case ECircuitBreakerState::Open:
            {
                // Check if we should try half-open
                double CurrentTime = FPlatformTime::Seconds();
                if (CurrentTime - LastFailureTime >= Config.ResetTimeoutSeconds)
                {
                    State = ECircuitBreakerState::HalfOpen;
                    UE_LOG(LogTemp, Log, TEXT("CircuitBreaker[%s]: Moving to HalfOpen state"), *Config.Name);
                    return true;  // Allow probe request
                }
                return false;  // Fail fast
            }
                
            case ECircuitBreakerState::HalfOpen:
                return true;  // Allow probe request
        }
        
        return true;
    }
    
    /**
     * Record the result of an operation
     */
    void RecordResult(bool bSuccess)
    {
        FScopeLock Lock(&StateLock);
        
        if (bSuccess)
        {
            TotalSuccesses++;
            RecordSuccess();
        }
        else
        {
            TotalFailures++;
            RecordFailure();
        }
    }
    
    /**
     * Force the circuit to open (e.g., for manual intervention)
     */
    void Trip()
    {
        FScopeLock Lock(&StateLock);
        State = ECircuitBreakerState::Open;
        LastFailureTime = FPlatformTime::Seconds();
        UE_LOG(LogTemp, Warning, TEXT("CircuitBreaker[%s]: Manually tripped"), *Config.Name);
    }
    
    /**
     * Force the circuit to close
     */
    void Reset()
    {
        FScopeLock Lock(&StateLock);
        State = ECircuitBreakerState::Closed;
        ConsecutiveFailures = 0;
        ConsecutiveSuccesses = 0;
        UE_LOG(LogTemp, Log, TEXT("CircuitBreaker[%s]: Reset to Closed state"), *Config.Name);
    }
    
    // Status queries
    ECircuitBreakerState GetState() const 
    { 
        FScopeLock Lock(&StateLock);
        return State; 
    }
    
    FString GetStateString() const
    {
        switch (GetState())
        {
            case ECircuitBreakerState::Closed: return TEXT("Closed");
            case ECircuitBreakerState::Open: return TEXT("Open");
            case ECircuitBreakerState::HalfOpen: return TEXT("HalfOpen");
            default: return TEXT("Unknown");
        }
    }
    
    int32 GetConsecutiveFailures() const { return ConsecutiveFailures; }
    int32 GetTotalFailures() const { return TotalFailures; }
    int32 GetTotalSuccesses() const { return TotalSuccesses; }
    
    FString GetStatusReport() const
    {
        return FString::Printf(
            TEXT("CircuitBreaker[%s]: State=%s, ConsecFail=%d/%d, Total=%d success / %d fail"),
            *Config.Name,
            *GetStateString(),
            ConsecutiveFailures,
            Config.FailureThreshold,
            TotalSuccesses,
            TotalFailures
        );
    }

private:
    void RecordSuccess()
    {
        ConsecutiveFailures = 0;
        ConsecutiveSuccesses++;
        
        switch (State)
        {
            case ECircuitBreakerState::Closed:
                // Good, stay closed
                break;
                
            case ECircuitBreakerState::HalfOpen:
                // Check if we can close the circuit
                if (ConsecutiveSuccesses >= Config.SuccessThreshold)
                {
                    State = ECircuitBreakerState::Closed;
                    ConsecutiveSuccesses = 0;
                    UE_LOG(LogTemp, Log, TEXT("CircuitBreaker[%s]: Closed after successful probe"), *Config.Name);
                }
                break;
                
            case ECircuitBreakerState::Open:
                // Shouldn't happen - we don't allow requests in open state
                break;
        }
    }
    
    void RecordFailure()
    {
        ConsecutiveSuccesses = 0;
        ConsecutiveFailures++;
        LastFailureTime = FPlatformTime::Seconds();
        
        switch (State)
        {
            case ECircuitBreakerState::Closed:
                if (ConsecutiveFailures >= Config.FailureThreshold)
                {
                    State = ECircuitBreakerState::Open;
                    UE_LOG(LogTemp, Warning, TEXT("CircuitBreaker[%s]: OPENED after %d consecutive failures"), 
                        *Config.Name, ConsecutiveFailures);
                }
                break;
                
            case ECircuitBreakerState::HalfOpen:
                // Failed probe - back to open
                State = ECircuitBreakerState::Open;
                UE_LOG(LogTemp, Warning, TEXT("CircuitBreaker[%s]: Probe failed, returning to Open"), *Config.Name);
                break;
                
            case ECircuitBreakerState::Open:
                // Already open
                break;
        }
    }

private:
    FCircuitBreakerConfig Config;
    
    mutable FCriticalSection StateLock;
    ECircuitBreakerState State;
    int32 ConsecutiveFailures;
    int32 ConsecutiveSuccesses;
    double LastFailureTime;
    
    // Lifetime stats
    int32 TotalFailures;
    int32 TotalSuccesses;
};

/**
 * Global circuit breakers for common services
 */
class FRiftbornCircuitBreakers
{
public:
    static FCircuitBreaker& GetOllamaBreaker()
    {
        static FCircuitBreaker Breaker(FCircuitBreakerConfig::ForOllama());
        return Breaker;
    }
    
    static FCircuitBreaker& GetToolBreaker()
    {
        static FCircuitBreaker Breaker(FCircuitBreakerConfig::ForTool());
        return Breaker;
    }
    
    static FString GetStatusReport()
    {
        return FString::Printf(TEXT("%s\n%s"),
            *GetOllamaBreaker().GetStatusReport(),
            *GetToolBreaker().GetStatusReport());
    }
};
