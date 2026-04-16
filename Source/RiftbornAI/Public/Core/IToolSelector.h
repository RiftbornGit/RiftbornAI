// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Tool selection result with confidence metadata
 */
struct FToolSelectionResult
{
	/** Ordered list of selected tool names (most likely first) */
	TArray<FString> SelectedTools;
	
	/** Confidence per tool: ToolName -> P(success | context) */
	TMap<FString, float> ToolConfidence;
	
	/** Tools explicitly excluded (hard negatives) */
	TArray<FString> ExcludedTools;
	
	/** Reason for exclusions */
	TMap<FString, FString> ExclusionReasons;
	
	/** Selection source for debugging */
	FString SelectionSource;  // "brain", "default", "profile"
	
	/** Total public tools available before filtering */
	int32 TotalAvailable = 0;
};

/**
 * Tool step confidence for proposal display
 */
struct FToolStepConfidence
{
	FString ToolName;
	float PSuccess = 0.5f;           // P(success | context)
	float PCatastrophic = 0.0f;      // P(unrecoverable failure)
	float Uncertainty = 1.0f;        // Model uncertainty (0 = certain, 1 = no idea)
	bool bNeedsEscalation = false;   // Should force user confirmation
	FString ConfidenceReason;        // Human-readable explanation
};

/**
 * Failure routing decision after a tool fails
 */
UENUM(BlueprintType)
enum class EFailureRouting : uint8
{
	RetryWithCorrection,  // Same tool, adjusted args
	SwitchTool,           // Different tool for same intent
	ProbeFirst,           // Add diagnostic tool before retry
	AskUser,              // Need more info from user
	Abort                 // Give up with explanation
};

struct FFailureRoutingResult
{
	EFailureRouting Decision = EFailureRouting::Abort;
	
	/** For RetryWithCorrection: suggested arg changes */
	TMap<FString, FString> CorrectedArgs;
	
	/** For SwitchTool: alternative tool to try */
	FString AlternativeTool;
	
	/** For ProbeFirst: diagnostic tool to run */
	FString ProbeToolName;
	
	/** For AskUser: the question to ask */
	FString ClarifyingQuestion;
	
	/** For Abort: reason to show user */
	FString AbortReason;
	
	/** Confidence in this routing decision */
	float RoutingConfidence = 0.0f;
	
	/** Source of decision */
	FString RoutingSource;  // "brain", "heuristic", "default"
};

/**
 * Minimal editor/level context for tool selection
 */
struct FToolSelectionContext
{
	/** Current user message/intent */
	FString UserMessage;
	
	/** Is a level currently loaded? */
	bool bLevelLoaded = false;
	
	/** Are we in PIE? */
	bool bInPIE = false;
	
	/** Number of selected actors */
	int32 SelectedActorCount = 0;
	
	/** Is current selection a blueprint? */
	bool bBlueprintSelected = false;
	
	/** Recent failure tool names (for negative signal) */
	TArray<FString> RecentFailures;
	
	/** Recent success tool names (for positive signal) */
	TArray<FString> RecentSuccesses;
	
	/** Agent profile for base filtering */
	EAgentProfile AgentProfile = EAgentProfile::CodingAgent;
};

/**
 * Abstract interface for tool selection
 * 
 * Implement this to control which tools are sent to the LLM.
 * The default implementation returns all public tools (no filtering).
 * The brain-backed implementation ranks and filters based on learned outcomes.
 */
class RIFTBORNAI_API IToolSelector
{
public:
	virtual ~IToolSelector() = default;
	
	/**
	 * Select and rank tools for a given context
	 * 
	 * @param Context - Current editor state and user intent
	 * @param MaxTools - Maximum number of tools to return (0 = no limit)
	 * @return Selection result with ordered tools and confidence
	 */
	virtual FToolSelectionResult SelectTools(
		const FToolSelectionContext& Context,
		int32 MaxTools = 25
	) = 0;
	
	/**
	 * Score proposed plan steps for confidence display
	 * 
	 * @param Steps - Tool names in execution order
	 * @param Context - Current context
	 * @return Confidence per step
	 */
	virtual TArray<FToolStepConfidence> ScorePlanSteps(
		const TArray<FString>& Steps,
		const FToolSelectionContext& Context
	) = 0;
	
	/**
	 * Get failure routing decision
	 * 
	 * @param FailedTool - Tool that just failed
	 * @param ErrorType - Error classification
	 * @param Context - Current context
	 * @return Routing decision
	 */
	virtual FFailureRoutingResult RouteFailure(
		const FString& FailedTool,
		const FString& ErrorType,
		const FToolSelectionContext& Context
	) = 0;
	
	/**
	 * Record a tool execution outcome for learning
	 * 
	 * @param ToolName - Tool that was executed
	 * @param bSuccess - Whether it succeeded
	 * @param ErrorType - Error classification if failed
	 * @param ExecutionTimeMs - How long it took
	 * @param Context - Context at execution time
	 * @param ProofBundleId - ID of proof bundle (for verified-only learning)
	 * @param bVerifierPassed - Whether verifier passed (for verified-only learning)
	 * @param bEnvHealthOk - Was environment healthy at execution? (Bridge connected, no errors)
	 * @param bTickFresh - Was main thread tick responsive? (Not blocked/frozen)
	 * 
	 * NOTE: Brain learning REQUIRES ALL of:
	 *   - ProofBundleId is provided
	 *   - bVerifierPassed is true
	 *   - bEnvHealthOk is true (environment was healthy)
	 *   - bTickFresh is true (main thread was responsive)
	 * 
	 * This enforces verified-only learning with environment health gating.
	 * Outcomes not meeting all criteria are logged but NOT used for training.
	 */
	virtual void RecordOutcome(
		const FString& ToolName,
		bool bSuccess,
		const FString& ErrorType,
		float ExecutionTimeMs,
		const FToolSelectionContext& Context,
		const FString& ProofBundleId = TEXT(""),
		bool bVerifierPassed = false,
		bool bEnvHealthOk = false,
		bool bTickFresh = false
	) = 0;
	
	/**
	 * Get selector name for diagnostics
	 */
	virtual FString GetSelectorName() const = 0;
};

/**
 * Default tool selector - returns all public tools (no brain)
 * 
 * Use this when brain is not available or for A/B testing.
 */
class RIFTBORNAI_API FDefaultToolSelector : public IToolSelector
{
public:
	virtual FToolSelectionResult SelectTools(
		const FToolSelectionContext& Context,
		int32 MaxTools = 25
	) override;
	
	virtual TArray<FToolStepConfidence> ScorePlanSteps(
		const TArray<FString>& Steps,
		const FToolSelectionContext& Context
	) override;
	
	virtual FFailureRoutingResult RouteFailure(
		const FString& FailedTool,
		const FString& ErrorType,
		const FToolSelectionContext& Context
	) override;
	
	virtual void RecordOutcome(
		const FString& ToolName,
		bool bSuccess,
		const FString& ErrorType,
		float ExecutionTimeMs,
		const FToolSelectionContext& Context,
		const FString& ProofBundleId = TEXT(""),
		bool bVerifierPassed = false,
		bool bEnvHealthOk = false,
		bool bTickFresh = false
	) override;
	
	virtual FString GetSelectorName() const override { return TEXT("Default"); }
};

/**
 * Factory for tool selectors
 */
class RIFTBORNAI_API FToolSelectorFactory
{
public:
	/** Get the configured tool selector (brain-backed if available) */
	static TSharedPtr<IToolSelector> GetSelector();
	
	/** Get default selector (no brain) */
	static TSharedPtr<IToolSelector> GetDefaultSelector();
	
	/** Set custom selector (for testing) */
	static void SetSelector(TSharedPtr<IToolSelector> Selector);
	
private:
	static TSharedPtr<IToolSelector> CurrentSelector;
};
