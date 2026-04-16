// Copyright Riftborn. All Rights Reserved.
// Voxel Query Library — Blueprint-callable functions to query voxel terrain data.
// Use from gameplay, PCG, AI, and any Blueprint context.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelQueryLibrary.generated.h"

/**
 * UVoxelQueryLibrary — global voxel terrain query functions.
 * Call from any Blueprint to sample terrain density, material, surface height, etc.
 * Finds the VoxelTerrainComponent in the world automatically.
 */
UCLASS()
class RIFTBORNAI_API UVoxelQueryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Sample density at a world position. Positive = solid, negative = air.
	UFUNCTION(BlueprintPure, Category = "Voxel Query", meta = (WorldContext = "WorldContextObject"))
	static float SampleVoxelDensity(const UObject* WorldContextObject, FVector WorldPos);

	// Get the material layer ID at a world position (0-7 geological strata).
	UFUNCTION(BlueprintPure, Category = "Voxel Query", meta = (WorldContext = "WorldContextObject"))
	static int32 SampleVoxelMaterial(const UObject* WorldContextObject, FVector WorldPos);

	// Find the terrain surface Z at a given XY position. Returns false if no terrain.
	UFUNCTION(BlueprintPure, Category = "Voxel Query", meta = (WorldContext = "WorldContextObject"))
	static bool FindVoxelSurfaceZ(const UObject* WorldContextObject, float X, float Y, float& OutZ);

	// Check if a point is inside solid voxel terrain.
	UFUNCTION(BlueprintPure, Category = "Voxel Query", meta = (WorldContext = "WorldContextObject"))
	static bool IsInsideVoxelTerrain(const UObject* WorldContextObject, FVector WorldPos);

	// Get the geological layer name for a material ID (grass, moss, topsoil, etc.)
	UFUNCTION(BlueprintPure, Category = "Voxel Query")
	static FString GetLayerName(int32 MaterialID);
};
