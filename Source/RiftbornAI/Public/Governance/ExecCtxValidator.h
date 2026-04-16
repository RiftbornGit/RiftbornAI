// Copyright 2026 RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * ERiftbornRiskTier - Risk classification for operations
 * Must match Python's RiskTier enum exactly!
 */
UENUM(BlueprintType)
enum class ERiftbornRiskTier : uint8
{
    Safe = 0              UMETA(DisplayName = "Safe"),            // Read-only, no state change
    Recovery = 1          UMETA(DisplayName = "Recovery"),        // Rollback/undo operations
    MutatingReversible = 2 UMETA(DisplayName = "Mutating Reversible"), // Can be undone
    MutatingProject = 3   UMETA(DisplayName = "Mutating Project"), // Changes project files
    Destructive = 4       UMETA(DisplayName = "Destructive"),     // Cannot be undone
    Unknown = 255         UMETA(DisplayName = "Unknown")          // Unregistered tool - blocks ALL tiers
};

/**
 * FExecCtx - Signed Execution Context
 * 
 * This is the unbypassable token that authorizes mutating operations.
 * Python Gateway mints it, UE validates it.
 * 
 * If you can't produce a valid ExecCtx, you can't mutate UE. Period.
 * 
 * NOTE: This is a plain C++ struct (not USTRUCT) because it's only used
 * for C++<->Python communication, not Blueprint exposure.
 */
struct RIFTBORNAI_API FExecCtx
{
    // Identity - INITIALIZED to eliminate nondeterminism
    FString CtxId = TEXT("");              // UUID - unique execution instance
    FString PlanId = TEXT("");             // Hash of frozen plan JSON
    int32 StepId = 0;           // Monotonic within plan (replay protection)
    
    // Intent binding - INITIALIZED to eliminate nondeterminism
    FString ToolName = TEXT("");           // What tool is being executed
    FString ArgsHash = TEXT("");           // SHA256 of canonical args JSON
    int32 RiskTier = 0;         // ERiftbornRiskTier value
    
    // Provenance - INITIALIZED to eliminate nondeterminism
    FString Caller = TEXT("");             // Who requested this
    double IssuedAt = 0.0;      // Unix timestamp when minted
    double ExpiresAt = 0.0;     // Must execute before this time
    
    // Rollback readiness - INITIALIZED to eliminate nondeterminism
    bool bRollbackReady = false;
    FString RollbackProofId = TEXT("");
    
    // Replay protection - INITIALIZED to eliminate nondeterminism
    FString Nonce = TEXT("");
    
    // Signature - INITIALIZED to eliminate nondeterminism
    FString Signature = TEXT("");          // HMAC-SHA256 of canonical form
    
    /** Parse from JSON object */
    static bool FromJson(const TSharedPtr<FJsonObject>& JsonObj, FExecCtx& OutCtx);
    
    /** Serialize to JSON for canonical form (excludes signature) */
    FString ToCanonicalJson() const;
    
    /** Get risk tier as enum */
    ERiftbornRiskTier GetRiskTierEnum() const
    {
        return static_cast<ERiftbornRiskTier>(FMath::Clamp(RiskTier, 0, 4));
    }
};

/**
 * FExecCtxValidator - Validates ExecCtx tokens
 * 
 * This is the enforcement point. No valid ExecCtx = no mutation.
 */
class RIFTBORNAI_API FExecCtxValidator
{
public:
    FExecCtxValidator();
    
    /** Initialize with signing secret from config */
    void Initialize();
    
    /**
     * Validate an execution context.
     * 
     * @param Ctx The ExecCtx to validate
     * @param ActualArgsJson The actual request args as JSON string
     * @param EndpointMaxTier Maximum risk tier allowed by this endpoint
     * @param OutError Error message if validation fails
     * @return true if valid, false otherwise
     */
    bool Validate(
        const FExecCtx& Ctx,
        const FString& ActualArgsJson,
        const FString& RequestedToolName,
        ERiftbornRiskTier EndpointMaxTier,
        FString& OutError
    );
    
    /**
     * Validate with a specified timestamp (for testing with historical vectors).
     * 
     * @param Ctx The ExecCtx to validate
     * @param ActualArgsJson The actual request args as JSON string
     * @param EndpointMaxTier Maximum risk tier allowed by this endpoint
     * @param TestTimeUnix Unix timestamp to use as "now" (must be > 0)
     * @param OutError Error message if validation fails
     * @return true if valid, false otherwise
     */
    bool ValidateWithTime(
        const FExecCtx& Ctx,
        const FString& ActualArgsJson,
        const FString& RequestedToolName,
        ERiftbornRiskTier EndpointMaxTier,
        double TestTimeUnix,
        FString& OutError
    );
    
    /** Reset step counters (for testing) */
    void Reset();
    
    /** Check if validator is properly initialized */
    bool IsInitialized() const;
    
private:
    /** Internal validation with explicit timestamp - shared by Validate and ValidateWithTime */
    bool ValidateInternal(
        const FExecCtx& Ctx,
        const FString& ActualArgsJson,
        const FString& RequestedToolName,
        ERiftbornRiskTier EndpointMaxTier,
        double TimeUnix,
        FString& OutError
    );
    
    /** Compute HMAC-SHA256 signature */
    FString ComputeSignature(const FExecCtx& Ctx) const;
    
    /** Compute SHA256 hash of args JSON */
    static FString HashArgs(const FString& ArgsJson);
    
    /** The shared secret for HMAC */
    TArray<uint8> SigningSecret;
    
    /** Last seen step_id per plan_id (replay protection) */
    TMap<FString, int32> LastStepIds;
    
    /** Maximum entries in LastStepIds before eviction (prevent unbounded growth) */
    static constexpr int32 MaxLastStepIdEntries = 10000;
    
    /** Is the validator properly initialized? */
    bool bInitialized = false;

    /** Protects signing secret and replay-protection state on threaded bridge routes. */
    mutable FCriticalSection StateLock;
    
    /** Allow 5 second clock skew */
    static constexpr double ClockSkewToleranceSeconds = 5.0;
};

/**
 * FRouteRiskTierRegistry - Maps endpoints to their maximum allowed risk tier
 * 
 * This is where UE enforces policy independently of Python.
 * The client-supplied tier is validated against this registry.
 */
class RIFTBORNAI_API FRouteRiskTierRegistry
{
public:
    static FRouteRiskTierRegistry& Get();
    
    /** Register a route with its max tier */
    void RegisterRoute(const FString& Route, ERiftbornRiskTier MaxTier);
    
    /** Get max tier for a route (returns Safe if not found) */
    ERiftbornRiskTier GetMaxTier(const FString& Route) const;
    
    /** Check if a tier is allowed for a route */
    bool IsTierAllowed(const FString& Route, ERiftbornRiskTier RequestedTier) const;
    
    /** Initialize default route mappings */
    void InitializeDefaults();
    
private:
    TMap<FString, ERiftbornRiskTier> RouteTiers;
    mutable FCriticalSection RouteTiersLock;
    
    // Singleton
    FRouteRiskTierRegistry() = default;
};
