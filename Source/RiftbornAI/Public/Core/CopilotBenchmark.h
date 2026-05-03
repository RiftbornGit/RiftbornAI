// Copyright RiftbornAI. All Rights Reserved.
// CopilotBenchmark — Evaluation Harness for measuring agent quality.
//
// This is the single biggest gap identified in external audit.
// Without benchmarks, we cannot measure if changes help or hurt.
//
// Benchmark Tasks:
//   - Spawn/Create/Delete actors
//   - Set/Get properties
//   - Blueprint creation + compile
//   - Code generation + build
//   - Multi-step workflows (spawn + position + material)
//   - Error recovery (fix a deliberately broken build)
//
// Scoring:
//   - Success rate (binary: did it achieve the goal?)
//   - Tool efficiency (fewer tool calls = better)
//   - Time (wall-clock seconds)
//   - Verification pass rate (WorldStateDigest claims satisfied?)
//   - No false-positive rate (didn't claim success when it failed?)

#pragma once

#include "CoreMinimal.h"
#include "AgenticLoopRunner.h"
#include "WorldStateDigest.h"

// Forward declarations
class IAIProvider;

/**
 * Difficulty tiers for benchmark tasks
 */
enum class EBenchmarkDifficulty : uint8
{
	Trivial,    // Single tool call, obvious answer
	Easy,       // 1-3 tool calls, clear path
	Medium,     // 3-8 tool calls, some reasoning required
	Hard,       // 8+ tool calls, multi-step workflow
	Adversarial // Deliberately broken input, error recovery
};

/**
 * Category of benchmark task
 */
enum class EBenchmarkCategory : uint8
{
	SceneManipulation,   // Spawn, delete, move, scale, rotate
	PropertyEditing,     // Get/set actor properties
	BlueprintOps,        // Create BP, add variables, compile
	CodeGeneration,      // Create C++ source, build
	MultiStep,           // Compound workflows requiring planning
	ErrorRecovery,       // Fix broken states, handle failures gracefully
	ContextAwareness,    // Tasks that test if the agent reads context correctly
	Verification         // Tasks that test if the agent verifies its own work
};

/**
 * Expected proof requirement for a benchmark workflow.
 * These double as scorecard rows for flagship workflows.
 */
struct RIFTBORNAI_API FBenchmarkProofRequirement
{
	/** Stable requirement ID, e.g. "runtime_proof" */
	FString RequirementId;

	/** Human-readable label for reports and scorecards */
	FString Label;

	/** Any observed proof signal satisfying one of these counts as a pass */
	TArray<FString> AcceptedSignals;

	/** Relative weight for proof completeness scoring */
	float Weight = 1.0f;
};

/**
 * A single benchmark task definition
 */
struct RIFTBORNAI_API FBenchmarkTask
{
	/** Unique task ID */
	FName TaskId;

	/** Human-readable name */
	FString Name;

	/** The prompt to send to the agent */
	FString Prompt;

	/** Difficulty tier */
	EBenchmarkDifficulty Difficulty = EBenchmarkDifficulty::Easy;

	/** Task category */
	EBenchmarkCategory Category = EBenchmarkCategory::SceneManipulation;

	/** State claims that MUST be satisfied after task completes for it to count as a pass */
	TArray<FStateClaim> SuccessClaims;

	/** Maximum allowed tool calls (0 = unlimited) */
	int32 MaxToolCalls = 0;

	/** Maximum allowed time in seconds (0 = use default) */
	float MaxTimeSeconds = 0.0f;

	/** Expected minimum tool count (for detecting shortcuts) */
	int32 MinExpectedTools = 0;

	/** Tags for filtering */
	TArray<FString> Tags;

	/** Required proof lanes / scorecard rows for workflow-grade tasks */
	TArray<FBenchmarkProofRequirement> ProofScorecard;

	/** Optional setup function (runs before task to set world state) */
	TFunction<void()> SetupFunc;

	/** Optional teardown function (runs after task to clean up) */
	TFunction<void()> TeardownFunc;
};

/**
 * Result of running a single benchmark task
 */
struct RIFTBORNAI_API FBenchmarkResult
{
	/** Task that was run */
	FName TaskId;
	FString TaskName;
	EBenchmarkDifficulty Difficulty = EBenchmarkDifficulty::Easy;
	EBenchmarkCategory Category = EBenchmarkCategory::SceneManipulation;

	/** Did it pass? */
	bool bPassed = false;

	/** Why it failed (empty if passed) */
	FString FailureReason;

	/** Tool calls made */
	int32 ToolCallCount = 0;

	/** Tool calls that succeeded */
	int32 ToolSuccessCount = 0;

	/** Wall-clock time */
	float TimeSeconds = 0.0f;

	/** Submit -> first visible feedback (status, streamed token, or tool activity) */
	float TimeToFirstFeedbackSeconds = -1.0f;

	/** Submit -> first tool start */
	float TimeToFirstToolStartSeconds = -1.0f;

	/** Submit -> first proof-bearing verification signal */
	float TimeToFirstProofSeconds = -1.0f;

	/** Count of proof-bearing tool results observed during the run */
	int32 ProofArtifactCount = 0;

	/** Coverage score across the expected proof scorecard (0-1) */
	float ProofCoverage = 1.0f;

	/** Signals observed in the tool trace, e.g. runtime_proof, navmesh_proof */
	TArray<FString> ObservedProofSignals;

	/** Human-readable proof requirements that were not satisfied */
	TArray<FString> MissingProofRequirements;

	/** Human-readable scorecard rows with PASS/MISS state */
	TArray<FString> ProofScorecardLines;

	/** Claim satisfaction score (0-1) */
	float ClaimSatisfaction = 0.0f;

	/** Individual claim results */
	TArray<FClaimResult> ClaimResults;

	/** The agent's final response */
	FString AgentResponse;

	/** Did the agent claim success? */
	bool bAgentClaimedSuccess = false;

	/** False positive: agent claimed success but verification failed */
	bool bFalsePositive = false;

	/** Count of failed tool invocations during the run */
	int32 FailedToolCallCount = 0;

	/** True if the run required retries, recovery, or failed verification turns */
	bool bFlaky = false;

	/** Tool names used (in order) */
	TArray<FString> ToolSequence;

	/** Tool responsible for the terminal failed/blocking outcome, if any */
	FString FailureToolName;

	/** Stable failure code emitted by the tool or benchmark classifier */
	FString FailureCode;

	/** One-pass diagnosis fields captured for benchmark transparency */
	FString DiagnosisWhatFailed;
	FString DiagnosisWhatChanged;
	FString DiagnosisNextStep;
	FString DiagnosisProofArtifactId;

	/** True when benchmark output can answer what failed / what changed / next step directly */
	bool bHasOnePassDiagnosis = false;

	/** True when diagnosis used structured failure/proof metadata rather than freeform fallback only */
	bool bDiagnosisBackedByArtifact = false;

	/** Retry attempts inferred from the tool trace after failure/recovery loops */
	bool bObservedRetry = false;
	int32 RetryAttemptsObserved = 0;

	/** Count of redundant verification steps avoided via in-session fast paths */
	int32 VerificationFastPathCount = 0;

	/** Which redundant verification lanes were skipped, e.g. visual, playtest */
	TArray<FString> VerificationFastPathKinds;

	/** Provider prompt-cache token deltas observed during this run */
	int64 PromptCacheCreationTokens = 0;
	int64 PromptCacheReadTokens = 0;

	/** Share of prompt-cache telemetry served from cache (0-1, -1 when unsupported/unused) */
	float PromptCacheHitRate = -1.0f;

	/** True when provider-level prompt-cache telemetry was observed for this run */
	bool bUsedPromptCache = false;
};

/**
 * Aggregate results for a benchmark suite run
 */
struct RIFTBORNAI_API FBenchmarkSuiteResult
{
	/** When the run started */
	FDateTime StartTime;

	/** When the run ended */
	FDateTime EndTime;

	/** Total time for entire suite */
	float TotalTimeSeconds = 0.0f;

	/** Individual task results */
	TArray<FBenchmarkResult> Results;

	/** Provider used */
	FString ProviderName;

	/** Model used */
	FString ModelName;

	// =========================================================================
	// AGGREGATE METRICS
	// =========================================================================

	/** Overall pass rate (0-1) */
	float GetPassRate() const;

	/** Pass rate by difficulty */
	float GetPassRateByDifficulty(EBenchmarkDifficulty Difficulty) const;

	/** Pass rate by category */
	float GetPassRateByCategory(EBenchmarkCategory Category) const;

	/** Average tool calls per task */
	float GetAverageToolCalls() const;

	/** Average time per task */
	float GetAverageTimeSeconds() const;

	/** Average submit -> first visible feedback */
	float GetAverageFirstFeedbackSeconds() const;

	/** Average first visible feedback -> first tool start */
	float GetAverageFeedbackToFirstToolSeconds() const;

	/** Average submit -> first tool start */
	float GetAverageFirstToolStartSeconds() const;

	/** Average first tool start -> first proof-bearing verification signal */
	float GetAverageToolStartToFirstProofSeconds() const;

	/** Average submit -> first proof-bearing verification signal */
	float GetAverageFirstProofSeconds() const;

	/** Average first proof-bearing signal -> task completion */
	float GetAverageProofToCompletionSeconds() const;

	/** Average proof coverage across workflow scorecards */
	float GetAverageProofCoverage() const;

	/** Average claim satisfaction */
	float GetAverageClaimSatisfaction() const;

	/** False positive rate */
	float GetFalsePositiveRate() const;

	/** Tool success rate across all tasks */
	float GetToolSuccessRate() const;

	/** Share of runs that showed retry/recovery behavior */
	float GetFlakeRate() const;

	/** Share of failed runs that emitted a one-pass diagnosis */
	float GetDiagnosisCoverage() const;

	/** Share of failed runs whose diagnosis was backed by structured artifacts */
	float GetStructuredDiagnosisCoverage() const;

	/** Share of runs that showed retry behavior in the tool trace */
	float GetObservedRetryRate() const;

	/** Share of runs that avoided at least one redundant verification step */
	float GetVerificationFastPathRate() const;

	/** Average prompt-cache creation tokens per task */
	float GetAveragePromptCacheCreationTokens() const;

	/** Average prompt-cache read tokens per task */
	float GetAveragePromptCacheReadTokens() const;

	/** Average prompt-cache hit rate across runs where telemetry was present */
	float GetAveragePromptCacheHitRate() const;

	/** Share of runs that emitted any prompt-cache telemetry */
	float GetPromptCacheUsageRate() const;

	/** Latency stage with the highest average duration across the suite */
	FString GetWorstLatencyBucketLabel() const;

	/** Average duration of the worst latency stage */
	float GetWorstLatencyBucketSeconds() const;

	/** Get count of tasks that passed / total */
	int32 GetPassCount() const;
	int32 GetTotalCount() const { return Results.Num(); }

	// =========================================================================
	// SERIALIZATION
	// =========================================================================

	/** Serialize to JSON for persistence */
	TSharedPtr<FJsonObject> ToJson() const;

	/** Save to disk */
	bool SaveToFile(const FString& FilePath) const;

	/** Load from disk */
	static TOptional<FBenchmarkSuiteResult> LoadFromFile(const FString& FilePath);

	/** Print human-readable summary to log */
	void LogSummary() const;

	/** Get human-readable report string */
	FString ToReportString() const;
};

/**
 * FCopilotBenchmark — The Evaluation Harness
 *
 * Runs a suite of predefined tasks against the agent and measures quality.
 * Results are persisted to Saved/RiftbornAI/benchmarks/ for comparison.
 *
 * Usage:
 *   FCopilotBenchmark& Bench = FCopilotBenchmark::Get();
 *   Bench.RunSuite(Provider, OnComplete);
 *   // or
 *   Bench.RunTask(TaskId, Provider, OnTaskComplete);
 */
class RIFTBORNAI_API FCopilotBenchmark
{
public:
	static FCopilotBenchmark& Get();

	// =========================================================================
	// TASK MANAGEMENT
	// =========================================================================

	/** Get all registered benchmark tasks */
	const TArray<FBenchmarkTask>& GetTasks() const { return Tasks; }

	/** Get tasks by category */
	TArray<FBenchmarkTask> GetTasksByCategory(EBenchmarkCategory Category) const;

	/** Get tasks by difficulty */
	TArray<FBenchmarkTask> GetTasksByDifficulty(EBenchmarkDifficulty Difficulty) const;

	/** Get tasks by tag */
	TArray<FBenchmarkTask> GetTasksByTag(const FString& Tag) const;

	/** Find a task by ID */
	const FBenchmarkTask* FindTask(FName TaskId) const;

	// =========================================================================
	// EXECUTION
	// =========================================================================

	/** Delegate for suite completion */
	DECLARE_DELEGATE_OneParam(FOnSuiteComplete, const FBenchmarkSuiteResult&);

	/** Delegate for single task completion */
	DECLARE_DELEGATE_OneParam(FOnTaskComplete, const FBenchmarkResult&);

	/** Delegate for progress updates */
	DECLARE_DELEGATE_TwoParams(FOnBenchmarkProgress, int32 /* Current */, int32 /* Total */);

	/**
	 * Run the full benchmark suite
	 * @param InProvider - AI provider to test
	 * @param OnComplete - Called when all tasks finish
	 * @param OnProgress - Called after each task
	 */
	void RunSuite(
		TSharedPtr<IAIProvider> InProvider,
		FOnSuiteComplete OnComplete,
		FOnBenchmarkProgress OnProgress = FOnBenchmarkProgress());

	/**
	 * Run a single benchmark task
	 * @param TaskId - Which task to run
	 * @param InProvider - AI provider to test
	 * @param OnComplete - Called when task finishes
	 */
	void RunTask(
		FName TaskId,
		TSharedPtr<IAIProvider> InProvider,
		FOnTaskComplete OnComplete);

	/**
	 * Run tasks matching a filter
	 * @param Category - Only run tasks in this category (or all if not specified)
	 * @param RequiredTag - Only run tasks carrying this tag (or all if empty)
	 * @param MaxDifficulty - Only run tasks at or below this difficulty
	 */
	void RunFiltered(
		TSharedPtr<IAIProvider> InProvider,
		FOnSuiteComplete OnComplete,
		TOptional<EBenchmarkCategory> Category = TOptional<EBenchmarkCategory>(),
		const FString& RequiredTag = FString(),
		EBenchmarkDifficulty MaxDifficulty = EBenchmarkDifficulty::Hard,
		FOnBenchmarkProgress OnProgress = FOnBenchmarkProgress());

	/** Cancel a running benchmark */
	void Cancel();

	/** Is a benchmark currently running? */
	bool IsRunning() const { return bRunning; }

	// =========================================================================
	// HISTORY
	// =========================================================================

	/** Get the directory where benchmark results are stored */
	static FString GetBenchmarkDir();

	/** Load the most recent benchmark result */
	static TOptional<FBenchmarkSuiteResult> LoadMostRecent();

	/** Load all benchmark results (for trend analysis) */
	static TArray<FBenchmarkSuiteResult> LoadAll(int32 MaxResults = 20);

	/** Compare two results (returns human-readable diff) */
	static FString CompareResults(const FBenchmarkSuiteResult& A, const FBenchmarkSuiteResult& B);

private:
	FCopilotBenchmark();

	/** Register all built-in benchmark tasks */
	void RegisterBuiltinTasks();

	/** Register scene manipulation tasks */
	void RegisterSceneTasks();

	/** Register property editing tasks */
	void RegisterPropertyTasks();

	/** Register blueprint operation tasks */
	void RegisterBlueprintTasks();

	/** Register code generation tasks */
	void RegisterCodeGenTasks();

	/** Register multi-step workflow tasks */
	void RegisterMultiStepTasks();

	/** Register error recovery tasks */
	void RegisterErrorRecoveryTasks();

	/** Register context awareness tasks */
	void RegisterContextTasks();

	/** Register code patching tasks (edit_file, apply_unified_diff) */
	void RegisterPatchingTasks();

	/** Register build and compile verification tasks */
	void RegisterBuildTasks();

	/** Register verification-specific tasks (prove the agent actually verifies) */
	void RegisterVerificationTasks();

	/** Internal: run next task in queue */
	void RunNextTask();

	/** Internal: evaluate a completed session against task claims */
	FBenchmarkResult EvaluateSession(
		const FBenchmarkTask& Task,
		const FAgenticSession& Session,
		const FWorldStateDigest& BeforeState,
		const FWorldStateDigest& AfterState,
		float ElapsedSeconds);

	// Task registry
	TArray<FBenchmarkTask> Tasks;

	// Running state
	bool bRunning = false;
	TSharedPtr<IAIProvider> CurrentProvider;
	FBenchmarkSuiteResult CurrentSuiteResult;
	int32 CurrentTaskIndex = 0;
	TArray<int32> TaskQueue;  // Indices into Tasks array
	FOnSuiteComplete SuiteCompleteCallback;
	FOnBenchmarkProgress ProgressCallback;
	FGuid CurrentSessionId;
	FWorldStateDigest PreTaskState;
	FDateTime TaskStartTime;
	float CurrentTaskFirstFeedbackSeconds = -1.0f;
	float CurrentTaskFirstToolStartSeconds = -1.0f;
	float CurrentTaskFirstProofSeconds = -1.0f;
	int64 CurrentTaskPromptCacheCreationBaseline = 0;
	int64 CurrentTaskPromptCacheReadBaseline = 0;
};
