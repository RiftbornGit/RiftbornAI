// RiftLumenTypes.h
// Core configuration for the RiftLumen natural light source engine.
// RiftLumen drives real UE5 lights with physically-motivated behavior —
// fire flickers, candles have blackbody spectrum, lightning flashes and decays.
// Lumen handles the actual GI rendering.

#pragma once

#include "CoreMinimal.h"

// Global environment settings for the RiftLumen light source system
struct RIFTBORNAI_API FRiftLumenConfig
{
	// Sky/sun defaults (used by Sun/Moon source archetypes)
	FLinearColor SkyColor;
	FVector SunDirection;
	float SunIntensity;

	// Global scales applied to all RiftLumen-managed lights
	float IntensityScale;       // Master intensity multiplier for all sources
	int32 MaxManagedLights;     // Hard cap on total UE5 light components spawned

	FRiftLumenConfig()
		: SkyColor(0.4f, 0.6f, 0.9f)
		, SunDirection(FVector(0.5f, 0.3f, -0.8f).GetSafeNormal())
		, SunIntensity(3.0f)
		, IntensityScale(1.0f)
		, MaxManagedLights(64)
	{}

	static FRiftLumenConfig MakeDefault();
};
