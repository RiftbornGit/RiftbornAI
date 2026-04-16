// Copyright RiftbornAI. All Rights Reserved.
// Plan Executor - Step-by-step execution of frozen plans with full evidence chain

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include <atomic>  // For thread-safe cancel flag

/**
 * Error taxonomy - structured failure classification
 * Maps to Python bridge error types for consistent learning
 */
UENUM(BlueprintType)
enum class EPlanStepError : uint8
{
	None = 0,
	
	// Actor errors
	ActorNotFound,
	ActorNotFoundGuid,
	ActorNotFoundLabel,
	MissingActorIdentifier,
	
	// Property errors
	PropertyNotFound,
	PropertyReadonly,
	PropertyError,
	InvalidArgument,
	
	// Rollback errors
	RollbackGuidNotFound,
	RollbackLabelNotFound,
	RollbackPartial,
	
	// Context errors
	PieNotRunning,
	ContextMismatch,
	
	// System errors
	Timeout,
	BridgeDisconnected,
	PermissionDenied,
	CompileError,
	
	// Contract errors (PROOF mode enforcement)
	ContractViolation,       // Missing required witness or evidence
	NoContract,              // Tool has no contract in PROOF mode
	
	// Unknown
	UnknownCommand,
	Unclassified
};

/**
 * Parse error_type string from Python bridge into enum
 */
RIFTBORNAI_API EPlanStepError ParseErrorType(const FString& ErrorTypeStr);
RIFTBORNAI_API FString ErrorTypeToString(EPlanStepError Error);

/**
 * Single step execution result with full evidence
 */
struct RIFTBORNAI_API FPlanStepResult
{
	// Identity
	int32 StepIndex = -1;
	FString ToolName;
	FString ToolUseId;
	
	// Outcome
	bool bSuccess = false;
	EPlanStepError ErrorType = EPlanStepError::None;
	FString ErrorMessage;
	
	// Evidence chain
	FString ProofBundleId;
	FString WitnessJson;  // State change evidence
	TMap<FString, FString> OutputWitness;  // For chaining (guid, actor_name, etc.)
	
	// =========================================================================
	// WITNESS ENFORCEMENT (Priority 4 - PROOF mode)
	// =========================================================================
	
	/** Required witness keys from contract */
	TArray<FString> RequiredWitnessExpected;
	
	/** Required witness keys actually present */
	TArray<FString> RequiredWitnessPresent;
	
	/** Missing witness keys (if any) */
	TArray<FString> MissingWitness;
	
	/** Whether witness enforcement passed */
	bool bWitnessEnforcementPassed = true;
	
	/** Was this step verified in PROOF mode? (false = DEV mode skip) */
	bool bProofModeVerified = false;
	
	// =========================================================================
	
	// Undo support
	bool bCanUndo = false;
	FString UndoToken;  // GUID for rollback
	
	// Timing
	double ExecutionTimeMs = 0.0;
	FDateTime ExecutedAt;
	
	// Chain data for next step
	bool HasWitness(const FString& Key) const { return OutputWitness.Contains(Key); }
	FString GetWitness(const FString& Key) const { return OutputWitness.FindRef(Key); }
};

/**
 * Probe result - precondition check before execution
 */
struct RIFTBORNAI_API FProbeResult
{
	FString ProbeName;
	bool bPassed = false;
	FString Reason;
	TMap<FString, FString> ProbeData;  // Additional context
	
	static FProbeResult Pass(const FString& Name) 
	{ 
		FProbeResult R; 
		R.ProbeName = Name; 
		R.bPassed = true; 
		return R; 
	}
	
	static FProbeResult Fail(const FString& Name, const FString& Reason) 
	{ 
		FProbeResult R; 
		R.ProbeName = Name; 
		R.bPassed = false; 
		R.Reason = Reason; 
		return R; 
	}
};

/**
 * How a step's success is verified after execution.
 * This is the MOAT — every step must declare how its claim is checked.
 */
enum class EVerificationMethod : uint8
{
	None,              // No verification (read-only / informational tools only)
	DigestDiff,        // Before/after WorldStateDigest comparison
	WitnessKey,        // Required witness keys present in result
	AssetRegistry,     // Asset exists in registry after step
	BlueprintCompile,  // Blueprint compiles without error
	BuildSucceeded,    // C++ build succeeds
	PIEProbe,          // PIE running + state check
	FileExists,        // Output file exists on disk
	CustomProbe,       // Run a named probe and check result
	Screenshot         // Visual verification via screenshot comparison
};

/**
 * Evidence artifact produced by verification
 */
struct RIFTBORNAI_API FStepEvidence
{
	/** Verification method used */
	EVerificationMethod Method = EVerificationMethod::None;
	
	/** Whether verification passed */
	bool bPassed = false;
	
	/** Before-state hash (for digest diff) */
	FString BeforeHash;
	
	/** After-state hash (for digest diff) */
	FString AfterHash;
	
	/** The tool call result JSON */
	FString ToolResultHash;
	
	/** Witness keys captured */
	TMap<FString, FString> CapturedWitness;
	
	/** Human-readable summary */
	FString Summary;
	
	/** Timestamp */
	FDateTime Timestamp;
	
	/** Serialize to JSON for proof bundle */
	TSharedPtr<FJsonObject> ToJson() const;
};

/**
 * Execution step with args and probes
 * NOTE: Named FExecStep to avoid conflict with AgentPlan.h's FPlanStep
 */
struct RIFTBORNAI_API FExecStep
{
	int32 Index = 0;
	FString ToolName;
	TMap<FString, FString> Arguments;
	FString Description;
	
	// Dependencies
	TArray<int32> DependsOn;  // Step indices that must complete first
	
	// Probes (preconditions to check before execution)
	TArray<FString> RequiredProbes;  // e.g., ["target_exists", "pie_running"]
	
	// === VERIFICATION CONTRACT (ChatGPT Audit Priority 1) ===
	/** How this step's success is verified. No step completes without passing. */
	EVerificationMethod VerificationMethod = EVerificationMethod::WitnessKey;
	
	/** Custom probe name (when VerificationMethod == CustomProbe) */
	FString VerificationProbeName;
	
	/** Expected state claims for DigestDiff verification */
	TArray<FString> ExpectedEffects;  // e.g., ["ActorCount.Total:Increased", "Errors.Total:Equals:0"]
	
	/** Postcondition probes to run after execution */
	TArray<FString> PostconditionProbes;
	
	/** Verification evidence (filled after execution) */
	FStepEvidence Evidence;
	
	// Risk
	EToolRisk Risk = EToolRisk::Safe;
	bool bNeedsConfirmation = false;
};

/**
 * Parsed plan ready for execution
 */
struct RIFTBORNAI_API FExecutablePlan
{
	// Identity
	FGuid PlanId;
	FString OriginalRequest;
	FString IntentSummary;
	FString PlanHash;
	FString PlanHashKind;  // "canonical", "raw_fallback", "canonicalization_failed"
	FString PlanHashCanonicalizationError;
	
	// Steps
	TArray<FExecStep> Steps;
	
	// Execution state
	int32 CurrentStepIndex = 0;
	bool bAborted = false;
	FString AbortReason;
	bool bStepConfirmed = false;  // Set to true to confirm high-risk step
	bool bUserApprovedPlan = false;  // True when user explicitly approved the entire plan via UI - bypasses per-step confirmation
	
	// Results
	TArray<FPlanStepResult> StepResults;
	
	// Master undo
	FString OverallUndoToken;  // If plan supports atomic undo
	TArray<FString> StepUndoTokens;  // Per-step rollback
	
	// Computed
	EToolRisk HighestRisk = EToolRisk::Safe;
	bool bFullyReversible = true;
	
	// Status helpers
	bool IsComplete() const { return CurrentStepIndex >= Steps.Num() || bAborted; }
	bool HasFailures() const;
	int32 SucceededCount() const;
	int32 FailedCount() const;
	FExecStep* GetCurrentStep() { return Steps.IsValidIndex(CurrentStepIndex) ? &Steps[CurrentStepIndex] : nullptr; }
};

/**
 * Repair strategy when a step fails
 */
UENUM(BlueprintType)
enum class ERepairStrategy : uint8
{
	Continue,      // Log and continue to next step
	Retry,         // Retry same step (with backoff)
	Probe,         // Run probe to understand state, then decide
	Switch,        // Switch to alternative tool
	AskUser,       // Escalate to user for decision
	Abort          // Stop execution entirely
};

/**
 * Execution Engine - Step-by-step plan executor
 * NOTE: Named FExecEngine to avoid conflict with AgentPlan.h's FPlanExecutor
 * 
 * Executes frozen plans step-by-step with:
 * - Precondition probes before each step
 * - Full evidence chain (proof bundles, witnesses)
 * - Per-step undo tokens
 * - Structured error taxonomy
 * - Repair routing on failure
 */
class RIFTBORNAI_API FExecEngine
{
	// Allow FBridgePlanExecutor to access ExecuteTool
	friend class FBridgePlanExecutor;
	
public:
	static FExecEngine& Get();
	
	// =========================================================================
	// GOVERNED TOOL EXECUTION (PUBLIC GATEWAY)
	// 
	// This is the ONLY way internal systems should execute
	// tools. It enforces:
	// - Contract existence check (PROOF mode blocks uncontracted tools)
	// - Undo-by-tier enforcement (Dangerous/Destructive blocked without undo)
	// - Production tool surface restrictions
	// 
	// DO NOT bypass this by calling FClaudeToolRegistry::ExecuteTool directly.
	// =========================================================================
	
	/**
	 * Execute a tool with full governance enforcement
	 * 
	 * @param ToolName Tool to execute (must be in contracts.json for PROOF mode)
	 * @param Args Tool arguments
	 * @param OutError Filled on failure with rejection reason
	 * @return True if tool executed successfully, false if blocked or failed
	 * 
	 * This method:
	 * 1. Checks contract existence (PROOF mode blocks uncontracted)
	 * 2. Enforces undo-by-tier (Dangerous/Destructive require undo support)
	 * 3. Executes via the governed path
	 * 4. Returns structured result
	 */
	static bool ExecuteGovernedTool(
		const FString& ToolName, 
		const TMap<FString, FString>& Args,
		FString& OutResult,
		FString& OutError
	);
	
	// =========================================================================
	// PLAN PARSING
	// =========================================================================
	
	/**
	 * Parse frozen PlanJSON into executable plan
	 * Returns false if plan is malformed
	 */
	bool ParsePlan(const FString& PlanJSON, FExecutablePlan& OutPlan, FString& OutError);
	
	// =========================================================================
	// EXECUTION
	// =========================================================================
	
	/**
	 * Execute next step in plan
	 * Returns step result with full evidence
	 */
	FPlanStepResult ExecuteNextStep(FExecutablePlan& Plan);
	
	/**
	 * Execute entire plan with callbacks
	 * OnStepComplete called after each step (for UI updates)
	 * OnPlanComplete called when done
	 */
	void ExecutePlan(
		FExecutablePlan& Plan,
		TFunction<void(const FPlanStepResult&)> OnStepComplete,
		TFunction<void(const FExecutablePlan&)> OnPlanComplete
	);
	
	/**
	 * Execute plan asynchronously (non-blocking)
	 * Takes ownership of Plan via copy to avoid dangling references
	 */
	void ExecutePlanAsync(
		FExecutablePlan Plan,  // By value - copied for async ownership
		TFunction<void(const FPlanStepResult&)> OnStepComplete,
		TFunction<void(const FExecutablePlan&)> OnPlanComplete
	);
	
	/**
	 * Resume plan execution from a specific step (after confirmation/escalation)
	 * Used when execution paused for user decision and user chose to continue.
	 * (2026-01-31)
	 */
	void ResumePlanAsync(
		FExecutablePlan Plan,
		int32 FromStepIndex,
		TFunction<void(const FPlanStepResult&)> OnStepComplete,
		TFunction<void(const FExecutablePlan&)> OnPlanComplete
	);
	
	/**
	 * Request cancellation of current async execution.
	 * Safe to call from any thread. The current step will complete,
	 * then execution will stop with bAborted=true.
	 */
	void RequestCancel();
	
	/**
	 * Check if cancellation has been requested
	 */
	bool IsCancelRequested() const;
	
	/**
	 * Reset cancel flag (call before starting new execution)
	 */
	void ResetCancel();
	
	/**
	 * Confirm a step that requires confirmation.
	 * Call this when user approves a high-risk step, then retry execution.
	 */
	void ConfirmStep(FExecutablePlan& Plan);
	
	/**
	 * Abort running plan
	 */
	void AbortPlan(FExecutablePlan& Plan, const FString& Reason);
	
	// =========================================================================
	// PROBES (Precondition checks)
	// =========================================================================
	
	/**
	 * Run precondition probes for a step
	 * Returns false if any required probe fails
	 */
	bool RunProbes(const FExecStep& Step, TArray<FProbeResult>& OutResults);
	
	/**
	 * Available probes
	 */
	FProbeResult ProbeTargetExists(const FString& ActorRef);
	FProbeResult ProbePieRunning();
	FProbeResult ProbeAssetPathValid(const FString& AssetPath);
	FProbeResult ProbeBridgeConnected();
	
	// =========================================================================
	// UNDO
	// =========================================================================
	
	/**
	 * Undo a specific step by token
	 */
	bool UndoStep(const FString& UndoToken, FString& OutError);
	
	/**
	 * Undo entire plan (all reversible steps, in reverse order)
	 */
	bool UndoPlan(FExecutablePlan& Plan, FString& OutError);
	
	/**
	 * Undo the last N successful steps
	 */
	bool UndoLastSteps(FExecutablePlan& Plan, int32 Count, FString& OutError);
	
	// =========================================================================
	// REPAIR ROUTING
	// =========================================================================
	
	/**
	 * Determine repair strategy for a failed step
	 */
	ERepairStrategy GetRepairStrategy(const FExecStep& Step, const FPlanStepResult& Result);
	
	/**
	 * Attempt repair based on strategy
	 */
	FPlanStepResult AttemptRepair(FExecutablePlan& Plan, ERepairStrategy Strategy);
	
	// =========================================================================
	// PROOF BUNDLE SUPPORT
	// =========================================================================
	
	/**
	 * Get internal action log entries for proof bundle generation.
	 * These are direct C++ mutations that bypassed tool registry (logged for audit).
	 */
	static TArray<TSharedPtr<FJsonValue>> GetInternalActionsForProof();
	
	/**
	 * Clear internal action log (call after proof bundle is written)
	 */
	static void ClearInternalActionLog();
	
	/**
	 * Get count of internal actions logged
	 */
	static int32 GetInternalActionCount();
	
	// =========================================================================
	// PROBE CANONICALIZATION
	// =========================================================================
	
	/**
	 * Canonicalize probe result for deterministic hashing.
	 * Sorts lists, normalizes identifiers.
	 */
	static FString CanonicalizeProbeResult(const FProbeResult& Result);
	
	/**
	 * Hash a canonicalized probe result for proof chain
	 */
	static FString HashProbeResult(const FProbeResult& Result);

private:
	FExecEngine();
	
	// Execute single tool via bridge
	FPlanStepResult ExecuteTool(
		const FString& ToolName, 
		const TMap<FString, FString>& Args, 
		int32 StepIndex,
		const FString& PlanHash = TEXT(""),
		const FString& PlanHashKind = TEXT(""),
		const FString& PlanHashCanonicalizationError = TEXT("")
	);

	// Single dispatch point for tool execution (shared by plan and governed calls)
	FClaudeToolResult DispatchToolCall(
		const FString& ToolName,
		const TMap<FString, FString>& Args,
		const FString& ToolUseId
	);
	
	// Chain witness data from previous step results into current args
	TMap<FString, FString> ResolveChainedArgs(
		const FExecStep& Step, 
		const TArray<FPlanStepResult>& PreviousResults
	);
	
	// Extract error type from bridge response
	EPlanStepError ExtractErrorType(const TSharedPtr<FJsonObject>& Response);
	
	// Extract undo token from successful execution
	FString ExtractUndoToken(const TSharedPtr<FJsonObject>& Response);
	
	// Retry state
	int32 MaxRetries = 2;
	float RetryBackoffMs = 200.0f;
	
	// Cancellation support (thread-safe atomic flag)
	std::atomic<bool> bCancelRequested{false};
};
