// Copyright RiftbornAI. All Rights Reserved.
// VFX Heuristics System - Phase 5 Implementation

#pragma once

#include "CoreMinimal.h"
#include "VFXRecipe.h"
#include "VFXHeuristics.generated.h"

/**
 * VFX Heuristic Type - what kind of guidance this is
 */
UENUM(BlueprintType)
enum class EHeuristicType : uint8
{
	Constraint,     // Hard rule (fire rises, smoke fades)
	Preference,     // Soft bias (flames should flicker)
	SearchHint,     // Parameter range guidance
	Exclusion,      // Things to avoid
	Correlation     // Things that go together
};

/**
 * VFX Effect Type - which effects this heuristic applies to
 */
UENUM(BlueprintType)
enum class EHeuristicVFXType : uint8
{
	Any,            // Applies to all VFX
	Fire,
	Smoke,
	Explosion,
	Magic,
	Water,
	Electricity,
	Debris,
	Weather
};

/**
 * Single heuristic entry
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVFXHeuristic
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Id;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	EHeuristicType Type = EHeuristicType::Preference;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	EHeuristicVFXType Category = EHeuristicVFXType::Any;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Description;

	// Human-readable rule (for documentation)
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString NaturalLanguage;

	// Parameters this heuristic affects
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> AffectedParameters;

	// Expression (simple rules like "Velocity.Z > 0")
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Expression;

	// For SearchHint type - parameter ranges
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TMap<FString, FString> SuggestedRanges;  // "ParamName" -> "0.5..2.0"

	// Confidence/strength of this heuristic (0-1)
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float Strength = 0.8f;

	// Source (tutorial, observation, manual)
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Source;
};

/**
 * Complete heuristics database
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVFXHeuristicsDB
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Version = TEXT("1.0");

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime LastUpdated = FDateTime::UtcNow();

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FVFXHeuristic> Heuristics;
};

/**
 * Mutation bias based on heuristics
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMutationBias
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ParameterName;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float BiasDirection = 0.0f;  // -1 to 1, 0 = no bias

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float MinValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float MaxValue = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> ReasoningHeuristics;  // Which heuristics contributed
};

/**
 * VFX Heuristics Engine - Guides mutation and generation
 * 
 * Phase 5: Weak-Prior Ingestion
 * 
 * Key Principle: Heuristics guide SEARCH, not EXECUTION.
 * Tutorial knowledge constrains exploration, never authors directly.
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UVFXHeuristicsEngine : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Load heuristics from JSON file
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static bool LoadHeuristics(const FString& FilePath, FVFXHeuristicsDB& OutDB);

	/**
	 * Save heuristics to JSON file
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static bool SaveHeuristics(const FVFXHeuristicsDB& DB, const FString& FilePath);

	/**
	 * Get heuristics for a specific VFX category
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static TArray<FVFXHeuristic> GetHeuristicsForCategory(const FVFXHeuristicsDB& DB, EHeuristicVFXType Category);

	/**
	 * Get heuristics that affect a specific parameter
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static TArray<FVFXHeuristic> GetHeuristicsForParameter(const FVFXHeuristicsDB& DB, const FString& ParameterName);

	/**
	 * Generate mutation biases based on heuristics
	 * This guides the mutation system to explore in "sensible" directions
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static TArray<FMutationBias> GenerateMutationBiases(
		const FVFXHeuristicsDB& DB,
		const FVFXRecipe& Recipe,
		EHeuristicVFXType Category);

	/**
	 * Apply biased mutations to a recipe
	 * Uses heuristics to guide mutation directions
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static TArray<FRecipeMutation> GenerateBiasedMutations(
		const FVFXHeuristicsDB& DB,
		const FVFXRecipe& Recipe,
		EHeuristicVFXType Category,
		int32 NumMutations,
		float Strength);

	/**
	 * Validate recipe against heuristics
	 * Returns constraint violations
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static TArray<FString> ValidateAgainstHeuristics(
		const FVFXHeuristicsDB& DB,
		const FVFXRecipe& Recipe,
		EHeuristicVFXType Category);

	/**
	 * Create default heuristics database
	 * Built-in knowledge about VFX physics and aesthetics
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static FVFXHeuristicsDB CreateDefaultHeuristicsDB();

	/**
	 * Parse tutorial text for heuristic extraction
	 * NOTE: This extracts hints, NOT executable instructions
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Heuristics")
	static TArray<FVFXHeuristic> ParseTutorialText(const FString& TutorialText, EHeuristicVFXType Category);

private:
	/** Parse a range string like "0.5..2.0" */
	static bool ParseRange(const FString& RangeStr, float& OutMin, float& OutMax);

	/** Evaluate simple expression against recipe */
	static bool EvaluateExpression(const FString& Expression, const FVFXRecipe& Recipe);
};
