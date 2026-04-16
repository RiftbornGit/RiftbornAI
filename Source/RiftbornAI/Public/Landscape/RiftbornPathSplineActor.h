// Copyright RiftbornAI. All Rights Reserved.
// ARiftbornPathSplineActor — helper actor that hosts USplineMeshComponent
// instances along a URiftbornLandscapePath. Spawned automatically when a path
// asset has SplineMesh set and ApplyToLandscape() runs.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineMeshComponent.h"

#include "RiftbornPathSplineActor.generated.h"

class URiftbornLandscapePath;
class UStaticMesh;
class USceneComponent;

UCLASS()
class RIFTBORNAI_API ARiftbornPathSplineActor : public AActor
{
	GENERATED_BODY()

public:
	ARiftbornPathSplineActor();

	/**
	 * Find an existing spline actor that owns the given path asset in World,
	 * or spawn a new one. Returns nullptr on failure.
	 */
	static ARiftbornPathSplineActor* FindOrSpawnFor(URiftbornLandscapePath* PathAsset, UWorld* World);

	/**
	 * Replace the current spline mesh segments with new ones built from the
	 * given polyline. Destroys old components and creates fresh
	 * USplineMeshComponent instances, one per segment, with auto-computed tangents.
	 */
	void RebuildFromPath(
		const TArray<FVector>& WorldPolyline,
		UStaticMesh* Mesh,
		TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis,
		float ZOffset);

	/** The path asset this actor visualizes — used by FindOrSpawnFor to avoid duplicates. */
	UPROPERTY()
	TWeakObjectPtr<URiftbornLandscapePath> SourcePath;

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> RootSceneComponent;

	/** Spline mesh segments currently owned by this actor. */
	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> MeshSegments;

	void ClearMeshSegments();
};
