// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RiftbornAssetSnapshot.h"
#include "RiftbornVerificationPipeline.generated.h"

class UBlueprint;
class UWidgetBlueprint;
class UWorld;
class UMaterialInterface;

/**
 * Verification check types
 */
UENUM(BlueprintType)
enum class ERiftbornVerificationCheck : uint8
{
	BlueprintCompile UMETA(DisplayName = "Blueprint Compilation"),
	WidgetCompile UMETA(DisplayName = "Widget Blueprint Compilation"),
	ReferenceIntegrity UMETA(DisplayName = "Reference Integrity"),
	CyclicDependencies UMETA(DisplayName = "Cyclic Dependencies"),
	LevelLoad UMETA(DisplayName = "Level Load"),
	PIEStartup UMETA(DisplayName = "PIE Startup"),
	RuntimeErrors UMETA(DisplayName = "Runtime Errors"),
	EditorMessages UMETA(DisplayName = "Editor Message Log"),
	AssetSaved UMETA(DisplayName = "Asset Saved State"),
	AssetLocks UMETA(DisplayName = "Asset Locks")
};

/**
 * Result of a single verification check
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornVerificationResult
{
	GENERATED_BODY()

	/** Type of check performed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	ERiftbornVerificationCheck CheckType = ERiftbornVerificationCheck::BlueprintCompile;

	/** Whether check passed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bPassed = false;

	/** Error messages if failed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Errors;

	/** Warning messages */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Warnings;

	/** Assets that failed verification */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> FailedAssets;

	/** Execution time in milliseconds */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float ExecutionTimeMs = 0.0f;
};

/**
 * Complete verification pipeline result
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornPipelineResult
{
	GENERATED_BODY()

	/** Whether ALL checks passed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bAllChecksPassed = false;

	/** Individual check results */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FRiftbornVerificationResult> CheckResults;

	/** Total execution time */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float TotalTimeMs = 0.0f;

	/** Number of checks passed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 ChecksPassed = 0;

	/** Number of checks failed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 ChecksFailed = 0;

	/** Summary message */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Summary;
};

/**
 * Verification pipeline for post-action validation
 * Runs compile checks, reference integrity, PIE validation, etc.
 */
UCLASS()
class RIFTBORNAI_API URiftbornVerificationPipeline : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Run full verification pipeline on modified assets
	 * @param ModifiedAssets - Assets that were modified by action
	 * @param EnabledChecks - Which checks to run (empty = run all)
	 * @return Pipeline result with all check outcomes
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Verification")
	FRiftbornPipelineResult RunVerification(
		const TArray<UObject*>& ModifiedAssets,
		const TArray<ERiftbornVerificationCheck>& EnabledChecks
	);

	/**
	 * Quick verification (Blueprint compile + reference integrity only)
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Verification")
	FRiftbornPipelineResult QuickVerification(const TArray<UObject*>& ModifiedAssets);

	/**
	 * Full verification (all checks including PIE)
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Verification")
	FRiftbornPipelineResult FullVerification(const TArray<UObject*>& ModifiedAssets);

private:
	/** Verify Blueprint compilation */
	FRiftbornVerificationResult VerifyBlueprintCompile(const TArray<UObject*>& Assets);

	/** Verify Widget Blueprint compilation */
	FRiftbornVerificationResult VerifyWidgetCompile(const TArray<UObject*>& Assets);

	/** Verify no broken object references */
	FRiftbornVerificationResult VerifyReferenceIntegrity(const TArray<UObject*>& Assets);

	/** Verify no cyclic dependencies created */
	FRiftbornVerificationResult VerifyCyclicDependencies(const TArray<UObject*>& Assets);

	/** Verify level loads without errors */
	FRiftbornVerificationResult VerifyLevelLoad(const TArray<UObject*>& Assets);

	/** Verify PIE starts cleanly */
	FRiftbornVerificationResult VerifyPIEStartup();

	/** Verify no new runtime errors in first 5 seconds */
	FRiftbornVerificationResult VerifyRuntimeErrors();

	/** Verify no new editor errors/warnings */
	FRiftbornVerificationResult VerifyEditorMessages();

	/** Verify all assets saved or safe to leave unsaved */
	FRiftbornVerificationResult VerifyAssetSaved(const TArray<UObject*>& Assets);

	/** Verify no assets locked by external processes */
	FRiftbornVerificationResult VerifyAssetLocks(const TArray<UObject*>& Assets);

	/** Helper: Extract Blueprints from asset list */
	TArray<UBlueprint*> ExtractBlueprints(const TArray<UObject*>& Assets) const;

	/** Helper: Extract Widget Blueprints from asset list */
	TArray<UWidgetBlueprint*> ExtractWidgetBlueprints(const TArray<UObject*>& Assets) const;

	/** Helper: Extract Levels from asset list */
	TArray<UWorld*> ExtractLevels(const TArray<UObject*>& Assets) const;
};
