// RiftLumenBlackbody.h
// Planck's law: temperature in Kelvin → linear color.
// Named constants for real-world light source temperatures.

#pragma once

#include "CoreMinimal.h"

struct RIFTBORNAI_API FRiftLumenBlackbody
{
	// Convert color temperature to linear RGB via UE5's built-in Planck approximation
	static FLinearColor FromTemperature(float Kelvin);

	// Named temperature constants (Kelvin)
	static constexpr float TemperatureLava        = 1200.f;  // Barely visible dark red
	static constexpr float TemperatureCandle       = 1800.f;  // Warm orange
	static constexpr float TemperatureFire         = 1900.f;  // Orange-yellow
	static constexpr float TemperatureSunset       = 3500.f;  // Warm amber
	static constexpr float TemperatureTungsten     = 2700.f;  // Classic incandescent bulb
	static constexpr float TemperatureHalogen      = 3200.f;  // Slightly cooler bulb
	static constexpr float TemperatureFluorescent  = 4000.f;  // Cool white
	static constexpr float TemperatureDaylight     = 6500.f;  // Noon sunlight / overcast
	static constexpr float TemperatureOvercast     = 7500.f;  // Cloudy sky
	static constexpr float TemperatureMoonlight    = 4100.f;  // Reflected sunlight
	static constexpr float TemperatureLightning    = 30000.f; // Blue-white flash
	static constexpr float TemperatureNeon         = 0.f;     // Special: user-defined color, not blackbody
	static constexpr float TemperatureSunriseClear = 5000.f;  // Clear-sky sun near the horizon
	static constexpr float TemperatureSunriseDusty = 4200.f;  // Hazy / dusty sunrise, not default

	// Get sun color temperature from elevation angle (0° = horizon, 90° = zenith)
	// Returns a restrained clear-sky sunrise color near the horizon and
	// approaches daylight white aloft. The atmosphere should do most of the
	// dramatic warming, not the directional light itself.
	static float SunTemperatureFromElevation(float ElevationDegrees);
};
