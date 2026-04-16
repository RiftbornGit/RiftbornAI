// Copyright RiftbornAI. All Rights Reserved.
// DeterministicCatchup.h — Resolves world state for actors transitioning Dormant -> Active.
// When a dormant actor re-enters the observation bubble, registered providers predict
// what should have changed during the dormancy period and apply corrections.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UClass;
class UPrimitiveComponent;

// ============================================================================
// PROVIDER INTERFACE
// ============================================================================

/**
 * Interface for catchup providers that resolve dormant-to-active transitions.
 * Each provider handles a specific actor class (or class hierarchy) and
 * predicts what should have changed during dormancy.
 */
class RIFTBORNAI_API IObservationCatchupProvider
{
public:
	virtual ~IObservationCatchupProvider() = default;

	/**
	 * Resolve state changes that should have occurred while the actor was dormant.
	 *
	 * @param Actor                  The actor transitioning to Active.
	 * @param DormantDurationSeconds How long the actor was dormant (seconds).
	 * @param CurrentWorldTime       Current UWorld time for phase synchronization.
	 */
	virtual void ResolveCatchup(AActor* Actor, double DormantDurationSeconds, double CurrentWorldTime) = 0;
};

// ============================================================================
// REGISTRY (SINGLETON)
// ============================================================================

/**
 * Registry mapping UClass -> catchup provider. When an actor wakes from Dormant,
 * ExecuteCatchup looks up the most specific registered provider (exact class first,
 * then walks the super class chain) and calls ResolveCatchup.
 *
 * Not a UObject -- pure C++ singleton with manual lifetime.
 */
class RIFTBORNAI_API FDeterministicCatchupRegistry
{
public:
	static FDeterministicCatchupRegistry& Get();

	/**
	 * Register a catchup provider for a specific actor class.
	 * Overwrites any existing provider for the same class.
	 */
	void RegisterProvider(UClass* ActorClass, TSharedPtr<IObservationCatchupProvider> Provider);

	/** Remove the provider registered for the given class. No-op if none registered. */
	void UnregisterProvider(UClass* ActorClass);

	/**
	 * Find the best matching provider and execute catchup.
	 * Lookup order: exact class match, then walk super chain until a match or nullptr.
	 * If no provider matches, the actor wakes as-is (safe default).
	 *
	 * @return true if a provider was found and executed, false otherwise.
	 */
	bool ExecuteCatchup(AActor* Actor, double DormantDuration, double CurrentWorldTime);

	/** Returns true if a provider is registered for this exact class (does not walk supers). */
	bool HasProvider(UClass* ActorClass) const;

	/** Number of registered providers. */
	int32 NumProviders() const;

private:
	FDeterministicCatchupRegistry();

	TMap<UClass*, TSharedPtr<IObservationCatchupProvider>> Providers;
};

// ============================================================================
// BUILT-IN: PHYSICS CATCHUP
// ============================================================================

/**
 * Catchup for physics-simulating actors. On wake:
 * 1. Line traces downward to find the ground surface
 * 2. Snaps the actor to ground + half-height offset
 * 3. Prevents the "floating actor" problem when physics re-enables
 *
 * v1 does not predict velocity -- only ensures ground contact.
 * Displacement is clamped to MaxCatchupDistance to prevent teleportation.
 */
class RIFTBORNAI_API FPhysicsCatchupProvider : public IObservationCatchupProvider
{
public:
	static constexpr float MaxCatchupDistance = 50000.0f;
	static constexpr float TraceLength = 100000.0f;

	virtual void ResolveCatchup(AActor* Actor, double DormantDurationSeconds, double CurrentWorldTime) override;

private:
	/** Get half-height of the actor's bounding box for ground offset. */
	static float GetActorHalfHeight(AActor* Actor);

	/** Perform a downward line trace from the given origin. */
	static bool TraceToGround(UWorld* World, const FVector& Origin, float Length, FHitResult& OutHit);
};

// ============================================================================
// BUILT-IN: VEGETATION CATCHUP
// ============================================================================

/**
 * Catchup for vegetation actors. Resets wind/time material parameters
 * to the current world time offset, preventing a visible "snap" when
 * vegetation sway re-enters the Active zone.
 *
 * Scans the actor's mesh components for materials with "WindOffset" or
 * "TimeOffset" scalar parameters and sets them to fmod(WorldTime, 1000).
 */
class RIFTBORNAI_API FVegetationCatchupProvider : public IObservationCatchupProvider
{
public:
	virtual void ResolveCatchup(AActor* Actor, double DormantDurationSeconds, double CurrentWorldTime) override;

private:
	/** Try to set a scalar parameter on a material instance. Returns true if found and set. */
	static bool TrySetScalarParam(UMaterialInterface* Material, const FName& ParamName, float Value);
};
