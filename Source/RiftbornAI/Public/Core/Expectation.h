// Expectation.h
// The Counterfactual Layer - Explicit, Checkable Expectations
// 
// This is the architectural move that crosses the line from "copilot" to "agent".
// 
// Without this: Success = "nothing broke"
// With this: Success = "matched or exceeded predicted outcome"
//
// The key insight: Intelligence is being surprised, and knowing WHY.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "WorldStateDigest.h"  // Typed state values and claims

/**
 * FExpectation - What the brain predicts will happen
 * 
 * This is NOT just "will it succeed?" - that's what we had before.
 * This is "WHAT will the world look like after this action?"
 * 
 * The expectation is explicit and machine-checkable.
 * After execution, we compare reality to expectation.
 * The DELTA is where learning happens.
 */
struct RIFTBORNAI_API FExpectation
{
    // =========================================================================
    // WHAT WE EXPECT
    // =========================================================================
    
    /** Unique ID for this expectation (for tracking) */
    FGuid ExpectationId;
    
    /** The tool being called */
    FString ToolName;
    
    /** Predicted probability of technical success (0.0 - 1.0) */
    float PredictedSuccessProbability = 0.5f;
    
    /** Predicted execution time in milliseconds */
    float PredictedExecutionTimeMs = 0.0f;
    
    /** 
     * TYPED state claims - machine-checkable predictions
     * 
     * These are concrete, typed claims about what will be true after execution.
     * NOT stringly-typed - we can actually compare and compute distances.
     */
    TArray<FStateClaim> ExpectedStateClaims;
    
    /** World state BEFORE execution (for delta comparison) */
    FWorldStateDigest PreActionState;
    
    /**
     * Expected goal progress - does this move us forward?
     * 
     * -1.0 = expected to hurt goal
     *  0.0 = neutral
     * +1.0 = expected to help goal
     */
    float ExpectedGoalProgress = 0.0f;
    
    /**
     * Optionality impact - does this close off future choices?
     * 
     * -1.0 = severely reduces future options
     *  0.0 = neutral
     * +1.0 = opens up new options
     */
    float ExpectedOptionalityDelta = 0.0f;
    
    /** Confidence in this expectation (based on similar past actions) */
    float Confidence = 0.0f;
    
    /** Timestamp when expectation was formed */
    FDateTime CreatedAt;
    
    // =========================================================================
    // CONSTRUCTORS
    // =========================================================================
    
    FExpectation()
    {
        ExpectationId = FGuid::NewGuid();
        CreatedAt = FDateTime::UtcNow();
    }
    
    FExpectation(const FString& InToolName)
        : ToolName(InToolName)
    {
        ExpectationId = FGuid::NewGuid();
        CreatedAt = FDateTime::UtcNow();
    }
    
    /** Add a typed state claim */
    void ExpectState(FName Key, EStateOp Op, FStateValue Value, float Weight = 1.0f)
    {
        ExpectedStateClaims.Add(FStateClaim(Key, Op, Value, Weight));
    }
    
    /** Convenience: expect actor count to increase */
    void ExpectActorCountIncrease(int32 MinIncrease = 1)
    {
        int32 CurrentCount = PreActionState.GetValue(StateKeys::ActorCount_Total).IntValue;
        ExpectedStateClaims.Add(FStateClaim(
            StateKeys::ActorCount_Total,
            EStateOp::GreaterOrEqual,
            FStateValue(CurrentCount + MinIncrease),
            1.0f
        ));
    }
    
    /** Convert to JSON for logging/storage */
    TSharedPtr<FJsonObject> ToJson() const;
    
    /** Create from JSON */
    static FExpectation FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * FOutcomeObservation - What actually happened
 * 
 * This is the reality we compare against the expectation.
 */
struct RIFTBORNAI_API FOutcomeObservation
{
    /** ID of the expectation this observation is for */
    FGuid ExpectationId;
    
    /** Did the tool technically succeed? */
    bool bTechnicalSuccess = false;
    
    /** Actual execution time */
    float ActualExecutionTimeMs = 0.0f;
    
    /** 
     * TYPED observed state AFTER execution
     * This is a full snapshot that we compare against claims
     */
    FWorldStateDigest PostActionState;
    
    /** Results of checking each claim against observed state */
    TArray<FClaimResult> ClaimResults;
    
    /** Aggregate claim satisfaction (0-1) */
    float ClaimSatisfaction = 0.0f;
    
    /** Any error message */
    FString ErrorMessage;
    
    /** Timestamp of observation */
    FDateTime ObservedAt;
    
    FOutcomeObservation()
    {
        ObservedAt = FDateTime::UtcNow();
    }
};

/**
 * FExpectationDelta - The difference between expectation and reality
 * 
 * THIS IS WHERE INTELLIGENCE LIVES.
 * 
 * A delta of zero = world model was accurate
 * A non-zero delta = world model needs updating
 * 
 * The key insight: A "successful" action with wrong expectations
 * is MORE informative than a failed action with correct expectations.
 */
struct RIFTBORNAI_API FExpectationDelta
{
    /** ID of the expectation */
    FGuid ExpectationId;
    
    /** Tool that was executed */
    FString ToolName;
    
    // =========================================================================
    // THE DELTAS - What we got wrong
    // =========================================================================
    
    /** Delta between predicted and actual success */
    float SuccessDelta = 0.0f;  // positive = better than expected
    
    /** Delta between predicted and actual execution time */
    float ExecutionTimeDelta = 0.0f;
    
    /**
     * TYPED claim violations - exactly what we got wrong
     * Contains only the claims that were NOT satisfied
     */
    TArray<FClaimResult> ClaimViolations;
    
    /** World state delta - what actually changed (pre → post) */
    TMap<FName, TPair<FStateValue, FStateValue>> WorldStateDelta;
    
    /**
     * Surprise score (0.0 - 1.0)
     * 
     * 0.0 = no surprise, exactly as expected
     * 1.0 = maximum surprise, completely unexpected
     * 
     * Computed from:
     * - Success delta (weighted heavily)
     * - Claim violation rate and magnitude
     * - Execution time deviation
     * 
     * High surprise = high learning signal
     */
    float SurpriseScore = 0.0f;
    
    /**
     * Goal progress - OBJECTIVE MEASUREMENT
     * 
     * Computed from actual world state changes:
     * - Objectives completed delta
     * - Error count delta (negative is bad)
     * - Loop completeness delta
     */
    float ActualGoalProgress = 0.0f;
    
    /**
     * Was this action a mistake?
     * 
     * TRUE if ANY of these are true:
     * - Action succeeded but GoalProgress < ExpectedGoalProgress - threshold
     * - Action succeeded but introduced new errors
     * - Action succeeded but reduced optionality below threshold
     * - Claims violated despite technical success
     * 
     * THIS IS THE KEY FLAG.
     * When bWasSuboptimal=true on a "successful" action,
     * we have crossed the line from copilot to agent.
     */
    bool bWasSuboptimal = false;
    
    /** 
     * Suboptimal reason - ENUMERATED CODE, not prose
     */
    enum class ESuboptimalReason : uint8
    {
        None,
        GoalProgressBelowExpected,
        NewErrorsIntroduced,
        OptionsReduced,
        ClaimsViolated,
        UnexpectedSideEffects
    };
    ESuboptimalReason SuboptimalReasonCode = ESuboptimalReason::None;
    
    /** Human-readable reason (for logging only) */
    FString SuboptimalReason;
    
    /** Timestamp */
    FDateTime ComputedAt;
    
    FExpectationDelta()
    {
        ComputedAt = FDateTime::UtcNow();
    }
    
    // =========================================================================
    // COMPUTATION
    // =========================================================================
    
    /** 
     * Compute delta between expectation and observation
     * 
     * This is where we answer: "Was I wrong?"
     */
    static FExpectationDelta Compute(
        const FExpectation& Expected,
        const FOutcomeObservation& Observed);
    
    /** Should this delta update our world model? */
    bool ShouldUpdateBelief() const
    {
        // Update if surprising OR if we were wrong about being right
        return SurpriseScore > 0.3f || bWasSuboptimal;
    }
    
    /** Log this delta for analysis */
    void Log() const;
    
    /** Convert to JSON */
    TSharedPtr<FJsonObject> ToJson() const;
};

/**
 * FExpectationTracker - Tracks expectations and computes deltas
 * 
 * This is the counterfactual layer that makes us an agent runtime.
 */
class RIFTBORNAI_API FExpectationTracker
{
public:
    static FExpectationTracker& Get();
    
    /**
     * Form an expectation before action
     * 
     * @param ToolName The tool being called
     * @param Args Tool arguments (used to predict outcome)
     * @param Context Current world state
     * @return The expectation to attach to this action
     */
    FExpectation FormExpectation(
        const FString& ToolName,
        const TSharedPtr<FJsonObject>& Args,
        const TSharedPtr<FJsonObject>& Context = nullptr);
    
    /**
     * Observe outcome after action
     * 
     * @param ExpectationId ID of the expectation
     * @param bSuccess Did the tool technically succeed?
     * @param Result Tool result
     * @param ExecutionTimeMs How long it took
     * @return The observation
     */
    FOutcomeObservation ObserveOutcome(
        const FGuid& ExpectationId,
        bool bSuccess,
        const FString& Result,
        float ExecutionTimeMs);
    
    /**
     * Compare expectation to observation and compute delta
     * 
     * @param Expected What we predicted
     * @param Observed What happened
     * @return The delta (surprise + learning signal)
     */
    FExpectationDelta ComputeDelta(
        const FExpectation& Expected,
        const FOutcomeObservation& Observed);
    
    /**
     * Update world model based on delta
     * 
     * @param Delta The expectation delta
     */
    void UpdateBelief(const FExpectationDelta& Delta);
    
    /**
     * Get recent suboptimal actions
     * 
     * These are actions that SUCCEEDED but were WRONG.
     * This is the key indicator that we're an agent, not a copilot.
     */
    TArray<FExpectationDelta> GetRecentSuboptimalActions(int32 Count = 10) const;
    
    /**
     * Get surprise statistics
     * 
     * High average surprise = our world model is inaccurate
     * Low average surprise = our predictions are good
     */
    float GetAverageSurprise(int32 RecentCount = 100) const;
    
private:
    FExpectationTracker();
    
    /** Active expectations (waiting for observation) */
    TMap<FGuid, FExpectation> PendingExpectations;
    
    /** Recent deltas for learning */
    TArray<FExpectationDelta> RecentDeltas;
    
    /** Maximum deltas to keep */
    static constexpr int32 MAX_RECENT_DELTAS = 1000;
    
    /** Lock for thread safety */
    mutable FCriticalSection Lock;
    
    /** 
     * Form expectations based on tool type
     * Each tool type has different expectations
     */
    void FormToolSpecificExpectations(FExpectation& Exp, const FString& ToolName, 
        const TSharedPtr<FJsonObject>& Args);
    
    /**
     * Observe tool-specific state changes
     */
    void ObserveToolSpecificState(FOutcomeObservation& Obs, const FString& ToolName);
};
