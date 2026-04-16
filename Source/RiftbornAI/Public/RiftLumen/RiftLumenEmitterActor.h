// RiftLumenEmitterActor.h
// Spawnable actor wrapper for the Stage 1 RiftLumen emitter component.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiftLumenEmitterActor.generated.h"

class URiftLumenEmitterComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, meta = (DisplayName = "RiftLumen Emitter Actor"))
class RIFTBORNAI_API ARiftLumenEmitterActor : public AActor
{
	GENERATED_BODY()

public:
	ARiftLumenEmitterActor();

	UFUNCTION(BlueprintPure, Category = "RiftLumen")
	URiftLumenEmitterComponent* GetEmitterComponent() const { return EmitterComponent; }

	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	void RefreshSourceVisual();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RiftLumen")
	TObjectPtr<URiftLumenEmitterComponent> EmitterComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RiftLumen")
	TObjectPtr<UStaticMeshComponent> SourceVisualComponent;
};
