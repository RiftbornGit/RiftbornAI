// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RiftbornCompilerSettings.generated.h"

/**
 * Compiler configuration - where to generate code
 */
UCLASS(config=EditorPerProjectUserSettings, defaultconfig)
class RIFTBORNAI_API URiftbornCompilerSettings : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Target module for generated C++ code
	 * Must be an existing C++ module in your project
	 * Example: "RiftbornGenerated" or your game module name
	 */
	UPROPERTY(EditAnywhere, config, Category = "Code Generation")
	FString TargetModuleName; // Defaults to FApp::GetProjectName() at runtime when empty

	/**
	 * Try Live Coding first before falling back to external build.
	 * Live Coding is fast but can only modify existing code, not add new classes.
	 * When false, always require external build.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Code Generation")
	bool bTryLiveCodingFirst = true;

	/** Toggle Phase 2 (asset generation). Disable to allow Phase 1 only. */
	UPROPERTY(EditAnywhere, config, Category = "Asset Generation")
	bool bEnablePhase2 = true;

	/** Clean previous generated outputs (code + assets) using last manifest before regenerating. */
	UPROPERTY(EditAnywhere, config, Category = "Asset Generation")
	bool bCleanupBeforeGenerate = true;

	/**
	 * Base path for generated Blueprint assets
	 * Default: /Game/RiftbornGenerated/Blueprints
	 */
	UPROPERTY(EditAnywhere, config, Category = "Asset Generation")
	FString BlueprintOutputPath = TEXT("/Game/RiftbornGenerated/Blueprints");

	/**
	 * Base path for generated Map assets
	 * Default: /Game/RiftbornGenerated/Maps
	 */
	UPROPERTY(EditAnywhere, config, Category = "Asset Generation")
	FString MapOutputPath = TEXT("/Game/RiftbornGenerated/Maps");

	/**
	 * Base path for generated HUD assets
	 * Default: /Game/RiftbornGenerated/HUD
	 */
	UPROPERTY(EditAnywhere, config, Category = "Asset Generation")
	FString HUDOutputPath = TEXT("/Game/RiftbornGenerated/HUD");

	/**
	 * Directory under /Saved for compiler manifests
	 * Default: Saved/RiftbornCompiler/
	 */
	UPROPERTY(EditAnywhere, config, Category = "Asset Generation")
	FString ManifestDir = TEXT("RiftbornCompiler");
};
