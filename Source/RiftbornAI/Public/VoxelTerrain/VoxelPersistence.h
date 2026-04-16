// Copyright Riftborn. All Rights Reserved.
#pragma once

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING

#include "CoreMinimal.h"
#include "VoxelTerrain/VoxelChunk.h"

// ---- Delta Encoding ----

/** A single voxel edit: stores the flat index, density change, and material override. */
struct FVoxelEditDelta
{
	uint16 Index;        // Flat index into density array
	float DensityDelta;  // Change from procedural baseline
	uint8 MaterialID;    // 0 = unchanged from procedural
};

static_assert(VOXEL_CHUNK_TOTAL_SAMPLES <= 65535, "Flat index must fit uint16");

enum class EVoxelSaveMode : uint8
{
	NoiseDelta = 0,
	Snapshot = 1
};

/** All edits for a single chunk, with RLE compression support. */
struct FVoxelChunkEditRecord
{
	FIntVector ChunkKey;
	TArray<FVoxelEditDelta> Deltas;
	TArray<uint8> CompressedData;

	/** RLE-compress Deltas into CompressedData. */
	void Compress();

	/** Decompress CompressedData back into Deltas. */
	void Decompress();

	/**
	 * Compare an edited chunk against a clean procedural baseline.
	 * @param Edited    The chunk with player edits applied.
	 * @param Procedural A freshly-generated chunk from the same noise params.
	 * @param Key       The chunk coordinate key.
	 * @param Epsilon   Minimum density change to record (filters noise).
	 */
	static FVoxelChunkEditRecord ComputeDeltas(
		const FVoxelChunk& Edited,
		const FVoxelChunk& Procedural,
		const FIntVector& Key,
		float Epsilon = 0.01f);
};

// ---- File Header ----

struct FVoxelSaveHeader
{
	uint32 Magic = 0x56584C54; // 'VXLT'
	uint32 Version = 2;
	int32 ChunkCount = 0;
	FVoxelNoiseParams NoiseParams;
	float CellSize = VOXEL_DEFAULT_CELL_SIZE;
	EVoxelSaveMode SaveMode = EVoxelSaveMode::NoiseDelta;
};

// ---- Persistence API ----

/** Delta-compressed save/load for voxel terrain worlds. */
class FVoxelPersistence
{
public:
	FVoxelPersistence() = delete;

	/**
	 * Save all edited chunks to a binary file.
	 * Only chunks with bHasEdits are written (delta-compressed against procedural).
	 */
	static bool SaveToFile(
		const FString& FilePath,
		const TMap<FIntVector, TUniquePtr<FVoxelChunk>>& Chunks,
		const FVoxelNoiseParams& Noise,
		float CellSize,
		EVoxelSaveMode SaveMode = EVoxelSaveMode::NoiseDelta);

	/**
	 * Load chunks from a save file.
	 * Regenerates procedural baselines and applies stored deltas.
	 */
	static bool LoadFromFile(
		const FString& FilePath,
		TMap<FIntVector, TUniquePtr<FVoxelChunk>>& OutChunks,
		FVoxelNoiseParams& OutNoise,
		float& OutCellSize,
		EVoxelSaveMode* OutSaveMode = nullptr);

	/** Default save path: {ProjectSavedDir}/VoxelTerrain/{LevelName}.vxlt */
	static FString GetDefaultSavePath(const FString& LevelName);

	// ---- Terrain Cache ----
	// Saves ALL chunks (density + material arrays) for instant loading.
	// Skips noise generation entirely on load — BeginPlay goes from
	// 3-20 seconds down to <0.5 seconds.

	/**
	 * Save full terrain state to a cache file (.vxcache).
	 * Stores raw density + material arrays for every chunk.
	 */
	static bool SaveCache(
		const FString& FilePath,
		const TMap<FIntVector, TUniquePtr<FVoxelChunk>>& Chunks,
		const FVoxelNoiseParams& Noise,
		float CellSize);

	/**
	 * Load terrain from cache. No noise regeneration — arrays are read directly.
	 * Returns false if cache is missing, corrupt, or noise params don't match.
	 */
	static bool LoadCache(
		const FString& FilePath,
		TMap<FIntVector, TUniquePtr<FVoxelChunk>>& OutChunks,
		const FVoxelNoiseParams& ExpectedNoise,
		float ExpectedCellSize);

	/** Default cache path: {ProjectSavedDir}/VoxelTerrain/{LevelName}.vxcache */
	static FString GetDefaultCachePath(const FString& LevelName);
};

#endif // RIFTBORN_WITH_GEOMETRY_SCRIPTING
