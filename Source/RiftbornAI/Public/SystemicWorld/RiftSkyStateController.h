// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SystemicWorld/RiftWorldTypes.h"
#include "RiftSkyStateController.generated.h"

class URiftClimateProfile;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class UMaterialParameterCollection;
class UPostProcessComponent;
class USceneComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UVolumetricCloudComponent;
class UWindDirectionalSourceComponent;

UCLASS()
class RIFTBORNAI_API ARiftSkyStateController : public AActor
{
	GENERATED_BODY()

public:
	ARiftSkyStateController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	void ApplyRegionalState(const FRiftRegionalWorldState& State);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloudComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<UExponentialHeightFogComponent> HeightFogComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<UWindDirectionalSourceComponent> WindComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<UDirectionalLightComponent> SunLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<USkyLightComponent> SkyLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<UPostProcessComponent> ExposureComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<URiftClimateProfile> ClimateProfile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	TObjectPtr<UMaterialParameterCollection> WeatherMaterialParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	bool bAutoDriveFromWorldState = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	bool bPushClimateProfileToSubsystem = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	bool bDriveRegionalAnchorFromActorLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float BaseSunIntensity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float StormSunIntensityScale = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float BaseSkyLightIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float StormSkyLightIntensityScale = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World")
	float MaxFogDensity = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Exposure")
	bool bLockExposureForLookDev = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Systemic World|Exposure", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float LockedExposureBrightness = 1.0f;
};
