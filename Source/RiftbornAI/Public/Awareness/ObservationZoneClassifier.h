// Copyright RiftbornAI. All Rights Reserved.
// ObservationZoneClassifier.h — Stateless, pure-functional zone classification.
// Determines which observation zone an actor belongs to based on distance and frustum.

#pragma once

#include "CoreMinimal.h"
#include "Awareness/ObservationBubbleTypes.h"

struct FObservationZoneInput
{
	FVector ActorLocation;
	EObservationZone CurrentZone;
	bool bIsProtected;
};

struct FObservationZoneResult
{
	EObservationZone Zone;
	float Distance;
	float DotProduct;
};

/**
 * Stateless zone classifier. All methods are static and side-effect free.
 *
 * Zone logic:
 *   Active     — within ActiveRadius AND inside frustum cone
 *   Peripheral — within PeripheralRadius but outside frustum (or outside ActiveRadius)
 *   Dormant    — beyond PeripheralRadius (protected actors clamp to Peripheral)
 *
 * Hysteresis prevents flickering at zone boundaries by requiring actors to
 * overshoot the threshold by HysteresisDistance before transitioning.
 */
class RIFTBORNAI_API FObservationZoneClassifier
{
public:
	FObservationZoneClassifier() = delete;

	/**
	 * Classify a single actor into an observation zone.
	 *
	 * @param ActorLocation   World position of the actor.
	 * @param CameraLocation  World position of the camera/player.
	 * @param CameraForward   Normalized forward vector of the camera.
	 * @param CameraFOVDeg    Horizontal field of view in degrees.
	 * @param Config          Bubble configuration (radii, hysteresis, padding).
	 * @param RadiusScale     Runtime scale factor for all radii (from adaptation).
	 * @param CurrentZone     Actor's current zone (for hysteresis).
	 * @param bIsProtected    If true, actor never enters Dormant.
	 * @return The classified zone.
	 */
	static EObservationZone Classify(
		const FVector& ActorLocation,
		const FVector& CameraLocation,
		const FVector& CameraForward,
		float CameraFOVDeg,
		const FBubbleConfig& Config,
		float RadiusScale,
		EObservationZone CurrentZone,
		bool bIsProtected);

	/**
	 * Classify a batch of actors for cache-friendly contiguous processing.
	 * Precomputes shared values (cos threshold) once outside the loop.
	 *
	 * @param Inputs     Array of actor locations, current zones, and protection flags.
	 * @param CameraLocation  World position of the camera.
	 * @param CameraForward   Normalized forward vector of the camera.
	 * @param CameraFOVDeg    Horizontal field of view in degrees.
	 * @param Config          Bubble configuration.
	 * @param RadiusScale     Runtime scale factor for all radii.
	 * @param OutResults      Populated with zone, distance, and dot product per actor.
	 */
	static void ClassifyBatch(
		const TArray<FObservationZoneInput>& Inputs,
		const FVector& CameraLocation,
		const FVector& CameraForward,
		float CameraFOVDeg,
		const FBubbleConfig& Config,
		float RadiusScale,
		TArray<FObservationZoneResult>& OutResults);

private:
	/** Cheap cone check: is the actor within the padded camera frustum cone? */
	static bool IsInFrustumCone(
		const FVector& ActorLocation,
		const FVector& CameraLocation,
		const FVector& CameraForward,
		float CosHalfFOVPadded);

	/**
	 * Apply hysteresis to prevent zone flickering at boundaries.
	 * Transitions to a "lower" zone require overshooting outward by HysteresisDistance.
	 * Transitions to a "higher" zone require being within the threshold by HysteresisDistance.
	 */
	static EObservationZone ApplyHysteresis(
		EObservationZone CandidateZone,
		EObservationZone CurrentZone,
		float Distance,
		float ActiveRadius,
		float PeripheralRadius,
		float DormantRadius,
		float HysteresisDistance);

	/** Zone priority: Active=2, Peripheral=1, Dormant=0, Unloaded=-1 */
	static int32 ZonePriority(EObservationZone Zone);
};
