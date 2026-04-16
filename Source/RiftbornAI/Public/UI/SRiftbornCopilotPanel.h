// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Views/SListView.h"
#include "DiffPreviewWidget.h"
#include "RiftbornAgentCore.h"
#include "ClaudeToolUse.h"  // For EToolRisk
#include "PlanExecutor.h"  // For EPlanStepError, FExecutablePlan
#include "UI/CopilotStateMachine.h"  // For FStepExecutionResult, EFailureType
#include "UI/CopilotGitInfo.h"       // For FCopilotGitInfo (footer git bar)
#include "Core/ConversationMemory.h" // For FConversationSession — needed as a full type for legacy restore signature

class IAIProvider;
class FActionHistory;
class FConversationMemory;
class URiftbornBridgeMonitor;
class FRiftbornCopilotController;  // ViewModel+Controller architecture
class FRiftbornCopilotViewModel;
struct FPlanVM;                      // Forward declaration for SyncStepResultsFromViewModel
class SAgentTimeline;       // Agent event timeline widget
class FAgentTaskRunner;     // Task runner for bounded execution
class SEditableTextBox;     // Forward declaration for inline API key input

struct FCopilotResolvedTurnModel
{
	bool bValid = false;
	bool bAutoSelected = false;
	FString DisplayModelId;
	FString ProviderName;
	FString ProviderModelId;
	FString DisplayLabel;
	FString RoutingReason;
};

/**
 * Policy decision from governance layer
 * MUST match ERiftbornPolicyDecision semantically - don't collapse categories
 */
enum class EPolicyDecision : uint8
{
	Allowed,         // Policy satisfied, execution proceeds
	NeedsEscalation, // Policy requires user confirmation
	Forbidden,       // Policy/trust blocks a valid request
	FailedValidation // Malformed request (schema/args error) - distinct from Forbidden
};

/**
 * Turn Type - The cognitive mode of the conversation
 * This is the KEY architectural primitive that separates chat from action
 */
enum class ETurnType : uint8
{
	Conversation,   // LLM talks freely - NO tools, NO tasks
	Proposal,       // LLM proposes a plan - shows what it WOULD do, awaits confirmation
	Action,         // User-approved execution - tools enabled, tasks created
	System          // Logs, warnings, errors, status updates
};

/**
 * Execution State - Canonical state machine for plan lifecycle
 * NOT booleans. NOT implicit. Explicit state transitions only.
 */
enum class EExecutionState : uint8
{
	None,              // Default state - no plan loaded
	Idle,              // No plan exists
	PendingApproval,   // Plan proposed, awaiting user authorization
	Executing,         // User authorized, execution in progress
	Completed,         // All steps succeeded
	PartialSuccess,    // Some steps succeeded, some failed
	Failed,            // Execution failed (one or more steps)
	Refused,           // User rejected the plan
	Blocked,           // Policy/safety/validation denied the plan (not user choice)
	RolledBack         // User undid the execution
};

/**
 * Escalation Type - Why execution is paused for user decision
 * Maps to specific UI cards with action buttons
 */
enum class EEscalationType : uint8
{
	None,              // No escalation - execution proceeds normally
	NeedsConfirmation, // High-risk tier step requires explicit consent before execution
	ProbeFailed,       // Precondition probe failed - target missing, state invalid
	StepFailed,        // Tool execution failed - shows repair options
	CatastrophicRisk   // Brain predicts high catastrophic failure probability
};

/**
 * Escalation Decision - User response to an escalation card
 * Persisted in proof bundle for audit trail
 */
enum class EEscalationDecision : uint8
{
	Pending,           // No decision yet
	Proceed,           // User chose to proceed despite warning/failure
	Retry,             // User chose to retry the step
	TryAlternative,    // User chose to retry with an alternative tool
	Repair,            // User chose to apply suggested repair
	Skip,              // User chose to skip this step
	Abort,             // User chose to abort the entire plan
	AskForHelp         // User chose to escalate to chat for guidance
};

/**
 * Escalation Card Data - All info needed to render an escalation UI
 */
struct FEscalationCard
{
	EEscalationType Type = EEscalationType::None;
	int32 StepIndex = -1;               // Which step triggered escalation
	FString StepName;                   // e.g., "delete_asset"
	FString Reason;                     // Why escalation is needed
	FString Evidence;                   // Probe result, error message, risk data
	TArray<FString> AvailableActions;   // e.g., ["Proceed", "Retry", "Abort"]
	FString RepairSuggestion;           // Suggested fix from repair strategy
	float RiskLevel = 0.0f;             // For UI coloring (0=green, 1=red)
	
	// Decision outcome (populated after user responds)
	EEscalationDecision Decision = EEscalationDecision::Pending;
	FString DecisionReason;             // Optional user-provided explanation
	bool bUserOverride = false;         // True if user overrode safety warning
	FDateTime DecisionTime;
};

// EFailureType and FStepExecutionResult are now defined in CopilotStateMachine.h

/** Schema version for execution history persistence */
static constexpr int32 ExecutionHistorySchemaVersion = 1;

/**
 * Proposed Plan - Frozen snapshot of what will execute
 * 
 * CRITICAL: This is an EXECUTION CONTRACT, not a description.
 * The PlanJSON is what WILL run when user authorizes.
 * No re-planning, no drift, no reinterpretation.
 */
/**
 * Per-step confidence from brain prediction
 * Advisory display - never blocks execution, just warns user
 */
struct FStepConfidence
{
	float PSuccess = 0.5f;              // P(success) from brain [0,1]
	float PCatastrophic = 0.0f;         // P(catastrophic failure) [0,1]
	bool bNeedsEscalation = false;      // Brain recommends human review
	FString Source;                     // "brain" or "default" (for calibration)
};

struct FProposedPlan
{
	// === GOAL (what the user asked for) ===
	FString OriginalRequest;            // "Can you place a box in my scene?" - verbatim
	FString GoalSummary;                // "Place a box in the scene" - normalized goal line
	
	// === DISPLAY (for user) ===
	FString IntentSummary;              // "Add cube to active level" - terse, declarative
	TArray<FString> Steps;              // MUST be tool names, NOT tutorial steps. See FStepExecutionResult comment.
	int32 StepCount = 0;                // For "6 steps" display
	
	// === EXECUTION SNAPSHOT (frozen contract) ===
	FString PlanJSON;                   // Full structured plan - THIS is what executes
	FString PlanHash;                   // SHA256 of canonicalized PlanJSON - integrity check
	FGuid ProposalId;                   // Unique ID for audit trail
	
	// === EXECUTION RESULTS (populated during/after execution) ===
	EExecutionState State = EExecutionState::Idle;  // Current lifecycle state
	TArray<FStepExecutionResult> StepResults;       // Per-step execution evidence
	FString OverallUndoToken;                       // Master undo token for entire plan
	TArray<FString> StepUndoTokens;                 // Per-step undo tokens for granular rollback
	FDateTime ExecutedAt;                           // When execution completed
	
	// Helper methods for execution tracking
	int32 SucceededCount() const
	{
		int32 Count = 0;
		for (const FStepExecutionResult& R : StepResults)
		{
			if (R.bSuccess) ++Count;
		}
		return Count;
	}
	int32 FailedCount() const
	{
		int32 Count = 0;
		for (const FStepExecutionResult& R : StepResults)
		{
			if (R.bExecuted && !R.bSuccess) ++Count;
		}
		return Count;
	}
	
	// === RECOMMENDATION (advisory only, never affects execution) ===
	FString RecommendedTool;            // e.g., "spawn_actor" - from brain.recommended_tool
	float RecommendedConfidence = 0.0f; // e.g., 0.86 - from brain.confidence
	FString RecommendationStrategy;     // e.g., "pattern_match" - for optional expander
	
	// === BRAIN CONFIDENCE (per-step, advisory display) ===
	TArray<FStepConfidence> StepConfidences;  // Parallel to Steps array
	bool bHasConfidenceData = false;          // True if brain provided predictions
	
	// === METADATA ===
	TArray<FString> ToolsRequired;      // ["spawn_actor", "set_property"]
	EToolRisk HighestRisk = EToolRisk::Safe;
	bool bReversible = true;            // Can this be undone?
	FDateTime ProposedAt;
	
	// Generate hash from PlanJSON
	void ComputeHash();
	bool VerifyHash() const;
};

/**
 * Execution event - single tool invocation with full context
 * This is the GROUND TRUTH from backend, not parsed from strings
 */
struct FExecutionEvent
{
	// === CORRELATION (binds event to plan/step) ===
	FGuid PlanId;               // Which plan this event belongs to - MUST match CurrentPlan.ProposalId
	int32 StepIndex = -1;       // Which step triggered this event (-1 = unknown/legacy)
	
	// Identity
	FString ToolName;           // e.g., "create_basic_geometry"
	FString ToolUseId;          // Unique invocation ID
	FDateTime Timestamp;

	// Governance (from tool registry + policy engine)
	EToolRisk Risk = EToolRisk::Safe;          // From tool metadata
	EPolicyDecision PolicyDecision = EPolicyDecision::Allowed;
	FString PolicyReason;                       // Why allowed/blocked
	int32 RequiredAutonomyLevel = 0;            // L0-L6
	int32 CurrentAutonomyLevel = 0;             // User's current level

	// Reversibility (from undo subsystem)
	bool bCanUndo = false;
	FString UndoToken;          // Transaction ID for undo
	FString UndoReason;         // If can't undo, why not

	// Proof (from proof bundle system)
	FString ProofBundleId;      // Artifact reference
	FString ProofHash;          // SHA256 or similar
	FString ReplayPointer;      // How to reproduce

	// Payload
	TMap<FString, FString> InputArgs;   // What was requested
	FString RawResultJson;              // Full response (for Details expander)
	TArray<FDiffEntry> Diffs;           // Optional file diffs (post-change evidence)
	bool bDiffTruncated = false;
	double ExecutionTimeMs = 0.0;
	bool bSuccess = false;
	FString ErrorMessage;
};

/**
 * Chat message structure for the conversation
 * Now SCHEMA-DRIVEN: UI renders, does not interpret
 * Styled like Claude Code / Codex with thinking, actions, and results
 */
struct FRiftbornChatMessage
{
	enum class ERole : uint8
	{
		User,
		Assistant,
		System
	};

	ERole Role;
	ETurnType TurnType = ETurnType::Conversation;  // NEW: Cognitive mode of this turn
	FDateTime Timestamp;

	// === TURN-SPECIFIC DATA ===
	TOptional<FProposedPlan> ProposedPlan;  // For Proposal turns - what AI would do
	bool bAwaitingConfirmation = false;      // True if Proposal needs user approval

	// === THINKING LAYER (AI reasoning - collapsible) ===
	FString ThinkingText;       // "I need to create a cube actor. Let me use spawn_actor..."
	bool bThinkingCollapsed = true;  // Default collapsed

	// === INTENT LAYER (human readable summary) ===
	FString IntentText;         // "Created a cube in your scene at (0,0,100)"

	// === PLAN LAYER (semi-technical activity log) ===
	TArray<FString> PlanStepSummaries;  // Brief description of each step

	// === EXECUTION LAYER (full ground truth) ===
	TArray<FExecutionEvent> ExecutionEvents;

	// === RESULT LAYER (what was accomplished) ===
	FString ResultSummary;      // "Created 1 actor: MyCube"
	bool bSuccess = true;

	// === LEGACY (for plain text messages) ===
	FString Content;            // Raw text (used only for non-tool messages)

	// === ATTACHED IMAGES (snapshot of PendingImageAttachments at send time) ===
	TArray<FString> AttachedImagePaths;

	// Helpers
	bool HasToolCalls() const { return ExecutionEvents.Num() > 0; }
	EToolRisk GetHighestRisk() const
	{
		EToolRisk Highest = EToolRisk::Safe;
		for (const auto& Evt : ExecutionEvents)
		{
			if (static_cast<uint8>(Evt.Risk) > static_cast<uint8>(Highest))
				Highest = Evt.Risk;
		}
		return Highest;
	}
	EPolicyDecision GetOverallPolicyDecision() const
	{
		for (const auto& Evt : ExecutionEvents)
		{
			if (Evt.PolicyDecision == EPolicyDecision::Forbidden)
				return EPolicyDecision::Forbidden;
			if (Evt.PolicyDecision == EPolicyDecision::NeedsEscalation)
				return EPolicyDecision::NeedsEscalation;
		}
		return EPolicyDecision::Allowed;
	}
	bool IsFullyReversible() const
	{
		for (const auto& Evt : ExecutionEvents)
		{
			if (!Evt.bCanUndo) return false;
		}
		return ExecutionEvents.Num() > 0;
	}
	
	// Turn type helpers
	bool IsConversation() const { return TurnType == ETurnType::Conversation; }
	bool IsProposal() const { return TurnType == ETurnType::Proposal; }
	bool IsAction() const { return TurnType == ETurnType::Action; }
	bool NeedsConfirmation() const { return bAwaitingConfirmation && ProposedPlan.IsSet(); }
};

/**
 * Thinking step for task progress display
 * Styled like Codex/Claude Code activity log
 */
struct FRiftbornThinkingStep
{
	FString Text;
	FString ToolName;
	EToolRisk Risk = EToolRisk::Safe;
	EPolicyDecision PolicyDecision = EPolicyDecision::Allowed;
	bool bCompleted = false;
	bool bSucceeded = true;          // Outcome: true=success, false=failed (only meaningful when bCompleted)
	double StartTimeSeconds = 0.0;   // When this step started
	double DurationMs = 0.0;         // How long it took (filled on completion)
};

/**
 * Plan step for UI display with status tracking
 */
struct FPlanStepUI
{
	FString Label;              // Human readable
	FString ToolName;           // Technical
	EToolRisk Risk = EToolRisk::Safe;
	EPolicyDecision PolicyDecision = EPolicyDecision::Allowed;
	float Confidence = 1.0f;    // 0.0-1.0
	enum class EStatus : uint8 { NotStarted, InProgress, Succeeded, Failed, Skipped };
	EStatus Status = EStatus::NotStarted;
	bool bCanUndo = true;
	FString UndoToken;
	FGuid StepId;

	FPlanStepUI()
	{
		StepId = FGuid::NewGuid();
	}
};

/**
 * Execution mode for governance
 */
enum class EExecutionMode : uint8
{
	Contracted,   // Riftborn-compliant, strong verification
	Exploratory   // Hypothesis-based, requires confirmation
};

// NOTE: ERequestMode (Chat/Act) was DELETED. ETurnType is the canonical routing primitive.
// See ClassifyTurnType() for the single authoritative routing decision.

/** Asset search result for @ mention autocomplete */
struct FAssetMentionResult
{
	FString DisplayName;
	FString AssetPath;
	FString AssetClassName;
};

/** Pending image attachment in chat input */
struct FImageAttachment
{
	FString FilePath;
	FString FileName;
};

/** Conversation thread for multi-tab chat.
 *
 *  Token counters here are the per-thread SNAPSHOT — the live counter is
 *  on the panel (see SRiftbornCopilotPanel::TotalInputTokens). They are
 *  copied panel→thread by SaveCurrentThreadState() and thread→panel by
 *  RestoreThreadState() on every tab switch. Same pattern as Messages /
 *  ThinkingSteps below: the active tab's working state lives on the panel,
 *  inactive tabs' state lives on the thread. Don't sync them here. */
struct FChatThread
{
	FString ThreadId;
	FString Title;
	TArray<FRiftbornChatMessage> Messages;
	TArray<FRiftbornThinkingStep> ThinkingSteps;
	TArray<FExecutionEvent> LiveExecutionEvents;
	int32 TotalInputTokens = 0;
	int32 TotalOutputTokens = 0;
	bool bThinkingCollapsed = false;
	bool bLastProcessingSucceeded = true;
	FDateTime CreatedAt;
};

/**
 * SRiftbornCopilotPanel - GitHub Copilot-style chat interface
 *
 * A modern, dark-themed AI assistant panel matching the UI prototype design.
 * Features:
 * - Collapsible side panels (conversations, context)
 * - Model selector dropdown
 * - Token counter
 * - Inline thinking stream for tasks
 * - Animated thinking indicator for chat
 * - Round send/stop buttons
 */
class RIFTBORNAI_API SRiftbornCopilotPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRiftbornCopilotPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SRiftbornCopilotPanel();

	// Public interface
	void ClearChat();
	void SetModel(const FString& ModelId);
	FString GetCurrentModel() const { return CurrentModelId; }
	void SubmitMessage(const FString& Message) { SendMessage(Message); }

	// Content Browser drag-drop support
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;

private:
	// === UI Construction ===
	TSharedRef<SWidget> BuildWelcomeScreen();
	TSharedRef<SWidget> BuildWelcomeScreen_Normal();
	TSharedRef<SWidget> BuildLicenseGatePanel();

	// === Message Widgets ===
	TSharedRef<SWidget> BuildMessageWidget(const FRiftbornChatMessage& Message, int32 Index);
	TSharedRef<SWidget> BuildRunCapsuleFromMessage(const FRiftbornChatMessage& Message, int32 Index);
	TSharedRef<SWidget> BuildUserMessage(const FRiftbornChatMessage& Message, int32 Index);
	TSharedRef<SWidget> BuildAssistantMessage(const FRiftbornChatMessage& Message, int32 Index);
	TSharedRef<SWidget> BuildProposalMessage(const FRiftbornChatMessage& Message, int32 Index);  // Plan preview with confirm/cancel
	TSharedRef<SWidget> BuildThinkingStream(const TArray<FRiftbornThinkingStep>& Steps, bool bFinished, bool bSucceeded = true);
	TSharedRef<SWidget> BuildThinkingIndicator();
	TSharedRef<SWidget> BuildLiveExecutionWidget(const TArray<FExecutionEvent>& Events);
	TSharedRef<SWidget> BuildLogEventsWidget(const TArray<FExecutionEvent>& Events);  // Log panel helper

	// === Event Handlers ===
	FReply OnSendClicked();
	FReply OnStopClicked();
	FReply OnSettingsClicked();

	FReply OnQuickAction(const FString& Prompt);
	void OnModelSelected(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnInputTextChanged(const FText& NewText);
	void OnInputTextCommitted(const FText& NewText, ETextCommit::Type CommitType);
	FReply OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	// === Agent Core Callbacks ===
	void OnAgentCoreProgress(const FAgentTask& Task, const FString& Message);
	void OnAgentCoreTaskComplete(const FAgentTask& Task, bool bSuccess);
	void OnAgentCoreStateChanged(const FAgentTask& Task, EAgentState NewState);

	// === Widget Tick ===
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	// === Chat Logic ===
	void SendMessage(const FString& Message);
	bool TryHandleLocalCommand(const FString& Message);
	void RunCopilotWiringSelfTest();
	void AddUserMessage(const FString& Content);
	void AddAssistantMessage(const FString& Content, const TArray<FString>& AttachedImagePaths = {});  // Legacy: plain text
	void AddStructuredAssistantMessage(               // Schema-driven: full execution data
		const FString& IntentText,
		const TArray<FExecutionEvent>& Events,
		const FString& RawContent = TEXT(""));
	void AddStructuredAssistantMessage(               // Schema-driven: full execution data + activity log
		const FString& IntentText,
		const TArray<FExecutionEvent>& Events,
		const FString& RawContent,
		const TArray<FString>& PlanStepSummaries);
	void AddStructuredAssistantMessage(               // Full Claude Code style: thinking + actions + result
		const FString& ThinkingText,
		const FString& ResultSummary,
		bool bSuccess,
		const TArray<FExecutionEvent>& Events,
		const TArray<FString>& PlanStepSummaries);

	// Generate human-readable result summary from execution events
	static FString GenerateResultSummary(const TArray<FExecutionEvent>& Events, bool bSuccess);
	void StartThinkingStream();
	void AddThinkingStep(const FString& Text, const FString& ToolName = TEXT(""));
	void CompleteLatestThinkingStep(bool bSucceeded);
	void AddLiveExecutionEvent(const FExecutionEvent& Event);
	void ClearLiveExecutionState(bool bRefresh = true);
	void SyncLiveExecutionState();
	void CompleteThinkingStream(const FString& Summary = TEXT(""));
	void RefreshStreamingTextDisplay(bool bForceAll = false);
	void ShowThinkingIndicator();
	void HideThinkingIndicator();
	void RefreshHealthStatus();
	void StopGeneration();

	// =========================================================================
	// EXECUTION MODES - Unified Authority System (2026-01-30)
	// =========================================================================
	// ALL tool execution flows through: FAutonomousPlanner → FExecEngine
	// 
	// Chat Mode:     LLM response only, no tools
	// Act Mode:      Plan → execute immediately unless a step requires explicit confirmation
	// Proposal Mode: Plan → user confirm → PlanExecutor (deferred)
	//
	// ProcessWithLLM is DEV-ONLY and blocked in shipping builds.
	// See: Architecture review 2026-01-30
	// =========================================================================
	
	/** @deprecated DEV-ONLY: Direct LLM tool execution bypasses authority system */
	void ProcessWithLLM(const FString& Message);
	
	/** Chat mode: Direct LLM response, NO tools, NO task tree */
	void ProcessChatMode(const FString& Message);
	
	/** Act mode: Generate plan and execute immediately via PlanExecutor */
	void ProcessActMode(const FString& Message);
	
	/** Proposal mode: Generate plan, show in Tasks panel, await user confirmation */
	void ProcessProposalMode(const FString& Message);
	
	/**
	 * Agentic mode: Iterative tool-use loop via AgenticLoopRunner.
	 * The LLM calls tools, sees results, calls more tools, until done.
	 * This is the REAL agentic experience — the LLM adapts based on
	 * what actually happened, not a pre-frozen plan.
	 * 
	 * Used for complex requests that need multi-step reasoning with
	 * intermediate observation (e.g., "make a game level").
	 */
	void ProcessAgenticMode(const FString& Message);
	
	/** Deferred heavy setup for agentic mode — runs on next tick to avoid UI freeze */
	void ProcessAgenticModeDeferred(const FString& Message);
	
	/** Determine if a request should use agentic mode vs proposal mode */
	bool ShouldUseAgenticMode(const FString& Message) const;
	
	/**
	 * Build a system prompt section describing the honest capability boundaries
	 * of the available tool catalog. Groups tools by category, lists what the
	 * tools CAN do, and explicitly states what is OUTSIDE scope.
	 * 
	 * This prevents the "confidently wrong" failure mode where the LLM gets
	 * a complex request and wings it with scene-level tools, producing scattered
	 * cubes instead of acknowledging it can't build game systems.
	 * 
	 * @param ToolCatalog The tools selected for this agentic session
	 * @return Prompt section to append to system prompt (empty if no boundaries to add)
	 */
	FString BuildCapabilityBoundariesPrompt(const TArray<FClaudeTool>& ToolCatalog) const;
	
	/**
	 * Classify the user's request against available tool capabilities.
	 * Detects when the request implies domains (networking, game logic, UI widgets, etc.)
	 * that are beyond the scope of the editor-level tool catalog.
	 * 
	 * Returns a prompt section with per-domain scope warnings telling the LLM
	 * exactly what it CAN do toward the goal and what requires manual work.
	 * Returns empty string if the request appears fully within scope.
	 * 
	 * @param Message The user's original request
	 * @param ToolCatalog The tools selected for this agentic session
	 * @return Prompt section with scope warnings (empty if request is in-scope)
	 */
	FString ClassifyRequestScope(const FString& Message, const TArray<FClaudeTool>& ToolCatalog) const;
	
	/** Resolve the concrete provider/model pair for this turn. */
	FCopilotResolvedTurnModel ResolveTurnModelForRequest(const FString& Message, int32 EstimatedSteps) const;
	
	/**
	 * Post-execution verification — runs after plan or agentic session completes.
	 * 
	 * Uses FExpectationTracker to compute surprise/delta between pre and post state,
	 * runs quick verification checks (blueprint compile, reference integrity),
	 * and emits a verification summary to the chat.
	 * 
	 * This catches "succeeded but wrong" — the crucial gap between tool success
	 * and actual user intent satisfaction.
	 * 
	 * @param ToolTrace Tools that were executed (names + success status)
	 * @param bOverallSuccess Whether the plan/session reported success
	 * @param OriginalRequest The user's original message (for intent matching)
	 */
	void RunPostExecutionVerification(
		const TArray<TPair<FString, bool>>& ToolTrace,
		bool bOverallSuccess,
		const FString& OriginalRequest);
	
	/**
	 * LLM-driven plan generation fallback.
	 * Called when FAutonomousPlanner::CreatePlan() returns zero actions (regex miss).
	 * Sends the user's message to the LLM with the full tool catalog from
	 * FClaudeToolRegistry, collects tool_use calls, and builds an FProposedPlan.
	 * 
	 * This is the bridge that lets the copilot reach ALL 370+ registered tools
	 * instead of only the ~15 that the regex planner can parse.
	 * 
	 * The LLM is used ONLY for plan generation (tool selection + arg filling).
	 * Execution still goes through the standard PlanExecutor with governance.
	 */
	void CreatePlanFromLLM(const FString& Message);
	
	/** Canonical turn classifier - determines which execution mode to use */
	ETurnType ClassifyTurnType(const FString& Message) const;
	
	/** Detect when LLM response offers numbered actions for the user to select from */
	void DetectOfferedActions(const FString& AssistantResponse);
	
	// Turn-based conversation helpers
	void AddConversationTurn(const FString& Content);  // Chat bubble, no tools
	void AddProposalTurn(const FString& Summary, const TArray<FString>& Steps, const TArray<FString>& Tools);
	void AddActionTurn(const FString& IntentText, const TArray<FExecutionEvent>& Events);
	void AddSystemTurn(const FString& Content);  // Warnings, errors, status
	void AddToolSuccessToChat(const FString& ToolName, const TMap<FString, FString>& Witness);  // Step succeeded
	void AddToolFailureToChat(const FString& ToolName, EPlanStepError ErrorType, const FString& ErrorMessage);  // Step failed
	void RefreshTasksPanelStepStatus(int32 StepIndex, bool bSuccess, const FString& Message, const FString& UndoToken);  // Update step UI
	void RefreshTasksPanelFinalState();  // Update plan header for completion
	bool HasPendingProposal() const { return PendingProposal.IsSet(); }
	void ClearPendingProposal();
	void ExecutePendingProposal();  // User confirmed - executes SNAPSHOT, not re-plan
	bool IsConfirmationMessage(const FString& Message) const;
	bool IsCancellationMessage(const FString& Message) const;
	void HandleClarificationLoop();  // Detect and break loops
	
	// Tasks panel - center of gravity for plans
	void PopulateTasksPanelWithPlan(const FProposedPlan& Plan);
	void RestoreTasksPanelReadOnly();  // Show completed/failed plan without mutating state
	void ClearTasksPanel();
	void MarkTasksPlanExecuting(int32 ActiveStepIndex = 0);  // Update header to show EXECUTING + active step
	void MarkTasksPlanCompleted(bool bSuccess);  // Update header to show COMPLETED/FAILED
	void MarkTasksPlanRefused();                 // Update header to show REFUSED (user rejected)
	void MarkTasksPlanBlocked(const FString& PolicyName, const FString& Reason);  // Policy/safety blocked
	TSharedRef<SWidget> BuildPlanStepRow(const FString& StepText, int32 Index, bool bCompleted, bool bSuccess);
	TSharedRef<SWidget> BuildPlanStepRowWithConfidence(const FString& StepText, int32 Index, bool bCompleted, bool bSuccess, const FStepConfidence* Confidence);
	TSharedRef<SWidget> BuildPlanStepRowWithEvidence(const FString& StepText, int32 Index, bool bCompleted, bool bSuccess, const FString& ResultSummary);
	
	// === ESCALATION UI ===
	void ShowEscalationCard(const FEscalationCard& Card);   // Display escalation inline in tasks panel
	void DismissEscalationCard();                            // Remove escalation card
	void HandleEscalationDecision(EEscalationDecision Decision, const FString& Reason = TEXT(""));
	TSharedRef<SWidget> BuildEscalationCardWidget(const FEscalationCard& Card);  // Build the card UI
	void LogEscalationForProof(const FEscalationCard& Card); // Persist to proof bundle
	
	// === TOOLBOOK UI ===
	void ShowToolbook();                          // Toggle toolbook panel visibility
	void HideToolbook();
	void PopulateToolbookWithSearch(const FString& SearchTerm);  // Filter tools
	TSharedRef<SWidget> BuildToolbookEntry(const FString& ToolName, const struct FClaudeTool& Tool);
	void ShowToolDetail(const FString& ToolName);  // Show expanded tool view
	
	// === SHARED EXECUTION CALLBACKS (DRY — extracted from 5 duplicated sites) ===
	/** Shared OnStepComplete callback for plan execution (used by Execute, Resume, Retry, Repair, Skip) */
	void HandleStepComplete(const FPlanStepResult& StepResult);
	/** Shared OnPlanComplete callback for plan execution */
	void HandlePlanComplete(const FExecutablePlan& CompletedPlan, const FString& OriginalRequest = TEXT(""));
	/** Create a bound OnStepComplete delegate for async execution */
	TFunction<void(const FPlanStepResult&)> MakeStepCompleteCallback();
	/** Create a bound OnPlanComplete delegate for async execution */
	TFunction<void(const FExecutablePlan&)> MakePlanCompleteCallback(const FString& OriginalRequest = TEXT(""));
	
	// Execution persistence - evidence survives panel close
	void PersistCurrentPlan();                    // Save after state change
	void LoadExecutionHistory();                  // Load on panel open
	FString GetHistoryFilePath() const;           // Saved/RiftbornAI/execution_history.json
	static constexpr int32 MaxHistoryEntries = 10; // Keep last N executions
	TArray<FProposedPlan> ExecutionHistory;       // Loaded from disk

	// Chat session persistence - conversation survives panel close/reopen
	void SaveChatSession();                       // Serialize Messages to disk (auto-called on changes)
	void LoadChatSession();                       // Restore Messages from disk (called in Construct)
	FString GetChatSessionFilePath() const;       // Legacy import fallback: Saved/RiftbornAI/chat_session.json
	static constexpr int32 MaxChatSessionMessages = 200; // Trim old messages beyond this
	bool RestoreSerializedChatSession(const FString& SerializedChatJson, const FString& SourceLabel);
	void RestoreLegacyConversationSession(const FConversationSession& Session);

	bool IsTaskRequest(const FString& Text) const;
	bool ShouldUseAgentCore(const FString& Text) const;

	// === Token Tracking ===
	void UpdateTokens(int32 InputTokens, int32 OutputTokens);
	float GetEstimatedCost() const;
	
	/** Get the context window limit for a given model ID (in tokens) */
	static int32 GetModelContextLimit(const FString& ModelId);
	static FString GetModelDisplayLabel(const FString& ModelId);
	static FString FormatContextWindowLabel(int32 LimitTokens);

	/** Map a model ID from the dropdown to the IAIProvider implementation name
	 *  expected by FAIProviderFactory::CreateProvider. Recognised prefixes:
	 *  `claude-code:*` → "ClaudeCode", `claude*` → "Claude", `gpt*` → "OpenAI",
	 *  `gemini*` → "Gemini", unknown local IDs → "Ollama", `auto` → empty.
	 *  Centralised so every call site stays in sync. */
	static FString ProviderNameForModelId(const FString& ModelId);

	/** Strip the `claude-code:` prefix when present, returning the underlying
	 *  Anthropic model ID (e.g. `claude-code:opus-4-7` → `claude-opus-4-7`).
	 *  IDs that aren't claude-code-prefixed are returned unchanged. */
	static FString UnderlyingModelIdForProvider(const FString& ModelId);
	static FString MakeThreadTitle(const FString& SourceText);
	FString GetActiveThreadHeading() const;
	FString GetLatestUserPrompt() const;

	// === Session History (Improvement 2) ===
	/** Build the dropdown menu showing recent conversation sessions */
	TSharedRef<SWidget> BuildSessionHistoryMenu();
	/** Restore a saved conversation session by ID */
	void RestoreSession(const FString& SessionId);
	void ResetActiveAgenticSessionState(bool bFinalizeStreamingBubble = true);

	// === Action History (Undo/Redo) ===
	FReply OnUndoClicked();
	FReply OnRedoClicked();
	bool CanUndo() const;
	bool CanRedo() const;
	void RecordAction(const FString& ToolName, const FString& Args, const FString& Description);

	// === Session Management ===
	void NewSession();

	// === WOW FEATURES (2026-02-08) ===
	
	// Feature 1: Streaming chat — real-time token display during LLM responses
	void BeginStreamingBubble();                    // Create an empty assistant bubble for streaming
	void AppendStreamingToken(const FString& Token); // Append token to streaming bubble
	void FinalizeStreamingBubble();                 // Finalize streaming bubble into a real message
	
	// Feature 2: Auto-execute low-risk — skip approval for Safe-risk plans
	bool ShouldAutoExecute(const FProposedPlan& Plan) const;  // Policy: auto-execute if all Safe + reversible
	
	// Feature 3: Multi-turn memory — conversation context for LLM
	FString BuildConversationContext(int32 MaxTurns = 10) const;  // Last N turns as context string
	
	// Feature 3b: Context window auto-summarization — compress older turns when nearing limit
	FString SummarizeOlderMessages(int32 MaxTokenBudget = 500) const;  // Compress older half into digest
	int32 EstimateTokenCount(const FString& Text) const;               // Rough token estimate (~4 chars/token)
	bool IsContextWindowPressured() const;                              // True when >60% of model limit used
	
	// Feature 4: Contextual suggestions — dynamic quick actions
	TArray<FString> GenerateContextualSuggestions() const;  // Scene-aware suggestions
	void RefreshSuggestions();                               // Update suggestion chips in input area
	TSharedRef<SWidget> BuildSuggestionChips();              // Build clickable chip row
	
	// Feature 5: Smart error recovery — retry failed steps with alternatives
	void RetryFailedStep(int32 StepIndex);                    // Retry a failed step
	void RetryWithAlternative(int32 StepIndex, const FString& AlternativeTool);  // Retry with different tool
	TArray<FString> GetAlternativeTools(const FString& FailedTool) const;  // Find similar tools
	
	// Feature 6: Undo/Redo — prominent global undo button + redo implementation
	void UndoLastAction();                                   // Undo most recent action
	void RedoLastAction();                                   // Redo last undone action
	TArray<FString> UndoneTokens;                            // Stack of undone tokens for redo
	
	// Feature 7: Scene awareness — proactive context injection + working memory
	FString GatherSceneContext() const;                      // Query current scene state (spatial analysis, actor inventory)
	FString BuildWorkingMemory() const;                      // Summarize recent actions for LLM context
	mutable FString CachedSceneContext;                      // Cached scene context to avoid repeated actor iteration
	mutable double CachedSceneContextTimestamp = 0.0;        // When the cache was last populated
	static constexpr double SceneContextCacheTTL = 2.0;      // Cache TTL in seconds
	
	// Feature 8: One-click game templates — predefined multi-step plans
	struct FGameTemplate
	{
		FString Name;
		FString Description;
		FString IconEmoji;
		TArray<FString> Steps;      // Tool calls to execute
		TArray<FString> ToolNames;  // Required tools
	};
	TArray<FGameTemplate> GetAvailableTemplates() const;
	void ExecuteTemplate(const FGameTemplate& Template);
	TSharedRef<SWidget> BuildTemplateCard(const FGameTemplate& Template);
	
	// Feature 9: Explain blueprint — read and explain blueprint structure
	void ExplainBlueprint(const FString& BlueprintPath);     // Trigger blueprint explanation
	bool IsExplainRequest(const FString& Message) const;     // Detect "explain" intent
	FString ExtractBlueprintTarget(const FString& Message) const;  // Extract BP name from message

	// Feature 11: /fix command — thin wrapper over governed diagnostics/repair tools
	void ProcessFixCommand(const FString& Args);             // Entry point for /fix command
	bool IsFixCommand(const FString& Message) const;         // Detect "/fix" prefix

	// Feature 12: /look command — thin wrapper over governed viewport/vision tools
	void ProcessLookCommand(const FString& Question);        // Entry point for /look command
	bool IsLookCommand(const FString& Message) const;        // Detect "/look" prefix

	// Feature 10: Visual plan editor — wire SPlanEditor for drag/drop editing
	void ShowPlanEditor(const FProposedPlan& Plan);          // Open plan editor overlay
	void OnPlanEditorSaved(const FProposedPlan& EditedPlan); // User saved edited plan
	
	// === Plan Steps Management ===
	void ClearPlanSteps();
	void AddPlanStep(const FString& Label, const FString& ToolName, float Confidence = 1.0f, bool bCanUndo = true);
	void UpdatePlanStepStatus(int32 Index, FPlanStepUI::EStatus Status);
	void RefreshPlanStepsDisplay();

	// === Helpers ===
	void ClearDraftText();
	void AppendDraftText(const FString& Chunk);
	void ApplyTodosFromStepsText(const FString& StepsText);
	void ApplyTodosFromExecutionEvents(const TArray<FExecutionEvent>& Events);
	void FinalizeTodosFromExecution(const TArray<FExecutionEvent>& Events, bool bRequestSucceeded);
	void RefreshPanels();
	void RefreshPanelsImmediate();                           // Force immediate rebuild (bypass debounce)
	bool bPanelsDirty = false;                               // Dirty flag for deferred RefreshPanels
	void ScheduleScrollToBottom(int32 TickCount = 3);
	int32 PendingAutoScrollTicks = 0;                        // Continue scrolling for a few ticks after content/layout changes
	void ScrollToBottom();
	FSlateColor GetAccentColor() const;

private:
	// === VIEWMODEL + CONTROLLER (2026-01-31 refactor) ===
	// Canonical source of truth for plan/step state
	// Panel binds to ViewModel delegates, calls Controller methods
	TSharedPtr<FRiftbornCopilotController> CopilotController;
	
	// ViewModel change handlers
	void OnViewModelPlanChanged();
	void OnViewModelStepChanged(const FGuid& StepId);
	void OnViewModelEscalationChanged();
	
	/** Sync ViewModel step data → Panel's CurrentPlan.StepResults (Controller path fix) */
	void SyncStepResultsFromViewModel(const TSharedPtr<FPlanVM>& Plan);
	
	// === UI Elements ===
	TSharedPtr<SScrollBox> LeftTimelineScroll;
	TSharedPtr<SScrollBox> RightLogScroll;
	TSharedPtr<SVerticalBox> LeftTimelineContainer;  // Left panel cards
	TSharedPtr<SVerticalBox> RightLogContainer;      // Right panel logs (legacy)
	TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
	
	// === Agent Task Runner ===
	TSharedPtr<FAgentTaskRunner> AgentTaskRunner;    // Bounded task execution
	TSharedPtr<SAgentTimeline> AgentTimeline;        // Event stream visualization
	


	// === DIFF PANEL (right side - Claude Code style) ===
	TArray<FDiffEntry> CurrentDiffs;           // All diffs from current/selected message
	int32 SelectedDiffIndex = -1;              // Which file is selected (-1 = none)
	FString SelectedToolUseId;                 // Which tool call's diff we're showing
	bool bDiffPanelSideBySide = false;         // Toggle unified vs side-by-side view

	// Diff panel methods
	void ShowDiffPanel(const TArray<FDiffEntry>& Diffs, const FString& ToolUseId);
	void HideDiffPanel();
	void SelectDiffFile(int32 Index);
	TSharedRef<SWidget> BuildDiffFileList();
	TSharedRef<SWidget> BuildDiffViewer();
	TSharedRef<SWidget> BuildDiffLine(const FString& Line, bool bAdded, bool bRemoved, int32 LineNumber);
	FReply OnAcceptDiff();
	FReply OnRejectDiff();
	FReply OnEditInEditor();
	FReply OnToggleDiffView();

	// Send/Stop buttons
	TSharedPtr<SButton> SendButton;
	TSharedPtr<SButton> StopButton;
	bool bSendButtonHovered = false;
	bool bStopButtonHovered = false;
	bool bCreatePRHovered = false;
	bool bNewThreadButtonHovered = false;
	bool bSettingsButtonHovered = false;
	bool bAutonomyToggleHovered = false;

	// Model selector
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ModelComboBox;
	TArray<TSharedPtr<FString>> AvailableModels;
	FString CurrentModelId;
	
	/** Populate AvailableModels from configured providers (API keys, env vars, Ollama) */
	void PopulateAvailableModels();
	
	/** Get the initially selected item matching CurrentModelId */
	TSharedPtr<FString> GetInitialModelItem() const;

	// === State ===
	TArray<FRiftbornChatMessage> Messages;
	TArray<FRiftbornThinkingStep> CurrentThinkingSteps;
	TArray<FExecutionEvent> CurrentExecutionEvents;
	FString CurrentDraftText;
	
	// === WOW FEATURE STATE (2026-02-08) ===
	// Streaming bubble state
	bool bIsStreaming = false;                          // Currently streaming tokens
	FString StreamingAccumulator;                       // Accumulated streamed text
	int32 StreamingDisplayedChars = 0;                  // Visible character count for smooth reveal
	FString StreamingRenderedText;                      // Last rendered widget text (content + caret)
	double StreamingRevealCarryChars = 0.0;            // Fractional reveal budget across ticks
	double StreamingLastRevealTime = 0.0;              // Last reveal timestamp
	TSharedPtr<SMultiLineEditableTextBox> StreamingTextWidget;  // Live text widget
	TSharedPtr<SBorder> StreamingBubbleWidget;          // The bubble border for streaming
	
	// Auto-execute preference
	bool bAutoExecuteSafe = true;                       // Auto-execute Safe+Reversible plans

	// Read-only mode — only read/inspect tools are allowed, all mutations blocked.
	// Toggle via the footer or /readonly command.
	bool bReadOnlyMode = false;
	bool bReadOnlyToggleHovered = false;
	
	// Suggestion chips widget
	TSharedPtr<SHorizontalBox> SuggestionChipsContainer;
	
	// Redo stack for undo/redo
	// UndoneTokens declared in public section
	
	// === Execution State (canonical truth) ===
	FProposedPlan CurrentPlan;         // The active plan (pending, executing, or completed)
	// State is now inside CurrentPlan.State - no separate booleans!
	
	// === Step Row Widgets (for incremental updates) ===
	// Maps step index to its row widget for targeted refresh
	TMap<int32, TSharedPtr<SBorder>> StepRowWidgets;
	
	bool bIsProcessing = false;
	bool bLastProcessingSucceeded = true;
	int32 ProcessingThreadIndex = INDEX_NONE;
	bool bIsStopped = false;
	bool bShowWelcome = true;
	bool bShowLogPanel = false;        // Inspector drawer visibility
	bool bShowStepsSection = true;     // Toggle for Steps section expand/collapse
	TSharedPtr<SEditableTextBox> InlineApiKeyInput;  // Inline API key entry in welcome screen
	bool bShowTimelineSection = false; // Toggle for Agent Timeline section expand/collapse
	double ExecutionStartTime = 0.0;   // When execution began - for stuck state detection
	static constexpr double ExecutionTimeoutSeconds = 60.0;  // Max time before forcing failure
	double LLMRequestStartTime = 0.0;  // When LLM call started - for hang detection
	static constexpr double LLMTimeoutSeconds = 45.0;  // Max wait for LLM response
	float SuggestionAccumulatedTime = 0.0f;  // Timer for periodic suggestion refresh (was static local — bug)
	float HealthAccumulatedTime = 0.0f;
	float SceneContextRefreshTime = 0.0f;   // Timer for pre-computing scene context in Tick (avoids blocking SendMessage)

	// === CLAUDE CODE STYLE FOOTER (2026-04-16) ===
	double SessionStartTime = 0.0;            // When the active session started — kept for backward refs
	double ActiveSessionSeconds = 0.0;        // Accumulated time while bIsProcessing=true (CC behavior)
	float GitRefreshAccumulatedTime = 0.0f;   // Ticker for periodic git diff refresh
	FCopilotGitInfo CachedGitInfo;            // Mirror of FCopilotGitInfoProvider cache for lambda access
	bool bTokenIndicatorCompact = true;
	FReply OnCreatePRClicked();
	FReply OnAutonomyToggleClicked();         // Cycle bAutoExecuteSafe / CurrentExecutionMode
	mutable int32 CachedActorCount = -1;    // Cached actor count from GatherSceneContext for suggestions (avoids re-iterating)
	FString BridgeStatusText = TEXT("Bridge: Unknown");
	FSlateColor BridgeStatusColor = FSlateColor(FLinearColor::White);
	FString BridgeStatsText = TEXT("");
	FString BridgeTooltipText = TEXT("");
	FString ProviderStatusText = TEXT("Providers: Unknown");
	FSlateColor ProviderStatusColor = FSlateColor(FLinearColor::White);
	TWeakObjectPtr<class URiftbornBridgeMonitor> BridgeMonitor;

	// Iteration progress tracking
	int32 CurrentIteration = 0;
	int32 MaxIterations = 30;
	bool bShowBridgeWarning = false;  // Show warning banner when bridge is offline
	bool bIsRateLimited = false;
	FString RateLimitMessage;

	// Token tracking — these are the LIVE counters for the currently
	// active thread. Persisted snapshots for inactive threads live on
	// FChatThread::Total*Tokens; SaveCurrentThreadState / RestoreThreadState
	// shuttle values across the panel↔thread boundary on tab switch.
	// Do not read FChatThread directly outside the save/restore helpers.
	int32 TotalInputTokens = 0;
	int32 TotalOutputTokens = 0;
	float SessionCost = 0.0f;

	// Execution mode (Governance) - uses external enum
	EExecutionMode CurrentExecutionMode = EExecutionMode::Contracted;

	// === TURN-BASED CONVERSATION STATE ===
	TOptional<FProposedPlan> PendingProposal;  // Plan awaiting user confirmation
	int32 PendingProposalMessageIndex = -1;    // Which message contains the proposal
	int32 ConsecutiveClarificationCount = 0;   // Track clarification loops
	
	// === LLM-OFFERED ACTION TRACKING (2026-02-19 fix) ===
	// When the LLM chatbot response contains numbered action offers
	// (e.g., "Would you like me to: 1. Create X  2. Build Y  3. Design Z"),
	// we track that so the user's next response ("Do 1-3", "Yes", "all of them")
	// is routed to Proposal instead of back to Conversation.
	bool bLastAssistantOfferedAction = false;       // LLM's last response offered numbered actions
	TArray<FString> LastOfferedActions;              // The action descriptions from numbered items
	FString LastOfferedActionContext;                // The full LLM response that contained the offer
	
	// === ESCALATION STATE ===
	TOptional<FEscalationCard> ActiveEscalation;  // Currently displayed escalation card
	TArray<FEscalationCard> EscalationHistory;    // All escalation decisions for proof bundle
	static constexpr int32 MaxClarificationRetries = 2;  // After this, abort with message
	
	// === EXECUTOR RESUME STATE (2026-01-31) ===
	// Store the executing plan to enable resume after confirmation/escalation
	TOptional<FExecutablePlan> ExecutingPlan;     // Plan currently being executed
	int32 PausedAtStepIndex = -1;                  // Step where execution paused for confirmation
	
	// === AGENTIC LOOP STATE (2026-02-18) ===
	FGuid ActiveAgenticSessionId;                  // Currently running agentic session
	bool bAgenticModeActive = false;               // True while agentic loop is running
	bool bForceProposalMode = false;               // True when user used !plan prefix — skips agentic routing
	
	// === TOOLBOOK STATE ===
	bool bShowToolbook = false;                   // Toggle between Tasks and Toolbook
	FString ToolbookSearchTerm;                   // Current search filter
	FString SelectedToolName;                     // Currently selected tool for detail view
	TSharedPtr<SVerticalBox> ToolbookContainer;   // Toolbook list container
	TSharedPtr<SScrollBox> ToolbookScroll;        // Toolbook scroll

	// Plan steps for tracking - uses external struct
	TArray<FPlanStepUI> PlanSteps;
	bool bTodosCollapsed = true;
	bool bTodosAutoExpand = true;

	// Thinking stream collapse state (Codex-style expandable)
	bool bThinkingCollapsed = false;
	double ThinkingStartTime = 0.0;  // For total elapsed time display

	// AI Provider
	TSharedPtr<IAIProvider> AIProvider;

	// Action History for Undo/Redo
	TSharedPtr<FActionHistory> ActionHistory;

	// Current session ID
	FString CurrentSessionId;



	// Message actions tracking
	TSet<int32> PinnedMessageIndices;

	// Session export/import
	void ExportSessionToJson();
	void ExportSessionToMarkdown();

	// === @ MENTION AUTOCOMPLETE ===
	bool bShowAssetMentionPopup = false;
	FString AssetMentionQuery;
	TArray<TSharedPtr<FAssetMentionResult>> AssetMentionResults;
	int32 AssetMentionSelectedIndex = 0;
	TSharedPtr<SBorder> AssetMentionPopupWidget;
	TSharedPtr<SListView<TSharedPtr<FAssetMentionResult>>> AssetMentionListView;
	static constexpr int32 MaxMentionResults = 8;

	void UpdateAssetMentionSearch(const FString& Query);
	void ShowAssetMentionPopup();
	void HideAssetMentionPopup();
	void InsertAssetMention(TSharedPtr<FAssetMentionResult> Result);
	TSharedRef<ITableRow> OnGenerateAssetMentionRow(TSharedPtr<FAssetMentionResult> Item, const TSharedRef<STableViewBase>& OwnerTable);
	bool HandleMentionKeyDown(const FKeyEvent& InKeyEvent);

	// === SLASH COMMAND POPUP (CC-style command palette when user types `/`) ===
	bool bShowSlashPopup = false;
	FString SlashFilter;
	int32 SlashSelectedIndex = 0;
	TSharedPtr<class SBorder> SlashPopupWidget;
	void UpdateSlashPopup(const FString& InputText);
	void HideSlashPopup();
	void InsertSlashCommand(const FString& Cmd);
	bool HandleSlashKeyDown(const FKeyEvent& InKeyEvent);
	struct FSlashCommand { FString Cmd; FString Hint; };
	TArray<FSlashCommand> GetAvailableSlashCommands() const;

	// === IMAGE ATTACHMENT ===
	TArray<FImageAttachment> PendingImageAttachments;
	TSharedPtr<SHorizontalBox> ImageAttachmentBar;

	bool TryPasteImage();
	void AddImageAttachment(const FString& ImagePath);
	void RemoveImageAttachment(int32 Index);
	void ClearImageAttachments();
	FString GetAttachmentContext() const;

	// === MULTI-THREAD CHAT TABS ===
	TArray<FChatThread> ChatThreads;
	int32 ActiveThreadIndex = 0;
	TSharedPtr<SHorizontalBox> ThreadTabBar;
	bool bDragOverHighlight = false;
	int32 HoveredThreadTabIndex = INDEX_NONE;
	int32 HoveredThreadCloseIndex = INDEX_NONE;

	void CreateNewThread(const FString& Title = TEXT(""));
	void SwitchToThread(int32 Index);
	void CloseThread(int32 Index);
	void SaveThreadState();
	void RestoreThreadState();
	void RefreshThreadTabBar();
	TSharedRef<SWidget> BuildThreadTabBar();

	// Message actions
	void CopyMessageToClipboard(int32 MessageIndex);
	void RetryMessage(int32 MessageIndex);
	void TogglePinMessage(int32 MessageIndex);
	bool IsMessagePinned(int32 MessageIndex) const;
};
