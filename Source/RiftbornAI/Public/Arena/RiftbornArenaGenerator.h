// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "ArenaSpecAsset.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "RiftbornArenaGenerator.generated.h"

/**
 * Generation result status
 */
UENUM(BlueprintType)
enum class EArenaGenerationStatus : uint8
{
	NotStarted,
	InProgress,
	Succeeded,
	Failed
};

/**
 * Generation result
 */
USTRUCT(BlueprintType)
struct FArenaGenerationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	EArenaGenerationStatus Status = EArenaGenerationStatus::NotStarted;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FString> GeneratedFiles;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString MapPath;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString GameModePath;
};

/**
 * Riftborn Arena Generator
 * 
 * Consumes UArenaSpecAsset and generates an arena scaffold:
 * - Generated C++ champion classes
 * - Generated C++ ability classes
 * - Blueprint scaffolds from compiled classes when available
 * - A generated map with spawn points
 * - A generated GameMode scaffold with win-condition hooks
 * - A generated HUD scaffold with champion stat widgets
 * 
 * All generation is wrapped in atomic transactions via the existing
 * RiftbornCodingAgent snapshot/verify/commit pipeline.
 */
UCLASS()
class RIFTBORNAI_API URiftbornArenaGenerator : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	// Main generation entrypoint
	UFUNCTION(BlueprintCallable, Category = "Riftborn Arena Generator")
	FArenaGenerationResult GenerateArenaFromSpec(UArenaSpecAsset* Spec);

	// Get current generation status
	UFUNCTION(BlueprintCallable, Category = "Riftborn Arena Generator")
	EArenaGenerationStatus GetGenerationStatus() const { return CurrentStatus; }

	// Cancel ongoing generation
	UFUNCTION(BlueprintCallable, Category = "Riftborn Arena Generator")
	void CancelGeneration();

private:
	// Current generation status
	EArenaGenerationStatus CurrentStatus = EArenaGenerationStatus::NotStarted;

	// Current spec being generated
	UPROPERTY()
	UArenaSpecAsset* CurrentSpec = nullptr;

	// Generation result
	FArenaGenerationResult Result;

	// === Generation Pipeline Steps ===

	// Step 1: Validate spec
	bool ValidateSpec(FString& OutError);

	// Step 2: Generate C++ module structure
	bool GenerateModuleStructure(FString& OutError);

	// Step 3: Generate C++ base classes
	bool GenerateBaseClasses(FString& OutError);

	// Step 4: Generate champion C++ classes
	bool GenerateChampionClasses(FString& OutError);

	// Step 5: Generate ability C++ classes
	bool GenerateAbilityClasses(FString& OutError);

	// Step 6: Trigger C++ compilation via coding agent
	bool CompileGeneratedCode(FString& OutError);

	// Step 7: Generate Blueprints from C++ classes
	bool GenerateBlueprints(FString& OutError);

	// Step 8: Generate map with spawn points
	bool GenerateMap(FString& OutError);

	// Step 9: Generate GameMode
	bool GenerateGameMode(FString& OutError);

	// Step 10: Generate HUD
	bool GenerateHUD(FString& OutError);

	// === Code Generation Helpers ===

	// Generate C++ class header content
	FString GenerateChampionHeader(const FChampionSpec& Champion);
	FString GenerateChampionSource(const FChampionSpec& Champion);

	FString GenerateAbilityHeader(const FAbilitySpec& Ability);
	FString GenerateAbilitySource(const FAbilitySpec& Ability);

	// Get module paths
	FString GetGeneratedModulePath() const;
	FString GetChampionsPath() const;
	FString GetAbilitiesPath() const;

	// File writing (uses transaction system)
	bool WriteGeneratedFile(const FString& FilePath, const FString& Content, FString& OutError);

	// Build system integration
	bool TriggerBuild(FString& OutError);
	bool WaitForBuildCompletion(FString& OutError);
	
	// Check if a build is currently in progress
	bool IsBuildInProgress() const { return bBuildInProgress; }

private:
	// Build tracking
	bool bBuildInProgress = false;
	FDateTime BuildStartTime;
	FProcHandle BuildProcessHandle;
};
