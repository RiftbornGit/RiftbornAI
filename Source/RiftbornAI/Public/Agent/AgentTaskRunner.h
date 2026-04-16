// Copyright RiftbornAI. All Rights Reserved.
// AgentTaskRunner.h - Bounded, provable task execution loop

#pragma once

#include "CoreMinimal.h"
#include "Agent/AgentEvent.h"
#include "Agent/AgentEventStream.h"
#include "Async/Future.h"

// Forward declarations
class FRiftbornCopilotController;
class FPlanExecutor;
class IAIProvider;
struct FExecutablePlan;

/**
 * Failure classification - what went wrong
 */
UENUM()
enum class EFailureClass : uint8
{
	None,               // No failure
	
	// Build failures
	CompileError,       // C++ compilation failed
	LinkError,          // Linker error
	UHTError,           // Unreal Header Tool error
	
	// Runtime failures
	PIECrash,           // PIE crashed
	PIEStale,           // PIE tick stale (unresponsive)
	PIETimeout,         // PIE didn't start/respond in time
	
	// Environment failures
	BridgeDisconnected, // Python bridge not available
	TickFrozen,         // Main thread unresponsive
	MemoryExhausted,    // OOM or GPU memory issue
	
	// Gate failures
	PredicateFailed,    // Acceptance predicate not satisfied
	PreconditionFailed, // Task precondition not met
	
	// Tool failures
	ToolError,          // Tool returned error
	ToolBlocked,        // Tool blocked by risk tier
	ToolTimeout,        // Tool execution timeout
	
	// Scope failures
	MaxFilesExceeded,   // Would touch too many files
	RiskEscalation,     // Would need higher risk approval
	
	// Unknown
	Unknown             // Unclassified failure
};

/**
 * Failure signature - fingerprint for deduplication
 */
struct RIFTBORNAI_API FFailureSignature
{
	EFailureClass Class = EFailureClass::None;
	FString ErrorHash;      // Hash of error message (for dedup)
	FString Location;       // File:line or tool name
	FString Category;       // Subcategory (e.g., "missing include")
	
	bool operator==(const FFailureSignature& Other) const
	{
		return Class == Other.Class && ErrorHash == Other.ErrorHash;
	}
	
	FString ToString() const;
	static FFailureSignature FromCompileError(const FString& ErrorLog);
	static FFailureSignature FromToolError(const FString& ToolName, const FString& Error);
	static FFailureSignature FromPredicateFailure(const FString& PredicateName);
};

/**
 * Repair strategy - how to fix a failure class
 */
struct RIFTBORNAI_API FRepairStrategy
{
	EFailureClass ForClass = EFailureClass::None;
	FString StrategyName;
	FString Prompt;         // Prompt fragment to inject
	TArray<FString> PreferredTools;
	bool bRequiresContext = true;  // Need to read error context
	int32 Priority = 0;     // Higher = try first
};

/**
 * Task execution state
 */
UENUM()
enum class ETaskRunnerState : uint8
{
	Idle,               // Not running
	Setup,              // Preparing workspace
	Planning,           // Generating plan
	Executing,          // Running plan steps
	Building,           // Waiting for build
	Testing,            // Running PIE/validation
	Probing,            // Checking predicates
	Repairing,          // Attempting repair
	Stopped             // Execution complete
};

/**
 * FAgentTaskRunner - Bounded, provable task execution
 * 
 * Design principles:
 * - BOUNDED: max iterations, max time, max build failures
 * - PROVABLE: all actions emit events with evidence
 * - REPAIRABLE: classified failures get targeted repair
 * - AUDITABLE: full event log enables replay
 * 
 * This is NOT a chat agent. This is a task executor.
 */
class RIFTBORNAI_API FAgentTaskRunner : public TSharedFromThis<FAgentTaskRunner>
{
public:
	FAgentTaskRunner();
	~FAgentTaskRunner();
	
	// === Task Lifecycle ===
	
	/** Start a task (async) */
	TFuture<FAgentTaskResult> StartTask(const FAgentTaskSpec& Spec);
	
	/** Request cancellation */
	void RequestCancel();
	
	/** Check if running */
	bool IsRunning() const { return State != ETaskRunnerState::Idle && State != ETaskRunnerState::Stopped; }
	
	/** Get current state */
	ETaskRunnerState GetState() const { return State; }
	
	/** Get event stream (for UI binding) */
	TSharedPtr<FAgentEventStream> GetEventStream() const { return EventStream; }
	
	/** Ensure event stream exists (creates one if needed for LLM-direct path) */
	TSharedPtr<FAgentEventStream> EnsureEventStream();
	
	/** Emit a tool execution event (for LLM-direct execution path) */
	void EmitToolEvent(const FString& ToolName, bool bSuccess, const FString& ResultSummary, double ExecutionTimeMs);
	
	/** Emit a planning event (plan generation started/completed) */
	void EmitPlanEvent(bool bStart, const FString& Intent, int32 StepCount = 0);
	
	/** Emit an LLM request event (thinking started/completed) */
	void EmitThinkingEvent(bool bStart, const FString& Context = TEXT(""));
	
	/** Get current task spec */
	const FAgentTaskSpec& GetCurrentSpec() const { return CurrentSpec; }
	
	/** Set AI provider for plan generation (P1 Integration) */
	void SetProvider(TSharedPtr<IAIProvider> InProvider) { Provider = InProvider; }
	
	/** Get AI provider */
	TSharedPtr<IAIProvider> GetProvider() const { return Provider; }
	
	// === Delegates ===
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnStateChanged, ETaskRunnerState);
	FOnStateChanged OnStateChanged;
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnTaskComplete, const FAgentTaskResult&);
	FOnTaskComplete OnTaskComplete;
	
private:
	// === State ===
	ETaskRunnerState State = ETaskRunnerState::Idle;
	FAgentTaskSpec CurrentSpec;
	TSharedPtr<FAgentEventStream> EventStream;
	
	// === Execution Tracking ===
	int32 CurrentIteration = 0;
	int32 BuildAttempts = 0;
	FDateTime TaskStartTime;
	TArray<FFailureSignature> FailureHistory;
	bool bCancelRequested = false;
	
	// === References ===
	TWeakPtr<FRiftbornCopilotController> WeakController;
	TWeakPtr<FPlanExecutor> WeakExecutor;
	TSharedPtr<IAIProvider> Provider;  // P1 Integration: AI provider for plan generation
	
	// === Core Loop ===
	
	/** Main execution loop (runs on game thread tick) */
	void ExecutionTick();
	
	/** Setup phase */
	bool DoSetup();
	
	/** Generate plan from goal */
	TSharedPtr<FExecutablePlan> GeneratePlan();
	
	/** Parse LLM response into executable plan (P1 Integration) */
	TSharedPtr<FExecutablePlan> ParsePlanFromResponse(const FString& Response);
	
	/** Execute current plan */
	bool ExecutePlan(TSharedPtr<FExecutablePlan> Plan);
	
	/** Run build gate */
	bool RunBuildGate();
	
	/** Run PIE validation (if required) */
	bool RunPIEValidation();
	
	/** Check acceptance predicates */
	bool CheckAcceptancePredicates(TArray<FPredicateResult>& OutResults);
	
	/** Classify failure from logs/errors */
	FFailureSignature ClassifyFailure();
	
	/** Check if failure is repeated */
	bool IsRepeatedFailure(const FFailureSignature& Sig) const;
	
	/** Get repair strategy for failure */
	FRepairStrategy GetRepairStrategy(const FFailureSignature& Sig) const;
	
	/** Generate repair plan */
	TSharedPtr<FExecutablePlan> GenerateRepairPlan(const FFailureSignature& Failure, const FRepairStrategy& Strategy);
	
	// === Budget Checks ===
	
	bool IsOverTimeBudget() const;
	bool IsOverIterationBudget() const;
	bool IsOverBuildBudget() const;
	
	// === State Management ===
	
	void SetState(ETaskRunnerState NewState);
	void EmitStopEvent(EAgentStopReason Reason, const FString& Diagnosis);
	FAgentTaskResult BuildResult(bool bSuccess, EAgentStopReason StopReason, const FString& Diagnosis);
	
	/** Write proof bundle with execution evidence */
	void WriteProofBundle(const FAgentTaskResult& Result);
	
	// === Helpers ===
	
	FString GetArtifactsPath() const;
	FString GetDiffPath() const;
	FString ComputeEnvironmentHash() const;
	
	// === Repair Strategy Registry ===
	
	static TArray<FRepairStrategy> RepairStrategies;
	static void InitRepairStrategies();
};

/**
 * Global repair strategies - initialized once
 */
#define REGISTER_REPAIR_STRATEGY(Class, Name, Prompt, Tools) \
	{ EFailureClass::Class, TEXT(Name), TEXT(Prompt), Tools, true, 0 }
