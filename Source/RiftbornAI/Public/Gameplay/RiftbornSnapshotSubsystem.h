// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "RiftbornAssetSnapshot.h"
#include "RiftbornSnapshotSubsystem.generated.h"

class UBlueprint;
class UWidgetBlueprint;
class UWorld;
class UMaterialInterface;
class UDataAsset;
class UDataTable;
class UBehaviorTree;
class UAnimBlueprint;
class UNiagaraSystem;
class UMaterialFunction;

/**
 * Engine subsystem for taking and restoring asset snapshots
 * Provides UE-native snapshot/restore with proper serialization
 */
UCLASS()
class RIFTBORNAI_API URiftbornSnapshotSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Take a snapshot of an asset
	 * @param Asset - Asset to snapshot
	 * @param ActionId - ID of action triggering snapshot
	 * @param Description - Human-readable description
	 * @return Snapshot result with success/error info
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Snapshot")
	FRiftbornSnapshotResult TakeSnapshot(UObject* Asset, const FString& ActionId, const FString& Description);

	/**
	 * Restore an asset from a snapshot
	 * @param Snapshot - Snapshot to restore from
	 * @return Result with success/error info
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Snapshot")
	FRiftbornSnapshotResult RestoreSnapshot(const FRiftbornAssetSnapshot& Snapshot);

	/**
	 * Save snapshot to disk
	 * @param Snapshot - Snapshot to save
	 * @return Result with file path
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Snapshot")
	FRiftbornSnapshotResult SaveSnapshotToDisk(const FRiftbornAssetSnapshot& Snapshot);

	/**
	 * Load snapshot from disk
	 * @param FilePath - Path to snapshot file
	 * @param OutSnapshot - Loaded snapshot
	 * @return Success/failure
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Snapshot")
	bool LoadSnapshotFromDisk(const FString& FilePath, FRiftbornAssetSnapshot& OutSnapshot);

	/**
	 * Get all snapshots for an action ID
	 * @param ActionId - Action ID to query
	 * @return Array of snapshot file paths
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Snapshot")
	TArray<FString> GetSnapshotsForAction(const FString& ActionId);

	/**
	 * Delete all snapshots for an action ID
	 * @param ActionId - Action ID to delete snapshots for
	 * @return Number of snapshots deleted
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Snapshot")
	int32 DeleteSnapshotsForAction(const FString& ActionId);

private:
	/** Classify asset type for snapshot handling */
	ERiftbornAssetType ClassifyAsset(UObject* Asset) const;

	/** Snapshot a Blueprint */
	FRiftbornSnapshotResult SnapshotBlueprint(UBlueprint* Blueprint, const FString& ActionId, const FString& Description);

	/** Snapshot a Widget Blueprint */
	FRiftbornSnapshotResult SnapshotWidgetBlueprint(UWidgetBlueprint* WidgetBP, const FString& ActionId, const FString& Description);

	/** Snapshot a Level */
	FRiftbornSnapshotResult SnapshotLevel(UWorld* World, const FString& ActionId, const FString& Description);

	/** Snapshot a Material Instance */
	FRiftbornSnapshotResult SnapshotMaterialInstance(UMaterialInterface* Material, const FString& ActionId, const FString& Description);

	/** Snapshot a Data Asset */
	FRiftbornSnapshotResult SnapshotDataAsset(UDataAsset* DataAsset, const FString& ActionId, const FString& Description);

	/** Snapshot a Data Table */
	FRiftbornSnapshotResult SnapshotDataTable(UDataTable* DataTable, const FString& ActionId, const FString& Description);

	/** Snapshot a Behavior Tree */
	FRiftbornSnapshotResult SnapshotBehaviorTree(UBehaviorTree* BehaviorTree, const FString& ActionId, const FString& Description);

	/** Snapshot an Animation Blueprint */
	FRiftbornSnapshotResult SnapshotAnimBlueprint(UAnimBlueprint* AnimBP, const FString& ActionId, const FString& Description);

	/** Snapshot a Niagara System */
	FRiftbornSnapshotResult SnapshotNiagaraSystem(UNiagaraSystem* NiagaraSystem, const FString& ActionId, const FString& Description);

	/** Snapshot a Material Function */
	FRiftbornSnapshotResult SnapshotMaterialFunction(UMaterialFunction* MaterialFunction, const FString& ActionId, const FString& Description);

	/** Restore a Blueprint from snapshot */
	FRiftbornSnapshotResult RestoreBlueprint(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore a Widget Blueprint from snapshot */
	FRiftbornSnapshotResult RestoreWidgetBlueprint(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore a Level from snapshot */
	FRiftbornSnapshotResult RestoreLevel(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore a Material Instance from snapshot */
	FRiftbornSnapshotResult RestoreMaterialInstance(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore a Data Asset from snapshot */
	FRiftbornSnapshotResult RestoreDataAsset(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore a Data Table from snapshot */
	FRiftbornSnapshotResult RestoreDataTable(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore a Behavior Tree from snapshot */
	FRiftbornSnapshotResult RestoreBehaviorTree(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore an Animation Blueprint from snapshot */
	FRiftbornSnapshotResult RestoreAnimBlueprint(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore a Niagara System from snapshot */
	FRiftbornSnapshotResult RestoreNiagaraSystem(const FRiftbornAssetSnapshot& Snapshot);

	/** Restore a Material Function from snapshot */
	FRiftbornSnapshotResult RestoreMaterialFunction(const FRiftbornAssetSnapshot& Snapshot);

	/** Snapshot C++ source files (header/cpp) */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Snapshots")
	FRiftbornSnapshotResult SnapshotCPPFiles(const FString& FilePath, const FString& ActionId, const FString& Description);

	/** Restore C++ source files from snapshot */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Snapshots")
	FRiftbornSnapshotResult RestoreCPPFiles(const FRiftbornAssetSnapshot& Snapshot);

	/** Get snapshot directory for action */
	FString GetSnapshotDirectory(const FString& ActionId) const;

	/** Get base snapshots directory */
	FString GetBaseSnapshotsDirectory() const;

	/** Serialize object to byte array using UE's serialization */
	bool SerializeObjectToBytes(UObject* Object, TArray<uint8>& OutBytes);

	/** Deserialize object from byte array */
	bool DeserializeObjectFromBytes(UObject* Object, const TArray<uint8>& Bytes);
};
