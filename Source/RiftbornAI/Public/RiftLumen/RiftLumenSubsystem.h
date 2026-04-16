// RiftLumenSubsystem.h
// Editor subsystem for the RiftLumen natural light source engine.
// Manages source archetypes and provides tools for spawning/querying light sources.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "RiftLumen/RiftLumenTypes.h"
#include "RiftLumen/RiftLumenSourceArchetype.h"
#include "RiftLumenSubsystem.generated.h"

class URiftLumenEmitterComponent;

UCLASS()
class RIFTBORNAI_API URiftLumenSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Spawn a RiftLumen source actor at a location with the given type
	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	AActor* SpawnSource(ERiftLumenSourceType SourceType, const FVector& Location, const FString& Label = TEXT(""));

	// Get all active RiftLumen emitter components in the world
	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	TArray<URiftLumenEmitterComponent*> GetAllActiveSources() const;

	// Get stats: source count by type, total managed lights
	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	FString GetSourceStats() const;

	// Set the global intensity scale for all RiftLumen sources
	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	void SetIntensityScale(float NewScale);

	UFUNCTION(BlueprintPure, Category = "RiftLumen")
	float GetIntensityScale() const { return Config.IntensityScale; }

	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	void SetSunDirection(const FVector& NewDirection);

	UFUNCTION(BlueprintPure, Category = "RiftLumen")
	FVector GetSunDirection() const { return Config.SunDirection; }

	UFUNCTION(BlueprintCallable, Category = "RiftLumen")
	void SetSunIntensity(float NewIntensity);

	UFUNCTION(BlueprintPure, Category = "RiftLumen")
	float GetSunIntensity() const { return Config.SunIntensity; }

	const FRiftLumenConfig& GetConfig() const { return Config; }

private:
	FRiftLumenConfig Config;
	bool bInitialized = false;

	UWorld* GetEditorWorld() const;
};
