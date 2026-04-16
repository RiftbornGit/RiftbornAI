// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SystemicWorld/RiftWorldTypes.h"
#include "RiftClimateProfile.generated.h"

UCLASS(BlueprintType)
class RIFTBORNAI_API URiftClimateProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	FRiftWorldSimulationConfig SimulationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Riftborn|Systemic World")
	FRiftRegionalWorldState SeedState;
};
