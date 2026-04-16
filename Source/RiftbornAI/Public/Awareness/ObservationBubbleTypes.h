// Copyright RiftbornAI. All Rights Reserved.
// ObservationBubbleTypes.h — Core data types for the Observation Bubble system.
// Physics, AI, VFX, and ticking only simulate within the player's observation range.

#pragma once

#include "CoreMinimal.h"
#include "ObservationBubbleTypes.generated.h"

// ============================================================================
// ZONE CLASSIFICATION
// ============================================================================

/** Observation zone determines simulation fidelity for an actor. */
UENUM(BlueprintType)
enum class EObservationZone : uint8
{
	Active,       // In camera frustum + near range — full physics, full AI, full VFX
	Peripheral,   // Loaded, within bubble, outside frustum — reduced tick, sleeping physics
	Dormant,      // Loaded, outside bubble — no tick, no physics, state machine only
	Unloaded      // Not in memory — World Partition handles this automatically
};

/** How the bubble radius adapts at runtime. */
UENUM(BlueprintType)
enum class EBubbleAdaptationMode : uint8
{
	Fixed,        // Bubble radius is constant
	Performance,  // Adapts to FPS headroom (shrink when slow, expand when fast)
	Manual        // Controlled entirely via tool calls
};

// ============================================================================
// CONFIGURATION
// ============================================================================

/** Configuration for the observation bubble. All distances in centimeters. */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBubbleConfig
{
	GENERATED_BODY()

	/** Radius of the Active zone (full simulation). Actors in frustum within this range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble")
	float ActiveRadius = 5000.0f;

	/** Radius of the Peripheral zone (reduced simulation). Behind camera but still loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble")
	float PeripheralRadius = 10000.0f;

	/** Radius of the Dormant zone boundary. Beyond this, World Partition may unload. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble")
	float DormantRadius = 25000.0f;

	/** Extra degrees beyond camera FOV for the Active frustum cone. Prevents pop-in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble", meta = (ClampMin = "0", ClampMax = "45"))
	float FrustumPaddingDeg = 15.0f;

	/** Distance buffer at zone boundaries to prevent flickering transitions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble")
	float HysteresisDistance = 500.0f;

	/** How fast the radius adapts in Performance mode (0-1 per second). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float AdaptationRate = 0.1f;

	/** Target FPS for Performance adaptation mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble", meta = (ClampMin = "15", ClampMax = "240"))
	float TargetFPS = 60.0f;

	/** Minimum adaptive radius scale (fraction of configured radii). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinRadiusScale = 0.5f;

	/** Maximum adaptive radius scale (fraction of configured radii). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float MaxRadiusScale = 1.5f;

	/** Adaptation mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble")
	EBubbleAdaptationMode Mode = EBubbleAdaptationMode::Performance;

	/** Maximum milliseconds per frame for bubble evaluation. Excess actors amortize to next frame. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble", meta = (ClampMin = "0.5", ClampMax = "8.0"))
	float TickBudgetMs = 2.0f;

	/** Maximum zone transitions per frame. Prevents frame spikes from mass transitions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObservationBubble", meta = (ClampMin = "1", ClampMax = "256"))
	int32 MaxTransitionsPerFrame = 64;
};

// ============================================================================
// PER-ACTOR STATE
// ============================================================================

/** Tracked state for a single actor in the observation bubble. */
USTRUCT()
struct FObservationActorState
{
	GENERATED_BODY()

	/** The tracked actor (weak ref — may be destroyed). */
	TWeakObjectPtr<AActor> Actor;

	/** Current observation zone. */
	EObservationZone CurrentZone = EObservationZone::Unloaded;

	/** Previous zone (for transition detection). */
	EObservationZone PreviousZone = EObservationZone::Unloaded;

	/** World time when the actor last transitioned zones. */
	double LastTransitionTime = 0.0;

	/** World time when the actor was last in the Active zone. Used for catchup duration. */
	double LastActiveTime = 0.0;

	/** Cached distance to camera (updated each evaluation). */
	float DistanceToCamera = MAX_FLT;

	/** Dot product of (actor-camera) direction vs camera forward. 1 = directly ahead, -1 = behind. */
	float DotToCamera = 0.0f;

	/** Spatial grid cell index for fast spatial queries. */
	uint32 SpatialCellIndex = 0;

	/** Cached world location (avoid re-reading transform every frame). */
	FVector CachedLocation = FVector::ZeroVector;

	/** Set when transitioning Dormant→Active, cleared after catchup executes. */
	bool bNeedsCatchup = false;

	/** Hero/GameplayCritical actors never go Dormant. */
	bool bIsProtected = false;

	/** Returns true if the actor reference is still valid. */
	bool IsValid() const { return Actor.IsValid(); }
};

// ============================================================================
// EVENTS
// ============================================================================

/** Fired when an actor transitions between observation zones. */
USTRUCT()
struct FBubbleTransitionEvent
{
	GENERATED_BODY()

	UPROPERTY() TWeakObjectPtr<AActor> Actor;
	EObservationZone FromZone = EObservationZone::Unloaded;
	EObservationZone ToZone = EObservationZone::Unloaded;
	double Timestamp = 0.0;
};

// ============================================================================
// STATISTICS
// ============================================================================

/** Per-frame bubble statistics. */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBubbleStats
{
	GENERATED_BODY()

	UPROPERTY() int32 TotalRegistered = 0;
	UPROPERTY() int32 ActiveCount = 0;
	UPROPERTY() int32 PeripheralCount = 0;
	UPROPERTY() int32 DormantCount = 0;
	UPROPERTY() int32 TransitionsThisFrame = 0;
	UPROPERTY() int32 CatchupsThisFrame = 0;
	UPROPERTY() float EvaluationTimeMs = 0.0f;
	UPROPERTY() float CurrentRadiusScale = 1.0f;
	UPROPERTY() float CurrentFPS = 60.0f;
	UPROPERTY() int32 EvaluationCursor = 0;
};

// ============================================================================
// UTILITY
// ============================================================================

namespace ObservationBubble
{
	inline const TCHAR* ZoneToString(EObservationZone Zone)
	{
		switch (Zone)
		{
		case EObservationZone::Active:     return TEXT("Active");
		case EObservationZone::Peripheral: return TEXT("Peripheral");
		case EObservationZone::Dormant:    return TEXT("Dormant");
		case EObservationZone::Unloaded:   return TEXT("Unloaded");
		default:                           return TEXT("Unknown");
		}
	}

	inline const TCHAR* ModeToString(EBubbleAdaptationMode Mode)
	{
		switch (Mode)
		{
		case EBubbleAdaptationMode::Fixed:       return TEXT("Fixed");
		case EBubbleAdaptationMode::Performance: return TEXT("Performance");
		case EBubbleAdaptationMode::Manual:      return TEXT("Manual");
		default:                                  return TEXT("Unknown");
		}
	}
}
