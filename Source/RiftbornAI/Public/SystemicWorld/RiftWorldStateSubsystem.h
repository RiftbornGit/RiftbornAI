// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SystemicWorld/RiftWorldTypes.h"
#include "RiftWorldStateSubsystem.generated.h"

class URiftClimateProfile;

UCLASS()
class RIFTBORNAI_API URiftWorldStateSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World", meta = (WorldContext = "WorldContextObject"))
	static URiftWorldStateSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	void SetClimateProfile(URiftClimateProfile* InClimateProfile);

	UFUNCTION(BlueprintPure, Category = "Riftborn|Systemic World")
	URiftClimateProfile* GetClimateProfile() const { return ActiveClimateProfile; }

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	void ResetStateFromProfile();

	const FRiftRegionalWorldState& GetRegionalState() const { return RegionalState; }

	UFUNCTION(BlueprintPure, Category = "Riftborn|Systemic World")
	FRiftRegionalWorldState GetRegionalStateValue() const { return RegionalState; }

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	FRiftWorldSample SampleWorldStateAtLocation(const FVector& WorldPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	FString ExplainWorldStateAtLocation(const FVector& WorldPosition) const;

	const TArray<FRiftLocalizedWeatherCell>& GetLocalizedWeatherField() const { return LocalizedWeatherField; }

	UFUNCTION(BlueprintPure, Category = "Riftborn|Systemic World")
	bool HasLocalizedWeatherField() const { return LocalizedWeatherField.Num() > 0; }

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	TArray<FRiftWorldEvent> GetRecentEvents() const { return RecentEvents; }

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	void SetManualUplift(float InUplift);

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	void SetManualHumidity(float InHumidity);

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	void SetRegionalAnchorWorldPosition(const FVector& InWorldPosition);

	UFUNCTION(BlueprintCallable, Category = "Riftborn|Systemic World")
	void ClearRegionalAnchorWorldPosition();

	UFUNCTION(BlueprintPure, Category = "Riftborn|Systemic World")
	bool HasRegionalAnchorWorldPosition() const { return bHasRegionalAnchorWorldPosition; }

	UFUNCTION(BlueprintPure, Category = "Riftborn|Systemic World")
	FVector GetRegionalAnchorWorldPositionValue() const { return RegionalAnchorWorldPosition; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void AdvanceSimulation(float DeltaTime);
	void RunFixedSimulationStep(float StepSeconds);
	void EmitEvent(ERiftWorldEventType EventType, const FString& Summary);
	void RefreshDerivedState();
	void RefreshLocalizedWeatherField();
	void RefreshTerrainCoupling();
	const FRiftLocalizedWeatherCell* FindNearestLocalizedWeatherCell(const FVector& WorldPosition) const;

	float GetSeasonTemperatureBias(ESeason InSeason) const;
	float GetBiomeTemperatureBias(EBiome InBiome) const;
	float GetBiomeHumidityBias(EBiome InBiome) const;
	EWeatherCondition ResolveWeatherCondition() const;

	UPROPERTY(Transient)
	TObjectPtr<URiftClimateProfile> ActiveClimateProfile = nullptr;

	UPROPERTY(Transient)
	FRiftRegionalWorldState RegionalState;

	UPROPERTY(Transient)
	TArray<FRiftWorldEvent> RecentEvents;

	UPROPERTY(Transient)
	TArray<FRiftLocalizedWeatherCell> LocalizedWeatherField;

	float SimulationAccumulator = 0.0f;
	float TerrainSampleAccumulator = 0.0f;
	FVector RegionalAnchorWorldPosition = FVector::ZeroVector;
	bool bHasRegionalAnchorWorldPosition = false;
	bool bHasManualHumidityOverride = false;
	bool bHasManualUpliftOverride = false;
	float ManualHumidityOverride = 0.0f;
	float ManualUpliftOverride = 0.0f;
};
