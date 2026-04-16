// Copyright Riftborn. All Rights Reserved.
#pragma once

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"

// ============================================================
// Cluster merge configuration
// ============================================================

struct RIFTBORNAI_API FVoxelClusterConfig
{
	// Chunks per cluster axis (e.g., 4 = 4x4x4 = up to 64 chunks per cluster)
	int32 ClusterSize = 4;

	// Minimum non-empty chunks in a cluster before merging is attempted
	int32 MinChunksToMerge = 4;

	// Only merge clusters whose chunks haven't been edited recently
	bool bOnlyMergeStable = true;

	// Seconds since last edit before a cluster is considered stable
	float StabilityTimeout = 2.0f;
};

// ============================================================
// Cluster lifecycle states
// ============================================================

enum class RIFTBORNAI_API EVoxelClusterState : uint8
{
	Individual,  // Chunks rendered separately
	Merging,     // Merge in progress (reserved for async future)
	Merged,      // Single merged mesh active
	Dirty        // Was merged, now needs unmerge due to edit
};

// ============================================================
// Per-cluster bookkeeping
// ============================================================

struct RIFTBORNAI_API FVoxelClusterInfo
{
	FIntVector ClusterKey = FIntVector::ZeroValue;
	TArray<FIntVector> ChunkKeys;
	EVoxelClusterState State = EVoxelClusterState::Individual;
	double LastEditTime = 0.0;
	UE::Geometry::FDynamicMesh3 MergedMesh;

	// How many chunks in this cluster actually have geometry (triangle count > 0)
	int32 GeometryChunkCount = 0;
};

// ============================================================
// FVoxelClusterMerger — merge adjacent chunks to cut draw calls
// ============================================================

class RIFTBORNAI_API FVoxelClusterMerger
{
public:
	void SetConfig(const FVoxelClusterConfig& InConfig);

	// Register a chunk's mesh after it was built. Stores a copy for unmerge.
	void RegisterChunkMesh(const FIntVector& ChunkKey,
		const UE::Geometry::FDynamicMesh3& Mesh);

	// Remove a chunk entirely (chunk unloaded from world).
	void UnregisterChunk(const FIntVector& ChunkKey);

	// Mark a chunk's cluster as dirty after an edit.
	void NotifyChunkEdited(const FIntVector& ChunkKey, double CurrentTime);

	// Game-thread tick: process merge/unmerge transitions.
	// OutNewlyMerged: cluster keys that just finished merging.
	// OutUnmerged: cluster keys that were split back to individual.
	void Tick(double CurrentTime,
		TArray<FIntVector>& OutNewlyMerged,
		TArray<FIntVector>& OutUnmerged);

	// Get the merged mesh for a cluster. Returns nullptr if not merged.
	const UE::Geometry::FDynamicMesh3* GetMergedMesh(
		const FIntVector& ClusterKey) const;

	// Map a chunk key to its containing cluster key.
	FIntVector ChunkKeyToClusterKey(const FIntVector& ChunkKey) const;

	// Stats
	int32 GetMergedClusterCount() const;

private:
	// Per-chunk mesh copies for unmerge reconstruction
	TMap<FIntVector, UE::Geometry::FDynamicMesh3> StoredChunkMeshes;

	// All known clusters
	TMap<FIntVector, FVoxelClusterInfo> Clusters;

	FVoxelClusterConfig Config;

	void MergeCluster(FVoxelClusterInfo& Cluster);
	void UnmergeCluster(FVoxelClusterInfo& Cluster);
};

#endif // RIFTBORN_WITH_GEOMETRY_SCRIPTING
