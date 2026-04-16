// Copyright RiftbornAI. All Rights Reserved.
// ObservationActorRegistry.h — Flat array + spatial grid registry for observation bubble actors.

#pragma once

#include "CoreMinimal.h"

struct FObservationActorState;
enum class EObservationZone : uint8;
class AActor;
class UWorld;

/**
 * Observation Actor Registry
 *
 * Tracks all actors participating in the observation bubble system.
 * Uses a flat contiguous array for cache-friendly iteration, a hash map
 * for O(1) actor-to-index lookup, and a sparse spatial grid for
 * efficient radius queries.
 *
 * NOT a UObject — owned directly by the bubble component.
 * Thread-safety: call only from game thread.
 */
class RIFTBORNAI_API FObservationActorRegistry
{
public:
	FObservationActorRegistry();

	// =========================================================================
	// CONFIGURATION
	// =========================================================================

	/** Cell size in world units (cm). Larger = fewer cells, coarser queries. */
	float GridCellSize = 5000.0f;

	// =========================================================================
	// REGISTRATION
	// =========================================================================

	/** Register a single actor. Returns false if already registered or null. */
	bool RegisterActor(AActor* Actor, EObservationZone InitialZone);

	/** Unregister a single actor. Returns false if not found. */
	bool UnregisterActor(AActor* Actor);

	/** Unregister all actors and clear the grid. */
	void UnregisterAll();

	/** True if the actor is currently tracked. */
	bool IsRegistered(AActor* Actor) const;

	/** Number of currently registered actors. */
	int32 GetRegisteredCount() const;

	// =========================================================================
	// BULK REGISTRATION
	// =========================================================================

	/** Register all actors of a given class in the world. Returns count added. */
	int32 RegisterActorsByClass(UWorld* World, UClass* ActorClass, EObservationZone InitialZone);

	/** Register all actors with a specific gameplay tag. Returns count added. */
	int32 RegisterActorsWithTag(UWorld* World, FName Tag, EObservationZone InitialZone);

	/** Auto-register all meaningful actors, skipping trivial ones. Returns count added. */
	int32 AutoRegisterAll(UWorld* World, EObservationZone InitialZone);

	// =========================================================================
	// ACCESS
	// =========================================================================

	/** Get mutable state for a registered actor. Returns nullptr if not found. */
	FObservationActorState* GetActorState(AActor* Actor);

	/** Get const state for a registered actor. Returns nullptr if not found. */
	const FObservationActorState* GetActorState(AActor* Actor) const;

	/** Direct access to the flat state array (contiguous iteration). */
	TArray<FObservationActorState>& GetAllStates();
	const TArray<FObservationActorState>& GetAllStates() const;

	// =========================================================================
	// SPATIAL GRID
	// =========================================================================

	/** Rebuild the entire spatial grid from current actor positions. */
	void RebuildSpatialGrid();

	/** Get actor indices in a specific grid cell. */
	TArray<int32> GetActorsInCell(int32 CellX, int32 CellY) const;

	/**
	 * Get all actor indices within a world-space radius.
	 * Scans overlapping grid cells, then filters by actual distance.
	 */
	TArray<int32> GetActorsInRadius(const FVector& Center, float Radius) const;

	// =========================================================================
	// MAINTENANCE
	// =========================================================================

	/**
	 * Remove entries whose actor weak pointer is no longer valid.
	 * Returns the number of entries purged.
	 */
	int32 PurgeStaleEntries();

	/** Count actors per observation zone. Key = zone enum cast to uint8. */
	TMap<uint8, int32> GetZoneCounts() const;

private:
	/** Contiguous state storage for cache-friendly iteration. */
	TArray<FObservationActorState> States;

	/** Actor → index in States array. O(1) lookup. */
	TMap<TWeakObjectPtr<AActor>, int32> ActorIndexMap;

	/** Spatial grid: hash(cellX, cellY) → array of indices into States. */
	TMap<uint64, TArray<int32>> SpatialGrid;

	// -- Internal helpers --

	/** Compute grid cell coordinates from a world position. */
	void WorldToCell(const FVector& WorldPos, int32& OutCellX, int32& OutCellY) const;

	/** Hash two cell coordinates into a single uint64 key. */
	static uint64 CellHash(int32 CellX, int32 CellY);

	/** Add an actor index to the spatial grid at the correct cell. */
	void AddToGrid(int32 StateIndex);

	/** Remove an actor index from whatever grid cell it occupies. */
	void RemoveFromGrid(int32 StateIndex);

	/** Returns true if the actor class should be skipped by AutoRegisterAll. */
	static bool IsTrivialActor(const AActor* Actor);
};
