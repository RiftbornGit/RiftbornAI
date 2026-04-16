// Copyright Riftborn. All Rights Reserved.
#pragma once

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING

#include "CoreMinimal.h"
#include "VoxelTerrain/VoxelChunk.h" // VOXEL_MAX_LOD

// ---- LOD distance thresholds ----
struct RIFTBORNAI_API FVoxelLODConfig
{
	// Distance breakpoints per LOD level (cm). LOD 0 = highest detail (closest).
	float LODDistances[VOXEL_MAX_LOD] = {
		10000.f,    // LOD 0 → 1 at 100m
		40000.f,    // LOD 1 → 2 at 400m
		160000.f,   // LOD 2 → 3 at 1.6km
		640000.f,   // LOD 3 → 4 at 6.4km
		2560000.f,  // LOD 4 → 5 at 25.6km
		10240000.f  // LOD 5 → unload at 102.4km
	};

	// Returns LOD level for a given distance.
	// 0 = closest/highest detail. VOXEL_MAX_LOD = beyond all thresholds.
	int32 GetLODForDistance(float DistCm) const
	{
		for (int32 i = 0; i < VOXEL_MAX_LOD; ++i)
		{
			if (DistCm < LODDistances[i])
			{
				return i;
			}
		}
		return VOXEL_MAX_LOD;
	}
};

// ---- Octree node ----
struct RIFTBORNAI_API FVoxelOctreeNode
{
	FBox Bounds;
	int32 LODLevel = 0;
	FIntVector ChunkKey = FIntVector::ZeroValue;
	TUniquePtr<FVoxelOctreeNode> Children[8];
	int32 DesiredLOD = -1;   // -1 = should be unloaded
	bool bMeshCurrent = false;

	FVoxelOctreeNode() : Bounds(ForceInit) {}

	explicit FVoxelOctreeNode(const FBox& InBounds, int32 InLOD, const FIntVector& InKey)
		: Bounds(InBounds)
		, LODLevel(InLOD)
		, ChunkKey(InKey)
	{}

	bool IsLeaf() const
	{
		return Children[0] == nullptr;
	}

	bool HasChildren() const
	{
		return Children[0] != nullptr;
	}

	// Subdivide parent AABB into 8 octants.
	// Octant index bits: bit0=X, bit1=Y, bit2=Z (0=min half, 1=max half).
	FBox GetChildBounds(int32 Octant) const
	{
		check(Octant >= 0 && Octant < 8);

		const FVector Mid = Bounds.GetCenter();
		const FVector& Lo = Bounds.Min;
		const FVector& Hi = Bounds.Max;

		FVector ChildMin, ChildMax;
		ChildMin.X = (Octant & 1) ? Mid.X : Lo.X;
		ChildMax.X = (Octant & 1) ? Hi.X  : Mid.X;
		ChildMin.Y = (Octant & 2) ? Mid.Y : Lo.Y;
		ChildMax.Y = (Octant & 2) ? Hi.Y  : Mid.Y;
		ChildMin.Z = (Octant & 4) ? Mid.Z : Lo.Z;
		ChildMax.Z = (Octant & 4) ? Hi.Z  : Mid.Z;

		return FBox(ChildMin, ChildMax);
	}
};

// ---- Sparse octree for voxel LOD selection ----
class RIFTBORNAI_API FVoxelOctree
{
public:
	void Initialize(const FBox& WorldBounds, float InChunkWorldSize);

	// Traverse from camera position. Returns chunk keys that need meshing and unloading.
	void UpdateLOD(
		const FVector& CameraPos,
		const FVoxelLODConfig& Config,
		TArray<FIntVector>& OutToMesh,
		TArray<FIntVector>& OutToUnload);

	void MarkMeshCurrent(const FIntVector& Key);
	void InvalidateChunk(const FIntVector& Key);
	void InvalidateAll();
	int32 GetNodeCount() const;

private:
	TUniquePtr<FVoxelOctreeNode> Root;
	float ChunkWorldSize = 0.f;
	int32 NodeCount = 0;

	void UpdateNodeLOD(
		FVoxelOctreeNode& Node,
		const FVector& CameraPos,
		const FVoxelLODConfig& Config,
		TArray<FIntVector>& OutToMesh,
		TArray<FIntVector>& OutToUnload);

	void SubdivideNode(FVoxelOctreeNode& Node);
	void CollapseNode(FVoxelOctreeNode& Node, TArray<FIntVector>& OutToUnload);

	// Recursive helpers
	FVoxelOctreeNode* FindNode(FVoxelOctreeNode* Node, const FIntVector& Key);
	int32 CountNodes(const FVoxelOctreeNode* Node) const;
	void InvalidateRecursive(FVoxelOctreeNode* Node);
};

#endif // RIFTBORN_WITH_GEOMETRY_SCRIPTING
