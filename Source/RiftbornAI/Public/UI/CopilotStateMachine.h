// Copyright RiftbornAI. All Rights Reserved.
// CopilotStateMachine.h - Core state machine and data structures for Planning Copilot
//
// ARCHITECTURE:
// This file defines the SPINE of the planning copilot:
// - ECopilotMode: The 5 states (Conversation, Proposal, AwaitingApproval, Executing, Completed)
// - FConversationState: Extracted facts, missing info, constraints
// - FPlanDraft: Proposed plan with tool calls (mutable, editable)
// - FApprovedPlan: Immutable signed plan ready for execution
// - FExecutionTrace: Step results, proofs, undo tokens
//
// RULE: No execution without ApprovedPlan. No skipping states.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"  // For EToolRisk

// =============================================================================
// STATE MACHINE
// =============================================================================

/**
 * ECopilotMode - The copilot operates in exactly one of these modes at a time.
 * Transitions are explicit and enforced - no skipping.
 */
UENUM(BlueprintType)
enum class ECopilotMode : uint8
{
	/** Gathering info, clarifying intent. Tools OFF. */
	Conversation,
	
	/** Generating/editing a plan draft. Tools OFF. */
	Proposal,
	
	/** Plan draft shown to user, awaiting approval/edits. Tools OFF. */
	AwaitingApproval,
	
	/** Executing approved plan. Tools ON (only approved plan). */
	Executing,
	
	/** Execution complete (success, failure, or cancelled). Show summary. */
	Completed
};

/**
 * Get human-readable name for mode
 */
inline FString GetCopilotModeName(ECopilotMode Mode)
{
	switch (Mode)
	{
		case ECopilotMode::Conversation:      return TEXT("Conversation");
		case ECopilotMode::Proposal:          return TEXT("Proposal");
		case ECopilotMode::AwaitingApproval:  return TEXT("AwaitingApproval");
		case ECopilotMode::Executing:         return TEXT("Executing");
		case ECopilotMode::Completed:         return TEXT("Completed");
		default:                              return TEXT("Unknown");
	}
}

/**
 * Check if a mode transition is valid.
 * 
 * Valid transitions:
 * - Conversation → Proposal (enough info to plan)
 * - Conversation → Conversation (continue gathering)
 * - Proposal → AwaitingApproval (plan generated)
 * - Proposal → Conversation (need more info)
 * - AwaitingApproval → Executing (user approved)
 * - AwaitingApproval → Proposal (user requested edits)
 * - AwaitingApproval → Conversation (user rejected, wants new approach)
 * - Executing → Completed (plan finished/failed/cancelled)
 * - Completed → Conversation (new conversation)
 */
inline bool IsValidModeTransition(ECopilotMode From, ECopilotMode To)
{
	switch (From)
	{
		case ECopilotMode::Conversation:
			return To == ECopilotMode::Conversation || To == ECopilotMode::Proposal;
			
		case ECopilotMode::Proposal:
			return To == ECopilotMode::AwaitingApproval || To == ECopilotMode::Conversation;
			
		case ECopilotMode::AwaitingApproval:
			return To == ECopilotMode::Executing || To == ECopilotMode::Proposal || To == ECopilotMode::Conversation;
			
		case ECopilotMode::Executing:
			return To == ECopilotMode::Completed;
			
		case ECopilotMode::Completed:
			return To == ECopilotMode::Conversation;
			
		default:
			return false;
	}
}

// =============================================================================
// CONVERSATION STATE
// =============================================================================

/**
 * A single constraint/fact extracted from conversation.
 */
struct RIFTBORNAI_API FExtractedConstraint
{
	FString Name;       // e.g., "position", "color", "scale"
	FString Value;      // e.g., "0,0,100", "red", "0.5"
	FString Source;     // Which message it came from
	bool bUserProvided; // User said it vs AI assumed it
	
	FExtractedConstraint() : bUserProvided(false) {}
	FExtractedConstraint(const FString& InName, const FString& InValue, bool bFromUser = true)
		: Name(InName), Value(InValue), bUserProvided(bFromUser) {}
};

/**
 * A piece of info the copilot needs but doesn't have.
 */
struct RIFTBORNAI_API FMissingInfo
{
	FString Field;      // What's missing ("level", "position", "material")
	FString Question;   // How to ask for it
	bool bRequired;     // Can we proceed without it?
	FString DefaultValue; // What we'll assume if not provided
	
	FMissingInfo() : bRequired(false) {}
	FMissingInfo(const FString& InField, const FString& InQuestion, bool bIsRequired = false, const FString& InDefault = TEXT(""))
		: Field(InField), Question(InQuestion), bRequired(bIsRequired), DefaultValue(InDefault) {}
};

/**
 * FConversationState - What we know from the conversation.
 * 
 * Simplified: the LLM handles intent extraction, verb parsing, and clarification.
 * The controller just stores the original request and forwards it to the AI provider.
 */
struct RIFTBORNAI_API FConversationState
{
	/** The user's original request (first message in this task) */
	FString OriginalRequest;
	
	/** Extracted facts/constraints from UI interactions (e.g., SMissingInfoBox) */
	TArray<FExtractedConstraint> Constraints;
	
	/** What we still need to know (populated by UI widgets) */
	TArray<FMissingInfo> MissingInfo;
	
	// === Helpers ===
	
	/** Get constraint by name (or empty string if not found) */
	FString GetConstraint(const FString& Name) const
	{
		for (const FExtractedConstraint& C : Constraints)
		{
			if (C.Name.Equals(Name, ESearchCase::IgnoreCase))
			{
				return C.Value;
			}
		}
		return TEXT("");
	}
	
	/** Check if we have a constraint */
	bool HasConstraint(const FString& Name) const
	{
		return !GetConstraint(Name).IsEmpty();
	}
	
	/** Check if we have any required missing info */
	bool HasRequiredMissingInfo() const
	{
		for (const FMissingInfo& M : MissingInfo)
		{
			if (M.bRequired && M.DefaultValue.IsEmpty())
			{
				return true;
			}
		}
		return false;
	}
	
	/** Check if we have enough info to generate a plan */
	bool CanGeneratePlan() const
	{
		return !OriginalRequest.IsEmpty();
	}
	
	/** Clear for new conversation */
	void Reset()
	{
		OriginalRequest.Empty();
		Constraints.Empty();
		MissingInfo.Empty();
	}
};

// =============================================================================
// PLAN DRAFT (Mutable, Editable)
// =============================================================================

/**
 * A single tool call in a plan draft.
 */
struct RIFTBORNAI_API FDraftToolCall
{
	int32 StepIndex = 0;
	FString ToolName;
	TMap<FString, FString> Arguments;
	TArray<int32> Dependencies;  // Steps this depends on (by index)
	FString Description;
	
	// Computed from tool registry
	EToolRisk Risk = EToolRisk::Safe;
	bool bCanUndo = false;
	bool bNeedsConfirmation = false;
	
	// User edits
	bool bUserModified = false;
	bool bUserDisabled = false;  // User removed from plan
	
	FDraftToolCall() = default;
	FDraftToolCall(const FString& InToolName, const TMap<FString, FString>& InArgs)
		: ToolName(InToolName), Arguments(InArgs) {}
};

/**
 * FPlanDraft - A proposed plan that can be edited before approval.
 * 
 * This is what the user sees and can modify in the plan editor UI.
 * NOT yet executable - must be approved first.
 */
struct RIFTBORNAI_API FPlanDraft
{
	/** Unique ID for this draft */
	FGuid DraftId;
	
	/** What this plan accomplishes */
	FString GoalLine;
	
	/** Ordered steps (each is a tool call) */
	TArray<TSharedPtr<FDraftToolCall>> Steps;
	
	/** Success predicates - how we know the plan worked */
	TArray<FString> Predicates;  // e.g., "actor_count == 2", "blueprint_compiles"
	
	/** Expected outputs - what the user should see after execution */
	TArray<FString> ExpectedOutputs;  // e.g., "RedCube_1 at (0,0,100)", "BlueSphere_2 at (250,0,100)"
	
	/** Constraints/assumptions this plan is based on */
	TArray<FExtractedConstraint> BaseConstraints;
	
	// === Computed Properties ===
	
	/** Highest risk among enabled steps */
	EToolRisk ComputedRisk = EToolRisk::Safe;
	
	/** Can all enabled steps be undone? */
	bool bFullyReversible = true;
	
	/** Count of steps that mutate state */
	int32 MutatingStepCount = 0;
	
	/** Count of destructive steps */
	int32 DestructiveStepCount = 0;
	
	/** Estimated execution time (seconds) */
	float EstimatedTime = 0.0f;
	
	// === Methods ===
	
	FPlanDraft()
	{
		DraftId = FGuid::NewGuid();
	}
	
	/** Recompute derived properties from steps */
	void Recompute()
	{
		ComputedRisk = EToolRisk::Safe;
		bFullyReversible = true;
		MutatingStepCount = 0;
		DestructiveStepCount = 0;
		
		for (int32 i = 0; i < Steps.Num(); ++i)
		{
			if (!Steps[i].IsValid() || Steps[i]->bUserDisabled)
			{
				continue;
			}
			
			Steps[i]->StepIndex = i;
			
			if (Steps[i]->Risk > ComputedRisk)
			{
				ComputedRisk = Steps[i]->Risk;
			}
			
			if (!Steps[i]->bCanUndo)
			{
				bFullyReversible = false;
			}
			
			if (Steps[i]->Risk >= EToolRisk::Mutation)
			{
				MutatingStepCount++;
			}
			
			if (Steps[i]->Risk >= EToolRisk::Destructive)
			{
				DestructiveStepCount++;
			}
		}
	}
	
	/** Get enabled steps only */
	TArray<TSharedPtr<FDraftToolCall>> GetEnabledSteps() const
	{
		TArray<TSharedPtr<FDraftToolCall>> Enabled;
		for (const auto& Step : Steps)
		{
			if (Step.IsValid() && !Step->bUserDisabled)
			{
				Enabled.Add(Step);
			}
		}
		return Enabled;
	}
	
	/** Serialize to JSON for hashing/storage */
	FString ToJson() const;
	
	/** Compute hash of plan content (for integrity) */
	FString ComputeHash() const;
};

// =============================================================================
// APPROVED PLAN (Immutable, Signed)
// =============================================================================

/**
 * FApprovedPlan - A plan that has been approved and can be executed.
 * 
 * This is IMMUTABLE after creation. Any edits require creating a new draft.
 * The hash ensures integrity - if the plan changes, execution is blocked.
 */
struct RIFTBORNAI_API FApprovedPlan
{
	/** Source draft ID (for audit trail) */
	FGuid SourceDraftId;
	
	/** Unique ID for this approval */
	FGuid ApprovalId;
	
	/** When approved */
	FDateTime ApprovedAt;
	
	/** Who approved (supports multi-user - defaults to system username) */
	FString ApprovedBy;
	
	/** The frozen plan content */
	FString GoalLine;
	TArray<FDraftToolCall> Steps;  // Copied, not shared
	TArray<FString> Predicates;
	
	/** Integrity */
	FString PlanJson;
	FString PlanHash;  // SHA-256 of canonicalized PlanJson
	
	/** Risk summary (from draft) */
	EToolRisk HighestRisk = EToolRisk::Safe;
	bool bFullyReversible = true;
	int32 MutatingStepCount = 0;
	int32 DestructiveStepCount = 0;
	
	// === Methods ===
	
	FApprovedPlan()
	{
		ApprovalId = FGuid::NewGuid();
		ApprovedAt = FDateTime::Now();
		ApprovedBy = FPlatformProcess::UserName();
	}
	
	/** Create from draft (copies and freezes). Approver defaults to system username. */
	static FApprovedPlan FromDraft(const FPlanDraft& Draft, const FString& Approver = FString());
	
	/** Verify integrity (hash matches content) */
	bool VerifyIntegrity() const;
	
	/** Check if a step index is valid */
	bool IsValidStep(int32 Index) const { return Steps.IsValidIndex(Index); }
};

// =============================================================================
// EXECUTION TRACE
// =============================================================================

/**
 * Failure Type - Structured classification for failure reasons
 * Enables precise UI messaging and audit categorization
 */
enum class EFailureType : uint8
{
	None,              // Not a failure
	ToolError,         // Tool execution threw/returned error
	PolicyBlocked,     // Policy engine denied before execution
	Timeout,           // 60s execution timeout - backend unresponsive
	ValidationError,   // Schema/argument validation failed
	InvariantViolation,// Internal state machine corruption
	Unknown            // Catch-all for unclassified failures
};

/**
 * Result of a single step execution.
 * 
 * CRITICAL INVARIANT: NO TOOL CALL = NO STEP
 * The Tasks panel is an execution ledger, not a tutorial. Steps must
 * correspond 1:1 with actual tool invocations.
 */
struct RIFTBORNAI_API FStepExecutionResult
{
	int32 StepIndex = 0;
	FString ToolName;
	
	// Execution state
	bool bExecuted = false;             // Step was attempted (run or skipped)
	bool bCompleted = false;            // Step finished (success or fail)
	bool bSuccess = false;              // Step succeeded
	
	FString ResultSummary;
	FString ErrorMessage;
	FString FailureReason;              // Human-readable failure explanation
	
	// Undo support
	bool bCanUndo = false;
	FString UndoToken;
	
	// Proof/evidence
	FString ProofBundleId;
	FString ProofHash;                  // Hash of execution evidence
	FString WitnessJson;
	
	// Timing
	FDateTime StartedAt;
	FDateTime CompletedAt;
	float DurationSeconds = 0.0f;
	double ExecutionTimeMs = 0.0;       // How long it took
	
	// Failure classification
	EFailureType FailureType = EFailureType::None;
};

/**
 * Final state of a plan execution.
 */
UENUM(BlueprintType)
enum class EExecutionResult : uint8
{
	/** All steps succeeded */
	Completed,
	
	/** Some steps succeeded, some failed */
	PartialSuccess,
	
	/** All failed or critical failure */
	Failed,
	
	/** User cancelled */
	Cancelled,
	
	/** Rolled back by user */
	RolledBack
};

/**
 * FExecutionTrace - Complete record of plan execution.
 * 
 * Used for summary display, audit, and undo operations.
 */
struct RIFTBORNAI_API FExecutionTrace
{
	/** Which plan was executed */
	FGuid ApprovalId;
	FString PlanHash;  // For verification
	
	/** Execution timeline */
	FDateTime StartedAt;
	FDateTime CompletedAt;
	
	/** Step results in execution order */
	TArray<FStepExecutionResult> StepResults;
	
	/** Final state */
	EExecutionResult Result = EExecutionResult::Failed;
	FString ResultReason;
	
	/** Overall proof bundle ID */
	FString ProofBundleId;
	
	// === Computed Properties ===
	
	int32 SucceededCount() const
	{
		int32 Count = 0;
		for (const auto& R : StepResults) { if (R.bSuccess) Count++; }
		return Count;
	}
	
	int32 FailedCount() const
	{
		int32 Count = 0;
		for (const auto& R : StepResults) { if (!R.bSuccess) Count++; }
		return Count;
	}
	
	bool HasUndoableSteps() const
	{
		for (const auto& R : StepResults) { if (R.bCanUndo) return true; }
		return false;
	}
	
	/** Get all undo tokens for rollback */
	TArray<FString> GetUndoTokens() const
	{
		TArray<FString> Tokens;
		// Reverse order for proper rollback
		for (int32 i = StepResults.Num() - 1; i >= 0; --i)
		{
			if (StepResults[i].bCanUndo && !StepResults[i].UndoToken.IsEmpty())
			{
				Tokens.Add(StepResults[i].UndoToken);
			}
		}
		return Tokens;
	}
	
	/** Get summary for display */
	FString GetSummary() const
	{
		return FString::Printf(TEXT("%d/%d steps succeeded"), SucceededCount(), StepResults.Num());
	}
};

// =============================================================================
// STATE MACHINE AGGREGATE
// =============================================================================

/**
 * FCopilotStateMachine - Owns the mode and all associated state.
 * 
 * This is the SINGLE SOURCE OF TRUTH for copilot state.
 * All transitions go through this class and are validated.
 */
class RIFTBORNAI_API FCopilotStateMachine
{
public:
	FCopilotStateMachine();
	
	// === MODE TRANSITIONS ===
	
	/** Get current mode */
	ECopilotMode GetMode() const { return CurrentMode; }
	
	/** Attempt mode transition. Returns false if invalid. */
	bool TransitionTo(ECopilotMode NewMode, const FString& Reason = TEXT(""));
	
	/** Get reason for current mode */
	FString GetModeReason() const { return ModeReason; }
	
	// === STATE ACCESS ===
	
	/** Conversation state (mutable in Conversation mode only) */
	FConversationState& GetConversation() { return Conversation; }
	const FConversationState& GetConversation() const { return Conversation; }
	
	/** Plan draft (mutable in Proposal/AwaitingApproval modes) */
	TSharedPtr<FPlanDraft> GetDraft() const { return Draft; }
	void SetDraft(TSharedPtr<FPlanDraft> NewDraft);
	
	/** Approved plan (set when transitioning to Executing) */
	TSharedPtr<FApprovedPlan> GetApprovedPlan() const { return ApprovedPlan; }
	
	/** Execution trace (populated during/after execution) */
	TSharedPtr<FExecutionTrace> GetTrace() const { return Trace; }
	
	// === ACTIONS ===
	
	/** Start a new conversation (resets all state) */
	void StartNewConversation();
	
	/** Generate plan draft from conversation state */
	bool GenerateDraft();
	
	/** Approve current draft and prepare for execution.
	 *  @param Approver  Identity of the approver (defaults to system username) */
	bool ApproveDraft(const FString& Approver = FString());
	
	/** Start execution of approved plan */
	bool StartExecution();
	
	/** Record step result during execution */
	void RecordStepResult(const FStepExecutionResult& Result);
	
	/** Complete execution with final result */
	void CompleteExecution(EExecutionResult Result, const FString& Reason = TEXT(""));
	
	// === VALIDATION ===
	
	/** Can we generate a plan from current conversation? */
	bool CanGeneratePlan() const { return Conversation.CanGeneratePlan(); }
	
	/** Can we approve the current draft? */
	bool CanApproveDraft() const { return CurrentMode == ECopilotMode::AwaitingApproval && Draft.IsValid(); }
	
	/** Are tools enabled in current mode? */
	bool AreToolsEnabled() const { return CurrentMode == ECopilotMode::Executing; }
	
	// === DELEGATES ===
	
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnModeChanged, ECopilotMode /* Old */, ECopilotMode /* New */);
	FOnModeChanged OnModeChanged;
	
	DECLARE_MULTICAST_DELEGATE(FOnDraftChanged);
	FOnDraftChanged OnDraftChanged;
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnStepExecuted, const FStepExecutionResult&);
	FOnStepExecuted OnStepExecuted;
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnExecutionComplete, EExecutionResult);
	FOnExecutionComplete OnExecutionComplete;

private:
	ECopilotMode CurrentMode = ECopilotMode::Conversation;
	FString ModeReason;
	
	FConversationState Conversation;
	TSharedPtr<FPlanDraft> Draft;
	TSharedPtr<FApprovedPlan> ApprovedPlan;
	TSharedPtr<FExecutionTrace> Trace;
};
