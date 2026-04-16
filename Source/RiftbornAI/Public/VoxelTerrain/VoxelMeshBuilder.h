// Copyright Riftborn. All Rights Reserved.
#pragma once

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "VoxelTerrain/VoxelChunk.h"

// ============================================================
// Async mesh generation request — snapshot of chunk data
// ============================================================

struct RIFTBORNAI_API FVoxelMeshRequest
{
	FIntVector ChunkKey = FIntVector::ZeroValue;
	int32 LODLevel = 0;
	uint32 ContentRevision = 0;
	float CellSize = VOXEL_DEFAULT_CELL_SIZE;
	FVector ChunkWorldOrigin = FVector::ZeroVector;

	// Thread-safe COPIES of chunk data (worker reads, game thread writes original)
	TArray<float> DensitySnapshot;
	TArray<uint8> MaterialSnapshot;

	// Neighbor face density aprons for boundary-aware normals / stitching.
	// Each populated face stores two N*N planes:
	//   plane 0 = neighbor plane nearest the current chunk mesh domain
	//   plane 1 = next plane farther into the neighbor chunk
	// Index by (int32)EVoxelFace. Empty array = no neighbor data available.
	TArray<float> NeighborFaces[6];

	// Priority for queue ordering: squared distance to camera. Lower = higher priority.
	float Priority = 0.0f;
};

// ============================================================
// Async mesh generation result
// ============================================================

struct RIFTBORNAI_API FVoxelMeshResult
{
	FIntVector ChunkKey = FIntVector::ZeroValue;
	int32 LODLevel = 0;
	uint32 ContentRevision = 0;
	UE::Geometry::FDynamicMesh3 Mesh;
	bool bIsEmpty = true;
	int32 TriangleCount = 0;
};

// ============================================================
// FVoxelMeshBuilder — worker pool for off-thread MC extraction
// ============================================================

class RIFTBORNAI_API FVoxelMeshBuilder
{
public:
	explicit FVoxelMeshBuilder(int32 NumWorkers = 2);
	~FVoxelMeshBuilder();

	// Lifecycle
	void Startup();
	void Shutdown();

	// Enqueue a mesh request. Thread-safe. Deduplicates by ChunkKey (replaces
	// existing entry if already queued). Queue is kept sorted by Priority.
	void Enqueue(FVoxelMeshRequest&& Request);

	// Cancel a pending request by chunk key. No-op if already in flight.
	void CancelChunk(const FIntVector& Key);

	// Game thread: drain completed results. Returns number of results moved.
	int32 DequeueResults(TArray<FVoxelMeshResult>& OutResults, int32 MaxResults = 8);

	// Stats
	int32 GetPendingCount() const;
	int32 GetInFlightCount() const;

	// Static mesh building — can be called from any thread.
	static UE::Geometry::FDynamicMesh3 BuildMesh(const FVoxelMeshRequest& Request);

	// Density-gradient normals + material-identity vertex colors.
	// R=PrimaryMatID/255, G=SecondaryMatID/255, B=BlendWeight, A=Depth.
	// Uses neighbor density aprons when sampling just outside the local chunk
	// mesh domain so chunk-edge normals stay continuous.
	static void ApplyGradientNormalsAndColors(
		UE::Geometry::FDynamicMesh3& Mesh,
		const FVoxelMeshRequest& Request);

private:
	// ---- Worker runnable ----
	class FWorker : public FRunnable
	{
	public:
		FWorker(FVoxelMeshBuilder& InOwner) : Owner(InOwner) {}

		virtual uint32 Run() override;
		virtual void Stop() override { bStopRequested.Store(true); }

	private:
		FVoxelMeshBuilder& Owner;
		TAtomic<bool> bStopRequested{false};
	};

	// ---- Queue state ----
	TArray<FVoxelMeshRequest> RequestQueue;
	FCriticalSection RequestLock;

	TArray<FVoxelMeshResult> ResultQueue;
	FCriticalSection ResultLock;

	// ---- Thread pool ----
	TArray<TUniquePtr<FRunnableThread>> WorkerThreads;
	TArray<TSharedPtr<FWorker>> Workers;
	FEvent* WorkEvent = nullptr;

	// ---- Atomics ----
	TAtomic<int32> InFlightCount{0};
	TAtomic<bool> bShuttingDown{false};

	int32 NumWorkers;
};

#endif // RIFTBORN_WITH_GEOMETRY_SCRIPTING
