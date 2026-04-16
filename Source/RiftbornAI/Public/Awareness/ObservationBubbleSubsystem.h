// Copyright RiftbornAI. All Rights Reserved.
// ObservationBubbleSubsystem.h — World subsystem orchestrating the observation bubble.
// Drives zone classification, state transitions, radius adaptation, and catchup for all
// registered actors. One instance per game world / PIE session.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Awareness/ObservationBubbleTypes.h"
#include "Awareness/ObservationActorRegistry.h"
#include "Awareness/ObservationStateTransitioner.h"
#include "ObservationBubbleSubsystem.generated.h"

class AActor;

/**
 * Observation Bubble World Subsystem
 *
 * Manages a camera-centered observation bubble that controls simulation fidelity
 * for all registered actors. Actors close to the camera run at full fidelity
 * (Active), those further away run reduced (Peripheral), and distant actors
 * are paused entirely (Dormant) with deterministic catchup on re-entry.
 *
 * Ticks via FWorldDelegates::OnWorldTickStart rather than UTickableWorldSubsystem
 * so tick budget enforcement does not interact with the engine's tick groups.
 *
 * The bubble radius adapts automatically based on FPS headroom (Performance mode)
 * using a PID controller to maintain the configured target frame rate.
 */
UCLASS()
class RIFTBORNAI_API UObservationBubbleSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	// =========================================================================
	// USubsystem interface
	// =========================================================================

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// =========================================================================
	// Configuration
	// =========================================================================

	/** Replace the bubble configuration. Takes effect next tick. */
	void SetConfig(const FBubbleConfig& InConfig);

	/** Get the current configuration (const ref). */
	const FBubbleConfig& GetConfig() const { return Config; }

	// =========================================================================
	// Enable / Disable
	// =========================================================================

	/** Enable or disable the observation bubble. Enabling auto-registers actors if none exist. */
	void SetEnabled(bool bInEnabled);

	/** True if the bubble is currently active. */
	bool IsEnabled() const { return bEnabled; }

	// =========================================================================
	// Runtime queries
	// =========================================================================

	/** Current adaptive radius scale factor (1.0 = configured radii, <1 = shrunk, >1 = expanded). */
	float GetCurrentRadiusScale() const { return CurrentRadiusScale; }

	/** Per-frame statistics snapshot. */
	const FBubbleStats& GetStats() const { return Stats; }

	/** Direct access to the actor registry for registration / spatial queries. */
	FObservationActorRegistry& GetActorRegistry() { return ActorRegistry; }
	const FObservationActorRegistry& GetActorRegistry() const { return ActorRegistry; }

	/** Get the observation zone for a specific actor, or Unloaded if unregistered. */
	EObservationZone GetActorZone(AActor* Actor) const;

	/** Get all registered actors currently in a given zone. */
	TArray<AActor*> GetActorsInZone(EObservationZone Zone) const;

	// =========================================================================
	// Delegates
	// =========================================================================

	/** Access the transition delegate for external listeners. */
	FOnBubbleTransition& OnTransition() { return StateTransitioner.OnTransition; }

	// =========================================================================
	// Manual controls
	// =========================================================================

	/** Force a full evaluation of all actors this frame (ignores tick budget). */
	void ForceFullEvaluation();

	/** Draw debug visualization of the bubble zones in the viewport. */
	void DrawDebugBubble(float Duration = 0.0f) const;

private:

	// =========================================================================
	// Tick entry point (bound to FWorldDelegates::OnWorldTickStart)
	// =========================================================================

	void Tick(UWorld* InWorld, ELevelTick TickType, float DeltaTime);

	// =========================================================================
	// Core loop stages
	// =========================================================================

	/** Read camera location, forward vector, and FOV from the active viewport. */
	void UpdateCameraState();

	/** PID controller: adapt bubble radius based on FPS vs target. */
	void AdaptBubbleRadius(float DeltaTime);

	/** Classify actors into zones (amortized across frames). */
	void EvaluateActorZones(float DeltaTime);

	/** Sort, cap, and commit pending zone transitions. */
	void ProcessTransitions();

	/** Run deterministic catchup for actors waking from Dormant. */
	void ExecuteCatchups();

	// =========================================================================
	// State
	// =========================================================================

	UPROPERTY()
	FBubbleConfig Config;

	bool bEnabled = false;
	bool bForceFullEvaluation = false;
	float CurrentRadiusScale = 1.0f;
	FBubbleStats Stats;

	FObservationActorRegistry ActorRegistry;
	FObservationStateTransitioner StateTransitioner;
	FDelegateHandle TickDelegateHandle;

	// Cached camera state
	FVector CachedCameraLocation = FVector::ZeroVector;
	FVector CachedCameraForward = FVector::ForwardVector;
	float CachedCameraFOV = 90.0f;

	// PID controller state for radius adaptation
	float FPSErrorIntegral = 0.0f;
	float PreviousFPSError = 0.0f;

	// Amortized evaluation cursor (index into actor states array)
	int32 EvaluationCursor = 0;

	// Transitions queued during EvaluateActorZones, flushed in ProcessTransitions
	TArray<FBubbleTransitionEvent> PendingTransitions;
};
