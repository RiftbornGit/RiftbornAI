// Copyright Riftborn AI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornIdentityComponent.generated.h"

/**
 * Quality level of the identity - affects determinism guarantees.
 */
UENUM(BlueprintType)
enum class ERiftbornIdentityQuality : uint8
{
	/** Persistent GUID - full determinism support */
	Persistent UMETA(DisplayName = "Persistent"),
	
	/** Deterministic spawn - derived from spawn context, stable across runs */
	DeterministicSpawn UMETA(DisplayName = "Deterministic Spawn"),
	
	/** Fallback - derived from class/label, weak identity */
	Fallback UMETA(DisplayName = "Fallback"),
	
	/** Ephemeral - runtime-only, excluded from determinism claims */
	Ephemeral UMETA(DisplayName = "Ephemeral")
};

/**
 * URiftbornIdentityComponent - Provides stable actor identity for snapshots.
 * 
 * This component is the keystone for deterministic world snapshotting.
 * Without stable identity, diffs and hashes become noise.
 * 
 * Usage:
 * - Placed actors: Add component in editor, GUID persists in map
 * - Spawned actors: Inject via spawn pipeline with deterministic GUID
 * - Unknown actors: Fallback identity generated from class + label
 * 
 * @see Snapshot Contract v1
 */
UCLASS(ClassGroup=(Riftborn), meta=(BlueprintSpawnableComponent), DisplayName="Riftborn Identity")
class RIFTBORNAI_API URiftbornIdentityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornIdentityComponent();

	// ========== Identity Fields ==========
	
	/**
	 * Persistent GUID for this actor.
	 * - For placed actors: Generated once, saved with map
	 * - For spawned actors: Assigned deterministically from spawn context
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Identity")
	FGuid RiftbornGuid;
	
	/**
	 * Archetype name for grouping (e.g., "Enemy", "Pickup", "Objective").
	 * Optional but useful for queries.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Identity")
	FName RiftbornArchetype;
	
	/**
	 * Spawn epoch - increments on respawn if tracking respawns.
	 * Zero for placed actors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Identity")
	int32 SpawnEpoch = 0;
	
	/**
	 * Quality level of this identity.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Identity")
	ERiftbornIdentityQuality IdentityQuality = ERiftbornIdentityQuality::Persistent;

	// ========== Snapshot Tier Override ==========
	
	/**
	 * Override snapshot tier for this actor.
	 * -1 = use default tier for actor class
	 * 0 = Tier 0 (minimal gameplay)
	 * 1 = Tier 1 (full gameplay)
	 * 2 = Tier 2 (debug)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	int32 SnapshotTierOverride = -1;
	
	/**
	 * If true, exclude this actor from snapshots entirely.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
	bool bExcludeFromSnapshot = false;

	// ========== Methods ==========

	/**
	 * Get the stable identity string for this actor.
	 * Format: "quality:guid" or "quality:fallback_hash"
	 */
	UFUNCTION(BlueprintCallable, Category = "Identity")
	FString GetStableIdentity() const;
	
	/**
	 * Get identity as a sortable key (for canonical ordering).
	 * Returns (quality_rank, identity_string)
	 */
	FString GetSortKey() const;
	
	/**
	 * Ensure GUID is valid. Generates one if needed.
	 * Called automatically on BeginPlay for placed actors.
	 */
	UFUNCTION(BlueprintCallable, Category = "Identity")
	void EnsureGuidValid();
	
	/**
	 * Set identity from deterministic spawn context.
	 * 
	 * @param LevelGuid - GUID of the level/sublevel
	 * @param SpawnPointGuid - GUID of spawn point (or other anchor)
	 * @param ClassPath - Full class path
	 * @param SpawnIndex - Index within this spawn batch
	 */
	UFUNCTION(BlueprintCallable, Category = "Identity")
	void SetDeterministicSpawnIdentity(
		const FGuid& LevelGuid,
		const FGuid& SpawnPointGuid,
		const FString& ClassPath,
		int32 SpawnIndex
	);
	
	/**
	 * Generate fallback identity from actor properties.
	 * Used when no explicit GUID is available.
	 */
	UFUNCTION(BlueprintCallable, Category = "Identity")
	void GenerateFallbackIdentity();

	// ========== Static Helpers ==========
	
	/**
	 * Get or create identity component on an actor.
	 * If actor has no identity, creates one with fallback quality.
	 */
	UFUNCTION(BlueprintCallable, Category = "Identity", meta = (DefaultToSelf = "Actor"))
	static URiftbornIdentityComponent* GetOrCreateIdentity(AActor* Actor);
	
	/**
	 * Get identity component if it exists (no creation).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Identity", meta = (DefaultToSelf = "Actor"))
	static URiftbornIdentityComponent* GetIdentity(AActor* Actor);
	
	/**
	 * Compute deterministic GUID from spawn context.
	 */
	static FGuid ComputeSpawnGuid(
		const FGuid& LevelGuid,
		const FGuid& SpawnPointGuid,
		const FString& ClassPath,
		int32 SpawnIndex
	);

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** Compute fallback hash from actor properties */
	FGuid ComputeFallbackGuid() const;
};
