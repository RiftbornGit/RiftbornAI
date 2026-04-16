// Copyright RiftbornAI. All Rights Reserved.
// ObservationStateTransitioner.h — Applies simulation fidelity changes when actors
// transition between observation zones. Each subsystem (tick, physics, VFX, AI, audio)
// is toggled independently so the bubble can degrade gracefully.

#pragma once

#include "CoreMinimal.h"
#include "Awareness/ObservationBubbleTypes.h"

// Forward declarations
class AActor;
class UPrimitiveComponent;

// ============================================================================
// PENDING PHYSICS TOGGLE
// ============================================================================

/** Deferred physics state change. Batched so we can flush them all at once
 *  rather than thrashing Chaos wake/sleep per-actor during mass transitions. */
struct FPendingPhysicsToggle
{
	TWeakObjectPtr<AActor> Actor;
	EObservationZone TargetZone = EObservationZone::Dormant;
};

// ============================================================================
// DELEGATE
// ============================================================================

/** Broadcast after an actor completes its zone transition. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBubbleTransition, const FBubbleTransitionEvent& /*Event*/);

// ============================================================================
// OBSERVATION STATE TRANSITIONER
// ============================================================================

/**
 * Applies simulation fidelity changes when actors move between observation zones.
 *
 * Each subsystem is a static helper so it can be called individually (e.g. by
 * tests or gameplay code that needs to force a specific state). The main
 * TransitionActor() method calls all subsystems in order.
 *
 * Physics changes are queued rather than applied immediately because waking
 * and sleeping rigid bodies in Chaos is cheaper when batched. Call
 * BeginBatchTransition() before processing a frame's worth of transitions,
 * then CommitBatchTransition() to flush.
 */
class RIFTBORNAI_API FObservationStateTransitioner
{
public:

	// ----------------------------------------------------------------
	// Single-actor transition
	// ----------------------------------------------------------------

	/**
	 * Transition a single actor from one zone to another.
	 * Applies tick, physics (queued), VFX, AI, and audio state changes,
	 * then broadcasts OnTransition.
	 */
	void TransitionActor(AActor* Actor, EObservationZone FromZone, EObservationZone ToZone);

	// ----------------------------------------------------------------
	// Batch API — use when processing multiple transitions per frame
	// ----------------------------------------------------------------

	/** Enter batching mode. Physics toggles are queued until CommitBatchTransition(). */
	void BeginBatchTransition();

	/** Queue a transition for batch processing. Identical to TransitionActor but
	 *  defers the physics flush until CommitBatchTransition(). */
	void QueueTransition(AActor* Actor, EObservationZone FromZone, EObservationZone ToZone);

	/** Flush all queued physics toggles and exit batching mode. */
	void CommitBatchTransition();

	// ----------------------------------------------------------------
	// Per-subsystem controls (static — usable standalone)
	// ----------------------------------------------------------------

	/** Enable/disable actor + component ticking and set tick interval. */
	static void SetTickState(AActor* Actor, EObservationZone Zone);

	/** Set physics simulation state. In batch mode this is deferred;
	 *  call FlushPhysicsToggles() or CommitBatchTransition() to apply. */
	static void SetPhysicsState(AActor* Actor, EObservationZone Zone);

	/** Activate/deactivate Niagara particle systems. */
	static void SetVFXState(AActor* Actor, EObservationZone Zone);

	/** Resume/pause/stop AI brain logic. */
	static void SetAIState(AActor* Actor, EObservationZone Zone);

	/** Adjust audio volume multipliers per zone. */
	static void SetAudioState(AActor* Actor, EObservationZone Zone);

	// ----------------------------------------------------------------
	// Delegate
	// ----------------------------------------------------------------

	/** Fired after each actor transition completes. */
	FOnBubbleTransition OnTransition;

private:

	/** Pending physics state changes accumulated during a batch. */
	TArray<FPendingPhysicsToggle> PendingPhysicsToggles;

	/** True while inside a BeginBatchTransition / CommitBatchTransition pair. */
	bool bBatching = false;

	/** Apply all pending physics toggles and clear the queue. */
	void FlushPhysicsToggles();

	/** Internal: queue a physics toggle for later flush. */
	void EnqueuePhysicsToggle(AActor* Actor, EObservationZone Zone);

	/** Internal: apply a single physics toggle immediately. */
	static void ApplyPhysicsState(AActor* Actor, EObservationZone Zone);
};
