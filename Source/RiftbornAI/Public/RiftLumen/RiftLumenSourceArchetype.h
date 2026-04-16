// RiftLumenSourceArchetype.h
// Natural light source archetypes that generate UE5 light component parameters per tick.
// Each archetype models a real-world light source: fire, candle, lava, lightning, etc.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RiftLumen/RiftLumenNoise.h"
#include "RiftLumenSourceArchetype.generated.h"

UENUM(BlueprintType)
enum class ERiftLumenSourceType : uint8
{
	PointBulb    UMETA(DisplayName = "Light Bulb"),
	Candle       UMETA(DisplayName = "Candle"),
	Torch        UMETA(DisplayName = "Torch"),
	Campfire     UMETA(DisplayName = "Campfire"),
	Fireplace    UMETA(DisplayName = "Fireplace"),
	Lava         UMETA(DisplayName = "Lava"),
	Lightning    UMETA(DisplayName = "Lightning"),
	Sun          UMETA(DisplayName = "Sun"),
	Moon         UMETA(DisplayName = "Moon"),
	Neon         UMETA(DisplayName = "Neon"),
	Custom       UMETA(DisplayName = "Custom"),
};

UENUM(BlueprintType)
enum class ERiftLumenLightPrimitive : uint8
{
	Auto        UMETA(DisplayName = "Auto"),
	Point       UMETA(DisplayName = "Point"),
	Rect        UMETA(DisplayName = "Rect"),
	Directional UMETA(DisplayName = "Directional"),
};

// One micro-light output from archetype evaluation
struct RIFTBORNAI_API FRiftLumenMicroLight
{
	FVector RelativePosition = FVector::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	float Intensity = 5000.f;           // Lumens
	float AttenuationRadius = 500.f;    // cm
	float SourceRadius = 0.f;           // cm, for soft shadows
	bool bCastShadows = true;
};

// A source archetype evaluates per-tick to produce micro-light parameters
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTBORNAI_API URiftLumenSourceArchetype : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	ERiftLumenSourceType SourceType = ERiftLumenSourceType::PointBulb;

	// Color temperature in Kelvin. 0 = use manual Color instead.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source", meta = (ClampMin = "0", ClampMax = "40000"))
	float ColorTemperature = 2700.f;

	// Manual color override (used when ColorTemperature == 0, e.g. Neon)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	FLinearColor ManualColor = FLinearColor::White;

	// Base intensity in lumens before noise modulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source", meta = (ClampMin = "0"))
	float BaseIntensity = 5000.f;

	// Light reach in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source", meta = (ClampMin = "10"))
	float AttenuationRadius = 500.f;

	// Soft shadow source radius in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source", meta = (ClampMin = "0"))
	float SourceRadius = 0.f;

	// Offset from the anchored visible source geometry to the light-emitting portion of the source.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	FVector SourceOriginOffset = FVector::ZeroVector;

	// Backing UE light primitive. Auto resolves from SourceType.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	ERiftLumenLightPrimitive LightPrimitive = ERiftLumenLightPrimitive::Auto;

	// Area-light size for rect-backed sources.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source", meta = (ClampMin = "1"))
	float RectSourceWidth = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source", meta = (ClampMin = "1"))
	float RectSourceHeight = 60.f;

	// Temporal behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	FRiftLumenNoiseProfile NoiseProfile;

	// Lightning-specific: time of last flash (world seconds)
	UPROPERTY(Transient)
	float LightningFlashTime = -100.f;

	// Lightning-specific: decay half-life in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning", meta = (ClampMin = "0.01"))
	float LightningDecayRate = 0.08f;

	// How many micro-lights this source generates
	int32 GetMicroLightCount() const;

	// Compute light parameters for this frame
	TArray<FRiftLumenMicroLight> Evaluate(float WorldTimeSeconds) const;

	// Trigger a lightning flash at the current world time
	void TriggerLightningFlash(float WorldTimeSeconds);

	// Initialize noise profile from source type defaults
	void ApplySourceTypeDefaults();

	ERiftLumenLightPrimitive ResolveLightPrimitive() const;
	bool RequiresVisibleSource() const;
};
