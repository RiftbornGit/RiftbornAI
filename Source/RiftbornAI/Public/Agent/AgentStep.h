// Copyright RiftbornAI. All Rights Reserved.
// AgentStep.h - The atomic unit of agent action
//
// This is the AUTHORITATIVE interface for agent execution.
// Python proposes, C++ disposes.
//
// NOTE: This is pure C++ - no UHT reflection.
// The HTTP bridge serializes to JSON for Python consumption.

#pragma once

#include "CoreMinimal.h"
#include "CapabilityAdapter.h"
#include "Expectation.h"
#include "WorldStateDigest.h"
#include "ClaudeToolUse.h"
#include "ProductionHardening.h"

// =============================================================================
// AGENT CONTRACT - FROZEN INVARIANTS
// =============================================================================
// These constants define the agent's identity. Changing them is a breaking change.
// Version bump required for any modification.
//
// Contract version history:
//   1.0.0 (2026-01-08): Initial frozen contract
// =============================================================================

namespace FAgentContract
{
    /** Contract version - bump on breaking changes */
    static constexpr int32 VERSION_MAJOR = 1;
    static constexpr int32 VERSION_MINOR = 0;
    static constexpr int32 VERSION_PATCH = 0;
    
    /** Trust threshold for blocking tools (penalty > this = blocked) */
    static constexpr float BLOCK_THRESHOLD = 3.0f;
    
    /** Maximum penalty that can be applied in a single step (prevents spiraling) */
    static constexpr float MAX_PENALTY_PER_STEP = 1.0f;
    
    /** Default step timeout in milliseconds */
    static constexpr int32 DEFAULT_TIMEOUT_MS = 30000;
    
    /** 
     * EXPECTATIONS_AFFECT_TRUST - Master switch for expectation feedback
     * 
     * When false: Expectations are OBSERVATIONAL ONLY (log, don't act)
     * When true: Expectation violations can increment trust penalty
     * 
     * CRITICAL: This should remain false until proofs exist for:
     *   - Penalty magnitude bounded per step
     *   - No cascading penalties from single failure
     *   - False positive rate under 5%
     *   - Trust recovery path proven
     */
    static constexpr bool EXPECTATIONS_AFFECT_TRUST = false;
    
    /** Contract identity assertions - what the agent IS */
    static constexpr bool IS_REACTIVE = true;        // Requires external stimulus
    static constexpr bool IS_SINGLE_STEP = true;     // One tool per request
    static constexpr bool IS_REFUSABLE = true;       // May refuse actions
    static constexpr bool IS_SUBSTITUTING = true;    // May use alternatives
    static constexpr bool IS_EXPLAINABLE = true;     // Must explain decisions
    static constexpr bool IS_AUDITABLE = true;       // Records everything
    static constexpr bool IS_DETERMINISTIC = true;   // Same input = same output
    
    /** Contract identity assertions - what the agent IS NOT */
    static constexpr bool IS_AUTONOMOUS = false;     // Does NOT pursue goals alone
    static constexpr bool IS_PLANNING = false;       // Does NOT maintain plans
    static constexpr bool IS_LEARNING = false;       // Does NOT modify policy
    static constexpr bool IS_SELF_MODIFYING = false; // Does NOT change code
    
    /** Get version string */
    inline FString GetVersionString()
    {
        return FString::Printf(TEXT("%d.%d.%d"), VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    }
}

/**
 * Step outcome - what happened
 * 
 * INVARIANT: Every step MUST return one of these outcomes.
 * "Unknown" is NOT a valid outcome - it indicates plumbing failure.
 */
enum class EStepOutcome : uint8
{
    Success,            // Tool executed, expectations met
    Suboptimal,         // Tool executed, expectations violated  
    TechFailure,        // Tool threw a handled error
    Blocked,            // Tool blocked by trust gate, no alternative
    Recovered,          // Blocked but found and used alternative
    Timeout,            // Exceeded time budget
    Halted,             // Goal was halted by policy
    SystemError         // Internal error (crash prevented) - MUST STOP
};

inline FString StepOutcomeToString(EStepOutcome Outcome)
{
    switch (Outcome)
    {
        case EStepOutcome::Success: return TEXT("Success");
        case EStepOutcome::Suboptimal: return TEXT("Suboptimal");
        case EStepOutcome::TechFailure: return TEXT("TechFailure");
        case EStepOutcome::Blocked: return TEXT("Blocked");
        case EStepOutcome::Recovered: return TEXT("Recovered");
        case EStepOutcome::Timeout: return TEXT("Timeout");
        case EStepOutcome::Halted: return TEXT("Halted");
        case EStepOutcome::SystemError: return TEXT("SystemError");
        // No default - compiler will warn if new enum value added
    }
    return TEXT("SystemError"); // Fallback for corrupted enum
}

/**
 * FStepRequest - What the orchestrator wants to do
 * 
 * This comes from Python. C++ validates and executes.
 */
struct RIFTBORNAI_API FStepRequest
{
    /** Goal this step serves */
    FGuid GoalId;
    
    /** Tool to execute */
    FString ToolName;
    
    /** Arguments as JSON string */
    FString ArgumentsJson;
    
    /** Expected outcomes - claims that should be satisfied */
    TArray<FStateClaim> ExpectedOutcomes;
    
    /** Alternative tools if primary is blocked */
    TArray<FString> Alternatives;
    
    /** Maximum execution time in milliseconds */
    int32 MaxExecutionTimeMs = 30000;
    
    /** Step sequence number (for ordering) */
    int32 StepNumber = 0;
    
    /** If true, refuse to execute if lossy adaptation would occur */
    bool bStrictMode = false;
    
    // =========================================================================
    // P0.3 (2026-02-03): Plan Context for Proof Binding
    // 
    // Mutating proofs MUST include plan_hash + step_id for replayability.
    // These fields bind the execution to a specific governed plan.
    // =========================================================================
    
    /** SHA-256 hash of the plan this step belongs to (32 hex chars) */
    FString PlanHash;
    
    /** Unique identifier for the plan */
    FString PlanId;
    
    /** Index of this step within the plan (1-based to match ExecCtx step_id) */
    int32 PlanStepIndex = -1;

    // =========================================================================
    // P0.4 (2026-02-03): ExecCtx Binding for Proof Enforcement
    // These fields are populated at the HTTP boundary after validation.
    // =========================================================================

    /** Validated ExecCtx ID (for audit binding) */
    FString ExecCtxId;

    /** Validated ExecCtx signature (for audit binding) */
    FString ExecCtxSignature;

    /** Whether ExecCtx was fully validated at the route boundary */
    bool bExecCtxValidated = false;

    // =========================================================================
    // P0.5 (2026-02-05): Confirmation Token Pass-Through
    // 
    // Destructive tools in PROOF mode return NeedsConfirmation with a token.
    // The gateway re-submits the same step with this token attached.
    // The governance choke point validates and consumes the token, then
    // authorizes execution.
    // =========================================================================

    /** Single-use confirmation token for destructive operations */
    FString ConfirmationToken;
    
    /** Convert to JSON for logging */
    TSharedPtr<FJsonObject> ToJson() const;
    
    /** Parse from JSON */
    static FStepRequest FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * FStepResult - What actually happened
 * 
 * This is the ground truth. No hiding, no lying.
 */
struct RIFTBORNAI_API FStepResult
{
    /** Unique run identifier - binds this result to proof bundles */
    FGuid RunId;
    
    /** Refusal reason if step was refused (None if executed) */
    ERefusalReason RefusalReason = ERefusalReason::None;
    
    /** Request that was executed */
    FStepRequest Request;
    
    /** What tool was actually executed (may differ if recovered) */
    FString ExecutedTool;
    
    /** Outcome classification */
    EStepOutcome Outcome = EStepOutcome::TechFailure;
    
    /** The expectation that was formed before execution */
    FExpectation Expectation;
    
    /** The observation after execution */
    FOutcomeObservation Observation;
    
    /** The delta (surprise signal) */
    FExpectationDelta Delta;
    
    /** World state before execution */
    FWorldStateDigest PreState;
    
    /** World state after execution */
    FWorldStateDigest PostState;
    
    /** Tool result */
    FClaudeToolResult ToolResult;
    
    /** Execution time in milliseconds */
    float ExecutionTimeMs = 0.0f;
    
    /** Error message if failed */
    FString ErrorMessage;
    
    /** Was an alternative used? */
    bool bUsedAlternative = false;

    /** If recovered, what was the original blocked tool? */
    FString BlockedTool;

    /** Adapter proof record (populated when bUsedAlternative == true) */
    TOptional<FAdapterProof> AdapterProof;
    
    /** Timestamp when step completed */
    FDateTime CompletedAt;
    
    /** Was the belief updated as a result of this step? */
    bool bBeliefUpdated = false;
    
    /** Summary of next recommended action */
    FString NextActionSummary;
    
    /** Is the goal still achievable? */
    bool bGoalStillAchievable = true;
    
    /** Convert to JSON for proof bundle */
    TSharedPtr<FJsonObject> ToJson() const;
    
    /** Success check */
    bool IsSuccess() const { return Outcome == EStepOutcome::Success || Outcome == EStepOutcome::Recovered; }
    
    /** Should halt? */
    bool ShouldHalt() const { return Outcome == EStepOutcome::Halted || !bGoalStillAchievable; }
};

/**
 * FAgentRuntime - The C++ authority layer
 * 
 * This is where the buck stops. Python orchestrates, C++ executes.
 * 
 * Key invariants:
 * - All tool execution goes through Step()
 * - All expectations are formed and observed
 * - All deltas are computed and applied
 * - All trust gates are enforced
 * - All proofs are recorded
 */
class RIFTBORNAI_API FAgentRuntime
{
public:
    static FAgentRuntime& Get();
    
    /**
     * Execute a single step
     * 
     * This is THE interface. Everything else is sugar.
     * 
     * @param Request What the orchestrator wants
     * @return What actually happened
     */
    FStepResult Step(const FStepRequest& Request);
    
    /**
     * Check if a tool would be blocked
     * 
     * @param ToolName Tool to check
     * @return True if blocked by trust penalty
     */
    bool WouldBlock(const FString& ToolName) const;
    
    /**
     * Get best alternative for a blocked tool
     * 
     * @param BlockedTool The tool that's blocked
     * @param Alternatives List of alternatives to consider
     * @return Best alternative, or empty if none available
     */
    FString GetBestAlternative(const FString& BlockedTool, const TArray<FString>& Alternatives) const;
    
    /**
     * Get current goal progress
     * 
     * @param GoalId Goal to check
     * @return Progress 0.0 - 1.0
     */
    float GetGoalProgress(const FGuid& GoalId) const;
    
    /**
     * Halt execution for a goal
     * 
     * @param GoalId Goal to halt
     * @param Reason Why we're halting
     */
    void HaltGoal(const FGuid& GoalId, const FString& Reason);
    
    /**
     * Get recent steps for a goal
     * 
     * @param GoalId Goal to query
     * @param Count Maximum steps to return
     * @return Recent step results
     */
    TArray<FStepResult> GetRecentSteps(const FGuid& GoalId, int32 Count = 10) const;
    
    /**
     * Export proof bundle for a goal
     * 
     * @param GoalId Goal to export
     * @param OutputPath Where to write the bundle
     * @return True if successful
     */
    bool ExportProofBundle(const FGuid& GoalId, const FString& OutputPath) const;
    
private:
    FAgentRuntime();
    
    /** Internal step execution (can throw/crash - wrapped by Step()) */
    FStepResult StepInternal(const FStepRequest& Request);
    
    /** Create a SystemError result */
    FStepResult MakeSystemError(const FStepRequest& Request, const FString& ErrorMessage);
    
    /** Validate request before execution */
    FStepResult ValidateRequest(const FStepRequest& Request);
    
    /** Execute the actual tool (without plan context - marks as unplanned) */
    FClaudeToolResult ExecuteTool(const FString& ToolName, const FString& ArgumentsJson);
    
    /** Execute tool with plan context for proof binding (P0.3) */
    FClaudeToolResult ExecuteToolWithPlanContext(
        const FString& ToolName,
        const FString& ArgumentsJson,
        const FString& PlanHash,
        const FString& PlanId,
        int32 PlanStepIndex,
        const FString& ExecCtxId,
        const FString& ExecCtxSignature,
        bool bExecCtxValidated,
        const FString& ConfirmationToken = TEXT(""));
    
    /** Parse arguments JSON to TMap */
    TMap<FString, FString> ParseArguments(const FString& ArgumentsJson) const;
    
    /** Form expectation for a step */
    FExpectation FormExpectation(const FStepRequest& Request, const FWorldStateDigest& PreState);
    
    /** Observe outcome after execution */
    FOutcomeObservation ObserveOutcome(const FExpectation& Exp, const FClaudeToolResult& Result, 
                                        const FWorldStateDigest& PostState, float ExecutionTimeMs);
    
    /** Compute and apply delta */
    FExpectationDelta ComputeAndApplyDelta(const FExpectation& Exp, const FOutcomeObservation& Obs);
    
    /** Determine outcome classification */
    EStepOutcome ClassifyOutcome(const FClaudeToolResult& Result, const FExpectationDelta& Delta,
                                  bool bWasBlocked, bool bUsedAlternative);
    
    /** Determine if goal is still achievable */
    bool IsGoalStillAchievable(const FGuid& GoalId, const FExpectationDelta& Delta) const;
    
    /** Generate next action summary */
    FString GenerateNextActionSummary(const FStepResult& Result) const;

    /** Adapt arguments when swapping from a blocked tool to an alternative */
    bool AdaptArgumentsForAlternative(
        const FString& FromTool,
        const FString& ToTool,
        const TMap<FString, FString>& InArgs,
        TMap<FString, FString>& OutArgs,
        TOptional<FAdapterProof>& OutProof) const;
    
    /** Serialize arguments map to JSON string */
    FString SerializeArguments(const TMap<FString, FString>& Args) const;
    
    /** Emit step evidence to audit trail file */
    void EmitStepEvidence(const FStepResult& Result);
    
    /** Goal execution history */
    TMap<FGuid, TArray<FStepResult>> GoalHistory;
    
    /** Halted goals */
    TSet<FGuid> HaltedGoals;
    
    /** Lock for thread safety */
    mutable FCriticalSection Lock;
};
