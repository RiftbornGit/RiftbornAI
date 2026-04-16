// Copyright RiftbornAI. All Rights Reserved.
// Controller for Copilot UI - Bridges execution systems to ViewModel
//
// ARCHITECTURE (2026-01-31):
// The Controller owns the StateMachine and enforces the planning copilot loop:
// - Conversation → Proposal → AwaitingApproval → Executing → Completed
// - Tools are ONLY enabled during Executing mode
// - No execution without ApprovedPlan
//
// RESPONSIBILITIES:
// - Owns FCopilotStateMachine (source of truth for mode)
// - Converts user actions to state transitions
// - Coordinates FExecEngine for approved plan execution
// - Updates ViewModel for UI rendering
// - Does NOT touch Slate widgets directly
//
// PRIORITY 7 (2026-01-31): PREFLIGHT INTEGRATION
// - RunPreflight() validates draft before approval
// - Approval is BLOCKED if any step fails PROOF eligibility in PROOF mode
// - Preflight runs automatically after draft changes (debounced)

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "UI/RiftbornCopilotViewModel.h"
#include "UI/CopilotStateMachine.h"
#include "PlanPreflight.h"

class FAutonomousPlanner;
class IAIProvider;
struct FExecutablePlan;
struct FPlanStepResult;

/**
 * FRiftbornCopilotController - Bridges execution systems to ViewModel
 * 
 * NEW ARCHITECTURE:
 * - Owns FCopilotStateMachine (enforces mode transitions)
 * - ViewModel is for UI display only
 * - All state queries go through StateMachine
 * 
 * The UI binds to ViewModel delegates; Controller updates ViewModel from StateMachine.
 */
class RIFTBORNAI_API FRiftbornCopilotController : public TSharedFromThis<FRiftbornCopilotController>
{
public:
	FRiftbornCopilotController();
	~FRiftbornCopilotController();
	
	/** Get or create the singleton instance */
	static TSharedRef<FRiftbornCopilotController> Get();
	
	/** Get the ViewModel (for UI binding) */
	TSharedRef<FRiftbornCopilotViewModel> GetViewModel() const { return ViewModel.ToSharedRef(); }
	
	/** Get the StateMachine (source of truth) */
	TSharedRef<FCopilotStateMachine> GetStateMachine() const { return StateMachine.ToSharedRef(); }
	
	// =========================================================================
	// STATE MACHINE QUERIES (delegated)
	// =========================================================================
	
	/** Current mode */
	ECopilotMode GetMode() const { return StateMachine->GetMode(); }
	
	/** Are tools enabled? (only in Executing mode) */
	bool AreToolsEnabled() const { return StateMachine->AreToolsEnabled(); }
	
	/** Can we generate a plan from current conversation? */
	bool CanGeneratePlan() const { return StateMachine->CanGeneratePlan(); }
	
	/** Can we approve the current draft? */
	bool CanApproveDraft() const { return StateMachine->CanApproveDraft(); }
	
	/** Get conversation state */
	const FConversationState& GetConversation() const { return StateMachine->GetConversation(); }
	
	/** Get current draft (may be null) */
	TSharedPtr<FPlanDraft> GetDraft() const { return StateMachine->GetDraft(); }
	
	/** Get approved plan (may be null) */
	TSharedPtr<FApprovedPlan> GetApprovedPlan() const { return StateMachine->GetApprovedPlan(); }
	
	/** Get execution trace (may be null) */
	TSharedPtr<FExecutionTrace> GetTrace() const { return StateMachine->GetTrace(); }
	
	// =========================================================================
	// USER ACTIONS → STATE TRANSITIONS
	// =========================================================================
	
	/**
	 * User sent a message.
	 * - In Conversation mode: Updates constraints, may trigger plan generation
	 * - In AwaitingApproval mode: May be confirmation/rejection/edit
	 * - In Executing mode: Ignored (can't modify during execution)
	 * - In Completed mode: Starts new conversation
	 */
	void OnUserMessage(const FString& Message);
	
	/**
	 * User explicitly requests plan generation.
	 * Only valid in Conversation mode with sufficient info.
	 */
	bool GeneratePlan();

	/**
	 * Adopt a frozen plan JSON payload as the current draft.
	 * Used to route panel-authored proposals through the controller/state machine.
	 */
	bool AdoptPlanJsonAsDraft(const FString& GoalLine, const FString& PlanJson, FString& OutError);
	
	/**
	 * User approves the current draft.
	 * Transitions to Executing and starts execution.
	 */
	bool ApprovePlan();
	
	/**
	 * User rejects the current draft.
	 * Returns to Conversation mode.
	 */
	void RejectPlan(const FString& Reason = TEXT("User rejected"));
	
	/**
	 * User modifies the draft (changes args, removes step, etc.)
	 * Only valid in AwaitingApproval mode.
	 */
	bool ModifyDraftStep(int32 StepIndex, const FString& ArgumentName, const FString& NewValue);
	bool DisableDraftStep(int32 StepIndex);
	bool EnableDraftStep(int32 StepIndex);
	bool ReorderDraftSteps(int32 FromIndex, int32 ToIndex);
	
	/**
	 * User cancels execution in progress.
	 */
	void CancelExecution();
	
	/**
	 * User requests undo after execution.
	 */
	bool UndoExecution(FString& OutError);
	
	/**
	 * User starts a new conversation (clears all state).
	 */
	void StartNewConversation();
	
	// =========================================================================
	// =========================================================================
	// PREFLIGHT (Priority 7 - 2026-01-31)
	// =========================================================================
	
	/**
	 * Run preflight validation on current draft.
	 * Automatically called after draft changes (debounced).
	 * 
	 * @param bForceProofMode  Force PROOF mode checks even if not in PROOF mode
	 * @return The preflight result
	 */
	FPlanPreflightResult RunPreflight(bool bForceProofMode = false);
	
	/**
	 * Get the last preflight result.
	 * May be stale if draft changed since last preflight.
	 */
	const FPlanPreflightResult& GetLastPreflightResult() const { return LastPreflightResult; }
	
	/**
	 * Is the current draft approvable in PROOF mode?
	 * Quick check without full preflight if result is cached.
	 */
	bool IsProofApprovable() const;
	
	/**
	 * Are we in PROOF mode?
	 */
	bool IsProofMode() const { return bProofMode; }
	
	/**
	 * Set PROOF mode (strict validation).
	 */
	void SetProofMode(bool bEnabled);
	
	/**
	 * Delegate fired when preflight result changes.
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPreflightComplete, const FPlanPreflightResult&);
	FOnPreflightComplete OnPreflightComplete;
	
	bool UndoStep(int32 StepIndex, FString& OutError);
	
	// =========================================================================
	// ESCALATION HANDLING
	// =========================================================================
	
	void HandleEscalationResponse(const FGuid& EscalationId, const FString& Decision, const FString& Reason = TEXT(""));
	void ConfirmStep(int32 StepIndex);

private:
	TSharedPtr<FRiftbornCopilotViewModel> ViewModel;
	TSharedPtr<FCopilotStateMachine> StateMachine;
	
	// Execution state (for ExecEngine integration)
	TSharedPtr<FExecutablePlan> ExecutingPlan;
	bool bIsExecuting = false;
	
	// Preflight state (Priority 7)
	FPlanPreflightResult LastPreflightResult;
	bool bPreflightStale = true;         // True if draft changed since last preflight
	bool bProofMode = false;             // Synced with global in constructor; use SetProofMode() to change
	FTSTicker::FDelegateHandle PreflightDebounceTimer; // Debounce preflight after edits
	
	// === INTERNAL HELPERS ===
	
	/** Convert ApprovedPlan to FExecutablePlan for ExecEngine */
	TSharedPtr<FExecutablePlan> ConvertToExecutablePlan(TSharedPtr<FApprovedPlan> Plan);

	/** LLM-based plan generation — primary planning path. */
	void RequestLLMPlan(const FString& UserRequest);
	
	/** Sync ViewModel from StateMachine state */
	void SyncViewModelFromStateMachine();
	
	/** Handle step completion from ExecEngine */
	void OnStepComplete(const FPlanStepResult& StepResult);
	
	/** Handle plan completion from ExecEngine */
	void OnPlanComplete(const FExecutablePlan& CompletedPlan);
	
	/** Create escalation for failed step */
	void CreateStepFailureEscalation(int32 StepIndex, const FString& ToolName, const FString& ErrorMessage);
	
	// State machine event handlers
	void OnModeChanged(ECopilotMode OldMode, ECopilotMode NewMode);
	void OnDraftChanged();
	void OnStepExecuted(const FStepExecutionResult& Result);
	void OnExecutionComplete(EExecutionResult Result);
	
	/** Schedule debounced preflight after draft edit */
	void SchedulePreflight();
	
	/** Actually run preflight (called after debounce) */
	void ExecuteScheduledPreflight();
};
