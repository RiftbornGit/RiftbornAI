// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// RegretRecord.h - FRegretEvent, FUnblockProof, and FRegretClassifier
//
// Core data structures for regret tracking and proof-based unblocking.
//
// Extracted from RegretScope.h for modularity.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

#include "RegretEnums.h"
#include "EnvironmentFingerprint.h"

/**
 * A single regret event with full context.
 * Persisted to enable cross-session learning.
 */
struct FRegretEvent
{
    // Identity
    FGuid EventId;
    FDateTime Timestamp;

    // What happened
    FString ToolName;
    FString ActionContext;          // What was the agent trying to do?
    FString WorldContext;           // Map, PIE state, etc.

    // ==========================================================================
    // CONTEXT FINGERPRINT - Regret is scoped to its environment
    // ==========================================================================
    /** Environment fingerprint when regret occurred */
    FEnvironmentFingerprint EnvironmentFingerprint;

    // The regret itself
    ERegretScope Scope = ERegretScope::Unknown;
    float BaseMagnitude = 1.0f;     // Raw regret amount before scope weighting
    float EffectivePenalty = 0.0f;  // After scope weight applied

    // Evidence
    FString ExpectedOutcome;        // What was expected
    FString ActualOutcome;          // What actually happened
    float ExpectationDelta = 0.0f;  // Numeric distance from expectation

    // Causality chain (for strategic regret)
    TArray<FGuid> ContributingEvents;   // Earlier events that led to this
    TArray<FString> ContributingTools;  // Tool sequence that caused this

    // Resolution
    bool bResolved = false;         // Has this regret been addressed?
    FString ResolutionMethod;       // How was it resolved? (proof, time, verified success)
    FDateTime ResolutionTime;

    // Flags
    bool bReversible = true;        // Can this regret be undone with evidence?
    bool bDeferredCandidate = false;// Heuristically looks like deferred regret

    // ==========================================================================
    // DEFERRED EXPIRATION - Deferred is a hypothesis, not a verdict
    // ==========================================================================

    /** Steps remaining before deferred expires (0 = not deferred or already expired) */
    int32 DeferredStepsRemaining = 0;

    /** Absolute deadline for deferred resolution */
    FDateTime DeferredDeadline;

    /** Has this deferred regret been escalated to Strategic? */
    bool bEscalatedFromDeferred = false;

    /** Original scope before escalation (for audit trail) */
    ERegretScope OriginalScope = ERegretScope::Unknown;

    // ==========================================================================
    // CLAIM FAMILY - For proof-based unblocking
    // ==========================================================================

    /** The claim family that was violated (e.g., "ActorCount", "LocationDelta") */
    FString ViolatedClaimFamily;

    /** Required evidence type for resolution */
    FString RequiredProofType;

    // ==========================================================================
    // CAUSAL SURFACE - Anchors penalty decay to causal invariance
    // ==========================================================================

    /**
     * How causally tied is this failure to the specific environment?
     * Invariant failures (e.g., "success_but_no_delta") should NOT decay across contexts.
     * Map-dependent failures (e.g., "can't find actor X") CAN decay in different maps.
     */
    ECausalSurface CausalSurface = ECausalSurface::MapDependent;

    // ==========================================================================
    // SCHEMA VERSION - For tolerant parsing across versions
    // ==========================================================================
    // v1: Initial regret events
    // v2: Deferred expiration, claim family tracking
    // v3: Environment fingerprint for context decay
    // v4: Causal surface tagging for invariant failure detection
    static constexpr int32 CURRENT_SCHEMA_VERSION = 4;
    int32 SchemaVersion = CURRENT_SCHEMA_VERSION;

    /** Calculate effective penalty based on scope */
    float CalculateEffectivePenalty() const
    {
        return BaseMagnitude * GetRegretScopeWeight(Scope);
    }

    /**
     * Calculate context-adjusted penalty based on environment similarity.
     * A penalty from Map A shouldn't fully apply in Map B.
     *
     * CRITICAL: Causal surface anchors the decay - invariant failures don't decay.
     * This prevents "penalty laundering" by changing context for fundamental failures.
     */
    float CalculateContextAdjustedPenalty(const FEnvironmentFingerprint& CurrentEnv) const
    {
        float DecayFactor = EnvironmentFingerprint.GetCausalDecayFactor(CurrentEnv, CausalSurface);
        return EffectivePenalty * DecayFactor;
    }

    /** Serialize to JSON */
    TSharedPtr<FJsonObject> ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject);

        Obj->SetStringField(TEXT("event_id"), EventId.ToString());
        Obj->SetStringField(TEXT("timestamp"), Timestamp.ToIso8601());
        Obj->SetStringField(TEXT("tool_name"), ToolName);
        Obj->SetStringField(TEXT("action_context"), ActionContext);
        Obj->SetStringField(TEXT("world_context"), WorldContext);
        Obj->SetStringField(TEXT("scope"), RegretScopeToString(Scope));
        Obj->SetNumberField(TEXT("base_magnitude"), BaseMagnitude);
        Obj->SetNumberField(TEXT("effective_penalty"), EffectivePenalty);
        Obj->SetStringField(TEXT("expected_outcome"), ExpectedOutcome);
        Obj->SetStringField(TEXT("actual_outcome"), ActualOutcome);
        Obj->SetNumberField(TEXT("expectation_delta"), ExpectationDelta);
        Obj->SetBoolField(TEXT("resolved"), bResolved);
        Obj->SetStringField(TEXT("resolution_method"), ResolutionMethod);
        if (bResolved)
        {
            Obj->SetStringField(TEXT("resolution_time"), ResolutionTime.ToIso8601());
        }
        Obj->SetBoolField(TEXT("reversible"), bReversible);
        Obj->SetBoolField(TEXT("deferred_candidate"), bDeferredCandidate);

        // Deferred expiration fields
        Obj->SetNumberField(TEXT("deferred_steps_remaining"), DeferredStepsRemaining);
        if (DeferredDeadline.GetTicks() > 0)
        {
            Obj->SetStringField(TEXT("deferred_deadline"), DeferredDeadline.ToIso8601());
        }
        Obj->SetBoolField(TEXT("escalated_from_deferred"), bEscalatedFromDeferred);
        Obj->SetStringField(TEXT("original_scope"), RegretScopeToString(OriginalScope));

        // Claim family fields
        Obj->SetStringField(TEXT("violated_claim_family"), ViolatedClaimFamily);
        Obj->SetStringField(TEXT("required_proof_type"), RequiredProofType);

        // Causal surface (v4+)
        Obj->SetStringField(TEXT("causal_surface"), CausalSurfaceToString(CausalSurface));

        // Schema version and formula hash (for cross-language verification)
        Obj->SetNumberField(TEXT("schema_version"), SchemaVersion);
        Obj->SetStringField(TEXT("cxx_formulas_hash"), GetFormulaHash());

        // Contributing events
        TArray<TSharedPtr<FJsonValue>> ContribArray;
        for (const FGuid& Id : ContributingEvents)
        {
            ContribArray.Add(MakeShareable(new FJsonValueString(Id.ToString())));
        }
        Obj->SetArrayField(TEXT("contributing_events"), ContribArray);

        // Contributing tools
        TArray<TSharedPtr<FJsonValue>> ToolArray;
        for (const FString& Tool : ContributingTools)
        {
            ToolArray.Add(MakeShareable(new FJsonValueString(Tool)));
        }
        Obj->SetArrayField(TEXT("contributing_tools"), ToolArray);

        // Environment fingerprint (v3+)
        Obj->SetObjectField(TEXT("environment_fingerprint"), EnvironmentFingerprint.ToJson());

        return Obj;
    }

    /** Deserialize from JSON */
    static FRegretEvent FromJson(const TSharedPtr<FJsonObject>& Obj)
    {
        FRegretEvent Event;

        FGuid::Parse(Obj->GetStringField(TEXT("event_id")), Event.EventId);
        FDateTime::ParseIso8601(*Obj->GetStringField(TEXT("timestamp")), Event.Timestamp);
        Event.ToolName = Obj->GetStringField(TEXT("tool_name"));
        Event.ActionContext = Obj->GetStringField(TEXT("action_context"));
        Event.WorldContext = Obj->GetStringField(TEXT("world_context"));
        Event.Scope = StringToRegretScope(Obj->GetStringField(TEXT("scope")));
        Event.BaseMagnitude = static_cast<float>(Obj->GetNumberField(TEXT("base_magnitude")));
        Event.EffectivePenalty = static_cast<float>(Obj->GetNumberField(TEXT("effective_penalty")));
        Event.ExpectedOutcome = Obj->GetStringField(TEXT("expected_outcome"));
        Event.ActualOutcome = Obj->GetStringField(TEXT("actual_outcome"));
        Event.ExpectationDelta = static_cast<float>(Obj->GetNumberField(TEXT("expectation_delta")));
        Event.bResolved = Obj->GetBoolField(TEXT("resolved"));
        Event.ResolutionMethod = Obj->GetStringField(TEXT("resolution_method"));

        FString ResTimeStr;
        if (Obj->TryGetStringField(TEXT("resolution_time"), ResTimeStr))
        {
            FDateTime::ParseIso8601(*ResTimeStr, Event.ResolutionTime);
        }

        Event.bReversible = Obj->GetBoolField(TEXT("reversible"));
        Obj->TryGetBoolField(TEXT("deferred_candidate"), Event.bDeferredCandidate);

        // ======================================================================
        // TOLERANT PARSING - Schema v2 fields (safe to miss on v1 data)
        // ======================================================================

        // Schema version (defaults to 1 if missing = legacy data)
        double SchemaVer = 1.0;
        Obj->TryGetNumberField(TEXT("schema_version"), SchemaVer);
        Event.SchemaVersion = static_cast<int32>(SchemaVer);

        // Deferred expiration fields (v2+)
        double DeferredSteps = 0.0;
        Obj->TryGetNumberField(TEXT("deferred_steps_remaining"), DeferredSteps);
        Event.DeferredStepsRemaining = static_cast<int32>(DeferredSteps);

        FString DeadlineStr;
        if (Obj->TryGetStringField(TEXT("deferred_deadline"), DeadlineStr))
        {
            FDateTime::ParseIso8601(*DeadlineStr, Event.DeferredDeadline);
        }

        Obj->TryGetBoolField(TEXT("escalated_from_deferred"), Event.bEscalatedFromDeferred);

        FString OrigScopeStr;
        if (Obj->TryGetStringField(TEXT("original_scope"), OrigScopeStr))
        {
            Event.OriginalScope = StringToRegretScope(OrigScopeStr);
        }

        // Claim family fields (v2+)
        Obj->TryGetStringField(TEXT("violated_claim_family"), Event.ViolatedClaimFamily);
        Obj->TryGetStringField(TEXT("required_proof_type"), Event.RequiredProofType);

        // Environment fingerprint (v3+)
        const TSharedPtr<FJsonObject>* FpObj;
        if (Obj->TryGetObjectField(TEXT("environment_fingerprint"), FpObj))
        {
            Event.EnvironmentFingerprint = FEnvironmentFingerprint::FromJson(*FpObj);
        }

        // Causal surface (v4+) - defaults to MapDependent for legacy data
        FString CausalSurfaceStr;
        if (Obj->TryGetStringField(TEXT("causal_surface"), CausalSurfaceStr))
        {
            Event.CausalSurface = StringToCausalSurface(CausalSurfaceStr);
        }
        else if (!Event.ViolatedClaimFamily.IsEmpty())
        {
            // Auto-classify from claim family for upgraded v2/v3 data
            Event.CausalSurface = ClassifyFailureCausalSurface(Event.ViolatedClaimFamily);
        }

        // ======================================================================
        // END TOLERANT PARSING
        // ======================================================================

        const TArray<TSharedPtr<FJsonValue>>* ContribArray;
        if (Obj->TryGetArrayField(TEXT("contributing_events"), ContribArray))
        {
            for (const auto& Val : *ContribArray)
            {
                FGuid Id;
                FGuid::Parse(Val->AsString(), Id);
                Event.ContributingEvents.Add(Id);
            }
        }

        const TArray<TSharedPtr<FJsonValue>>* ToolArray;
        if (Obj->TryGetArrayField(TEXT("contributing_tools"), ToolArray))
        {
            for (const auto& Val : *ToolArray)
            {
                Event.ContributingTools.Add(Val->AsString());
            }
        }

        return Event;
    }

    // =========================================================================
    // DEFERRED EXPIRATION LOGIC
    // =========================================================================

    /** Initialize as deferred with expiration window */
    void SetDeferredWithExpiration(int32 MaxSteps, int32 MaxSeconds)
    {
        Scope = ERegretScope::Deferred;
        OriginalScope = ERegretScope::Deferred;
        DeferredStepsRemaining = MaxSteps;
        DeferredDeadline = FDateTime::Now() + FTimespan::FromSeconds(MaxSeconds);
        EffectivePenalty = 0.0f;  // No penalty while deferred
    }

    /** Decrement steps remaining. Returns true if still valid. */
    bool DecrementDeferredStep()
    {
        if (Scope != ERegretScope::Deferred) return true;
        if (DeferredStepsRemaining > 0)
        {
            DeferredStepsRemaining--;
        }
        return !ShouldEscalate();
    }

    /** Check if deferred should escalate to Strategic */
    bool ShouldEscalate() const
    {
        if (Scope != ERegretScope::Deferred) return false;
        if (bResolved) return false;

        // Escalate if out of steps
        if (DeferredStepsRemaining <= 0) return true;

        // Escalate if past deadline
        if (DeferredDeadline.GetTicks() > 0 && FDateTime::Now() > DeferredDeadline)
        {
            return true;
        }

        return false;
    }

    /** Escalate from Deferred to Strategic with retroactive penalty */
    void EscalateToStrategic()
    {
        if (Scope != ERegretScope::Deferred) return;

        OriginalScope = ERegretScope::Deferred;
        Scope = ERegretScope::Strategic;
        bEscalatedFromDeferred = true;

        // Apply retroactive penalty (Strategic weight)
        EffectivePenalty = BaseMagnitude * GetRegretScopeWeight(ERegretScope::Strategic);
    }

    /** Mark as resolved by goal progress */
    void ResolveWithEvidence(const FString& Evidence)
    {
        bResolved = true;
        ResolutionMethod = Evidence;
        ResolutionTime = FDateTime::Now();
        DeferredStepsRemaining = 0;  // No longer needs tracking
    }
};

/**
 * Proof for unblocking a tool.
 * Evidence-based recovery, not time-based.
 *
 * ANTI-FARMING RULES:
 * 1. Proof must match the violated claim family
 * 2. Proof must be for the same tool (or capability class)
 * 3. Velocity limit: max N reductions per epoch
 * 4. Minimum task difficulty required (world delta threshold)
 */
struct FUnblockProof
{
    FGuid ProofId;
    FDateTime Timestamp;

    FString ToolName;
    FString ProofType;              // "verified_success", "user_override", "ci_proof", "deferred_resolved"

    // Evidence
    TArray<FGuid> ResolvedRegrets;  // Which regret events this proof addresses
    float PenaltyReduction = 0.0f;  // How much penalty to remove

    // ==========================================================================
    // CLAIM FAMILY MATCHING - Proof must repair the violated claim type
    // ==========================================================================

    /** The claim family this proof addresses (must match violated claim) */
    FString ClaimFamily;

    /** Capability class (for cross-tool proof, e.g., "LevelEditing") */
    FString CapabilityClass;

    // ==========================================================================
    // ANTI-FARMING GUARDS
    // ==========================================================================

    /** World delta achieved by this proof (complexity measure) */
    float WorldDelta = 0.0f;

    /** Minimum world delta required for this proof to be valid */
    static constexpr float MIN_WORLD_DELTA = 0.1f;

    /** Is this proof valid (meets anti-farming requirements)? */
    bool IsValid() const
    {
        // Proof must achieve minimum complexity
        if (WorldDelta < MIN_WORLD_DELTA) return false;

        // Must have at least one resolved regret
        if (ResolvedRegrets.Num() == 0) return false;

        // Must specify claim family
        if (ClaimFamily.IsEmpty()) return false;

        return true;
    }

    // Verification
    FString VerificationMethod;     // How was success verified?
    TSharedPtr<FJsonObject> Evidence; // Supporting data

    /** Serialize to JSON */
    TSharedPtr<FJsonObject> ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject);

        Obj->SetStringField(TEXT("proof_id"), ProofId.ToString());
        Obj->SetStringField(TEXT("timestamp"), Timestamp.ToIso8601());
        Obj->SetStringField(TEXT("tool_name"), ToolName);
        Obj->SetStringField(TEXT("proof_type"), ProofType);
        Obj->SetNumberField(TEXT("penalty_reduction"), PenaltyReduction);
        Obj->SetStringField(TEXT("verification_method"), VerificationMethod);

        TArray<TSharedPtr<FJsonValue>> ResolvedArray;
        for (const FGuid& Id : ResolvedRegrets)
        {
            ResolvedArray.Add(MakeShareable(new FJsonValueString(Id.ToString())));
        }
        Obj->SetArrayField(TEXT("resolved_regrets"), ResolvedArray);

        if (Evidence.IsValid())
        {
            Obj->SetObjectField(TEXT("evidence"), Evidence);
        }

        return Obj;
    }
};

/**
 * Regret classifier - heuristics for determining scope.
 * This is where the real intelligence lives.
 */
class FRegretClassifier
{
public:
    /**
     * Classify a regret event based on available context.
     * Returns the scope and confidence level.
     */
    static ERegretScope Classify(
        const FString& ToolName,
        const FString& ActionContext,
        const TArray<FString>& RecentToolSequence,
        float ExpectationDelta,
        bool bReducedOptions,
        bool bPartOfPlan,
        float& OutConfidence)
    {
        OutConfidence = 0.5f; // Default medium confidence

        // DEFERRED DETECTION:
        // If this looks like setup for a larger plan, don't penalize
        if (IsLikelyDeferredRegret(ToolName, ActionContext, RecentToolSequence))
        {
            OutConfidence = 0.7f;
            return ERegretScope::Deferred;
        }

        // OPTIONALITY DETECTION:
        // If action closed doors (reduced future options), highest penalty
        if (bReducedOptions)
        {
            OutConfidence = 0.9f;
            return ERegretScope::Optionality;
        }

        // STRATEGIC DETECTION:
        // Multi-step regression - plan level failure
        if (bPartOfPlan && RecentToolSequence.Num() >= 3)
        {
            // Check if we're in a chain of related failures
            if (HasRecentRelatedFailures(ToolName, RecentToolSequence))
            {
                OutConfidence = 0.8f;
                return ERegretScope::Strategic;
            }
        }

        // TACTICAL (default):
        // Single step failure
        OutConfidence = 0.9f;
        return ERegretScope::Tactical;
    }

private:
    /** Detect if this might be acceptable deferred regret */
    static bool IsLikelyDeferredRegret(
        const FString& ToolName,
        const FString& ActionContext,
        const TArray<FString>& RecentToolSequence)
    {
        // Heuristics for deferred regret:

        // 1. "Setup" tools that prepare for later success
        static const TSet<FString> SetupTools = {
            TEXT("spawn_actor"),
            TEXT("create_folder"),
            TEXT("begin_transaction")
        };

        if (SetupTools.Contains(ToolName.ToLower()))
        {
            // If followed by related tools, likely setup
            return true;
        }

        // 2. Context mentions "prepare", "setup", "for later"
        if (ActionContext.Contains(TEXT("setup")) ||
            ActionContext.Contains(TEXT("prepare")) ||
            ActionContext.Contains(TEXT("prerequisite")))
        {
            return true;
        }

        // 3. Part of a known multi-step pattern
        // E.g., delete + create is a "replace" pattern
        if (RecentToolSequence.Num() >= 2)
        {
            const FString& Prev = RecentToolSequence.Last(1);
            if ((Prev.Contains(TEXT("delete")) && ToolName.Contains(TEXT("create"))) ||
                (Prev.Contains(TEXT("remove")) && ToolName.Contains(TEXT("add"))))
            {
                return true;
            }
        }

        return false;
    }

    /** Check if there are recent related failures */
    static bool HasRecentRelatedFailures(
        const FString& ToolName,
        const TArray<FString>& RecentToolSequence)
    {
        // If the same tool has failed multiple times recently, it's strategic
        int32 SameToolCount = 0;
        for (const FString& Tool : RecentToolSequence)
        {
            if (Tool == ToolName) SameToolCount++;
        }

        return SameToolCount >= 2;
    }
};
