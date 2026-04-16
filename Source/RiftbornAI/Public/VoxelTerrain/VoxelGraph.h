// Copyright Riftborn. All Rights Reserved.
// Voxel Graph — data-driven terrain generation graphs.
// Nodes define procedural operations (noise, combine, transform, output).
// Evaluates during chunk generation to produce density + material values.
// Phase 1: runtime evaluation from data asset. Visual editor in future sprint.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VoxelGraph.generated.h"

// Node types for voxel generation graphs
UENUM(BlueprintType)
enum class EVoxelGraphNodeType : uint8
{
	Noise2D       UMETA(DisplayName = "2D Noise (FBM)"),
	Noise3D       UMETA(DisplayName = "3D Noise"),
	RidgedNoise   UMETA(DisplayName = "Ridged Noise"),
	Constant      UMETA(DisplayName = "Constant Value"),
	Add           UMETA(DisplayName = "Add"),
	Multiply      UMETA(DisplayName = "Multiply"),
	Lerp          UMETA(DisplayName = "Lerp (blend)"),
	Clamp         UMETA(DisplayName = "Clamp"),
	HeightOutput  UMETA(DisplayName = "Height Output"),
	MaterialOutput UMETA(DisplayName = "Material Output"),
};

// Single node in a voxel generation graph
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVoxelGraphNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FName NodeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	EVoxelGraphNodeType Type = EVoxelGraphNodeType::Noise2D;

	// Parameters (meaning depends on Type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float ParamA = 0.0f; // Frequency/Value/Min

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float ParamB = 0.0f; // Amplitude/Octaves/Max

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	int32 Seed = 42;

	// Input connections: indices into the graph's node array (-1 = none)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	int32 InputA = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	int32 InputB = -1;

	// Editor position (for future visual editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FVector2D EditorPosition = FVector2D::ZeroVector;
};

/**
 * UVoxelGraphAsset — data asset defining a terrain generation graph.
 * Contains an array of nodes that are evaluated to produce terrain density.
 * The last HeightOutput node determines the final terrain height.
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UVoxelGraphAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Graph")
	TArray<FVoxelGraphNode> Nodes;

	// Evaluate the graph at a world XY position. Returns terrain height.
	UFUNCTION(BlueprintPure, Category = "Voxel Graph")
	float EvaluateHeight(float WorldX, float WorldY) const;

	// Evaluate what material should be at a given depth
	UFUNCTION(BlueprintPure, Category = "Voxel Graph")
	uint8 EvaluateMaterial(float DepthFromSurface) const;

private:
	float EvaluateNode(int32 NodeIndex, float WorldX, float WorldY) const;
};
