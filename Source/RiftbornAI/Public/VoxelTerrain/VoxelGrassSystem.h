// Copyright Riftborn. All Rights Reserved.
// Voxel Grass System — scatters grass blade geometry on voxel terrain surface.
// Uses HISM for performance. Grass only appears on grass-layer voxels.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelGrassSystem.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

/**
 * UVoxelGrassComponent — auto-scatters grass blade meshes on voxel terrain.
 * Queries the terrain query API for surface positions and material layers.
 * Only places grass on grass/moss material layers.
 * Uses HISM for GPU instancing performance.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API UVoxelGrassComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelGrassComponent();

	// Grass blade mesh (use a simple card mesh or import grass blades)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass")
	TObjectPtr<UStaticMesh> GrassMesh;

	// Grass instances per 1000x1000 cm area
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass",
		meta = (ClampMin = "10", ClampMax = "50000"))
	int32 GrassDensity = 2000;

	// Radius around the component to scatter grass
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass",
		meta = (ClampMin = "100.0", ClampMax = "50000.0"))
	float GrassRadius = 3000.0f;

	// Height range of grass blades
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass")
	FVector2D HeightRange = FVector2D(15.0f, 40.0f);

	// Width range of grass blades
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass")
	FVector2D WidthRange = FVector2D(8.0f, 15.0f);

	// Maximum slope angle for grass placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass",
		meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxSlopeAngle = 40.0f;

	// Cull distance start (grass begins fading)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass",
		meta = (ClampMin = "100"))
	int32 CullDistanceStart = 3000;

	// Cull distance end (grass fully hidden)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass",
		meta = (ClampMin = "200"))
	int32 CullDistanceEnd = 8000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Grass")
	int32 Seed = 12345;

	// Generate grass instances on the terrain
	UFUNCTION(BlueprintCallable, Category = "Voxel Grass")
	void GenerateGrass();

	// Remove all grass instances
	UFUNCTION(BlueprintCallable, Category = "Voxel Grass")
	void ClearGrass();

	// Get instance count
	UFUNCTION(BlueprintPure, Category = "Voxel Grass")
	int32 GetGrassInstanceCount() const;

private:
	UPROPERTY()
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> GrassHISM;
};
