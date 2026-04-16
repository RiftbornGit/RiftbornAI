// Copyright RiftbornAI. All Rights Reserved.
// CircuitBreaker.h - Legacy compatibility wrapper over the canonical gameplay circuit breaker surface

#pragma once

#include "CoreMinimal.h"
#include "RiftbornCircuitBreaker.h"

/**
 * Legacy global circuit-breaker accessors kept for older provider call sites.
 * The canonical circuit-breaker implementation now lives in RiftbornCircuitBreaker.h.
 */
class FRiftbornCircuitBreakers
{
public:
    static FCircuitBreaker& GetOllamaBreaker()
    {
        static TSharedPtr<FCircuitBreaker> Breaker = FCircuitBreakerManager::Get().GetCircuitBreaker(
            TEXT("Ollama"),
            FCircuitBreakerConfig::ForOllama());
        return *Breaker;
    }

    static FCircuitBreaker& GetToolBreaker()
    {
        static TSharedPtr<FCircuitBreaker> Breaker = FCircuitBreakerManager::Get().GetCircuitBreaker(
            TEXT("Tool"),
            FCircuitBreakerConfig::ForTool());
        return *Breaker;
    }

    static FString GetStatusReport()
    {
        const FCircuitBreaker& OllamaBreaker = GetOllamaBreaker();
        const FCircuitBreaker& ToolBreaker = GetToolBreaker();
        return FString::Printf(
            TEXT("CircuitBreaker[%s]: State=%s, ConsecFail=%d/%d\nCircuitBreaker[%s]: State=%s, ConsecFail=%d/%d"),
            *OllamaBreaker.GetName(),
            *OllamaBreaker.GetStateString(),
            OllamaBreaker.GetConsecutiveFailures(),
            OllamaBreaker.GetFailureThreshold(),
            *ToolBreaker.GetName(),
            *ToolBreaker.GetStateString(),
            ToolBreaker.GetConsecutiveFailures(),
            ToolBreaker.GetFailureThreshold());
    }
};
