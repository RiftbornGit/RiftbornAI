// Copyright Riftborn. All Rights Reserved.
// Voxel Stamps — non-destructive shapes that compose terrain.
// World = f(stamps) + edit deltas. Stamps are evaluated per-chunk during generation.
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "VoxelStamp.generated.h"

class USplineComponent;

// How a stamp modifies the density field
UENUM(BlueprintType)
enum class EVoxelStampBlend : uint8
{
	Add       UMETA(DisplayName = "Add (fill terrain)"),
	Subtract  UMETA(DisplayName = "Subtract (carve hole)"),
	Replace   UMETA(DisplayName = "Replace (override density)"),
};

// Stamp primitive shape
UENUM(BlueprintType)
enum class EVoxelStampShape : uint8
{
	Sphere    UMETA(DisplayName = "Sphere"),
	Box       UMETA(DisplayName = "Box"),
	Noise     UMETA(DisplayName = "Procedural Noise"),
	Spline    UMETA(DisplayName = "Spline (road/river/tunnel)"),
};

// Stamp falloff curve
UENUM(BlueprintType)
enum class EVoxelStampFalloff : uint8
{
	None      UMETA(DisplayName = "Hard edge"),
	Linear    UMETA(DisplayName = "Linear falloff"),
	Smooth    UMETA(DisplayName = "Smooth (cubic)"),
};

/**
 * UVoxelStampComponent — defines a non-destructive terrain shape.
 * Place on an actor in the level. The VoxelTerrainComponent evaluates
 * all stamps during chunk generation, composited by priority order.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API UVoxelStampComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UVoxelStampComponent();

	// Shape of this stamp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp")
	EVoxelStampShape Shape = EVoxelStampShape::Sphere;

	// How this stamp blends with the terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp")
	EVoxelStampBlend BlendMode = EVoxelStampBlend::Add;

	// Falloff at the stamp boundary
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp")
	EVoxelStampFalloff Falloff = EVoxelStampFalloff::Smooth;

	// Priority: lower numbers evaluated first, higher numbers "on top"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp",
		meta = (ClampMin = "0", ClampMax = "1000"))
	int32 Priority = 100;

	// Extent of the stamp shape (half-size for box, radius for sphere)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp",
		meta = (ClampMin = "10.0", ClampMax = "100000.0"))
	FVector Extent = FVector(500.0f);

	// Material ID to assign inside the stamp (0-7 for geological layers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp",
		meta = (ClampMin = "0", ClampMax = "7"))
	uint8 MaterialID = 0;

	// Noise parameters (only used when Shape == Noise)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp|Noise",
		meta = (EditCondition = "Shape == EVoxelStampShape::Noise"))
	float NoiseAmplitude = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp|Noise",
		meta = (EditCondition = "Shape == EVoxelStampShape::Noise"))
	float NoiseFrequency = 0.0005f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp|Noise",
		meta = (EditCondition = "Shape == EVoxelStampShape::Noise"))
	int32 NoiseSeed = 42;

	// Spline parameters (only used when Shape == Spline)
	// Width of the spline path in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp|Spline",
		meta = (EditCondition = "Shape == EVoxelStampShape::Spline", ClampMin = "10.0"))
	float SplineWidth = 300.0f;

	// Depth of the spline carving in cm (how deep it cuts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Stamp|Spline",
		meta = (EditCondition = "Shape == EVoxelStampShape::Spline", ClampMin = "10.0"))
	float SplineDepth = 100.0f;

	/**
	 * Evaluate this stamp's density contribution at a world position.
	 * Returns the signed distance: positive = inside stamp, negative = outside.
	 * The VoxelTerrainComponent calls this for every voxel during chunk generation.
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel Stamp")
	float EvaluateAtWorldPosition(const FVector& WorldPos) const;

	/** Get the world-space bounding box of this stamp. */
	UFUNCTION(BlueprintPure, Category = "Voxel Stamp")
	FBox GetWorldBounds() const;
};

/**
 * AVoxelStampActor — convenience actor with a stamp component.
 * Place these in the level to shape terrain non-destructively.
 */
UCLASS()
class RIFTBORNAI_API AVoxelStampActor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelStampActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel Stamp")
	TObjectPtr<UVoxelStampComponent> StampComponent;
};

/**
 * AVoxelSplineStampActor — stamp with a spline for roads, rivers, tunnels.
 * The spline defines the path; SplineWidth/SplineDepth control the shape.
 */
UCLASS()
class RIFTBORNAI_API AVoxelSplineStampActor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelSplineStampActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel Stamp")
	TObjectPtr<UVoxelStampComponent> StampComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel Stamp")
	TObjectPtr<USplineComponent> SplineComponent;
};
