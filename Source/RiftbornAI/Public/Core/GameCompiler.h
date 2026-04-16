// Copyright Epic Games, Inc. All Rights Reserved.
// Game Compiler - Compiles GameSpec into UE Project

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "GameSpec.h"
#include "RiftbornCompilerSettings.h"
#include "GameCompiler.generated.h"

// ============================================================================
// COMPILER PASSES
// ============================================================================

/**
 * Compilation result
 */
UENUM(BlueprintType)
enum class ECompileStatus : uint8
{
	NotStarted,
	InProgress,
	Succeeded,
	Failed
};

USTRUCT(BlueprintType)
struct FCompileResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	ECompileStatus Status = ECompileStatus::NotStarted;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> GeneratedFiles;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString OutputPath;
};

/**
 * Genre profile - constraints and templates for specific game types
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UGenreProfileAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FName ProfileID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString Description;

	// Required entity tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
	TArray<FName> RequiredEntityTags;

	// Default stat templates
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
	TArray<FStatDef> DefaultStats;

	// Movement defaults
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
	float DefaultMoveSpeed = 600.0f;

	// Team configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
	int32 MinTeams = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
	int32 MaxTeams = 2;
};

/**
 * UGameCompilerSubsystem - Compiles GameSpec into UE project
 * 
 * This is the SINGLE compiler for all game types.
 * Profiles modify behavior, they don't replace the compiler.
 */
UCLASS()
class RIFTBORNAI_API UGameCompilerSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Main entry point: Compile a GameSpec into a UE project
	 * 
	 * @param Spec - The game specification (source of truth)
	 * @param Profile - Optional genre profile for constraints/templates
	 * @return Compilation result with status and generated files
	 */
	UFUNCTION(BlueprintCallable, Category = "Game Compiler")
	FCompileResult CompileGame(UGameSpecAsset* Spec, UGenreProfileAsset* Profile = nullptr);

	/**
	 * Cancel ongoing compilation
	 */
	UFUNCTION(BlueprintCallable, Category = "Game Compiler")
	void CancelCompilation();

private:
	// Current compilation state
	UPROPERTY(Transient)
	UGameSpecAsset* CurrentSpec = nullptr;
	UPROPERTY(Transient)
	UGenreProfileAsset* CurrentProfile = nullptr;
	FString CurrentSpecId;
	FString CurrentProfileId;
	FCompileResult Result;
	ECompileStatus Status = ECompileStatus::NotStarted;

	// Phase 1: Code Generation (can run in commandlet)
	bool ValidateSpec(FString& OutError);
	bool ValidateProfile(FString& OutError);
	bool ValidateTargetModule(FString& OutError);
	bool CodeGenPass(FString& OutError);
	bool CompileCodePass(FString& OutError);

	// Phase 2: Asset Generation (requires compiled classes)
	bool BlueprintGenPass(FString& OutError);
	bool MapGenPass(FString& OutError);
	bool HUDGenPass(FString& OutError);
	bool RunPhase2(FString& OutError);

	// Code generation helpers
	FString GenerateEntityClass(const FEntityDef& Entity);
	FString GenerateAbilityClass(const FAbilityDef& Ability);
	FString GenerateGameModeClass(const FString& ClassName, const FString& PlayerControllerClassName, const FGameRulesDef& Rules);
	FString GeneratePlayerControllerClass(const FString& ClassName, const FGameRulesDef& Rules);
	FString GenerateMapBuilderClass(const FString& ClassName, const FMapDef& MapDef, const FGameRulesDef& Rules);
	FString GenerateHUDWidgetClass(const FString& ClassName, const FGameRulesDef& Rules);
	FString GenerateGameStateClass();
	FString GeneratePlayerStateClass();

	// File I/O (wrapped in transactions)
	bool WriteGeneratedFile(const FString& FilePath, const FString& Content, FString& OutError);
	FString GetOutputModulePath() const;     // Generated code root (…/Generated)
	FString GetTargetModulePath() const;     // Base module path (no Generated/)
	FString GetGeneratedCodePath() const;    // Convenience alias for Output path

	// Settings access
	URiftbornCompilerSettings* GetSettings() const;

	// Manifest + asset helpers
	bool WriteManifest(FString& OutError);
	bool CleanupPreviousOutputs(FString& OutError);
	bool LoadManifestJson(const FString& ManifestPath, TSharedPtr<FJsonObject>& OutJson, FString& OutError) const;
	bool SaveAssetPackage(UPackage* Package, UObject* Asset, const FString& PackagePath, FString& OutError) const;

	// Tracking
	TArray<FString> GeneratedAssetPaths;
};
