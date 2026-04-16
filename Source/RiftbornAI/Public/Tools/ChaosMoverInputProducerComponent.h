// Copyright RiftbornAI. All Rights Reserved.
// Injects one-shot Chaos Mover inputs through the public Mover input-producer API.

#pragma once

#include "CoreMinimal.h"
#include "MoverSimulationTypes.h"

#if RIFTBORN_WITH_CHAOS_MOVER
#include "ChaosMover/Character/ChaosCharacterInputs.h"
#endif

#include "Components/ActorComponent.h"
#include "ChaosMoverInputProducerComponent.generated.h"

UCLASS(ClassGroup = (RiftbornAI), meta = (BlueprintSpawnableComponent))
/**
 * UChaosMoverInputProducerComponent
 * 
 * Injects one-shot Chaos Mover inputs through the public Mover input-producer API.
 */
class RIFTBORNAI_API UChaosMoverInputProducerComponent : public UActorComponent, public IMoverInputProducerInterface
{
    GENERATED_BODY()

public:
    UChaosMoverInputProducerComponent();

#if RIFTBORN_WITH_CHAOS_MOVER
    void QueueMovementSettingsOverrideClear(FName ModeName);
    bool HasPendingMovementSettingsOverrideClear() const;
#endif

    virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

private:
#if RIFTBORN_WITH_CHAOS_MOVER
    bool bPendingMovementSettingsOverrideClear = false;
    FName PendingModeName = NAME_None;
#endif
};
