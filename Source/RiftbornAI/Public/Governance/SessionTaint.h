// SessionTaint.h
// Auto-taint tracking for session integrity
//
// SECURITY FIX 2026-02-02: Centralized taint tracking
// Any action that compromises proof integrity must auto-taint the session.
// This ensures "PROOF mode" claims are honest.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Misc/DateTime.h"
#include "Misc/CommandLine.h"
#include <atomic>

/**
 * Taint sources - reasons why a session cannot produce valid proofs
 */
enum class ETaintSource : uint8
{
    None = 0,
    DevModeEnabled = 1,          // RIFTBORN_DEV_MODE=1
    NetworkExposed = 2,          // Bound to non-localhost
    TestEndpointUsed = 3,        // Test-only endpoint was called
    ContractlessTool = 4,        // Tool executed without contract
    ExecCtxBypassed = 5,         // Mutator called without ExecCtx
    ArbitraryPython = 6,         // execute_python was used
    SafetyOverride = 7,          // Safety check was overridden
    ManualTaint = 8,             // Explicitly tainted by operator
    BindAnyFallback = 9,         // Localhost bind failed, fell back to 0.0.0.0
    UnregisteredRoute = 10,      // Unregistered route was accessed
    ProofModeDisabled = 11,      // RIFTBORN_PROOF_MODE not enabled
    ProofEmissionFailed = 12,    // Failed to emit proof in PROOF mode
    UnplannedMutation = 13,      // Mutating tool executed without plan context
    BridgeAttestationFailed = 14,// Bridge governance attestation failed
};

inline FString TaintSourceToString(ETaintSource Source)
{
    switch (Source)
    {
        case ETaintSource::None: return TEXT("none");
        case ETaintSource::DevModeEnabled: return TEXT("dev_mode_enabled");
        case ETaintSource::NetworkExposed: return TEXT("network_exposed");
        case ETaintSource::TestEndpointUsed: return TEXT("test_endpoint_used");
        case ETaintSource::ContractlessTool: return TEXT("contractless_tool");
        case ETaintSource::ExecCtxBypassed: return TEXT("exec_ctx_bypassed");
        case ETaintSource::ArbitraryPython: return TEXT("arbitrary_python");
        case ETaintSource::SafetyOverride: return TEXT("safety_override");
        case ETaintSource::ManualTaint: return TEXT("manual_taint");
        case ETaintSource::BindAnyFallback: return TEXT("bind_any_fallback");
        case ETaintSource::UnregisteredRoute: return TEXT("unregistered_route");
        case ETaintSource::ProofModeDisabled: return TEXT("proof_mode_disabled");
        case ETaintSource::ProofEmissionFailed: return TEXT("proof_emission_failed");
        case ETaintSource::UnplannedMutation: return TEXT("unplanned_mutation");
        case ETaintSource::BridgeAttestationFailed: return TEXT("bridge_attestation_failed");
        default: return TEXT("unknown");
    }
}

/**
 * A single taint event
 */
struct FTaintEvent
{
    ETaintSource Source;
    FString Details;
    FDateTime Timestamp;
    FString StackTrace;  // Optional - for debugging
    
    TSharedPtr<FJsonObject> ToJson() const
    {
        TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
        Json->SetStringField(TEXT("source"), TaintSourceToString(Source));
        Json->SetStringField(TEXT("details"), Details);
        Json->SetStringField(TEXT("timestamp"), Timestamp.ToIso8601());
        if (!StackTrace.IsEmpty())
        {
            Json->SetStringField(TEXT("stack_trace"), StackTrace);
        }
        return Json;
    }
};

/**
 * Session taint tracker - singleton that tracks all taint events
 * Once tainted, a session cannot produce valid proofs until restart.
 */
class RIFTBORNAI_API FSessionTaint
{
public:
    static FSessionTaint& Get()
    {
        static FSessionTaint Instance;
        return Instance;
    }
    
    /**
     * Mark the session as tainted with a specific source
     * Thread-safe, can be called from any thread.
     */
    void Taint(ETaintSource Source, const FString& Details)
    {
        FScopeLock Lock(&TaintLock);
        
        // Record the taint event
        FTaintEvent Event;
        Event.Source = Source;
        Event.Details = Details;
        Event.Timestamp = FDateTime::UtcNow();
        
        // Record first taint time if this is the first
        if (!bTainted.load())
        {
            FirstTaintTime = Event.Timestamp;
        }
        
        TaintEvents.Add(Event);
        bTainted.store(true);
        
        // Log for observability
        UE_LOG(LogTemp, Warning, TEXT("[TAINT] Session tainted: %s - %s"), 
            *TaintSourceToString(Source), *Details);
    }
    
    /**
     * Check if session is tainted
     */
    bool IsTainted() const { return bTainted.load(); }
    
    /**
     * Get all taint events
     */
    TArray<FTaintEvent> GetTaintEvents() const
    {
        FScopeLock Lock(&TaintLock);
        return TaintEvents;
    }
    
    /**
     * Get first taint time (for proof rejection)
     */
    FDateTime GetFirstTaintTime() const
    {
        FScopeLock Lock(&TaintLock);
        return FirstTaintTime;
    }
    
    /**
     * Export taint status as JSON
     */
    TSharedPtr<FJsonObject> ToJson() const
    {
        FScopeLock Lock(&TaintLock);
        
        TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
        Json->SetBoolField(TEXT("tainted"), bTainted.load());
        Json->SetNumberField(TEXT("taint_count"), TaintEvents.Num());
        
        if (bTainted.load())
        {
            Json->SetStringField(TEXT("first_taint_time"), FirstTaintTime.ToIso8601());
            
            // Collect unique sources
            TSet<ETaintSource> UniqueSources;
            for (const FTaintEvent& Event : TaintEvents)
            {
                UniqueSources.Add(Event.Source);
            }
            
            TArray<TSharedPtr<FJsonValue>> SourcesArray;
            for (ETaintSource Source : UniqueSources)
            {
                SourcesArray.Add(MakeShared<FJsonValueString>(TaintSourceToString(Source)));
            }
            Json->SetArrayField(TEXT("taint_sources"), SourcesArray);
        }
        
        // Include recent events (last 10)
        TArray<TSharedPtr<FJsonValue>> EventsArray;
        int32 StartIdx = FMath::Max(0, TaintEvents.Num() - 10);
        for (int32 i = StartIdx; i < TaintEvents.Num(); ++i)
        {
            EventsArray.Add(MakeShared<FJsonValueObject>(TaintEvents[i].ToJson()));
        }
        Json->SetArrayField(TEXT("recent_events"), EventsArray);
        
        return Json;
    }
    
    /**
     * Check environment and auto-taint if needed
     * Call this on startup and periodically.
     */
    void CheckEnvironmentTaint()
    {
        // Check DEV mode
        FString DevModeEnv = FPlatformMisc::GetEnvironmentVariable(TEXT("RIFTBORN_DEV_MODE"));
        if (DevModeEnv == TEXT("1"))
        {
            Taint(ETaintSource::DevModeEnabled, TEXT("RIFTBORN_DEV_MODE=1 environment variable set"));
        }
        
        // Check PROOF mode not enabled
        FString ProofModeEnv = FPlatformMisc::GetEnvironmentVariable(TEXT("RIFTBORN_PROOF_MODE"));
        bool bProofMode = !ProofModeEnv.IsEmpty() || FParse::Param(FCommandLine::Get(), TEXT("RiftbornProofMode"));
        if (!bProofMode)
        {
            Taint(ETaintSource::ProofModeDisabled, TEXT("RIFTBORN_PROOF_MODE not enabled"));
        }
    }
    
    /**
     * Record taint for network exposure
     */
    void TaintNetworkExposed(const FString& BindAddress)
    {
        Taint(ETaintSource::NetworkExposed, 
            FString::Printf(TEXT("HTTP server bound to non-localhost: %s"), *BindAddress));
    }
    
    /**
     * Record taint for test endpoint usage
     */
    void TaintTestEndpoint(const FString& Endpoint)
    {
        Taint(ETaintSource::TestEndpointUsed,
            FString::Printf(TEXT("Test-only endpoint used: %s"), *Endpoint));
    }
    
    /**
     * Record taint for contractless tool execution
     */
    void TaintContractlessTool(const FString& ToolName)
    {
        Taint(ETaintSource::ContractlessTool,
            FString::Printf(TEXT("Tool executed without contract: %s"), *ToolName));
    }
    
    /**
     * Record taint for ExecCtx bypass
     */
    void TaintExecCtxBypass(const FString& ToolName)
    {
        Taint(ETaintSource::ExecCtxBypassed,
            FString::Printf(TEXT("Mutator called without ExecCtx: %s"), *ToolName));
    }
    
    /**
     * Record taint for arbitrary Python execution
     */
    void TaintArbitraryPython()
    {
        Taint(ETaintSource::ArbitraryPython,
            TEXT("execute_python endpoint was used"));
    }
    
    /**
     * Record taint for safety override
     */
    void TaintSafetyOverride(const FString& Override)
    {
        Taint(ETaintSource::SafetyOverride,
            FString::Printf(TEXT("Safety check overridden: %s"), *Override));
    }
    
    /**
     * Record taint for bind fallback
     */
    void TaintBindFallback()
    {
        Taint(ETaintSource::BindAnyFallback,
            TEXT("Localhost bind failed, fell back to 0.0.0.0"));
    }
    
    /**
     * Record taint for unregistered route access
     */
    void TaintUnregisteredRoute(const FString& Route)
    {
        Taint(ETaintSource::UnregisteredRoute,
            FString::Printf(TEXT("Unregistered route accessed: %s"), *Route));
    }
    
private:
    FSessionTaint()
    {
        bTainted.store(false);
        FirstTaintTime = FDateTime::MinValue();
    }
    
    mutable FCriticalSection TaintLock;
    std::atomic<bool> bTainted;
    TArray<FTaintEvent> TaintEvents;
    FDateTime FirstTaintTime;
};

/**
 * RAII guard that auto-taints if a condition is met at scope exit
 * Use this to ensure taint happens even if code throws or returns early.
 */
class FAutoTaintGuard
{
public:
    FAutoTaintGuard(ETaintSource Source, const FString& Details, bool bCondition = true)
        : TaintSource(Source)
        , TaintDetails(Details)
        , bShouldTaint(bCondition)
    {
    }
    
    ~FAutoTaintGuard()
    {
        if (bShouldTaint)
        {
            FSessionTaint::Get().Taint(TaintSource, TaintDetails);
        }
    }
    
    // Call this to cancel the taint (e.g., if operation succeeded cleanly)
    void Cancel() { bShouldTaint = false; }
    
    // Call this to force the taint even if initially false
    void Force() { bShouldTaint = true; }
    
private:
    ETaintSource TaintSource;
    FString TaintDetails;
    bool bShouldTaint;
};
