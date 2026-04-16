// LatentJobExecutor.h
// Tick-driven latent job executor for async verification operations
// This system enables non-blocking PIE and predicate waiting for proof bundles

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"
#include "PlaytestRunner.h"
#include <atomic>  // For thread-safe state machine

// Forward declarations
class FDebugBridgeServer;

/**
 * ELatentJobState - State machine for latent jobs
 */
enum class ELatentJobState : uint8
{
    Pending,      // Job created, waiting to start
    Running,      // Job is actively polling
    Succeeded,    // Predicate passed, job complete
    Failed,       // Timeout or error
    Canceled      // Manually canceled
};

inline FString LatentJobStateToString(ELatentJobState State)
{
    switch (State)
    {
        case ELatentJobState::Pending:   return TEXT("pending");
        case ELatentJobState::Running:   return TEXT("running");
        case ELatentJobState::Succeeded: return TEXT("succeeded");
        case ELatentJobState::Failed:    return TEXT("failed");
        case ELatentJobState::Canceled:  return TEXT("canceled");
        default:                         return TEXT("unknown");
    }
}

/**
 * FLatentJobRecord - State for a single latent job
 */
struct RIFTBORNAI_API FLatentJobRecord
{
    // Identity
    FString JobId;
    FString Kind;           // "wait_predicate" | "pie_session" | "quick_playtest" | "record_replay" | "custom"
    
    // State
    ELatentJobState State = ELatentJobState::Pending;
    double StartTimeUtc = 0.0;
    double EndTimeUtc = 0.0;
    double TimeoutSeconds = 30.0;
    
    // Predicate polling
    FString PredicateName;          // Tool name that returns bool (e.g., "is_pie_running")
    TMap<FString, FString> PredicateArgs;
    bool bExpectedValue = true;     // What the predicate should return
    int32 PollCount = 0;
    int32 SuccessCount = 0;         // Consecutive successes needed for stability
    int32 RequiredStablePolls = 3;  // Must pass N times consecutively
    double PollIntervalSeconds = 0.1; // How often to poll
    double LastPollTime = 0.0;
    
    // PIE session specific
    double PieRunDurationSeconds = 0.0; // How long to keep PIE running
    double PieStartedTime = 0.0;        // When PIE actually started
    bool bPieStarted = false;
    bool bPieStopRequested = false;

    // Quick playtest specific
    double QuickPlaytestDurationSeconds = 0.0;
    bool bQuickPlaytestRequested = false;
    bool bQuickPlaytestCompleted = false;
    bool bQuickPlaytestPassed = false;
    FQuickPlaytestResult QuickPlaytestResult;

    // Replay recording specific
    double ReplayCaptureIntervalSeconds = 0.1;
    double ReplayLastCaptureTime = 0.0;
    TArray<TSharedPtr<FJsonObject>> ReplayKeyframes;
    
    // PIE session witness data (structured timing for proof bundles)
    double PieStartedAfterMs = 0.0;     // Time from job start to PIE running
    double PieStoppedAfterMs = 0.0;     // Time from stop request to PIE stopped
    int32 PieStablePolls = 0;           // Polls needed to confirm PIE stable
    FString PieMode;                    // Mode used (viewport, new_window, etc.)
    
    // =========================================================================
    // Probe Configuration (Priority 8E)
    // Probes sample predicates during PIE session for verification evidence
    // =========================================================================
    struct FProbeConfig
    {
        FString Predicate;          // Predicate name (e.g., "arena_state_snapshot")
        int32 IntervalMs = 500;     // Polling interval in milliseconds
        double LastPollTime = 0.0;  // Last time this probe was polled
    };
    TArray<FProbeConfig> Probes;
    
    // Probe samples (for PIE session with probes)
    TArray<TSharedPtr<FJsonObject>> ProbeSamples;
    static const int32 MaxProbeSamples = 200;  // Cap to prevent unbounded growth
    bool bProbeSamplesTruncated = false;
    
    // Probe summary (computed on completion)
    bool bPlayerValidSeen = false;
    int32 MaxEnemiesAlive = 0;
    int32 MaxWaveIndex = 0;
    bool bWinSeen = false;
    bool bLoseSeen = false;
    
    // Result
    FString Result;
    FString ErrorMessage;
    
    // Proof integration
    TArray<FString> Evidence;   // Logs/snapshots during execution
    FString PreConditionHash;   // State hash before
    FString PostConditionHash;  // State hash after
    
    // Convert to JSON for API responses
    TSharedPtr<FJsonObject> ToJson() const;
};

/**
 * FLatentJobExecutor - Singleton that manages tick-driven latent jobs
 * 
 * Design:
 * - Jobs are created and return immediately with a job ID
 * - A single ticker callback polls all active jobs
 * - Each job transitions through its state machine independently
 * - Results are polled by clients via HTTP /riftborn/latent/status/{id}
 */
class RIFTBORNAI_API FLatentJobExecutor
{
public:
    static FLatentJobExecutor& Get();
    
    /** Initialize the executor and register ticker */
    void Initialize();
    
    /** Shutdown and cleanup all jobs */
    void Shutdown();
    
    // =========================================================================
    // Job Creation
    // =========================================================================
    
    /**
     * Create a predicate wait job
     * Returns immediately with job ID; polls predicate until it returns expected value
     * 
     * @param PredicateName - Tool that returns {"ok":true, "result":"true"|"false"}
     * @param PredicateArgs - Arguments to pass to predicate tool
     * @param bExpectedValue - Wait until predicate returns this value
     * @param TimeoutSeconds - Maximum time to wait
     * @param RequiredStablePolls - Must pass N consecutive times
     * @param PollInterval - Seconds between polls
     * @return Job ID for status polling
     */
    FString CreateWaitPredicateJob(
        const FString& PredicateName,
        const TMap<FString, FString>& PredicateArgs,
        bool bExpectedValue,
        double TimeoutSeconds = 30.0,
        int32 RequiredStablePolls = 3,
        double PollInterval = 0.1
    );
    
    /**
     * Create a PIE session job
     * Starts PIE, waits for it to stabilize, runs for duration, then stops and verifies
     * 
     * @param DurationSeconds - How long to keep PIE running after it stabilizes
     * @param Mode - PIE mode ("viewport", "new_window", "standalone", "simulate")
     * @param TimeoutSeconds - Maximum time for entire operation
     * @param Probes - Optional array of probe configurations for sampling predicates
     * @return Job ID for status polling
     */
    FString CreatePieSessionJob(
        double DurationSeconds,
        const FString& Mode = TEXT("viewport"),
        double TimeoutSeconds = 60.0,
        const TArray<FLatentJobRecord::FProbeConfig>& Probes = TArray<FLatentJobRecord::FProbeConfig>()
    );

    /**
     * Create a quick playtest job.
     * Starts the async playtest runner on the game thread and completes when the
     * runner callback returns a final report.
     */
    FString CreateQuickPlaytestJob(
        double DurationSeconds,
        double TimeoutSeconds = 120.0
    );

    /**
     * Create a replay-recording job.
     * Starts PIE, samples the player camera at a fixed interval, stops PIE, and
     * returns the recorded keyframes as JSON when complete.
     */
    FString CreateReplayRecordingJob(
        double DurationSeconds,
        double CaptureIntervalSeconds = 0.1,
        double TimeoutSeconds = 120.0,
        const FString& Mode = TEXT("viewport")
    );
    
    // =========================================================================
    // Job Status
    // =========================================================================
    
    /** Get job status by ID */
    bool GetJobStatus(const FString& JobId, FLatentJobRecord& OutRecord) const;
    
    /** Check if job is complete (succeeded, failed, or canceled) */
    bool IsJobComplete(const FString& JobId) const;
    
    /** Cancel a running job */
    bool CancelJob(const FString& JobId);
    
    /** Get all active job IDs */
    TArray<FString> GetActiveJobIds() const;
    
    /** Get summary of all jobs */
    TSharedPtr<FJsonObject> GetStatusSummary() const;
    
private:
    FLatentJobExecutor() = default;
    ~FLatentJobExecutor();
    
    // Non-copyable
    FLatentJobExecutor(const FLatentJobExecutor&) = delete;
    FLatentJobExecutor& operator=(const FLatentJobExecutor&) = delete;
    
    // =========================================================================
    // Tick Processing
    // =========================================================================
    
    /** Main ticker callback - processes all active jobs */
    bool TickJobs(float DeltaTime);
    
    /** Process a single wait_predicate job */
    void TickWaitPredicateJob(FLatentJobRecord& Job, double CurrentTime);
    
    /** Process a single pie_session job */
    void TickPieSessionJob(FLatentJobRecord& Job, double CurrentTime);

    /** Process a single quick_playtest job */
    void TickQuickPlaytestJob(const FString& JobId, double CurrentTime);

    /** Process a single record_replay job */
    void TickReplayRecordingJob(FLatentJobRecord& Job, double CurrentTime);
    
    /** Execute predicate tool and get result */
    bool EvaluatePredicate(const FString& ToolName, const TMap<FString, FString>& Args);
    
    /** Execute predicate and return full result (for probe sampling) */
    FString EvaluatePredicateWithResult(const FString& ToolName, const TMap<FString, FString>& Args, bool& bOutPassed);
    
    /** Poll all configured probes and record samples */
    void PollProbes(FLatentJobRecord& Job, double CurrentTime);
    
    /** Update probe summary from sample */
    void UpdateProbeSummary(FLatentJobRecord& Job, const FString& Predicate, const FString& Result);
    
    /** Helper to start PIE */
    bool RequestStartPie(const FString& Mode);
    
    /** Helper to stop PIE */
    bool RequestStopPie();
    
    /** Check if PIE is currently running */
    bool IsPieRunning() const;
    
    /** 
     * Pre-flight checks before starting PIE 
     * Detects and fixes common blockers like invalid navmesh, compile errors, etc.
     * Returns array of issues fixed (empty if none)
     */
    TArray<FString> RunPreFlightChecks();
    
    /** Generate unique job ID */
    FString GenerateJobId() const;
    
    /** Add evidence entry to job */
    void AddEvidence(FLatentJobRecord& Job, const FString& Evidence);
    
    // =========================================================================
    // State
    // =========================================================================
    
    TMap<FString, FLatentJobRecord> Jobs;
    mutable FCriticalSection JobsLock;
    
    FTSTicker::FDelegateHandle TickerHandle;
    
    // Initialization state machine: Uninitialized -> Initializing -> Initialized
    enum class EInitState : uint8
    {
        Uninitialized = 0,
        Initializing = 1,
        Initialized = 2
    };
    std::atomic<EInitState> InitState{EInitState::Uninitialized};
    
    // PIE session mutex - only one PIE session can run at a time
    // Other requests will fail fast with clear error
    std::atomic<bool> bPieSessionActive{false};
    FString ActivePieSessionJobId;
    
    // Legacy flag for backwards compat (prefer InitState)
    bool bInitialized = false;
    
    // Configuration
    int32 MaxConcurrentJobs = 10;
    int32 MaxTotalJobs = 100;  // Cleanup old completed jobs
};
