// Copyright Riftborn. All Rights Reserved.
#pragma once

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"

namespace UE { namespace Geometry { class FDynamicMesh3; } }

// ---- Constants ----
constexpr int32 VOXEL_CHUNK_RES = 32;
constexpr int32 VOXEL_CHUNK_SAMPLES = VOXEL_CHUNK_RES + 1;
constexpr int32 VOXEL_CHUNK_TOTAL_SAMPLES = VOXEL_CHUNK_SAMPLES * VOXEL_CHUNK_SAMPLES * VOXEL_CHUNK_SAMPLES;
constexpr float VOXEL_DEFAULT_CELL_SIZE = 25.0f;  // 25cm = 2x detail vs 50cm
constexpr uint8 VOXEL_MATERIAL_DEFAULT = 0;
constexpr uint8 VOXEL_MATERIAL_MAX = 254;
constexpr int32 VOXEL_MAX_LOD = 6;

// Material layer IDs — assigned from depth plus local pedology / parent material context.
// When a player digs, deeper layers are exposed as volumetric strata rather than
// a flat texture stack.
constexpr uint8 VOXEL_MAT_GRASS     = 0;  // 0–3 cm — surface foliage
constexpr uint8 VOXEL_MAT_MOSS      = 1;  // 3–10 cm — organic mat
constexpr uint8 VOXEL_MAT_TOPSOIL   = 2;  // 10–40 cm — rich dark earth
constexpr uint8 VOXEL_MAT_CLAY      = 3;  // 40–100 cm — dense reddish
constexpr uint8 VOXEL_MAT_GRAVEL    = 4;  // 100–200 cm — loose stones
constexpr uint8 VOXEL_MAT_SANDSTONE = 5;  // 200–500 cm — sedimentary
constexpr uint8 VOXEL_MAT_GRANITE   = 6;  // 500–1000 cm — hard igneous
constexpr uint8 VOXEL_MAT_BEDROCK   = 7;  // 1000+ cm — deep base

// Legacy aliases for code that uses the old 4-layer names.
constexpr uint8 VOXEL_MAT_DIRT = VOXEL_MAT_TOPSOIL;
constexpr uint8 VOXEL_MAT_ROCK = VOXEL_MAT_GRANITE;

constexpr int32 VOXEL_MAT_LAYER_COUNT = 8;

struct FVoxelStrataContext
{
	float Moisture = 0.5f;       // 0 = dry ridge, 1 = saturated hollow
	float Slope = 0.0f;          // 0 = flat, 1 = steep
	float Concavity = 0.5f;      // 0 = convex / shedding, 1 = depositional hollow
	float Vegetation = 0.5f;     // 0 = sparse / exposed, 1 = biologically active surface
	float ParentMaterial = 0.5f; // 0 = crystalline, 1 = sedimentary / weathered
};

inline float VoxelResolveHorizonThickness(float DesiredCm, float ResolutionCm, float MinVoxelFraction)
{
	const float EffectiveResolution = FMath::Max(ResolutionCm, 1.0f);
	return FMath::Max(DesiredCm, EffectiveResolution * MinVoxelFraction);
}

// Context-aware stratigraphy. Depth is in cm below the local ground surface.
// TransitionNoise slightly perturbs boundaries so horizons undulate organically.
inline uint8 VoxelMaterialFromDepthAndContext(
	float DepthCm,
	const FVoxelStrataContext& Context,
	float TransitionNoise = 0.0f,
	float ResolutionCm = VOXEL_DEFAULT_CELL_SIZE,
	float FormationNoise = 0.0f)
{
	const float Moisture = FMath::Clamp(Context.Moisture, 0.0f, 1.0f);
	const float Slope = FMath::Clamp(Context.Slope, 0.0f, 1.0f);
	const float Concavity = FMath::Clamp(Context.Concavity, 0.0f, 1.0f);
	const float Vegetation = FMath::Clamp(Context.Vegetation, 0.0f, 1.0f);
	const float ParentMaterial = FMath::Clamp(Context.ParentMaterial, 0.0f, 1.0f);
	const float EffectiveResolution = FMath::Max(ResolutionCm, 1.0f);

	const float D = FMath::Max(0.0f, DepthCm + TransitionNoise);
	const float FormationBias = FMath::Clamp(
		FormationNoise / FMath::Max(EffectiveResolution * 2.5f, 80.0f),
		-1.0f,
		1.0f);
	const float GrassDepth = 2.0f + 4.5f * Vegetation * (1.0f - 0.25f * Slope) + 1.5f * Moisture;
	const float OrganicDepth = GrassDepth + FMath::Max(
		6.0f + 12.0f * Vegetation + 10.0f * Moisture * (1.0f - 0.45f * Slope),
		VoxelResolveHorizonThickness(8.0f, EffectiveResolution, 0.35f));
	const float DesiredTopsoilDepth =
		OrganicDepth +
		16.0f +
		55.0f * Vegetation * (1.0f - 0.45f * Slope) +
		48.0f * Moisture +
		24.0f * (Concavity - 0.5f);
	const float TopsoilDepth = FMath::Max(
		DesiredTopsoilDepth,
		OrganicDepth + VoxelResolveHorizonThickness(24.0f, EffectiveResolution, 0.85f));
	const float DesiredSubsoilDepth =
		TopsoilDepth +
		70.0f +
		180.0f * Moisture +
		55.0f * (Concavity - 0.5f) -
		28.0f * Slope;
	const float SubsoilDepth = FMath::Max(
		DesiredSubsoilDepth,
		TopsoilDepth + VoxelResolveHorizonThickness(85.0f, EffectiveResolution, 1.8f));
	const float DesiredColluviumDepth =
		SubsoilDepth +
		120.0f +
		135.0f * Slope +
		80.0f * (1.0f - Moisture) +
		40.0f * FMath::Abs(Concavity - 0.5f);
	const float ColluviumDepth = FMath::Max(
		DesiredColluviumDepth,
		SubsoilDepth + VoxelResolveHorizonThickness(180.0f, EffectiveResolution, 3.6f)) +
		FormationBias * EffectiveResolution * 0.8f;
	const float RockTransitionDepth = FMath::Max(
		TopsoilDepth + VoxelResolveHorizonThickness(
			480.0f + 220.0f * ParentMaterial - 60.0f * Slope,
			EffectiveResolution,
			9.0f),
		ColluviumDepth + VoxelResolveHorizonThickness(
			260.0f + 220.0f * ParentMaterial + 90.0f * (1.0f - Moisture),
			EffectiveResolution,
			5.0f) + FormationBias * EffectiveResolution * 1.4f);
	const float HardRockDepth =
		RockTransitionDepth +
		VoxelResolveHorizonThickness(
			1100.0f + 1300.0f * ParentMaterial + 220.0f * Slope,
			EffectiveResolution,
			18.0f) +
		FormationBias * EffectiveResolution * 2.0f;

	const bool bWetSurface = Moisture > 0.6f && Slope < 0.4f;
	const bool bClayDominant = (Moisture + 0.5f * Concavity) > (0.55f + 0.25f * Slope);
	const bool bColluvial = Slope > 0.55f || Moisture < 0.28f;
	const bool bSedimentaryRock = (ParentMaterial + 0.18f * FormationBias) >= 0.5f;

	if (D < GrassDepth)
	{
		return bWetSurface ? VOXEL_MAT_MOSS : VOXEL_MAT_GRASS;
	}
	if (D < OrganicDepth)
	{
		return bWetSurface ? VOXEL_MAT_MOSS : VOXEL_MAT_TOPSOIL;
	}
	if (D < TopsoilDepth)
	{
		return VOXEL_MAT_TOPSOIL;
	}
	if (D < SubsoilDepth)
	{
		if (bClayDominant)
		{
			return VOXEL_MAT_CLAY;
		}
		return bColluvial ? VOXEL_MAT_GRAVEL : VOXEL_MAT_TOPSOIL;
	}
	if (D < ColluviumDepth)
	{
		if (bClayDominant && FormationBias < -0.35f)
		{
			return VOXEL_MAT_CLAY;
		}
		return VOXEL_MAT_GRAVEL;
	}
	if (D < RockTransitionDepth)
	{
		return bSedimentaryRock ? VOXEL_MAT_SANDSTONE : VOXEL_MAT_GRANITE;
	}
	if (D < HardRockDepth)
	{
		return bSedimentaryRock ? VOXEL_MAT_SANDSTONE : VOXEL_MAT_GRANITE;
	}
	return VOXEL_MAT_BEDROCK;
}

// Legacy depth-only fallback. Use the context-aware helper when real subsurface
// stratification is available.
inline uint8 VoxelMaterialFromDepth(
	float DepthCm,
	float LayerNoise = 0.0f,
	float ResolutionCm = VOXEL_DEFAULT_CELL_SIZE)
{
	const float D = DepthCm + LayerNoise;
	const float EffectiveResolution = FMath::Max(ResolutionCm, 1.0f);
	if (D < FMath::Max(3.0f, EffectiveResolution * 0.08f))   return VOXEL_MAT_GRASS;
	if (D < FMath::Max(10.0f, EffectiveResolution * 0.20f))  return VOXEL_MAT_MOSS;
	if (D < FMath::Max(40.0f, EffectiveResolution * 0.60f))  return VOXEL_MAT_TOPSOIL;
	if (D < FMath::Max(100.0f, EffectiveResolution * 1.40f)) return VOXEL_MAT_CLAY;
	if (D < FMath::Max(200.0f, EffectiveResolution * 2.40f)) return VOXEL_MAT_GRAVEL;
	if (D < FMath::Max(500.0f, EffectiveResolution * 4.00f)) return VOXEL_MAT_SANDSTONE;
	if (D < FMath::Max(1000.0f, EffectiveResolution * 7.00f)) return VOXEL_MAT_GRANITE;
	return VOXEL_MAT_BEDROCK;
}

// Chunk faces for Transvoxel boundary stitching
enum class EVoxelFace : uint8 { NegX = 0, PosX, NegY, PosY, NegZ, PosZ, Count };

// ---- Terrain noise parameters ----
struct FVoxelNoiseParams
{
	int32 Seed = 42;
	float BaseAmplitude = 800.0f;   // Max height variation (cm)
	float BaseFrequency = 0.0004f;  // World-space frequency (1/wavelength)
	int32 Octaves = 6;
	float Lacunarity = 2.0f;        // Frequency multiplier per octave
	float Persistence = 0.5f;       // Amplitude multiplier per octave
	float MicroNoiseScale = 0.003f; // High-frequency surface roughness
	float MicroNoiseAmp = 15.0f;    // Micro noise amplitude (cm)

	// Domain warping — distorts coordinates before sampling, creates organic shapes
	float WarpAmplitude = 400.0f;   // How far coordinates get displaced (cm)
	float WarpFrequency = 0.0003f;  // Frequency of the warp noise field
	int32 WarpOctaves = 3;          // FBM octaves for warp field

	// Ridge noise — fraction of ridged noise mixed in (0=none, 1=all ridges)
	float RidgeFraction = 0.3f;     // Blend factor: lerp(FBM, ridged, fraction)
};

// ---- FVoxelChunk ----
struct RIFTBORNAI_API FVoxelChunk
{
	TArray<float> Density;
	TArray<uint8> MaterialIDs;
	bool bDirty = true;
	bool bUniform = false;
	bool bHasEdits = false;
	uint32 ContentRevision = 1;

	FVoxelChunk();

	// Initialize density from landscape heightmap.
	void InitFromLandscapeHeight(
		const FVector& ChunkWorldOrigin,
		float CellSize,
		TFunctionRef<float(float WorldX, float WorldY)> GetLandscapeZ,
		const FVoxelNoiseParams& Noise,
		float SkinDepth = 0.0f);

	// Initialize density from procedural noise (no landscape needed).
	void InitFromNoise(
		const FVector& ChunkWorldOrigin,
		float CellSize,
		const FVoxelNoiseParams& Noise);

	// Initialize from a heightmap array with noise detail on top.
	// HeightData is a 2D grid (SizeX × SizeY) of heights in cm.
	// WorldOriginXY/WorldExtentXY map the heightmap to world coordinates.
	// Noise is layered on top for voxel-scale detail + material layer assignment.
	void InitFromHeightmapAndNoise(
		const FVector& ChunkWorldOrigin,
		float CellSize,
		const float* HeightData,
		int32 SizeX,
		int32 SizeY,
		const FVector2D& WorldOriginXY,
		const FVector2D& WorldExtentXY,
		const FVoxelNoiseParams& Noise);

	// Softens the outer edge of a finite generated terrain area so it fades out
	// before the chunk boundary instead of ending in a vertical rectangular wall.
	void ApplyGenerationBoundaryFalloff(
		const FVector& ChunkWorldOrigin,
		float CellSize,
		const FVector2D& AreaCenterXY,
		float AreaExtentXY,
		float AreaExtentZ,
		const FVoxelNoiseParams& Noise);

	// CSG subtract: carve a sphere out of the density field.
	bool SubtractSphere(
		const FVector& Center, float Radius,
		const FVector& ChunkWorldOrigin, float CellSize);

	// CSG add: fill a sphere back into the density field.
	bool AddSphere(
		const FVector& Center, float Radius,
		const FVector& ChunkWorldOrigin, float CellSize);

	// Smooth density values in a sphere region.
	bool SmoothSphere(
		const FVector& Center, float Radius, float Strength,
		const FVector& ChunkWorldOrigin, float CellSize);

	// CSG add with material ID: fill sphere and write material to affected voxels.
	bool AddSphereWithMaterial(
		const FVector& Center, float Radius, uint8 MaterialID,
		const FVector& ChunkWorldOrigin, float CellSize);

	// CSG subtract: carve a box out of the density field.
	bool SubtractBox(
		const FVector& BoxCenter, const FVector& BoxExtent,
		const FVector& ChunkWorldOrigin, float CellSize);

	// CSG add: fill a box back into the density field.
	bool AddBox(
		const FVector& BoxCenter, const FVector& BoxExtent,
		const FVector& ChunkWorldOrigin, float CellSize);

	// Density access
	float GetDensity(int32 IX, int32 IY, int32 IZ) const;
	float SampleDensity(float FX, float FY, float FZ) const;

	// Material access
	uint8 GetMaterial(int32 IX, int32 IY, int32 IZ) const;
	void SetMaterial(int32 IX, int32 IY, int32 IZ, uint8 MaterialID);

	// Extract a face's density slice (VOXEL_CHUNK_SAMPLES × VOXEL_CHUNK_SAMPLES) for Transvoxel.
	void GetFaceDensities(EVoxelFace Face, TArray<float>& OutSlice) const;

	// Downsample 2x: produces (VOXEL_CHUNK_SAMPLES/2+1)³ output for LOD.
	static void Downsample2x(const FVoxelChunk& Src,
		TArray<float>& OutDensity, TArray<uint8>& OutMaterial);

	void BumpContentRevision();

private:
	FORCEINLINE int32 FlatIndex(int32 IX, int32 IY, int32 IZ) const
	{
		// Bounds-check instead of silent clamping. Clamping hides bugs where callers
		// pass out-of-range indices — the wrong voxel is silently read/written and
		// the bug only manifests as subtle visual artifacts.
		checkf(IX >= 0 && IX < VOXEL_CHUNK_SAMPLES, TEXT("FlatIndex: IX=%d out of range [0,%d)"), IX, VOXEL_CHUNK_SAMPLES);
		checkf(IY >= 0 && IY < VOXEL_CHUNK_SAMPLES, TEXT("FlatIndex: IY=%d out of range [0,%d)"), IY, VOXEL_CHUNK_SAMPLES);
		checkf(IZ >= 0 && IZ < VOXEL_CHUNK_SAMPLES, TEXT("FlatIndex: IZ=%d out of range [0,%d)"), IZ, VOXEL_CHUNK_SAMPLES);
		return IX + IY * VOXEL_CHUNK_SAMPLES + IZ * VOXEL_CHUNK_SAMPLES * VOXEL_CHUNK_SAMPLES;
	}
};

// ---- Noise Utilities ----
// Deterministic 3D gradient noise (value range approximately -1..1).
RIFTBORNAI_API float VoxelNoise3D(float X, float Y, float Z, int32 Seed = 0);

// Fractal Brownian Motion — layered noise for natural terrain.
RIFTBORNAI_API float VoxelFBM(float X, float Y, int32 Octaves, float Lacunarity,
	float Persistence, float Frequency, int32 Seed = 0);

// Ridged FBM — sharp ridges and valleys for mountain-like features.
RIFTBORNAI_API float VoxelRidgedFBM(float X, float Y, int32 Octaves, float Lacunarity,
	float Persistence, float Frequency, int32 Seed = 0);

// ---- Mesh Extraction ----
RIFTBORNAI_API UE::Geometry::FDynamicMesh3 ExtractChunkMesh(
	const FVoxelChunk& Chunk,
	const FVector& ChunkWorldOrigin,
	float CellSize);

#endif // RIFTBORN_WITH_GEOMETRY_SCRIPTING
