// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "RiftbornEditorAction.h"
#include "RiftbornAssetSnapshot.h"
#include "RiftbornVerificationPipeline.h"
#include "RiftbornActionManifest.h"
#include "RiftbornActionRunner.generated.h"

class URiftbornSnapshotSubsystem;
class URiftbornVerificationPipeline;
class URiftbornManifestSubsystem;
class URiftbornJournalSubsystem;

/**
 * Subsystem for running editor actions with transaction safety
 * Integrates with GEditor->BeginTransaction for proper undo stack
 */
UCLASS()
class RIFTBORNAI_API URiftbornActionRunner : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Event: action started (manifest created) */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnActionManifest, const FRiftbornActionManifest&);
	FOnActionManifest OnActionStarted;

	/** Event: action finished (manifest finalized) */
	FOnActionManifest OnActionCompleted;

	/**
	 * Run an action with full transaction safety
	 * - Takes snapshots of all assets that will be touched
	 * - Begins UE transaction
	 * - Executes action
	 * - Runs verification
	 * - Commits or rolls back based on verification
	 * 
	 * @param Action - Action to execute
	 * @param Context - Action context
	 * @return Result with success/failure
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Actions")
	FRiftbornActionResult RunAction(URiftbornEditorAction* Action, const FRiftbornActionContext& Context);

	/**
	 * Run action from Python code string
	 * Executes Python via bridge, wraps in transaction
	 * 
	 * @param PythonCode - Python code to execute
	 * @param ActionName - Name for this action (for undo)
	 * @param RequestId - Request ID for tracking
	 * @param AssetPaths - Paths to assets that will be touched
	 * @return Result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Actions")
	FRiftbornActionResult RunPythonAction(
		const FString& PythonCode,
		const FString& ActionName,
		const FString& RequestId,
		const TArray<FString>& AssetPaths
	);

	/**
	 * Undo last Riftborn action (uses UE undo stack)
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Actions")
	bool UndoLastAction();

	/**
	 * Redo last undone action
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Actions")
	bool RedoAction();

	/**
	 * Enable/disable verification pipeline
	 * Default: enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Actions")
	void SetVerificationEnabled(bool bEnabled) { bVerificationEnabled = bEnabled; }

	/**
	 * Set which verification checks to run
	 * Empty array = run all checks
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Actions")
	void SetVerificationChecks(const TArray<ERiftbornVerificationCheck>& Checks) { VerificationChecks = Checks; }

private:
	/** Snapshot subsystem reference */
	UPROPERTY()
	URiftbornSnapshotSubsystem* SnapshotSubsystem;

	/** Verification pipeline */
	UPROPERTY()
	URiftbornVerificationPipeline* VerificationPipeline;

	/** Manifest subsystem */
	UPROPERTY()
	URiftbornManifestSubsystem* ManifestSubsystem;

	/** Journal subsystem (write-ahead log for crash recovery) */
	UPROPERTY()
	URiftbornJournalSubsystem* JournalSubsystem;

	/** Whether verification is enabled */
	bool bVerificationEnabled = true;

	/** Which verification checks to run (empty = all) */
	TArray<ERiftbornVerificationCheck> VerificationChecks;

	/** Current transaction index (-1 if no transaction) */
	int32 CurrentTransactionIndex = -1;

	/** Current action manifest (if action in progress) */
	FRiftbornActionManifest CurrentManifest;

	/** Last action name for undo display */
	FString LastActionName;

	/** Snapshots for current transaction */
	TArray<FRiftbornAssetSnapshot> CurrentSnapshots;

	/**
	 * Take snapshots of all assets
	 */
	bool TakeActionSnapshots(const TArray<UObject*>& Assets, const FString& ActionId);

	/**
	 * Restore all snapshots (rollback)
	 */
	bool RestoreActionSnapshots();

	/**
	 * Commit transaction and clear snapshots
	 */
	void CommitTransaction();

	/**
	 * Rollback transaction and restore snapshots
	 */
	void RollbackTransaction();
};
