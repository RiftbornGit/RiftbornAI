// Copyright Riftborn. All Rights Reserved.
// Voxel Foliage Spawner — auto-places foliage instances on voxel terrain surface.
// Uses the query API to find surface positions and scatter instances.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelFoliageSpawner.generated.h"

class UStaticMesh;
class UHierarchicalInstancedStaticMeshComponent;

/**
 * UVoxelFoliageSpawnerComponent — auto-scatters foliage on voxel terrain.
 * Add to any actor in the level. Queries voxel terrain surface and places
 * instanced static meshes (grass, rocks, bushes) based on density, material,
 * and slope rules.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API UVoxelFoliageSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelFoliageSpawnerComponent();

	// Mesh to scatter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	TObjectPtr<UStaticMesh> FoliageMesh;

	// Instances per 1000x1000 cm area
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage",
		meta = (ClampMin = "1", ClampMax = "10000"))
	int32 Density = 200;

	// Scatter radius from component location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage",
		meta = (ClampMin = "100.0", ClampMax = "100000.0"))
	float ScatterRadius = 5000.0f;

	// Only place on these material layers (-1 = any)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	int32 AllowedMaterialID = -1;

	// Maximum slope angle for placement (degrees). Steeper = no foliage.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage",
		meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxSlopeAngle = 45.0f;

	// Random scale range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

	// Random seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	int32 Seed = 42;

	// Scatter foliage instances on the voxel terrain surface
	UFUNCTION(BlueprintCallable, Category = "Voxel Foliage")
	void ScatterFoliage();

	// Remove all scattered instances
	UFUNCTION(BlueprintCallable, Category = "Voxel Foliage")
	void ClearFoliage();

private:
	UPROPERTY()
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> HISM;
};
