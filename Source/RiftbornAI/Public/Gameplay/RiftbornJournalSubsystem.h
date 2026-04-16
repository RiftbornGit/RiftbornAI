// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "RiftbornJournalSubsystem.generated.h"

/**
 * Entry in the write-ahead journal log.
 * Records an action before it executes, allowing crash recovery.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornJournalEntry
{
	GENERATED_BODY()

	/** Unique ID for this action (matches ActionManifest) */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	FString ActionId;

	/** Human-readable action name */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	FString ActionName;

	/** Request ID from client (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	FString RequestId;

	/** Timestamp when action started */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	FDateTime StartTime = FDateTime();

	/** Assets this action will modify */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TArray<FString> AffectedAssets;

	/** Current state: Pending, InProgress, Committed, RolledBack */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	FString State;

	/** Timestamp when state last changed */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	FDateTime StateChangeTime = FDateTime();

	FRiftbornJournalEntry()
		: StartTime(FDateTime::UtcNow())
		, State(TEXT("Pending"))
		, StateChangeTime(FDateTime::UtcNow())
	{
	}

	/** Convert to JSON for persistence */
	FString ToJson() const;

	/** Parse from JSON */
	static FRiftbornJournalEntry FromJson(const FString& JsonString);
};

/**
 * Result of a journal recovery operation
 */
USTRUCT(BlueprintType)
struct FRiftbornJournalRecoveryResult
{
	GENERATED_BODY()

	/** Did recovery succeed? */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	bool bSuccess = true;

	/** Number of incomplete transactions found */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	int32 IncompleteCount = 0;

	/** Number of transactions rolled back */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	int32 RolledBackCount = 0;

	/** IDs of recovered actions */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TArray<FString> RecoveredActionIds;

	/** Errors encountered during recovery */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TArray<FString> Errors;
};

/**
 * Transaction journal subsystem - Write-ahead log for crash recovery.
 * 
 * ARCHITECTURE:
 * - Before any action executes, we write a journal entry to disk
 * - Entry includes: ActionId, AffectedAssets, State (Pending)
 * - During action execution, we update state to InProgress
 * - On success/rollback, we update state to Committed/RolledBack
 * - On editor startup, we check for Pending/InProgress entries
 * - Any incomplete entries are automatically rolled back via snapshots
 * 
 * INVARIANTS:
 * 1. Every action is journaled BEFORE execution
 * 2. Journal entries are NEVER deleted, only marked complete
 * 3. Recovery is IDEMPOTENT (safe to run multiple times)
 * 4. Journal is append-only (no in-place edits to prevent corruption)
 * 
 * FILE STRUCTURE:
 * {ProjectSaved}/RiftbornAI/Journal/
 *   active/
 *     {ActionId}_pending.json      - Action about to start
 *     {ActionId}_inprogress.json   - Action currently executing
 *   complete/
 *     {ActionId}_committed.json    - Action succeeded
 *     {ActionId}_rolledback.json   - Action failed and rolled back
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API URiftbornJournalSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Write journal entry BEFORE action executes (write-ahead log).
	 * This is the FIRST step of any transaction-safe action.
	 */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	bool WriteJournalEntry(const FRiftbornJournalEntry& Entry);

	/**
	 * Update journal entry state (e.g., Pending → InProgress → Committed).
	 * State changes are written to NEW files (append-only log).
	 */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	bool UpdateJournalState(const FString& ActionId, const FString& NewState);

	/**
	 * Check for incomplete transactions on startup.
	 * Returns all actions in Pending or InProgress state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	TArray<FRiftbornJournalEntry> GetIncompleteTransactions();

	/**
	 * Recover from incomplete transactions.
	 * Automatically rolls back any Pending/InProgress actions using snapshots.
	 * Called automatically on subsystem Initialize.
	 */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	FRiftbornJournalRecoveryResult RecoverIncompleteTransactions();

	/**
	 * Get journal entry by action ID.
	 * Checks active/ folder first, then complete/ folder.
	 */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	FRiftbornJournalEntry GetJournalEntry(const FString& ActionId, bool& bFound);

	/**
	 * Roll back a specific action by ActionId using its recorded snapshots.
	 * Returns true if all snapshots restored successfully.
	 */
	/**
	 * Roll back a specific action by ActionId using its snapshots.
	 * This is a targeted rollback (without touching other entries).
	 */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	bool RollbackActionById(const FString& ActionId);

	/**
	 * Archive old completed journal entries.
	 * Moves entries older than RetentionDays from complete/ to archive/.
	 */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	int32 ArchiveOldEntries(int32 RetentionDays = 30);

	/**
	 * Get statistics about the journal (for monitoring).
	 */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void GetJournalStats(int32& OutActiveCount, int32& OutCompletedCount, int32& OutArchivedCount);

private:
	/** Get base journal directory: {ProjectSaved}/RiftbornAI/Journal/ */
	FString GetJournalDirectory() const;

	/** Get active journal directory: {Journal}/active/ */
	FString GetActiveDirectory() const;

	/** Get complete journal directory: {Journal}/complete/ */
	FString GetCompleteDirectory() const;

	/** Get archive journal directory: {Journal}/archive/ */
	FString GetArchiveDirectory() const;

	/** Get file path for a journal entry */
	FString GetEntryFilePath(const FString& ActionId, const FString& State, bool bActive) const;

	/** Ensure journal directories exist */
	void EnsureDirectoriesExist();

	/** Write journal entry to file (atomic write with temp + rename) */
	bool WriteEntryToFile(const FRiftbornJournalEntry& Entry, const FString& FilePath);

	/** Read journal entry from file */
	FRiftbornJournalEntry ReadEntryFromFile(const FString& FilePath, bool& bSuccess);

	/** Move entry from active/ to complete/ */
	bool MoveToComplete(const FString& ActionId, const FString& State);

	/** Flag: Has recovery been performed this session? */
	bool bRecoveryPerformed = false;
};
