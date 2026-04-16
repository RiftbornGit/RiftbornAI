// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// Proof Actor - Spawnable actor with proof binding component
//
// PURPOSE:
// Provides a spawnable actor with RiftbornProofBindingComponent as a default subobject.
// This allows programmatic creation of proof chains via the Python bridge.
//
// USAGE:
// 1. Spawn via Python: unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.RiftbornProofActor, ...)
// 2. Get component: actor.proof_component
// 3. Use component methods: proof_component.reset_proof_chain(), log_sensor_read(), etc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiftbornProofBindingComponent.h"
#include "RiftbornProofActor.generated.h"

/**
 * Actor with built-in proof binding component.
 * 
 * This actor exists primarily to allow Python bridge code to spawn
 * actors with proof chains without needing Blueprint setup.
 */
UCLASS(Blueprintable, meta=(DisplayName="Riftborn Proof Actor"))
class RIFTBORNAI_API ARiftbornProofActor : public AActor
{
	GENERATED_BODY()

public:
	ARiftbornProofActor();

	/** Get the proof binding component */
	UFUNCTION(BlueprintPure, Category = "Proof")
	URiftbornProofBindingComponent* GetProofComponent() const { return ProofComponent; }

protected:
	/** The proof binding component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Proof")
	TObjectPtr<URiftbornProofBindingComponent> ProofComponent;
};
