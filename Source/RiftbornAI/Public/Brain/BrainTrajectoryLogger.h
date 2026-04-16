// BrainTrajectoryLogger.h
// Real learning infrastructure - logs (state, action, outcome) sequences
// This is NOT theatre. This captures actual trajectories for training.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * State representation at a decision point
 */
struct RIFTBORNAI_API FBrainState
{
    FString TaskDescription;           // What the task is ("Create TDM gamemode")
    FString CurrentPhase;              // spec, codegen, test-fix, complete
    FString DomainOntology;            // TDM, arena, UI, etc.
    TArray<FString> ToolsUsedSoFar;    // History of tools already invoked
    TArray<FString> TestsPassing;      // Which tests currently pass
    TArray<FString> TestsFailing;      // Which tests currently fail
    FString LastError;                 // Most recent error if any
    int32 IterationCount;              // How many attempts so far
    
    /** Serialize to JSON for storage */
    TSharedPtr<FJsonObject> ToJson() const;
    
    /** Load from JSON */
    static FBrainState FromJson(const TSharedPtr<FJsonObject>& Json);
    
    /** Generate a hash for state deduplication */
    FString GetStateHash() const;
};

/**
 * Action taken at a decision point
 */
struct RIFTBORNAI_API FBrainAction
{
    FString ActionType;                // tool_call, pattern_select, template_use, strategy_switch
    FString ActionName;                // Specific action (e.g., "spawn_actor", "use_tdm_template")
    TMap<FString, FString> Parameters; // Action parameters
    FString RawLLMOutput;              // If LLM generated this, store the raw output
    bool bLLMGenerated;                // Was this action from LLM or brain policy?
    
    TSharedPtr<FJsonObject> ToJson() const;
    static FBrainAction FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Outcome of taking an action
 */
struct RIFTBORNAI_API FBrainOutcome
{
    bool bSuccess;                     // Did the action succeed?
    FString ResultSummary;             // Brief description
    TArray<FString> TestsNowPassing;   // Tests that now pass
    TArray<FString> TestsNowFailing;   // Tests that now fail
    int32 TokensUsed;                  // LLM tokens consumed
    int32 ToolCallsUsed;               // Number of tool invocations
    float TimeSeconds;                 // Wall clock time
    FString ErrorMessage;              // If failed, why
    
    TSharedPtr<FJsonObject> ToJson() const;
    static FBrainOutcome FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * A single step in a trajectory
 */
struct RIFTBORNAI_API FTrajectoryStep
{
    FBrainState State;
    FBrainAction Action;
    FBrainOutcome Outcome;
    FDateTime Timestamp;
    
    TSharedPtr<FJsonObject> ToJson() const;
    static FTrajectoryStep FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * A complete trajectory (spec → tests pass)
 */
struct RIFTBORNAI_API FTrajectory
{
    FGuid TrajectoryId;
    FString TaskFamily;                // "TDM", "Arena", "UI", etc.
    FString TaskSpec;                  // Original specification
    TArray<FTrajectoryStep> Steps;
    
    // Episode summary
    bool bEpisodeSuccess;              // Did the whole task succeed?
    float TotalReward;                 // Computed reward
    int32 TotalTokens;
    int32 TotalToolCalls;
    float TotalTimeSeconds;
    
    TSharedPtr<FJsonObject> ToJson() const;
    static FTrajectory FromJson(const TSharedPtr<FJsonObject>& Json);
    
    /** Compute reward based on outcome */
    void ComputeReward();
};

/**
 * Result of writing a trajectory to disk
 * Used by proof system to get exact file path without recomputation
 */
struct RIFTBORNAI_API FTrajectoryWriteResult
{
    /** Whether a trajectory was actually written */
    bool bWrote = false;

    /** Absolute path to the JSONL file written */
    FString FilePath;

    /** The trajectory ID that was written */
    FGuid TrajectoryId;

    /** Task family (for debugging/verification) */
    FString TaskFamily;
};

/**
 * Trajectory Logger - Records actual decision sequences for learning
 *
 * This is the REAL data pipeline. No faking.
 * - Each task execution creates a trajectory
 * - Trajectories are saved to disk as JSONL
 * - Can be used for imitation learning, RL, or analysis
 */
class RIFTBORNAI_API FBrainTrajectoryLogger
{
public:
    static FBrainTrajectoryLogger& Get();

    // ==========================================================================
    // Trajectory Lifecycle
    // ==========================================================================

    /** Start a new trajectory for a task (legacy single-slot API).
     *  Uses an internal sentinel handle; concurrent callers will abort each
     *  other's trajectories. Prefer BeginTrajectoryFor for concurrent agents. */
    void BeginTrajectory(const FString& TaskFamily, const FString& TaskSpec);

    /** Log a step in the current (legacy single-slot) trajectory */
    void LogStep(const FBrainState& State, const FBrainAction& Action, const FBrainOutcome& Outcome);

    /** End the current (legacy single-slot) trajectory and save it. */
    FTrajectoryWriteResult EndTrajectory(bool bSuccess, float FinalReward = 0.0f);

    /** Abort the current (legacy single-slot) trajectory without saving. */
    void AbortTrajectory();

    /** Check if the legacy single-slot is currently recording */
    bool IsRecording() const;

    // ==========================================================================
    // Concurrent-agent API (per-handle trajectories)
    // Multiple agents can run in parallel; each gets its own trajectory keyed
    // by a handle (typically FAgentTask::TaskId). Thread-safe.
    // ==========================================================================

    /** Start a trajectory bound to an explicit handle. Returns the handle for
     *  chaining. If the handle already has an active trajectory, the prior one
     *  is aborted (caller bug, not silent corruption). */
    FGuid BeginTrajectoryFor(const FGuid& Handle, const FString& TaskFamily, const FString& TaskSpec);

    /** Log a step in the trajectory bound to Handle. No-op if none active. */
    void LogStepFor(const FGuid& Handle, const FBrainState& State, const FBrainAction& Action, const FBrainOutcome& Outcome);

    /** End the trajectory bound to Handle and save it. */
    FTrajectoryWriteResult EndTrajectoryFor(const FGuid& Handle, bool bSuccess, float FinalReward = 0.0f);

    /** Abort the trajectory bound to Handle without saving. */
    void AbortTrajectoryFor(const FGuid& Handle);

    /** Is a trajectory currently recording for this handle? */
    bool IsRecordingFor(const FGuid& Handle) const;

    /** Number of trajectories currently recording across all handles. */
    int32 ActiveTrajectoryCount() const;
    
    // ==========================================================================
    // Data Access
    // ==========================================================================
    
    /** Get all trajectories for a task family */
    TArray<FTrajectory> LoadTrajectories(const FString& TaskFamily) const;
    
    /** Get successful trajectories only (for imitation learning) */
    TArray<FTrajectory> LoadSuccessfulTrajectories(const FString& TaskFamily) const;
    
    /** Get trajectory count by family */
    TMap<FString, int32> GetTrajectoryCounts() const;
    
    // ==========================================================================
    // Metrics (real numbers, no faking)
    // ==========================================================================
    
    /** Success rate for a task family */
    float GetSuccessRate(const FString& TaskFamily) const;
    
    /** Average tokens per successful task */
    float GetAverageTokens(const FString& TaskFamily, bool bSuccessOnly = true) const;
    
    /** Average tool calls per successful task */
    float GetAverageToolCalls(const FString& TaskFamily, bool bSuccessOnly = true) const;
    
    /** Get trend over last N trajectories */
    struct FMetricTrend
    {
        float SuccessRate;
        float AvgTokens;
        float AvgToolCalls;
        float AvgTime;
    };
    FMetricTrend GetRecentTrend(const FString& TaskFamily, int32 WindowSize = 10) const;
    
private:
    FBrainTrajectoryLogger();
    
    /** Get storage directory */
    FString GetTrajectoryDir() const;
    
    /** Save trajectory to disk. Returns file path on success, empty on failure. */
    FString SaveTrajectory(const FTrajectory& Trajectory);
    
    // Active trajectories — concurrent-safe storage keyed by handle.
    // The legacy single-slot API routes through a sentinel handle
    // (FGuid() zero-initialised) so old callers keep working.
    TMap<FGuid, FTrajectory> ActiveTrajectories;
    mutable FCriticalSection TrajectoryLock;

    // Sentinel used by the legacy BeginTrajectory/LogStep/EndTrajectory API.
    // Zero-GUID is never returned by FGuid::NewGuid so collision is impossible.
    static FGuid LegacyHandle() { return FGuid(); }

    // Cache of loaded trajectories
    mutable TMap<FString, TArray<FTrajectory>> TrajectoryCache;
    mutable bool bCacheValid = false;

    void InvalidateCache() { bCacheValid = false; }
};
