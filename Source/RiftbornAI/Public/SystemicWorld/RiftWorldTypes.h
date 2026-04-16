// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Awareness/TemporalAwareness.h"
#include "RiftWorldTypes.generated.h"

UENUM(BlueprintType)
enum class ERiftWorldEventType : uint8
{
	None UMETA(DisplayName = "None"),
	WeatherChanged UMETA(DisplayName = "Weather Changed"),
	PrecipitationStarted UMETA(DisplayName = "Precipitation Started"),
	PrecipitationStopped UMETA(DisplayName = "Precipitation Stopped"),
	LightningRiskRaised UMETA(DisplayName = "Lightning Risk Raised"),
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftWorldSimulationConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float FixedStepSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float GameHoursPerRealSecond = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float DailyTemperatureSwingC = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float HumidityRecoveryRate = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float CloudBuildRate = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float SurfaceWettingRate = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float SurfaceDryingRate = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float SoilAbsorptionRate = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float SoilDrainageRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float PrecipitationThreshold = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling")
	bool bEnableTerrainCoupling = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "0.1"))
	float TerrainResampleIntervalSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "100.0"))
	float TerrainSampleRadiusCm = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "1000.0"))
	float TerrainSurfaceSearchZ = 50000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float TerrainWindwardLiftScale = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TerrainConcavityMoistureScale = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TerrainMaterialMoistureScale = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field")
	bool bEnableLocalizedWeatherField = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field", meta = (ClampMin = "0", ClampMax = "8"))
	int32 LocalWeatherFieldRadiusCells = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field", meta = (ClampMin = "250.0"))
	float LocalWeatherCellSizeCm = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float LocalWeatherAdvectionStrength = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LocalWeatherCrossWindFalloff = 0.25f;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftRegionalWorldState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float TimeOfDayHours = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	ESeason Season = ESeason::Summer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	EBiome Biome = EBiome::Temperate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	EWeatherCondition Weather = EWeatherCondition::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float TemperatureC = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Humidity = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloudCoverage = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloudDensity = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PrecipitationIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SurfaceWetness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SoilMoisture = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Uplift = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Instability = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightningRisk = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float WindDirectionDegrees = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float WindSpeedMps = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	FVector WindVector = FVector(1.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling")
	float TerrainHeightCm = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TerrainSlope = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float TerrainConcavity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TerrainMoistureBias = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Terrain Coupling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OrographicLift = 0.0f;

	void Normalize();
	FTemporalState ToTemporalState() const;
	FString Describe() const;

	static FVector MakeWindVector(float WindDirectionDegrees, float WindSpeedMps);
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftWorldEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	ERiftWorldEventType EventType = ERiftWorldEventType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float WorldTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	FString Summary;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftWorldSample
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	FVector WorldPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	FRiftRegionalWorldState State;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftLocalizedWeatherCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field")
	FVector WorldPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindwardFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StormFactor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Localized Field")
	FRiftRegionalWorldState State;
};
