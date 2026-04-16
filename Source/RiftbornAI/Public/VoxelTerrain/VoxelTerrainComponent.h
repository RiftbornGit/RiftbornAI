// Copyright Riftborn. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelTerrainComponent.generated.h"

class UDynamicMeshComponent;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
class UTexture2D;
class UWorld;
class ALandscapeProxy;
class UVoxelStampComponent;
class UVoxelCollisionInvokerComponent;
#if RIFTBORN_WITH_GEOMETRY_SCRIPTING
#include "VoxelTerrain/VoxelChunk.h"
#include "VoxelTerrain/VoxelOctree.h"
#include "VoxelTerrain/VoxelMeshBuilder.h"
#include "VoxelTerrain/VoxelStreamingManager.h"
#include "VoxelTerrain/VoxelClusterMerger.h"
#include "VoxelTerrain/VoxelEditLog.h"
#endif

class UVoxelMaterialPalette;

UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API UVoxelTerrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelTerrainComponent();
	~UVoxelTerrainComponent();

	// World-space size of each voxel cell (cm). Smaller = more detail, more memory.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain",
		meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float CellSize = VOXEL_DEFAULT_CELL_SIZE;

	// Material applied to all generated voxel mesh chunks.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain")
	TObjectPtr<UMaterialInterface> TerrainMaterial;

	// ---- LOD ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|LOD")
	bool bEnableLOD = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|LOD", meta = (EditCondition = "bEnableLOD"))
	float LOD0Distance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|LOD", meta = (EditCondition = "bEnableLOD"))
	float LOD1Distance = 40000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|LOD", meta = (EditCondition = "bEnableLOD"))
	float LOD2Distance = 160000.0f;

	// ---- Streaming ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Streaming")
	bool bEnableStreaming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Streaming", meta = (EditCondition = "bEnableStreaming"))
	float StreamingInnerExtent = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Streaming", meta = (EditCondition = "bEnableStreaming"))
	float StreamingOuterExtent = 24000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Streaming", meta = (EditCondition = "bEnableStreaming", ClampMin = "50", ClampMax = "2000"))
	int32 MaxLoadedChunks = 500;

	// ---- Performance ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Performance")
	bool bEnableAsyncMeshing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Performance", meta = (EditCondition = "bEnableAsyncMeshing", ClampMin = "1", ClampMax = "8"))
	int32 MeshWorkerThreads = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Performance", meta = (ClampMin = "1", ClampMax = "32"))
	int32 MaxMeshUpdatesPerFrame = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Performance")
	bool bEnableClusterMerging = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Performance", meta = (EditCondition = "bEnableClusterMerging", ClampMin = "2", ClampMax = "8"))
	int32 ClusterSize = 4;

	// ---- Collision ----
	// When true, collision only generates on chunks near VoxelCollisionInvokerComponents.
	// When false (default), all chunks get collision. Enable for large worlds.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Collision")
	bool bUseCollisionInvokers = false;

	// ---- Heightmap ----
	// Optional heightmap texture for large-scale terrain shape.
	// When set, voxel density is derived from the heightmap with noise detail on top.
	// Without it, terrain is purely procedural noise.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Heightmap")
	TObjectPtr<UTexture2D> HeightmapTexture;

	// World-space origin XY of the heightmap (cm). The heightmap covers from
	// this point to (Origin + Extent).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Heightmap",
		meta = (EditCondition = "HeightmapTexture != nullptr"))
	FVector2D HeightmapWorldOrigin = FVector2D::ZeroVector;

	// World-space extent XY that the heightmap covers (cm).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Heightmap",
		meta = (EditCondition = "HeightmapTexture != nullptr"))
	FVector2D HeightmapWorldExtent = FVector2D(100000.0, 100000.0);

	// Height scale: heightmap pixel value (0-1) is multiplied by this to get cm.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Heightmap",
		meta = (EditCondition = "HeightmapTexture != nullptr", ClampMin = "1.0", ClampMax = "100000.0"))
	float HeightmapScale = 5000.0f;

	// Height offset (cm) added to heightmap values.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Heightmap",
		meta = (EditCondition = "HeightmapTexture != nullptr"))
	float HeightmapOffset = 0.0f;

	// ---- Materials ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Materials")
	TObjectPtr<UVoxelMaterialPalette> MaterialPalette;

	// ---- Persistence ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Persistence")
	bool bAutoSaveEdits = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Terrain|Persistence")
	FString SaveFilePath;

	// ---- Sculpting ----

	// Dig a spherical hole at the given world position.
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void DigSphere(FVector Center, float Radius);

	// Add terrain material in a sphere (fill holes back in).
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void RaiseSphere(FVector Center, float Radius);

	// Smooth terrain in a sphere region.
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void SmoothSphere(FVector Center, float Radius, float Strength = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void RaiseSphereWithMaterial(FVector Center, float Radius, uint8 MaterialID);

	// Dig a box-shaped hole at the given world position.
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void DigBox(FVector Center, FVector Extent);

	// Add terrain in a box shape (fill holes back in).
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void RaiseBox(FVector Center, FVector Extent);

	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	bool SaveTerrain();

	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	bool LoadTerrain();

	// Reconfigure runtime terrain resolution. Clears generated chunk state so the
	// next generation pass uses the new cell size honestly.
	void ReconfigureForCellSize(float NewCellSize);

	UFUNCTION(BlueprintPure, Category = "Voxel Terrain")
	int32 GetPendingMeshBuildCount() const;

	UFUNCTION(BlueprintPure, Category = "Voxel Terrain")
	int32 GetMergedClusterCount() const;

	// ---- Stamps ----

	// Register a stamp to influence terrain generation. Stamps are evaluated
	// during chunk generation and composited by priority order.
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain|Stamps")
	void RegisterStamp(UVoxelStampComponent* Stamp);

	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain|Stamps")
	void UnregisterStamp(UVoxelStampComponent* Stamp);

	// Regenerate all chunks affected by stamps (call after adding/moving stamps)
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain|Stamps")
	void RegenerateStampArea(FVector Center, float Radius);

	// ---- Query API ----

	// Sample the density field at a world position. Positive = solid, negative = air.
	UFUNCTION(BlueprintPure, Category = "Voxel Terrain|Query")
	float SampleDensityAtWorldPos(FVector WorldPos) const;

	// Get the material layer ID at a world position (0-7).
	UFUNCTION(BlueprintPure, Category = "Voxel Terrain|Query")
	uint8 SampleMaterialAtWorldPos(FVector WorldPos) const;

	// Find the terrain surface Z height at the given XY world position.
	// Traces downward from StartZ. Returns false if no surface found.
	UFUNCTION(BlueprintPure, Category = "Voxel Terrain|Query")
	bool FindSurfaceZ(float WorldX, float WorldY, float StartZ, float& OutZ) const;

	// ---- Terrain Generation ----

	// Generate procedural terrain from noise in a world-space region.
	// ExtentXY/ExtentZ define the half-size of the area to generate.
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain")
	void GenerateNoiseTerrainArea(FVector Center, float ExtentXY, float ExtentZ);

	// ---- Queries ----

	UFUNCTION(BlueprintPure, Category = "Voxel Terrain")
	int32 GetActiveChunkCount() const;

	UFUNCTION(BlueprintPure, Category = "Voxel Terrain")
	int32 GetTotalTriangleCount() const;

	// ---- Noise parameters (exposed for tuning) ----
#if RIFTBORN_WITH_GEOMETRY_SCRIPTING
	FVoxelNoiseParams NoiseParams;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TWeakObjectPtr<ALandscapeProxy> CachedLandscape;

	// Registered stamps sorted by priority (outside #if so UHT sees it)
	UPROPERTY()
	TArray<TWeakObjectPtr<UVoxelStampComponent>> RegisteredStamps;

	// If true, use noise-based terrain instead of landscape height.
	// Outside #if guard so UHT serializes it — PIE needs this to auto-regenerate.
	UPROPERTY()
	bool bUseNoiseTerrain = false;

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING
	float GetChunkWorldSize() const;
	FIntVector WorldToChunkKey(const FVector& WorldPos) const;
	FVector ChunkKeyToWorldOrigin(const FIntVector& Key) const;
	FVoxelChunk& EnsureChunk(const FIntVector& Key);
	FVoxelChunk& EnsureChunkFromNoise(const FIntVector& Key);
	void RebuildChunkMesh(const FIntVector& Key, const UE::Geometry::FDynamicMesh3* PrebuiltMesh = nullptr, int32 LODLevel = 0);
	void PopulateMeshRequestNeighborFaces(const FIntVector& Key, FVoxelMeshRequest& Request) const;
	ALandscapeProxy* FindLandscape() const;
	float SampleLandscapeHeight(float WorldX, float WorldY) const;
	void ResetRuntimeTerrainState();

	TMap<FIntVector, TUniquePtr<FVoxelChunk>> Chunks;
	TMap<FIntVector, TObjectPtr<UStaticMeshComponent>> ChunkMeshes;
	TMap<FIntVector, TObjectPtr<UStaticMesh>> ChunkStaticMeshes;

	TUniquePtr<FVoxelOctree> Octree;
	TUniquePtr<FVoxelMeshBuilder> MeshBuilder;
	TUniquePtr<FVoxelStreamingManager> StreamingManager;
	TUniquePtr<FVoxelClusterMerger> ClusterMerger;
	TUniquePtr<FVoxelEditLog> EditLog;
	TMap<FIntVector, TObjectPtr<UDynamicMeshComponent>> ClusterMeshComponents;
	TMap<FIntVector, int32> ChunkMeshLODLevels;
	TMap<FIntVector, int32> PendingChunkLODLevels;

	void TickStreaming(float DeltaTime);
	void TickAsyncMeshResults();
	void TickClusterMerging();
	void TickLOD();
	void RecordEdit(EVoxelEditType Type, const FVector& Center, float Radius, float Strength = 0.0f, uint8 MaterialID = 0);
	void RecordBoxEdit(EVoxelEditType Type, const FVector& Center, const FVector& Extent);
	FVector GetCameraWorldPosition() const;

	// Heightmap CPU data — read once from texture on BeginPlay, reused for chunk generation.
	TArray<float> HeightmapCPUData;
	int32 HeightmapResX = 0;
	int32 HeightmapResY = 0;
	bool ReadHeightmapToCPU();
	FVoxelChunk& EnsureChunkFromHeightmapAndNoise(const FIntVector& Key);
	FVoxelChunk& EnsureChunkWithStamps(const FIntVector& Key);
	void ApplyStampsToChunk(FVoxelChunk& Chunk, const FVector& ChunkWorldOrigin);
#endif
};
