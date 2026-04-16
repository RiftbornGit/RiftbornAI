// Copyright RiftbornAI. All Rights Reserved.
// VFX Recipe System - Phase 4: Executable Recipe System
// Stores VFX as PROGRAMS, not assets. Enables replay with mutations.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "VFXRecipe.generated.h"

/**
 * Recipe input parameter - controllable from outside
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRecipeInput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Type;  // float, vector, color, texture

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString DefaultValue;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float MinValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float MaxValue = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Description;
};

/**
 * Single step in recipe construction
 */
UENUM(BlueprintType)
enum class ERecipeStepType : uint8
{
	// System-level ops
	CloneSystem,        // Clone from template
	CreateSystem,       // Create empty system
	
	// Emitter ops
	AddEmitter,         // Add emitter from template
	RemoveEmitter,      // Remove emitter by index
	
	// Module ops
	AddModule,          // Add module to emitter
	RemoveModule,       // Remove module from emitter
	
	// Parameter ops
	SetParam,           // Set parameter value
	BindParam,          // Bind to input variable
	
	// Renderer ops
	SetRenderer,        // Configure renderer
	SetMaterial,        // Set material on renderer
	
	// Compile/Save
	Compile,            // Compile system
	Save                // Save to disk
};

/**
 * A single construction step
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRecipeStep
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 StepIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	ERecipeStepType StepType = ERecipeStepType::SetParam;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString TargetPath;  // e.g., "Emitter[0].Module[SpawnRate]"

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Operation;   // Specific operation name

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TMap<FString, FString> Arguments;  // Key-value args

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Comment;  // Why this step exists
};

/**
 * Constraint on recipe execution
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRecipeConstraint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Type;  // range, ratio, dependency

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Expression;  // e.g., "SpawnRate < Lifetime * 1000"

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Reason;  // Why this constraint exists
};

/**
 * Recipe metadata
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRecipeMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString RecipeId;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Version = TEXT("1.0");

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Category;  // Fire, Smoke, Explosion, Magic, etc.

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Author = TEXT("RiftbornAI");

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime CreatedAt = FDateTime::UtcNow();

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SourceAssetPath;  // If derived from existing asset

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ContentHash;  // For versioning/drift detection

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Tags;
};

/**
 * Complete VFX Recipe - a program that creates a VFX
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVFXRecipe
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FRecipeMetadata Metadata;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FRecipeInput> Inputs;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FRecipeStep> Steps;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FRecipeConstraint> Constraints;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString TemplateSystemPath;  // Base system to clone from
};

/**
 * Result of recipe execution
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRecipeExecutionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString CreatedAssetPath;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	int32 StepsExecuted = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	int32 StepsFailed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> Warnings;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> Errors;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	float ExecutionTimeMs = 0.0f;

	static FRecipeExecutionResult Success(const FString& Path, int32 Steps)
	{
		FRecipeExecutionResult R;
		R.bSuccess = true;
		R.Message = TEXT("Recipe executed successfully");
		R.CreatedAssetPath = Path;
		R.StepsExecuted = Steps;
		return R;
	}

	static FRecipeExecutionResult Fail(const FString& Msg, int32 ExecutedSteps = 0, int32 FailedSteps = 1)
	{
		FRecipeExecutionResult R;
		R.bSuccess = false;
		R.Message = Msg;
		R.StepsExecuted = ExecutedSteps;
		R.StepsFailed = FailedSteps;
		R.Errors.Add(Msg);
		return R;
	}
};

/**
 * Recipe mutation for exploration
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRecipeMutation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 StepIndex = -1;  // Which step to mutate (-1 = add new)

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ParameterName;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString OriginalValue;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString MutatedValue;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float MutationStrength = 0.1f;  // How much to change (0-1)
};

/**
 * VFX Recipe Builder - Creates and executes VFX recipes
 * 
 * Phase 4: Executable Recipe System
 * 
 * This class provides:
 * - InspectToRecipe: Convert existing Niagara system to recipe
 * - ExecuteRecipe: Replay recipe to create new system
 * - MutateRecipe: Apply mutations for exploration
 * - ValidateRecipe: Check recipe constraints
 * - SaveRecipe/LoadRecipe: Persist recipes as JSON
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UVFXRecipeBuilder : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Convert an existing Niagara system to a recipe
	 * @param System - System to convert
	 * @param Category - Recipe category (Fire, Smoke, etc.)
	 * @return Generated recipe
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static FVFXRecipe InspectToRecipe(UNiagaraSystem* System, const FString& Category);

	/**
	 * Execute a recipe to create a new Niagara system
	 * @param Recipe - Recipe to execute
	 * @param OutputPath - Where to save the created system
	 * @param InputOverrides - Override input values
	 * @return Execution result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static FRecipeExecutionResult ExecuteRecipe(
		const FVFXRecipe& Recipe,
		const FString& OutputPath,
		const TMap<FString, FString>& InputOverrides);

	/**
	 * Apply mutations to a recipe
	 * @param Recipe - Base recipe
	 * @param Mutations - Mutations to apply
	 * @return Mutated recipe
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static FVFXRecipe MutateRecipe(const FVFXRecipe& Recipe, const TArray<FRecipeMutation>& Mutations);

	/**
	 * Generate random mutations for exploration
	 * @param Recipe - Base recipe
	 * @param NumMutations - Number of mutations to generate
	 * @param Strength - Mutation strength (0-1)
	 * @return Array of mutations
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static TArray<FRecipeMutation> GenerateMutations(const FVFXRecipe& Recipe, int32 NumMutations, float Strength);

	/**
	 * Validate a recipe against its constraints
	 * @param Recipe - Recipe to validate
	 * @return True if all constraints pass
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static bool ValidateRecipe(const FVFXRecipe& Recipe, TArray<FString>& OutViolations);

	/**
	 * Save recipe to JSON file
	 * @param Recipe - Recipe to save
	 * @param FilePath - Output file path
	 * @return True if saved successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static bool SaveRecipe(const FVFXRecipe& Recipe, const FString& FilePath);

	/**
	 * Load recipe from JSON file
	 * @param FilePath - Input file path
	 * @param OutRecipe - Loaded recipe
	 * @return True if loaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static bool LoadRecipe(const FString& FilePath, FVFXRecipe& OutRecipe);

	/**
	 * Compute content hash for recipe versioning
	 * @param Recipe - Recipe to hash
	 * @return SHA1 hash of recipe content
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static FString ComputeRecipeHash(const FVFXRecipe& Recipe);

	/**
	 * Compare two recipes for drift detection
	 * @param RecipeA - First recipe
	 * @param RecipeB - Second recipe
	 * @return Similarity score (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|VFX")
	static float CompareRecipes(const FVFXRecipe& RecipeA, const FVFXRecipe& RecipeB);

private:
	/** Convert NiagaraInspector snapshot to recipe steps */
	static TArray<FRecipeStep> ConvertSnapshotToSteps(UNiagaraSystem* System);

	/** Extract input parameters from system */
	static TArray<FRecipeInput> ExtractInputs(UNiagaraSystem* System);

	/** Generate constraints from system analysis */
	static TArray<FRecipeConstraint> GenerateConstraints(UNiagaraSystem* System);

	/** Execute a single recipe step */
	static bool ExecuteStep(UNiagaraSystem* System, const FRecipeStep& Step, FString& OutError);

	/** Apply input overrides to recipe */
	static FVFXRecipe ApplyInputOverrides(const FVFXRecipe& Recipe, const TMap<FString, FString>& Overrides);
};
