// RiftLumenNoise.h
// Temporal noise profiles for natural light source behavior.
// Each profile modulates intensity and/or position over time.

#pragma once

#include "CoreMinimal.h"
#include "RiftLumenNoise.generated.h"

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftLumenNoiseProfile
{
	GENERATED_BODY()

	// Base oscillation frequency in Hz
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen", meta = (ClampMin = "0", ClampMax = "60"))
	float BaseFrequency = 1.f;

	// Intensity modulation depth (0 = steady, 0.3 = 30% variation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen", meta = (ClampMin = "0", ClampMax = "1"))
	float Amplitude = 0.1f;

	// Chaos factor — higher values add more random variation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen", meta = (ClampMin = "0", ClampMax = "1"))
	float Turbulence = 0.f;

	// Noise octave layers for detail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen", meta = (ClampMin = "1", ClampMax = "4"))
	int32 Octaves = 1;

	// Spatial wobble in cm (flame tip sway, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen", meta = (ClampMin = "0"))
	float PositionJitter = 0.f;

	// Upward drift bias (cm/s) — flames rise, not just wobble
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen", meta = (ClampMin = "0"))
	float UpwardDrift = 0.f;

	// Wind sway strength (cm) — shared lateral push across all micro-lights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen", meta = (ClampMin = "0"))
	float WindSway = 0.f;

	// Wind sway frequency (Hz) — how fast wind gusts change direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen", meta = (ClampMin = "0"))
	float WindFrequency = 0.3f;

	// Evaluate intensity multiplier at time T.
	// Returns value in [1 - Amplitude, 1 + Amplitude].
	float Evaluate(float TimeSeconds, int32 Seed = 0) const;

	// Evaluate position offset at time T (basic wobble).
	FVector EvaluatePositionOffset(float TimeSeconds, int32 Seed = 0) const;

	// Evaluate fire-aware position offset.
	// HeightFactor (0=base, 1=tip): controls how much the flame sways at this height.
	// SharedWindSeed: all micro-lights in one fire should use the same seed for correlated wind.
	FVector EvaluateFireOffset(float TimeSeconds, int32 Seed, float HeightFactor, int32 SharedWindSeed = 0) const;

	// ---- Presets ----
	static FRiftLumenNoiseProfile CandleFlicker();
	static FRiftLumenNoiseProfile FireFlicker();
	static FRiftLumenNoiseProfile TorchFlicker();
	static FRiftLumenNoiseProfile LightningFlash();
	static FRiftLumenNoiseProfile LavaPulse();
	static FRiftLumenNoiseProfile Steady();
};
