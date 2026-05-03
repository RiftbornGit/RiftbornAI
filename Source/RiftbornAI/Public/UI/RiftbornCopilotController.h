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
#include "Providers/ProviderUtils.h"

class FAutonomousPlanner;
class IAIProvider;
struct FExecutablePlan;
struct FPlanStepResult;

enum class ECopilotTurnRoute : uint8
{
	Conversation,
	Proposal,
	Action,
	Controller
};

enum class ECopilotToolProfile : uint8
{
	None,
	ReadOnlyChat,
	ProposalPlan,
	AgenticExecution,
	CodeAuthoring,
	Bridge
};

struct RIFTBORNAI_API FCopilotTurnRoutingContext
{
	ECopilotMode ControllerMode = ECopilotMode::Conversation;
	bool bHasPendingProposal = false;
	bool bHasOfferedActions = false;
	bool bRecentExecutionContext = false;
	bool bReadOnlyMode = false;
};

struct RIFTBORNAI_API FCopilotTurnRouteDecision
{
	ECopilotTurnRoute Route = ECopilotTurnRoute::Conversation;
	ECopilotToolProfile ToolProfile = ECopilotToolProfile::None;
	ProviderUtils::ECopilotPromptProfile PromptProfile = ProviderUtils::ECopilotPromptProfile::Default;
	FString RequestText;
	FString Reason;
	bool bRouteThroughController = false;
	bool bNeedsConfirmation = false;
	bool bPlanFirst = false;
	bool bReadOnlyOnly = false;
	bool bAutoApprove = false;
};

struct RIFTBORNAI_API FCopilotTelemetrySnapshot
{
	TMap<FString, int32> RouteCounts;
	TMap<FString, int32> PromptProfileCounts;
	TMap<FString, int32> ToolProfileCounts;
	int32 ApprovalCount = 0;
	int32 RejectionCount = 0;
	int32 ReplanCount = 0;
	int32 StalePlannerResponses = 0;
};

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

	/** Return the singleton only if it already exists. Does not force controller startup. */
	static TSharedPtr<FRiftbornCopilotController> TryGetIfInitialized();

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
	void OnUserMessage(const FString& Message, bool bRecordInChat = true);

	/** Build the canonical route decision for a user turn. */
	FCopilotTurnRouteDecision RouteUserTurn(const FString& Message, const FCopilotTurnRoutingContext& Context);

	/** Same as RouteUserTurn but suppresses telemetry — used by per-frame
	 *  composer preview so the log isn't spammed thousands of times per second. */
	FCopilotTurnRouteDecision PreviewRouteUserTurn(const FString& Message, const FCopilotTurnRoutingContext& Context);

	/** Handle a route decision that belongs to the controller-owned path. */
	bool HandleRoutedTurn(const FCopilotTurnRouteDecision& Decision, bool bRecordInChat = true);

	/** Start a proposal turn and generate a governed plan draft asynchronously. */
	bool BeginProposalTurn(const FString& UserRequest, bool bRecordInChat = true);

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

	/** Telemetry snapshot for route/prompt/tool-profile drift and approval behavior. */
	FCopilotTelemetrySnapshot GetTelemetrySnapshot() const { return Telemetry; }

	/** Sync the panel's currently selected model into controller-owned planner turns. */
	void SetSelectedModelId(const FString& ModelId) { SelectedModelId = ModelId; }
	const FString& GetSelectedModelId() const { return SelectedModelId; }
	void SetAutoApproveNextProposal(bool bEnabled) { bAutoApproveNextProposal = bEnabled; }

#if WITH_AUTOMATION_TESTS
	void DebugSetActivePlanRequestSerial(uint64 InSerial) { ActivePlanRequestSerial = InSerial; }
	bool DebugProcessPlannerResponse(const FString& UserRequest, uint64 RequestSerial, bool bSuccess, const FString& Response);
#endif

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
	uint64 ActivePlanRequestSerial = 0;  // Correlates async planner replies with the latest request
	FCopilotTelemetrySnapshot Telemetry;
	FString SelectedModelId;
	bool bAutoApproveNextProposal = false;

	// Per-frame composer-preview calls were hammering RecordRouteTelemetry
	// thousands of times per second. Suppress during preview, and dedupe
	// identical consecutive log lines even when suppression is missed.
	int32 SuppressRouteTelemetryDepth = 0;
	FString LastRouteTelemetryLogKey;

	// === SELF-HEAL / AGENTIC RETRY ===
	// When a plan finishes with step failures, the controller can re-prompt the
	// LLM with the failure context so it can reason about what went wrong and
	// propose a repair plan. Bounded to prevent infinite loops.
	FString SelfHealGoal;              // original user request to keep anchoring repair prompts
	int32 SelfHealRoundsRemaining = 0; // decremented each repair turn; 0 disables
	bool bInSelfHealRound = false;     // true while OnPlanComplete is issuing a repair proposal
	// Ten rounds: photorealistic visual goals (forest, landscape, biome)
	// routinely need 5+ build+critique cycles to satisfy vision — the first
	// few rounds place geometry, later rounds fix density, lighting, camera
	// framing, and material tuning. Ten is still bounded to prevent infinite
	// loops and caps session spend at roughly 10× a baseline plan.
	static constexpr int32 SelfHealMaxRounds = 10;

	// === INTERNAL HELPERS ===

	/** Convert ApprovedPlan to FExecutablePlan for ExecEngine */
	TSharedPtr<FExecutablePlan> ConvertToExecutablePlan(TSharedPtr<FApprovedPlan> Plan);

	/** LLM-based plan generation — primary planning path. */
	void RequestLLMPlan(const FString& UserRequest);

	/** Handle one async planner response on the game thread. */
	bool ProcessPlannerResponse(const FString& UserRequest, uint64 RequestSerial, bool bSuccess, const FString& Response);

	void RecordRouteTelemetry(const FCopilotTurnRouteDecision& Decision);
	void IncrementRouteCounter(TMap<FString, int32>& Counters, const FString& Key);

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
