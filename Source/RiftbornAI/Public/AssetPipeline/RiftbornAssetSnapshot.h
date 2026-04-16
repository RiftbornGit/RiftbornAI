// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/Archive.h"
#include "UObject/NoExportTypes.h"
#include "RiftbornAssetSnapshot.generated.h"

/**
 * Asset type classification for snapshot handling
 */
UENUM(BlueprintType)
enum class ERiftbornAssetType : uint8
{
	Blueprint UMETA(DisplayName = "Blueprint"),
	WidgetBlueprint UMETA(DisplayName = "Widget Blueprint"),
	Level UMETA(DisplayName = "Level"),
	Material UMETA(DisplayName = "Material"),
	MaterialInstance UMETA(DisplayName = "Material Instance"),
	DataAsset UMETA(DisplayName = "Data Asset"),
	DataTable UMETA(DisplayName = "Data Table"),
	BehaviorTree UMETA(DisplayName = "Behavior Tree"),
	AnimBlueprint UMETA(DisplayName = "Animation Blueprint"),
	NiagaraSystem UMETA(DisplayName = "Niagara System"),
	MaterialFunction UMETA(DisplayName = "Material Function"),
	CPPFiles UMETA(DisplayName = "C++ Files"),
	Other UMETA(DisplayName = "Other")
};

/**
 * Snapshot of a single UE asset for restoration
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornAssetSnapshot
{
	GENERATED_BODY()

	/** Full path to the asset (e.g., /Game/Blueprints/BP_MyActor) */
	UPROPERTY()
	FString AssetPath;

	/** Type of asset being snapshotted */
	UPROPERTY()
	ERiftbornAssetType AssetType = ERiftbornAssetType::Other;

	/** Serialized asset data (JSON or binary) */
	UPROPERTY()
	TArray<uint8> SerializedData;

	/** Optional metadata (JSON string) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	FString Metadata;

	/** Timestamp when snapshot was taken */
	UPROPERTY()
	FDateTime Timestamp = FDateTime();

	/** Action ID that triggered this snapshot */
	UPROPERTY()
	FString ActionId;

	/** Human-readable description of what was snapshotted */
	UPROPERTY()
	FString Description;

	/** Size of serialized data in bytes */
	UPROPERTY()
	int64 DataSizeBytes = 0;

	FRiftbornAssetSnapshot()
		: AssetType(ERiftbornAssetType::Other)
		, Timestamp(FDateTime::Now())
		, DataSizeBytes(0)
	{
	}

	/** Serialize snapshot to/from archive */
	void Serialize(FArchive& Ar)
	{
		Ar << AssetPath;
		uint8 TypeByte = static_cast<uint8>(AssetType);
		Ar << TypeByte;
		if (Ar.IsLoading())
		{
			AssetType = static_cast<ERiftbornAssetType>(TypeByte);
		}
		Ar << SerializedData;
		Ar << Metadata;
		int64 TimestampTicks = Timestamp.GetTicks();
		Ar << TimestampTicks;
		if (Ar.IsLoading())
		{
			Timestamp = FDateTime(TimestampTicks);
		}
		Ar << ActionId;
		Ar << Description;
		Ar << DataSizeBytes;
	}
};

/**
 * Result of a snapshot or restore operation
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornSnapshotResult
{
	GENERATED_BODY()

	/** Whether operation succeeded */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bSuccess = false;

	/** Error messages if any */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Errors;

	/** Warning messages */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Warnings;

	/** Path to snapshot file if saved */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SnapshotFilePath;

	/** Bytes written/read */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int64 BytesProcessed = 0;

	/**
	 * In-memory snapshot data, populated by TakeSnapshot so callers can avoid
	 * a redundant LoadSnapshotFromDisk round-trip (audit finding MED-2).
	 * Not a UPROPERTY — not serialized to disk, just carried in-process.
	 */
	TSharedPtr<FRiftbornAssetSnapshot> InMemorySnapshot;

	void AddError(const FString& Error)
	{
		bSuccess = false;
		Errors.Add(Error);
	}

	void AddWarning(const FString& Warning)
	{
		Warnings.Add(Warning);
	}
};
