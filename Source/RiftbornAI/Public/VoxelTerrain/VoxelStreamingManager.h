// Copyright Riftborn. All Rights Reserved.
#pragma once

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING

#include "CoreMinimal.h"

// ---- Streaming configuration ----
struct RIFTBORNAI_API FVoxelStreamingConfig
{
	float InnerExtent = 8000.f;       // Half-size of the mesh-ready box (cm)
	float OuterExtent = 24000.f;      // Half-size of the density-loaded box (cm)
	int32 MaxLoadedChunks = 500;
	int32 MaxMeshUpdatesPerFrame = 4;
	float HysteresisMargin = 500.f;   // Prevents thrashing at box boundaries (cm)
};

// ---- Chunk lifecycle states ----
enum class EVoxelChunkState : uint8
{
	Unloaded,      // Not in memory
	DensityOnly,   // Density field loaded, no mesh
	MeshPending,   // Mesh generation queued
	Loaded         // Mesh built and visible
};

// ---- Per-chunk streaming bookkeeping ----
struct FVoxelChunkStreamInfo
{
	EVoxelChunkState State = EVoxelChunkState::Unloaded;
	float DistanceSq = 0.f;
	double LastUpdateTime = 0.0;
};

// ---- Clipbox streaming manager ----
class RIFTBORNAI_API FVoxelStreamingManager
{
public:
	void SetConfig(const FVoxelStreamingConfig& InConfig);

	// Main update: given a world position (typically the camera/player), compute
	// which chunks to load density, generate meshes, unload, or purge entirely.
	void UpdateReferencePoint(
		const FVector& WorldPos,
		float ChunkWorldSize,
		TArray<FIntVector>& OutLoad,
		TArray<FIntVector>& OutMesh,
		TArray<FIntVector>& OutUnload,
		TArray<FIntVector>& OutPurge);

	// State confirmations from the chunk system
	void ConfirmLoaded(const FIntVector& Key);
	void ConfirmMeshed(const FIntVector& Key);
	void ConfirmUnloaded(const FIntVector& Key);

	// Register a chunk that already exists (e.g. from a save file)
	void RegisterExistingChunk(const FIntVector& Key, EVoxelChunkState State);

	int32 GetLoadedCount() const;
	EVoxelChunkState GetChunkState(const FIntVector& Key) const;

private:
	FVoxelStreamingConfig Config;
	TMap<FIntVector, FVoxelChunkStreamInfo> ChunkStates;

	void EnforceMemoryBudget(const FVector& WorldPos, float ChunkWorldSize, TArray<FIntVector>& OutPurge);
};

#endif // RIFTBORN_WITH_GEOMETRY_SCRIPTING
