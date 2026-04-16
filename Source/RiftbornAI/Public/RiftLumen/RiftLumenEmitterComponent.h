// RiftLumenEmitterComponent.h
// Natural light source component. Owns a SourceArchetype and spawns
// real UE5 point lights that Lumen renders at full quality.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "RiftLumen/RiftLumenSourceArchetype.h"
#include "RiftLumenEmitterComponent.generated.h"

class UPointLightComponent;
class ULightComponent;
class UPrimitiveComponent;

UCLASS(ClassGroup = (RiftLumen), meta = (BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftLumenEmitterComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	URiftLumenEmitterComponent();

	// Quick-set: pick a source type, auto-creates archetype with defaults
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen")
	ERiftLumenSourceType SourceType = ERiftLumenSourceType::PointBulb;

	// The archetype that defines this source's behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "RiftLumen")
	TObjectPtr<URiftLumenSourceArchetype> SourceArchetype;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen")
	bool bEnabled = true;

	// Optional explicit visible source component. If unset, the emitter searches the owner for a visible primitive.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen|Source")
	FComponentReference SourceAnchor;

	// Optional socket on the source anchor used as the physical light origin.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen|Source")
	FName SourceAnchorSocket = NAME_None;

	// Allows local emitters to operate without a visible source mesh. Off by default for realism-first authoring.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen|Source")
	bool bAllowInvisibleSource = false;

	UFUNCTION(BlueprintPure, Category = "RiftLumen")
	bool IsEmitterEnabled() const;

	UFUNCTION(BlueprintPure, Category = "RiftLumen")
	bool HasVisibleSource() const;

	// Get the number of UE5 lights this source is currently driving
	UFUNCTION(BlueprintPure, Category = "RiftLumen")
	int32 GetManagedLightCount() const { return ManagedLights.Num(); }

	// Force rebuild managed lights (call after changing SourceType)
	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	void RebuildManagedLights();

	// Trigger a lightning flash (only works on Lightning source type)
	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	void TriggerLightningFlash();

	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void DestroyComponent(bool bPromoteChildren) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Enable opt-in bounce proxy lights that fake extra fill when Lumen alone is insufficient.
	// Disabled by default so practical emitters do not appear to light walls and ceilings from nowhere.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen|Bounce")
	bool bEnableBounceProxies = false;

	// How much of the fire's intensity the bounce lights receive (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen|Bounce", meta = (ClampMin = "0", ClampMax = "1"))
	float BounceIntensityScale = 0.08f;

	// How far to trace for surfaces (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftLumen|Bounce", meta = (ClampMin = "100"))
	float BounceTraceDistance = 1500.f;

	// Ensure archetype exists and matches SourceType
	void EnsureArchetype();

private:
	// UE5 light components driven by the archetype each tick.
	// Sun and moon use directional lights; all other source types use point lights.
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULightComponent>> ManagedLights;

	// Bounce proxy lights placed at nearby surfaces to simulate multi-bounce fill
	struct FBounceProxy
	{
		TObjectPtr<UPointLightComponent> Light;
		FVector SurfaceNormal;      // Direction the surface faces
		float DistanceFromSource;   // How far from fire to surface
	};
	TArray<FBounceProxy> BounceProxies;

	// Previous source type — detect changes in editor
	ERiftLumenSourceType CachedSourceType = ERiftLumenSourceType::PointBulb;
	bool bLoggedMissingVisibleSource = false;

	void UpdateManagedLights(float WorldTime);
	void BuildBounceProxies();
	void UpdateBounceProxies(float AverageIntensity, const FLinearColor& FireColor);
	UPrimitiveComponent* ResolveSourcePrimitive() const;
	FTransform ResolveSourceTransform(const UPrimitiveComponent* SourcePrimitive) const;
	bool CanEmitFromVisibleSource(const UPrimitiveComponent* SourcePrimitive) const;
	void SetManagedLightVisibility(bool bVisible);
};
