// Copyright RiftbornAI. All Rights Reserved.
// Soil computation utilities — shared between compute_soil_map and simulate_ecosystem_growth.
// Pure math on heightmap data. No UE API calls beyond basic types.
//
// The model: bedrock weathers into soil. Slope controls erosion rate.
// Concave terrain accumulates material (colluvium). Water carries dissolved
// minerals downhill (TWI proxy). Elevation + drainage determine pH.

#pragma once

#include "CoreMinimal.h"

/**
 * @namespace SoilUtils
 *
 * Soil computation utilities — shared between compute_soil_map and simulate_ecosystem_growth.
 */
namespace SoilUtils
{

struct FSoilMetrics
{
	TArray<float> Depth;       // 0-1, relative soil depth (0=bare rock, 1=deep alluvial)
	TArray<float> Fertility;   // 0-1, nutrient availability (nitrogen, phosphorus, potassium proxy)
	TArray<float> pH;          // 4.0-8.0, acidic to alkaline
	TArray<float> Curvature;   // Profile curvature: negative=concave(accumulates), positive=convex(sheds)
};

// Compute profile curvature (second derivative along slope direction).
// Negative = concave (material accumulates), Positive = convex (material sheds).
inline void ComputeCurvature(const TArray<uint16>& Heights, int32 W, int32 H,
	TArray<float>& OutCurvature)
{
	OutCurvature.SetNumZeroed(W * H);
	for (int32 Y = 1; Y < H - 1; ++Y)
	{
		for (int32 X = 1; X < W - 1; ++X)
		{
			const int32 Idx = Y * W + X;
			const float C = (float)Heights[Idx];
			const float E = (float)Heights[Idx + 1];
			const float W_ = (float)Heights[Idx - 1];
			const float N = (float)Heights[(Y - 1) * W + X];
			const float S = (float)Heights[(Y + 1) * W + X];

			// Laplacian (sum of second derivatives) — negative = concave basin
			OutCurvature[Idx] = (E + W_ + N + S - 4.0f * C) * 0.25f;
		}
	}
}

// Compute soil metrics from heightmap-derived data.
// Inputs: Slope (radians), TWI (topographic wetness index), Curvature, Heights.
// All arrays must be W*H sized.
inline void ComputeSoilMetrics(
	const TArray<uint16>& Heights, int32 W, int32 H,
	const TArray<float>& Slope,     // radians
	const TArray<float>& TWI,       // topographic wetness index
	FSoilMetrics& Out)
{
	const int32 N = W * H;
	Out.Depth.SetNumZeroed(N);
	Out.Fertility.SetNumZeroed(N);
	Out.pH.SetNumZeroed(N);

	// Compute curvature
	ComputeCurvature(Heights, W, H, Out.Curvature);

	// Find height range for relative elevation
	uint16 MinH = 65535, MaxH = 0;
	for (const uint16 V : Heights) { MinH = FMath::Min(MinH, V); MaxH = FMath::Max(MaxH, V); }
	const float HeightRange = FMath::Max((float)(MaxH - MinH), 1.0f);

	// Find TWI range for normalization
	float TWIMin = FLT_MAX, TWIMax = -FLT_MAX;
	for (const float V : TWI) { TWIMin = FMath::Min(TWIMin, V); TWIMax = FMath::Max(TWIMax, V); }
	const float TWIRange = FMath::Max(TWIMax - TWIMin, 1.0f);

	for (int32 I = 0; I < N; ++I)
	{
		const float SlopeDeg = Slope[I] * (180.0f / PI);
		const float RelElev = ((float)Heights[I] - (float)MinH) / HeightRange;
		const float NormTWI = (TWI[I] - TWIMin) / TWIRange;
		const float Curv = Out.Curvature[I];

		// ── Soil Depth ──
		// Deep in valleys (low slope + concave curvature + low elevation)
		// Thin on ridges (steep slope + convex curvature + high elevation)
		float DepthFromSlope = FMath::Clamp(1.0f - SlopeDeg / 45.0f, 0.0f, 1.0f);
		float DepthFromCurvature = FMath::Clamp((-Curv + 5.0f) / 10.0f, 0.0f, 1.0f); // Concave = deeper
		float DepthFromElevation = FMath::Clamp(1.0f - RelElev * 0.5f, 0.3f, 1.0f);   // Lower = deeper
		Out.Depth[I] = DepthFromSlope * DepthFromCurvature * DepthFromElevation;

		// ── Soil Fertility ──
		// High where: deep soil + high moisture + organic matter accumulates
		// Organic accumulation proxy: low slope (leaf litter stays) + moisture (decomposition)
		float OrganicAccum = FMath::Clamp(DepthFromSlope * NormTWI, 0.0f, 1.0f);
		// Mineral transport: water carries dissolved nutrients downhill (TWI)
		float MineralTransport = FMath::Clamp(NormTWI * 0.8f, 0.0f, 1.0f);
		Out.Fertility[I] = FMath::Clamp(
			Out.Depth[I] * 0.4f + OrganicAccum * 0.35f + MineralTransport * 0.25f,
			0.0f, 1.0f);

		// ── Soil pH ──
		// High elevation + good drainage = acidic (rainfall leaches base cations: Ca, Mg, K)
		// Low elevation + poor drainage = neutral/alkaline (minerals accumulate)
		// Range: 4.0 (very acidic) to 8.0 (alkaline)
		float DrainageFactor = FMath::Clamp(1.0f - NormTWI, 0.0f, 1.0f); // Well-drained = more leaching
		float ElevFactor = RelElev; // Higher = more leaching from rainfall
		float AcidityFactor = (DrainageFactor * 0.6f + ElevFactor * 0.4f); // 0=alkaline, 1=acidic
		Out.pH[I] = FMath::Lerp(7.5f, 4.0f, AcidityFactor); // 4.0 acidic ← → 7.5 alkaline
	}
}

} // namespace SoilUtils
