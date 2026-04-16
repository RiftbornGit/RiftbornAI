// Copyright Riftborn. All Rights Reserved.
// Voxel Collision Invoker — controls where voxel terrain generates collision.
// Add to player/NPC actors. Collision only generates within the invoker range.
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VoxelCollisionInvoker.generated.h"

/**
 * UVoxelCollisionInvokerComponent — add to actors that need terrain collision.
 * The VoxelTerrainComponent generates collision meshes only within Range
 * of registered invokers, reducing collision cooking overhead.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API UVoxelCollisionInvokerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UVoxelCollisionInvokerComponent();

	// Range in cm within which collision is generated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Collision",
		meta = (ClampMin = "100.0", ClampMax = "50000.0"))
	float CollisionRange = 5000.0f;

	// If true, also generate navmesh data within range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Collision")
	bool bGenerateNavmesh = false;

	// Get the world position of this invoker
	UFUNCTION(BlueprintPure, Category = "Voxel Collision")
	FVector GetInvokerWorldPosition() const { return GetComponentLocation(); }
};
