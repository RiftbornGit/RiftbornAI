// Copyright Riftborn. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VoxelMaterialPalette.generated.h"

class UMaterialInterface;

/**
 * Single entry in a voxel material palette.
 * Maps a numeric material ID to an actual UMaterialInterface with triplanar parameters.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVoxelMaterialEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Material")
	FName MaterialName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Material")
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Material",
		meta = (ClampMin = "0.01", ClampMax = "100.0"))
	float TriplanarTiling = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Material",
		meta = (ClampMin = "0.1", ClampMax = "16.0"))
	float BlendSharpness = 2.0f;
};

/**
 * Data asset that maps uint8 material IDs to UMaterialInterface entries.
 * Referenced by UVoxelTerrainComponent for multi-material voxel terrain.
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UVoxelMaterialPalette : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Material Palette")
	TArray<FVoxelMaterialEntry> Entries;

	/** Get the material for a given ID. Returns nullptr if ID is out of range. */
	UFUNCTION(BlueprintPure, Category = "Voxel Material Palette")
	UMaterialInterface* GetMaterial(uint8 MaterialID) const;

	/** Get the material name for a given ID. Returns NAME_None if ID is out of range. */
	UFUNCTION(BlueprintPure, Category = "Voxel Material Palette")
	FName GetMaterialName(uint8 MaterialID) const;

	/** Number of entries in the palette. */
	UFUNCTION(BlueprintPure, Category = "Voxel Material Palette")
	int32 GetMaterialCount() const;
};
