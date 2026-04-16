// Copyright RiftbornAI. All Rights Reserved.
// ViewModel for Copilot UI - Pure state container, no Slate dependencies
//
// ARCHITECTURE:
// - ViewModel: Pure state, testable without Slate
// - Controller: Bridges ExecEngine/AgentCore to ViewModel
// - Renderers: Slate widgets that bind to ViewModel
//
// This separation allows unit testing of state machine logic.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"  // For EToolRisk

// Forward declarations
class FRiftbornCopilotController;

/**
 * Step execution status - unambiguous terminal states
 */
UENUM()
enum class EStepStatus : uint8
{
	Pending,       // Not yet executed
	Executing,     // Currently running
	Succeeded,     // Completed successfully
	Failed,        // Completed with error
	Skipped,       // Skipped (dependency failed or user decision)
	Cancelled      // Cancelled by user
};

/**
 * Plan lifecycle state for UI - single authoritative enum
 * NOTE: Different from EPlanState in AgentPlan.h (that's for execution engine)
 */
UENUM()
enum class EVMPlanState : uint8
{
	None,              // No plan
	PendingApproval,   // Awaiting user confirmation
	Executing,         // Running
	Completed,         // All steps succeeded
	PartialSuccess,    // Some succeeded, some failed
	Failed,            // All failed or aborted
	Blocked,           // Cannot execute (preconditions failed)
	Refused,           // User rejected
	RolledBack,        // Successfully undone
	Cancelled          // User cancelled during execution
};

/**
 * Single step in a plan - stable identity, observable state
 */
struct RIFTBORNAI_API FStepVM
{
	// Identity (stable across updates)
	FGuid StepId;
	int32 StepIndex = 0;
	
	// Tool info
	FString ToolName;
	FString Label;
	FString Description;
	
	// Risk assessment
	EToolRisk Risk = EToolRisk::Safe;
	bool bNeedsConfirmation = false;
	
	// Execution state
	EStepStatus Status = EStepStatus::Pending;
	
	// Evidence (populated after execution)
	bool bCanUndo = false;
	FString UndoToken;
	FString ResultSummary;
	FString ProofBundleId;
	FString ErrorMessage;
	FString WitnessJson;
	
	// Brain predictions (if available)
	float PSuccess = 0.0f;
	float PCatastrophic = 0.0f;
	bool bNeedsEscalation = false;
	
	FStepVM()
	{
		StepId = FGuid::NewGuid();
	}
	
	// Helpers
	bool IsTerminal() const
	{
		return Status == EStepStatus::Succeeded || 
		       Status == EStepStatus::Failed || 
		       Status == EStepStatus::Skipped ||
		       Status == EStepStatus::Cancelled;
	}
};

/**
 * Plan view model - contains all steps and aggregate state
 */
struct RIFTBORNAI_API FPlanVM
{
	// Identity
	FGuid PlanId;
	FString ProposalId;  // For integrity verification
	
	// Content
	FString GoalLine;       // What the user asked for
	FString IntentSummary;  // AI's interpretation
	FString PlanJson;       // Frozen plan for execution
	FString PlanHash;       // Integrity hash
	
	// Steps (stable identity, mutable state)
	TArray<TSharedPtr<FStepVM>> Steps;
	
	// Lifecycle state
	EVMPlanState State = EVMPlanState::None;
	FString StateReason;  // Why we're in this state (error message, etc.)
	// Alias for StateReason (used by some UI code)
	FString& CompletionReason = StateReason;
	
	// Execution tracking
	int32 CurrentStepIndex = 0;
	FDateTime ExecutedAt;
	
	// Undo support
	FString OverallUndoToken;
	TArray<FString> StepUndoTokens;
	
	// Risk assessment (computed from steps)
	EToolRisk HighestRisk = EToolRisk::Safe;
	bool bFullyReversible = true;
	
	FPlanVM()
	{
		PlanId = FGuid::NewGuid();
	}
	
	// === Computed Properties ===
	
	int32 SucceededCount() const
	{
		int32 Count = 0;
		for (const auto& Step : Steps)
		{
			if (Step.IsValid() && Step->Status == EStepStatus::Succeeded)
			{
				Count++;
			}
		}
		return Count;
	}
	
	int32 FailedCount() const
	{
		int32 Count = 0;
		for (const auto& Step : Steps)
		{
			if (Step.IsValid() && Step->Status == EStepStatus::Failed)
			{
				Count++;
			}
		}
		return Count;
	}
	
	int32 CompletedCount() const
	{
		int32 Count = 0;
		for (const auto& Step : Steps)
		{
			if (Step.IsValid() && Step->IsTerminal())
			{
				Count++;
			}
		}
		return Count;
	}
	
	bool HasUndoableSteps() const
	{
		for (const auto& Step : Steps)
		{
			if (Step.IsValid() && Step->bCanUndo && !Step->UndoToken.IsEmpty())
			{
				return true;
			}
		}
		return false;
	}
	
	bool IsComplete() const
	{
		return State == EVMPlanState::Completed ||
		       State == EVMPlanState::PartialSuccess ||
		       State == EVMPlanState::Failed ||
		       State == EVMPlanState::Blocked ||
		       State == EVMPlanState::Refused ||
		       State == EVMPlanState::RolledBack ||
		       State == EVMPlanState::Cancelled;
	}
	
	// Find step by ID
	TSharedPtr<FStepVM> GetStep(const FGuid& StepId)
	{
		for (const auto& Step : Steps)
		{
			if (Step.IsValid() && Step->StepId == StepId)
			{
				return Step;
			}
		}
		return nullptr;
	}
	
	TSharedPtr<FStepVM> GetStep(int32 Index)
	{
		if (Steps.IsValidIndex(Index))
		{
			return Steps[Index];
		}
		return nullptr;
	}
};

/**
 * Chat message view model
 */
struct RIFTBORNAI_API FChatMessageVM
{
	FGuid MessageId;
	
	enum class ESender : uint8 { User, Assistant, System };
	ESender Sender = ESender::User;
	
	FString Content;
	FDateTime Timestamp;
	
	// For assistant messages with thinking
	TArray<FString> ThinkingSteps;
	bool bIsStreaming = false;
	
	// For tool execution messages
	TOptional<FGuid> RelatedPlanId;
	
	FChatMessageVM()
	{
		MessageId = FGuid::NewGuid();
		Timestamp = FDateTime::Now();
	}
};

/**
 * Escalation view model
 */
struct RIFTBORNAI_API FEscalationVM
{
	FGuid EscalationId;
	
	enum class EType : uint8 { StepFailed, ProbeFailed, CatastrophicRisk, HighRisk, NeedsConfirmation };
	EType Type = EType::StepFailed;
	
	int32 StepIndex = -1;
	FString StepName;
	FString Reason;
	FString Evidence;
	float RiskLevel = 0.0f;
	FString RepairSuggestion;
	TArray<FString> AvailableActions;  // "Retry", "Skip", "Abort", etc.
	
	// Resolution
	bool bResolved = false;
	FString Decision;
	FString DecisionReason;
	FDateTime DecisionTime;
	
	FEscalationVM()
	{
		EscalationId = FGuid::NewGuid();
	}
};

// === DELEGATES ===

DECLARE_MULTICAST_DELEGATE(FOnPlanChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStepChanged, const FGuid& /* StepId */);
DECLARE_MULTICAST_DELEGATE(FOnChatChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEscalationAdded, const FGuid& /* EscalationId */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEscalationResolved, const FGuid& /* EscalationId */);
DECLARE_MULTICAST_DELEGATE(FOnEscalationChanged);  // Generic escalation state changed

/**
 * FRiftbornCopilotViewModel - Pure state container for Copilot UI
 * 
 * Owns all display state. Mutations only through explicit methods.
 * Fires delegates on state changes for UI binding.
 * 
 * TESTABLE: No Slate dependencies. Can be unit tested.
 */
class RIFTBORNAI_API FRiftbornCopilotViewModel : public TSharedFromThis<FRiftbornCopilotViewModel>
{
public:
	FRiftbornCopilotViewModel();
	~FRiftbornCopilotViewModel();
	
	// === PLAN STATE ===
	
	/** Get current plan (may be null) */
	TSharedPtr<FPlanVM> GetPlan() const { return CurrentPlan; }
	
	/** Set a new pending plan (replaces any existing) */
	void SetPendingPlan(TSharedPtr<FPlanVM> Plan);
	
	/** Transition plan to executing state */
	void BeginExecution();
	
	/** Update a step's status and evidence */
	void UpdateStep(const FGuid& StepId, EStepStatus NewStatus, const FString& ResultSummary = TEXT(""), const FString& ErrorMessage = TEXT(""));
	void UpdateStep(int32 StepIndex, EStepStatus NewStatus, const FString& ResultSummary = TEXT(""), const FString& ErrorMessage = TEXT(""));
	
	/** Set step's undo token */
	void SetStepUndoToken(const FGuid& StepId, const FString& UndoToken);
	void SetStepUndoToken(int32 StepIndex, const FString& UndoToken);
	
	/** Complete the plan with final state */
	void CompletePlan(EVMPlanState FinalState, const FString& Reason = TEXT(""));
	
	/** Clear plan entirely */
	void ClearPlan();
	
	// === CHAT STATE ===
	
	/** Get all messages */
	const TArray<TSharedPtr<FChatMessageVM>>& GetMessages() const { return Messages; }
	
	/** Add a message */
	void AddUserMessage(const FString& Content);
	void AddAssistantMessage(const FString& Content);
	void AddSystemMessage(const FString& Content);
	
	/** Clear chat */
	void ClearChat();
	
	// === ESCALATION STATE ===
	
	/** Get active escalations */
	const TArray<TSharedPtr<FEscalationVM>>& GetEscalations() const { return Escalations; }
	
	/** Add an escalation */
	FGuid AddEscalation(const FEscalationVM& Escalation);
	
	/** Resolve an escalation */
	void ResolveEscalation(const FGuid& EscalationId, const FString& Decision, const FString& Reason);
	
	// === DELEGATES ===
	
	FOnPlanChanged OnPlanChanged;
	FOnStepChanged OnStepChanged;
	FOnChatChanged OnChatChanged;
	FOnEscalationAdded OnEscalationAdded;
	FOnEscalationResolved OnEscalationResolved;
	FOnEscalationChanged OnEscalationChanged;  // Generic escalation state changed
	
	// === STATE MACHINE INVARIANTS ===
	
	/** Check if plan transition is valid */
	static bool IsValidTransition(EVMPlanState From, EVMPlanState To);
	
private:
	TSharedPtr<FPlanVM> CurrentPlan;
	TArray<TSharedPtr<FChatMessageVM>> Messages;
	TArray<TSharedPtr<FEscalationVM>> Escalations;
};
