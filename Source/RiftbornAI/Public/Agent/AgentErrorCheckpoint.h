// Copyright RiftbornAI. All Rights Reserved.
// AgentErrorCheckpoint.h - Persistent error tracking and crash recovery for agent sessions
//
// PURPOSE: If the editor crashes mid-agent execution, the checkpoint system allows
// recovery on next startup:
//   1. Detects incomplete sessions
//   2. Reports what was in progress
//   3. Optionally auto-rolls back partial operations via AgentTransactionManager
//
// ARCHITECTURE:
//   - Checkpoints written to disk BEFORE each tool execution
//   - Updated AFTER each tool completes (success or failure)
//   - On startup, any "InProgress" checkpoint = unclean shutdown
//   - Works with JournalSubsystem for asset-level WAL, but tracks higher-level agent state

#pragma once

#include "CoreMinimal.h"

RIFTBORNAI_API DECLARE_LOG_CATEGORY_EXTERN(LogAgentCheckpoint, Log, All);

/**
 * State of a single agent error checkpoint
 */
enum class ECheckpointState : uint8
{
    Pending,        // About to execute a tool
    InProgress,     // Tool is executing
    Succeeded,      // Tool succeeded
    Failed,         // Tool failed (error captured)
    Crashed,        // Detected on recovery — was InProgress when editor crashed
    RolledBack      // Recovered and rolled back
};

/**
 * Single checkpoint entry — one per tool execution
 */
struct RIFTBORNAI_API FAgentCheckpointEntry
{
    /** Unique ID for this checkpoint */
    FGuid CheckpointId;
    
    /** Session this belongs to */
    FGuid SessionId;
    
    /** Transaction ID (for rollback) */
    FGuid TransactionId;
    
    /** Tool being executed */
    FString ToolName;
    
    /** Tool arguments (JSON string) */
    FString ArgumentsJson;
    
    /** State */
    ECheckpointState State = ECheckpointState::Pending;
    
    /** Error message (if failed) */
    FString ErrorMessage;
    
    /** Tool result (truncated, for diagnostics) */
    FString ResultSummary;
    
    /** Iteration number within the agentic loop */
    int32 Iteration = 0;
    
    /** Timestamps */
    FDateTime CreatedAt;
    FDateTime UpdatedAt;
    
    /** User's original prompt (stored for context on recovery) */
    FString OriginalPrompt;
    
    FAgentCheckpointEntry()
        : CheckpointId(FGuid::NewGuid())
        , CreatedAt(FDateTime::UtcNow())
        , UpdatedAt(FDateTime::UtcNow())
    {}
};

/**
 * Recovery result from startup scan
 */
struct RIFTBORNAI_API FCheckpointRecoveryResult
{
    /** Number of incomplete sessions found */
    int32 IncompleteSessions = 0;
    
    /** Number of checkpoints in "InProgress" state (crashed) */
    int32 CrashedCheckpoints = 0;
    
    /** Number of checkpoints successfully rolled back */
    int32 RolledBackCount = 0;
    
    /** Session IDs that were incomplete */
    TArray<FGuid> IncompleteSessionIds;
    
    /** Error messages from recovery attempts */
    TArray<FString> RecoveryErrors;
    
    /** Human-readable summary */
    FString GetSummary() const
    {
        if (IncompleteSessions == 0)
        {
            return TEXT("No incomplete sessions found — clean startup.");
        }
        return FString::Printf(
            TEXT("Found %d incomplete sessions (%d crashed checkpoints). Rolled back %d. Errors: %d"),
            IncompleteSessions, CrashedCheckpoints, RolledBackCount, RecoveryErrors.Num());
    }
};

/**
 * FAgentErrorCheckpointManager
 * 
 * Persistent error tracking for agent sessions. Survives editor crashes.
 * 
 * Usage:
 *   // Before tool execution
 *   FGuid CpId = CheckpointMgr.CreateCheckpoint(SessionId, TxId, "spawn_actor", ArgsJson, Iteration, Prompt);
 *   CheckpointMgr.MarkInProgress(CpId);
 *   
 *   // After tool execution
 *   CheckpointMgr.MarkSucceeded(CpId, ResultSummary);
 *   // or
 *   CheckpointMgr.MarkFailed(CpId, ErrorMessage);
 *   
 *   // On editor startup
 *   FCheckpointRecoveryResult Result = CheckpointMgr.RecoverFromCrash();
 */
class RIFTBORNAI_API FAgentErrorCheckpointManager
{
public:
    static FAgentErrorCheckpointManager& Get();
    
    // =========================================================================
    // CHECKPOINT LIFECYCLE
    // =========================================================================
    
    /** Create a new checkpoint BEFORE tool execution. Writes to disk immediately. */
    FGuid CreateCheckpoint(
        const FGuid& SessionId,
        const FGuid& TransactionId,
        const FString& ToolName,
        const FString& ArgumentsJson,
        int32 Iteration,
        const FString& OriginalPrompt);
    
    /** Mark checkpoint as in-progress (tool is now executing). Writes to disk. */
    void MarkInProgress(const FGuid& CheckpointId);
    
    /** Mark checkpoint as succeeded. Writes to disk. */
    void MarkSucceeded(const FGuid& CheckpointId, const FString& ResultSummary = TEXT(""));
    
    /** Mark checkpoint as failed. Writes to disk. */
    void MarkFailed(const FGuid& CheckpointId, const FString& ErrorMessage);
    
    /** Mark an entire session as complete (cleans up active checkpoints). */
    void CompleteSession(const FGuid& SessionId);
    
    // =========================================================================
    // CRASH RECOVERY
    // =========================================================================
    
    /** Scan for incomplete sessions from a previous crash. Call on editor startup. */
    FCheckpointRecoveryResult RecoverFromCrash();
    
    // =========================================================================
    // QUERY
    // =========================================================================
    
    /** Get all checkpoints for a session */
    TArray<FAgentCheckpointEntry> GetSessionCheckpoints(const FGuid& SessionId) const;
    
    /** Get all active (non-complete) sessions */
    TArray<FGuid> GetActiveSessionIds() const;
    
    /** Get accumulated errors across all sessions (for diagnostics) */
    TArray<FAgentCheckpointEntry> GetRecentErrors(int32 MaxCount = 50) const;
    
    /** Get the persistence directory path */
    static FString GetCheckpointDir();
    
private:
    FAgentErrorCheckpointManager() = default;
    
    /** Active checkpoints in memory (flushed to disk on every state change) */
    TMap<FGuid, FAgentCheckpointEntry> ActiveCheckpoints;
    
    /** Error history (kept in memory for quick access) */
    TArray<FAgentCheckpointEntry> ErrorHistory;
    
    /** Write a single checkpoint to disk */
    void PersistCheckpoint(const FAgentCheckpointEntry& Entry);
    
    /** Read a checkpoint from disk */
    bool LoadCheckpoint(const FString& FilePath, FAgentCheckpointEntry& OutEntry);
    
    /** Move checkpoint file to completed directory */
    void ArchiveCheckpoint(const FGuid& CheckpointId);
    
    /** Critical section for thread safety */
    FCriticalSection CheckpointLock;
};
