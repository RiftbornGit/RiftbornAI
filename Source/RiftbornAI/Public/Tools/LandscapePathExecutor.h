// Copyright RiftbornAI. All Rights Reserved.
// LandscapePathExecutor — shared backend for drawing polyline paths on a landscape.
//
// Used by Tool_DrawLandscapePath (parses JSON args, saves the level, returns JSON).
//
// The executor itself performs the actual weight-map / height-map mutation via
// FLandscapeEditDataInterface. It does NOT save the level and does NOT create a
// transaction — those are the caller's responsibility.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NameTypes.h"

class ALandscapeProxy;

/** Input parameters for a single landscape-path raster operation. */
struct RIFTBORNAI_API FRiftbornLandscapePathParams
{
	/** Polyline in world-space X/Y (cm). Must contain at least 2 points. */
	TArray<FVector2D> WorldPoints;

	/**
	 * Optional per-point width overrides (cm). If Num() == WorldPoints.Num(),
	 * local width is interpolated along each segment. If empty or 0-valued
	 * per point, the default Width is used. This lets a single path widen/narrow
	 * along its length (e.g. river deepens downstream, forest trail narrows on cliffs).
	 */
	TArray<float> PerPointWidth;

	/** Optional per-point strength overrides (0..1). Same interpolation semantics as PerPointWidth. */
	TArray<float> PerPointStrength;

	/** Optional per-point height-delta overrides (cm). Same interpolation semantics. */
	TArray<float> PerPointHeightDelta;

	/** Weight-map layer to paint along the path. Empty = no paint (height-only). */
	FString LayerName;

	/** Full path width in world units (cm). Core painted at full strength; edges fall off. */
	float Width = 400.0f;

	/** Peak paint strength 0..1 at the path centerline. */
	float Strength = 1.0f;

	/** Edge falloff shape. 1 = linear, 2 = soft, 4 = very soft. */
	float FalloffPower = 2.0f;

	/**
	 * Optional height offset in world Z units (cm) applied along the path centerline.
	 * Negative = sunken (road bed), positive = raised (berm). 0 = no sculpting.
	 */
	float HeightDeltaCm = 0.0f;

	/**
	 * Endpoint taper distance in world units (cm). Paint strength fades linearly
	 * to 0 over the first and last EndpointTaperDistance along the path. Prevents
	 * the ugly hard-edge "stamp" at path endpoints where two paths meet. 0 disables.
	 */
	float EndpointTaperDistance = 0.0f;

	/** Rebuild landscape grass instances after painting so grass disappears on the painted path. */
	bool bRefreshGrass = true;
};

/** Output statistics from a landscape-path raster operation. */
struct RIFTBORNAI_API FRiftbornLandscapePathStats
{
	bool bSuccess = false;
	FString Error;

	FName ResolvedLayerName = NAME_None;

	int32 PaintedSamples = 0;
	int32 ChangedWeightSamples = 0;
	int32 ChangedHeightSamples = 0;
	int32 MaxWeightBefore = 0;
	int32 MaxWeightAfter = 0;
	int32 RefreshedProxyCount = 0;

	int32 DataWidth = 0;
	int32 DataHeight = 0;

	int32 RequestedMinX = 0;
	int32 RequestedMinY = 0;
	int32 RequestedMaxX = 0;
	int32 RequestedMaxY = 0;

	int32 AppliedMinX = 0;
	int32 AppliedMinY = 0;
	int32 AppliedMaxX = 0;
	int32 AppliedMaxY = 0;

	int32 ExtentMinX = 0;
	int32 ExtentMinY = 0;
	int32 ExtentMaxX = 0;
	int32 ExtentMaxY = 0;
};

/**
 * Shared executor for landscape path rasterization. All game-thread. Editor-only
 * (uses FLandscapeEditDataInterface). Does not save, does not transaction-wrap.
 */
class RIFTBORNAI_API FRiftbornLandscapePathExecutor
{
public:
	/**
	 * Rasterize a polyline into the landscape's weight map (and optionally height map).
	 *
	 * @param Landscape  Target landscape proxy. Must be non-null and fully initialized.
	 * @param Params     Input parameters.
	 * @param OutStats   Output stats (always populated; check OutStats.bSuccess).
	 * @return true if the operation completed (even with no changes); false on hard failure.
	 */
	static bool Execute(
		ALandscapeProxy* Landscape,
		const FRiftbornLandscapePathParams& Params,
		FRiftbornLandscapePathStats& OutStats);
};
