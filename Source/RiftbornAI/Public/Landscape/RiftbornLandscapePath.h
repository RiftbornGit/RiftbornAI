// Copyright RiftbornAI. All Rights Reserved.
// URiftbornLandscapePath — persistent, editable, re-applyable landscape path asset.
//
// Bridges the interactive editor mode and the AI tool: the editor mode can
// save a click-drawn path as an asset; the AI tool can load an asset via its
// content path and re-rasterize. The asset stores control points + per-point
// overrides + shared settings, and its ApplyToLandscape() method calls the
// shared executor to do the actual weight/height mutation.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/SoftObjectPtr.h"
#include "Components/SplineMeshComponent.h" // For ESplineMeshAxis

#include "RiftbornLandscapePath.generated.h"

class ALandscapeProxy;
class UStaticMesh;

/**
 * One control point on a landscape path. Stores the world location plus
 * optional per-point overrides for width/strength/height. An override value
 * of 0 (for width/strength) or FLT_MAX (sentinel, for height) means "inherit
 * the path's default for this field".
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornLandscapePathPoint
{
	GENERATED_BODY()

	/** World-space location of this control point. Z is preserved from the landscape line-trace at placement time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Point")
	FVector Location = FVector::ZeroVector;

	/** Per-point width override (cm). 0 = inherit path default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Point", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float WidthOverride = 0.0f;

	/** Per-point strength override (0..1). 0 = inherit path default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Point", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StrengthOverride = 0.0f;

	/** Per-point height-delta override (cm). Use 0 to inherit default — per-point override is only active if non-zero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Point", meta = (ClampMin = "-2000.0", ClampMax = "2000.0"))
	float HeightDeltaOverride = 0.0f;

	/** If true, this point uses its HeightDeltaOverride even if that value is 0 (so you can explicitly flatten a section). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Point")
	bool bHeightOverrideActive = false;
};

/**
 * Persistent landscape path asset. Editable in the content browser, re-applyable
 * via ApplyToLandscape(), and readable/writable from the editor mode tool.
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API URiftbornLandscapePath : public UDataAsset
{
	GENERATED_BODY()

public:
	URiftbornLandscapePath();

	// -------- Geometry --------

	/** Control points that define the path, in placement order. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Geometry")
	TArray<FRiftbornLandscapePathPoint> ControlPoints;

	/** Smooth the polyline with Catmull-Rom interpolation so paths curve naturally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Geometry")
	bool bSmoothPath = true;

	/** Interpolated samples per control-point segment when smoothing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Geometry", meta = (EditCondition = "bSmoothPath", ClampMin = "2", ClampMax = "64"))
	int32 SmoothingSegments = 12;

	// -------- Paint --------

	/** Weight-map layer painted along the path (must exist on the target landscape). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Paint")
	FString LayerName = TEXT("Dirt");

	/** Default width in cm. Overridden per-point if FRiftbornLandscapePathPoint::WidthOverride > 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Paint", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
	float DefaultWidth = 400.0f;

	/** Default paint strength 0..1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Paint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultStrength = 1.0f;

	/** Edge falloff power. 1=linear, 2=soft, 4=very soft. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Paint", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float FalloffPower = 2.0f;

	/** Rebuild grass after painting so grass disappears on the path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Paint")
	bool bRefreshGrass = true;

	// -------- Height --------

	/** Default height delta in cm. Negative = sunken road bed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Height", meta = (ClampMin = "-2000.0", ClampMax = "2000.0"))
	float DefaultHeightDelta = 0.0f;

	/**
	 * Distance (cm) over which paint strength fades to 0 at path endpoints.
	 * Prevents ugly hard-stamp edges where multiple paths meet. 0 disables.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Height", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float EndpointTaperDistance = 200.0f;

	// -------- Target --------

	/** Target landscape this path is applied to. Soft pointer so the asset survives across map loads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Target")
	TSoftObjectPtr<ALandscapeProxy> TargetLandscape;

	// -------- Optional spline mesh geometry --------

	/**
	 * Optional static mesh to place along the path as real 3D geometry (via
	 * USplineMeshComponent instances on a helper actor). Leave empty to paint
	 * the landscape only — the spline mesh is additive, not a replacement.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|SplineMesh")
	TSoftObjectPtr<UStaticMesh> SplineMesh;

	/** Forward axis of the SplineMesh asset used for the spline-mesh deformation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|SplineMesh")
	TEnumAsByte<ESplineMeshAxis::Type> SplineMeshForwardAxis = ESplineMeshAxis::X;

	/** Vertical offset applied to spline mesh segments (cm). Negative sits the mesh into a sunken road. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|SplineMesh")
	float SplineMeshZOffset = 0.0f;

	// -------- Preview --------

	/** Color used when previewing the path in the editor viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Preview")
	FLinearColor PreviewColor = FLinearColor(1.0f, 0.55f, 0.1f, 1.0f);

	// -------- Operations --------

	/**
	 * Rasterize this path to its target landscape. Callers are responsible
	 * for wrapping in FScopedTransaction + Landscape->Modify() if undo is desired,
	 * and for calling SaveCurrentLevel() afterwards if they want to persist.
	 * Returns true on success; OutError contains the reason on failure.
	 */
	bool ApplyToLandscape(FString& OutError);

	/** Resolve the target landscape, either by loading the soft pointer or by falling back to the first landscape in the given world. */
	ALandscapeProxy* ResolveTargetLandscape(UWorld* World) const;

	/** Build the polyline that will be painted: Catmull-Rom interpolated if bSmoothPath, otherwise raw control-point locations. */
	void BuildWorldPolyline(TArray<FVector>& OutPoints) const;

	/** Build the interpolated per-point attribute arrays aligned with BuildWorldPolyline output. */
	void BuildInterpolatedOverrides(
		const TArray<FVector>& SmoothedPoints,
		TArray<float>& OutPerPointWidth,
		TArray<float>& OutPerPointStrength,
		TArray<float>& OutPerPointHeight) const;
};
