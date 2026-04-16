// WorldStateDigest.h
// Minimal Viable Reality - Fast, Deterministic World State Snapshot
//
// This is NOT a full world serialization. It's a typed digest of ~20 signals
// that we actually care about for decision-making. Fast to compute, comparable,
// and hashable for tracking changes.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

// Forward declarations
class UWorld;
class AActor;

/**
 * Typed value union for state claims
 * NOT stringly-typed - we can actually compare these
 */
struct RIFTBORNAI_API FStateValue
{
    enum class EType : uint8
    {
        None,
        Bool,
        Int,
        Float,
        Name,
        Vector,
        ActorPath  // Soft reference to actor
    };
    
    EType Type = EType::None;
    
    union
    {
        bool BoolValue;
        int32 IntValue;
        float FloatValue;
    };
    
    FName NameValue;
    FVector VectorValue = FVector::ZeroVector;
    FString ActorPathValue;
    
    // Constructors
    FStateValue() : Type(EType::None), IntValue(0) {}
    explicit FStateValue(bool Value) : Type(EType::Bool), BoolValue(Value) {}
    explicit FStateValue(int32 Value) : Type(EType::Int), IntValue(Value) {}
    explicit FStateValue(float Value) : Type(EType::Float), FloatValue(Value) {}
    explicit FStateValue(FName Value) : Type(EType::Name), IntValue(0), NameValue(Value) {}
    explicit FStateValue(const FVector& Value) : Type(EType::Vector), IntValue(0), VectorValue(Value) {}
    explicit FStateValue(const FString& ActorPath) : Type(EType::ActorPath), IntValue(0), ActorPathValue(ActorPath) {}
    
    // Comparison
    bool operator==(const FStateValue& Other) const;
    bool operator!=(const FStateValue& Other) const { return !(*this == Other); }
    
    // Distance metric (for continuous values)
    float DistanceTo(const FStateValue& Other) const;
    
    // String for logging
    FString ToString() const;
    
    // JSON serialization
    TSharedPtr<FJsonObject> ToJson() const;
    static FStateValue FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Comparison operators for typed state claims
 */
enum class EStateOp : uint8
{
    Equals,         // Value == Expected
    NotEquals,      // Value != Expected
    GreaterThan,    // Value > Expected
    LessThan,       // Value < Expected
    GreaterOrEqual, // Value >= Expected
    LessOrEqual,    // Value <= Expected
    Within,         // |Value - Expected| <= Tolerance
    Contains,       // Name/String contains Expected
    Exists,         // Value is not None/null
    NotExists,      // Value is None/null
    Increased,      // Value > PreviousValue (requires delta)
    Decreased,      // Value < PreviousValue (requires delta)
    Unchanged       // Value == PreviousValue (requires delta)
};

FString RIFTBORNAI_API StateOpToString(EStateOp Op);
EStateOp RIFTBORNAI_API StringToStateOp(const FString& Str);

/**
 * A typed claim about expected state
 * This replaces TMap<FString, FString> with something we can actually reason about
 */
struct RIFTBORNAI_API FStateClaim
{
    /** What we're checking - e.g., "ActorCount.StaticMeshActor", "Level.Name", "PIE.Running" */
    FName Key;
    
    /** How we compare */
    EStateOp Op = EStateOp::Equals;
    
    /** Expected value (typed) */
    FStateValue Expected;
    
    /** Tolerance for Within/distance comparisons */
    float Tolerance = 0.0f;
    
    /** Weight for computing aggregate satisfaction (0-1) */
    float Weight = 1.0f;
    
    /** Scope of the claim */
    enum class EScope : uint8
    {
        Frame,      // Must hold this frame
        Episode,    // Must hold for episode duration
        Session     // Must hold for entire session
    };
    EScope Scope = EScope::Frame;
    
    // Constructors
    FStateClaim() = default;
    FStateClaim(FName InKey, EStateOp InOp, FStateValue InExpected, float InWeight = 1.0f)
        : Key(InKey), Op(InOp), Expected(InExpected), Weight(InWeight) {}
    
    /** Check if the claim is satisfied given an observed value */
    bool IsSatisfied(const FStateValue& Observed) const;
    
    /** Compute violation magnitude (0 = satisfied, >0 = violated) */
    float ViolationMagnitude(const FStateValue& Observed) const;
    
    /** String for logging */
    FString ToString() const;
    
    /** JSON serialization */
    TSharedPtr<FJsonObject> ToJson() const;
    static FStateClaim FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Result of checking a claim against observed state
 */
struct RIFTBORNAI_API FClaimResult
{
    FName Key;
    bool bSatisfied = false;
    float ViolationMagnitude = 0.0f;
    FStateValue Expected;
    FStateValue Observed;
    FString Reason;
    
    FString ToString() const;
};

/**
 * WorldStateDigest - The minimal viable reality snapshot
 * 
 * ~20 signals we actually care about:
 * - Actor counts by class/tags
 * - Level name / map hash
 * - PIE state
 * - Error/warn counts
 * - Camera metrics
 * - Key gameplay state
 */
struct RIFTBORNAI_API FWorldStateDigest
{
    // =========================================================================
    // CORE IDENTITY
    // =========================================================================
    
    /** Timestamp when captured */
    FDateTime CapturedAt;
    
    /** Stable hash of all values for quick comparison */
    uint32 ContentHash = 0;
    
    /** Frame number when captured */
    uint64 FrameNumber = 0;

    /** Whether capture succeeded - FALSE means world was invalid/transitioning */
    bool bCaptureSucceeded = false;

    /** Failure reason if bCaptureSucceeded is false */
    FString CaptureFailureReason;

    // =========================================================================
    // TYPED STATE VALUES
    // =========================================================================
    
    /** All state values, keyed by path */
    TMap<FName, FStateValue> Values;
    
    // =========================================================================
    // METHODS
    // =========================================================================
    
    /** Capture current world state */
    static FWorldStateDigest Capture(UWorld* World);
    
    /** Get a value (returns None type if not found) */
    FStateValue GetValue(FName Key) const;
    
    /** Check if a claim is satisfied */
    FClaimResult CheckClaim(const FStateClaim& Claim) const;
    
    /** Check multiple claims, return aggregate satisfaction */
    TArray<FClaimResult> CheckClaims(const TArray<FStateClaim>& Claims) const;
    
    /** Compute weighted satisfaction score (0-1) */
    float ComputeSatisfaction(const TArray<FStateClaim>& Claims) const;
    
    /** Compute delta from another digest */
    TMap<FName, TPair<FStateValue, FStateValue>> ComputeDelta(const FWorldStateDigest& Other) const;
    
    /** Compute content hash */
    void ComputeHash();
    
    /** JSON serialization */
    TSharedPtr<FJsonObject> ToJson() const;
    static FWorldStateDigest FromJson(const TSharedPtr<FJsonObject>& Json);
    
    /** Log summary */
    void LogSummary() const;
    
    /** Get human-readable summary string for LLM prompts (P1 Integration) */
    FString ToSummaryString() const;
    
private:
    // =========================================================================
    // CAPTURE HELPERS (called by Capture)
    // =========================================================================
    
    static void CaptureActorCounts(UWorld* World, TMap<FName, FStateValue>& OutValues);
    static void CaptureLevelInfo(UWorld* World, TMap<FName, FStateValue>& OutValues);
    static void CapturePIEState(UWorld* World, TMap<FName, FStateValue>& OutValues);
    static void CaptureErrorCounts(TMap<FName, FStateValue>& OutValues);
    static void CapturePerformanceMetrics(TMap<FName, FStateValue>& OutValues);
    static void CaptureGameplayState(UWorld* World, TMap<FName, FStateValue>& OutValues);
    static void CaptureHealthMetrics(TMap<FName, FStateValue>& OutValues);
};

/**
 * Utility: Standard state keys we capture
 */
namespace StateKeys
{
    // Actor counts
    const FName ActorCount_Total(TEXT("Actors.Total"));
    const FName ActorCount_StaticMesh(TEXT("Actors.StaticMeshActor"));
    const FName ActorCount_Character(TEXT("Actors.Character"));
    const FName ActorCount_Pawn(TEXT("Actors.Pawn"));
    const FName ActorCount_Light(TEXT("Actors.Light"));
    const FName ActorCount_Camera(TEXT("Actors.CameraActor"));
    const FName ActorCount_PlayerController(TEXT("Actors.PlayerController"));
    
    // Level info
    const FName Level_Name(TEXT("Level.Name"));
    const FName Level_MapHash(TEXT("Level.MapHash"));
    const FName Level_IsPersistent(TEXT("Level.IsPersistent"));
    const FName Level_StreamingLevelsLoaded(TEXT("Level.StreamingLevelsLoaded"));
    
    // PIE state
    const FName PIE_IsRunning(TEXT("PIE.IsRunning"));
    const FName PIE_IsPaused(TEXT("PIE.IsPaused"));
    const FName PIE_TimeDilation(TEXT("PIE.TimeDilation"));
    const FName PIE_GameTime(TEXT("PIE.GameTime"));
    
    // Error tracking
    const FName Errors_Total(TEXT("Errors.Total"));
    const FName Errors_Recent(TEXT("Errors.Recent"));
    const FName Warnings_Total(TEXT("Warnings.Total"));
    const FName Warnings_Recent(TEXT("Warnings.Recent"));
    
    // Performance
    const FName Perf_FPS(TEXT("Perf.FPS"));
    const FName Perf_FrameTimeMs(TEXT("Perf.FrameTimeMs"));
    const FName Perf_DrawCalls(TEXT("Perf.DrawCalls"));
    const FName Perf_MemoryMB(TEXT("Perf.MemoryMB"));
    
    // Gameplay (extensible)
    const FName Gameplay_ObjectiveCount(TEXT("Gameplay.ObjectiveCount"));
    const FName Gameplay_ObjectivesCompleted(TEXT("Gameplay.ObjectivesCompleted"));
    const FName Gameplay_PlayerHealth(TEXT("Gameplay.PlayerHealth"));
    const FName Gameplay_Score(TEXT("Gameplay.Score"));

    // Health metrics (from EnvironmentSteward)
    const FName Health_ToolSuccessRate(TEXT("Health.tool_success_rate"));
    const FName Health_ExpectationViolationRate(TEXT("Health.expectation_violation_rate"));
    const FName Health_AverageToolTrust(TEXT("Health.average_tool_trust"));
    const FName Health_ActorCountStability(TEXT("Health.actor_count_stability"));
    const FName Health_PlanSuccessRate(TEXT("Health.plan_success_rate"));
    const FName Health_ResponseLatency(TEXT("Health.response_latency"));
    const FName Health_BlockedToolCount(TEXT("Health.blocked_tool_count"));
    const FName Health_ClaimPrecision(TEXT("Health.claim_precision"));
}
