// Copyright Riftborn. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelTerrainActor.generated.h"

class UVoxelTerrainComponent;

/**
 * Actor that hosts a VoxelTerrainComponent as a default subobject.
 * Spawning this actor guarantees the component transfers to PIE/runtime.
 */
UCLASS(BlueprintType, Blueprintable)
class RIFTBORNAI_API AVoxelTerrainActor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelTerrainActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel Terrain")
	TObjectPtr<UVoxelTerrainComponent> VoxelTerrain;
};
