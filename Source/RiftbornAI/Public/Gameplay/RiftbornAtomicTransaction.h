// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RiftbornAssetSnapshot.h"
#include "RiftbornAtomicTransaction.generated.h"

class URiftbornSnapshotSubsystem;

/**
 * State of an atomic transaction
 */
UENUM(BlueprintType)
enum class ERiftbornTransactionState : uint8
{
	NotStarted UMETA(DisplayName = "Not Started"),
	InProgress UMETA(DisplayName = "In Progress"),
	Committed UMETA(DisplayName = "Committed"),
	RolledBack UMETA(DisplayName = "Rolled Back"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * Atomic transaction for multiple asset modifications
 * Guarantees all-or-nothing semantics across multiple assets
 */
UCLASS()
class RIFTBORNAI_API URiftbornAtomicTransaction : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Begin atomic transaction
	 * @param ActionId - ID for this transaction
	 * @param TransactionName - Human-readable name
	 * @param Assets - All assets that will be modified
	 * @return Success/failure
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Transaction")
	bool BeginTransaction(const FString& ActionId, const FString& TransactionName, const TArray<UObject*>& Assets);

	/**
	 * Mark an asset as being modified
	 * Must be called BEFORE modifying the asset
	 * @param Asset - Asset about to be modified
	 * @return Success/failure
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Transaction")
	bool TouchAsset(UObject* Asset);

	/**
	 * Commit all changes
	 * Makes all modifications permanent
	 * @return Success/failure
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Transaction")
	bool CommitTransaction();

	/**
	 * Rollback all changes
	 * Restores all assets to pre-transaction state
	 * @return Success/failure
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Transaction")
	bool RollbackTransaction();

	/**
	 * Get current transaction state
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Transaction")
	ERiftbornTransactionState GetState() const { return State; }

	/**
	 * Get number of assets in transaction
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Transaction")
	int32 GetAssetCount() const { return TouchedAssets.Num(); }

	/**
	 * Get number of snapshots taken
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Transaction")
	int32 GetSnapshotCount() const { return Snapshots.Num(); }

	/**
	 * Check if transaction is active
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Transaction")
	bool IsActive() const { return State == ERiftbornTransactionState::InProgress; }

	/**
	 * Get error messages
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Transaction")
	TArray<FString> GetErrors() const { return Errors; }

private:
	/** Current state */
	UPROPERTY()
	ERiftbornTransactionState State = ERiftbornTransactionState::NotStarted;

	/** Action ID */
	UPROPERTY()
	FString ActionId;

	/** Transaction name */
	UPROPERTY()
	FString TransactionName;

	/** All assets in this transaction */
	UPROPERTY()
	TArray<UObject*> TouchedAssets;

	/** Snapshots taken for rollback */
	UPROPERTY()
	TArray<FRiftbornAssetSnapshot> Snapshots;

	/** UE transaction index */
	UPROPERTY()
	int32 UnrealTransactionIndex = -1;

	/** Error messages */
	UPROPERTY()
	TArray<FString> Errors;

	/** Snapshot subsystem reference */
	UPROPERTY()
	URiftbornSnapshotSubsystem* SnapshotSubsystem;

	/**
	 * Take snapshots of all assets
	 */
	bool TakeSnapshots();

	/**
	 * Restore all snapshots
	 */
	bool RestoreSnapshots();

	/**
	 * Verify asset is in transaction
	 */
	bool VerifyAssetInTransaction(UObject* Asset) const;
};

/**
 * Subsystem for managing atomic transactions
 * Ensures only one atomic transaction can be active at a time
 */
UCLASS()
class RIFTBORNAI_API URiftbornTransactionSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Create new atomic transaction
	 * @param ActionId - ID for this transaction
	 * @param TransactionName - Human-readable name
	 * @param Assets - All assets that will be modified
	 * @return Transaction object (null if another transaction is active)
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Transaction")
	URiftbornAtomicTransaction* BeginAtomicTransaction(
		const FString& ActionId,
		const FString& TransactionName,
		const TArray<UObject*>& Assets
	);

	/**
	 * Get currently active transaction (if any)
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Transaction")
	URiftbornAtomicTransaction* GetActiveTransaction() const { return ActiveTransaction; }

	/**
	 * Check if a transaction is currently active
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Transaction")
	bool HasActiveTransaction() const { return ActiveTransaction != nullptr && ActiveTransaction->IsActive(); }

	/**
	 * Force abort active transaction (emergency only)
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Transaction")
	bool AbortActiveTransaction();

private:
	/** Currently active transaction */
	UPROPERTY()
	URiftbornAtomicTransaction* ActiveTransaction;

	/**
	 * Transaction completed (committed or rolled back)
	 */
	void OnTransactionCompleted(URiftbornAtomicTransaction* Transaction);

	friend class URiftbornAtomicTransaction;
};
