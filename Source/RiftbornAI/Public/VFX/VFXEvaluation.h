// Copyright RiftbornAI. All Rights Reserved.
// VFX Evaluation & Proof Generation - Phase 7 Implementation

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "VFXRecipe.h"
#include "VFXEditDelta.h"
#include "VFXHeuristics.h"
#include "VFXEvaluation.generated.h"

/**
 * Evaluation result type
 */
UENUM(BlueprintType)
enum class EEvaluationResult : uint8
{
	Pass,           // Test passed
	Fail,           // Test failed
	Skipped,        // Test skipped
	Error           // Test error
};

/**
 * Single test case result
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTestCaseResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString TestName;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString TestDescription;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	EEvaluationResult Result = EEvaluationResult::Skipped;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Message;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float DurationMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime Timestamp = FDateTime::UtcNow();

	// Evidence
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TMap<FString, FString> Evidence;  // Key-value pairs of proof
};

/**
 * Blind test configuration
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlindTestConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString TestName;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString VFXCategory;  // Fire, Smoke, Explosion, etc.

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString StyleDescription;  // "campfire", "volcanic", "ethereal"

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TMap<FString, FString> RequiredInputs;  // Input constraints

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> ExpectedEffects;  // What effects should be present

	// ===== TARGET METRICS - Each category must have different values =====
	
	// Coverage target (0-1): How much of frame should have particles
	// Fire: medium (0.2-0.4), Smoke: low-medium (0.1-0.3), Explosion: high then low
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float TargetCoverage = 0.3f;
	
	// Edge density target: Higher = more detail/complexity
	// Fire: high (flicker), Smoke: low (soft), Explosion: very high (debris)
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float TargetEdgeDensity = 0.1f;
	
	// Brightness target (0-1): Overall luminance
	// Fire: high (emissive), Smoke: low (dark), Explosion: flash then dark
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float TargetBrightness = 0.5f;
	
	// Color variance target: Higher = more color variation
	// Fire: low (orange-red), Smoke: very low (gray), Explosion: medium (flash colors)
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float TargetColorVariance = 0.1f;
	
	// Motion energy: Temporal change between frames (higher = more movement)
	// Fire: medium (flicker), Smoke: low (slow billow), Explosion: very high (rapid)
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float TargetMotionEnergy = 0.2f;
	
	// Weights for scoring (how much each metric matters for this category)
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float WeightCoverage = 1.0f;
	
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float WeightEdgeDensity = 1.0f;
	
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float WeightBrightness = 1.0f;
	
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float WeightColorVariance = 1.0f;
	
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float WeightMotionEnergy = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bUseHeuristics = true;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bUseLearning = true;
};

/**
 * Blind test result
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlindTestResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FBlindTestConfig Config;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bRecipeGenerated = false;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bRecipeExecuted = false;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bSystemCreated = false;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float ExecutionSuccessRate = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 StepsAttempted = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 StepsSucceeded = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString CreatedSystemPath;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Warnings;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Errors;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float DurationMs = 0.0f;
};

/**
 * Single step measurement during executed blind test
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FExecutedStepMeasurement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 StepIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ActionApplied;  // Description of action taken

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float Coverage = 0.0f;  // Pixel coverage [0,1]

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float EdgeDensity = 0.0f;  // Edge magnitude

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float ColorVariance = 0.0f;  // Color spread

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float MeanBrightness = 0.0f;  // Average brightness

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString FrameHash;  // SHA256 of captured frame

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float SimTimeMs = 0.0f;  // Simulation time to this point

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float StepReward = 0.0f;  // Reward from this step
};

/**
 * Executed blind test result with per-step measurements
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FExecutedBlindTestResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FBlindTestConfig Config;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 Seed = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FExecutedStepMeasurement> Steps;  // Per-step evidence

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float FinalScore = 0.0f;  // Overall score [0,1]

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float TotalDurationMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> FinalFrameHashes;  // Hashes of captured frames

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SystemPath;  // Path to created system

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Errors;
};

/**
 * Performance metrics
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVFXPerformanceMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float RecipeGenerationMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float RecipeExecutionMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float HeuristicLookupMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float TotalPipelineMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int64 MemoryUsedBytes = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 NumAPICallsMade = 0;
};

/**
 * Proof bundle - evidence that the system works
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FProofBundle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ProofId = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Version = TEXT("1.0");

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime GeneratedAt = FDateTime::UtcNow();

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString EngineVersion = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString PluginVersion = TEXT("");

	// Test results
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FTestCaseResult> TestResults;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FBlindTestResult> BlindTestResults;

	// Performance
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FVFXPerformanceMetrics Performance;

	// Summary
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 TotalTests = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 PassedTests = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 FailedTests = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float OverallSuccessRate = 0.0f;

	// Compliance
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bNoTemplatesUsed = true;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bDeterministic = true;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> ComplianceNotes;

	// Hashes for verification - INITIALIZED to eliminate nondeterminism
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString DataHash = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ConfigHash = TEXT("");
};

/**
 * Failure taxonomy entry
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FFailureTaxonomy
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString FailureClass;  // "RecipeGeneration", "StepExecution", "Validation"

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString FailureType;   // "MissingModule", "InvalidParam", etc.

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 OccurrenceCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> ExampleMessages;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SuggestedFix;
};

/**
 * VFX Evaluation System - Phase 7 Implementation
 * 
 * Prove this isn't theatre. Evidence only.
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UVFXEvaluator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Run a single blind regeneration test
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static FBlindTestResult RunBlindTest(
		const FBlindTestConfig& Config,
		const FVFXHeuristicsDB& Heuristics,
		const FEditDeltaDataset& LearningData);

	/**
	 * Run an EXECUTED blind test case with per-step measurements
	 * This actually spawns the system, simulates, and captures frames.
	 * @param Config - Blind test configuration
	 * @param Seed - Random seed for deterministic reproduction
	 * @param NumSteps - Number of simulation steps to run
	 * @param StepDeltaTime - Time per step in seconds
	 * @return Result with per-step measurements, frame hashes, and final score
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static FExecutedBlindTestResult RunExecutedBlindTest(
		const FBlindTestConfig& Config,
		int32 Seed,
		int32 NumSteps = 30,
		float StepDeltaTime = 0.033f);

	/**
	 * Run full evaluation suite
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static FProofBundle RunFullEvaluation(
		const FVFXHeuristicsDB& Heuristics,
		const FEditDeltaDataset& LearningData);

	/**
	 * Run performance benchmarks
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static FVFXPerformanceMetrics RunPerformanceBenchmarks(
		const FVFXHeuristicsDB& Heuristics,
		int32 NumIterations = 10);

	/**
	 * Verify no templates were used in generation
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static bool VerifyNoTemplatesUsed(const FVFXRecipe& Recipe);

	/**
	 * Verify deterministic execution
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static bool VerifyDeterministic(
		const FVFXRecipe& Recipe,
		int32 NumRuns = 3);

	/**
	 * Generate failure taxonomy from test results
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static TArray<FFailureTaxonomy> GenerateFailureTaxonomy(
		const FProofBundle& Proof);

	/**
	 * Save proof bundle to JSON
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static bool SaveProofBundle(
		const FProofBundle& Proof,
		const FString& FilePath);

	/**
	 * Load proof bundle from JSON
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static FProofBundle LoadProofBundle(const FString& FilePath);

	/**
	 * Get default blind test suite
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static TArray<FBlindTestConfig> GetDefaultBlindTests();

	/**
	 * Compute hash for proof bundle
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static FString ComputeProofHash(const FProofBundle& Proof);

	/**
	 * Verify proof bundle integrity
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static bool VerifyProofIntegrity(const FProofBundle& Proof);

	/**
	 * Generate compliance report
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static FString GenerateComplianceReport(const FProofBundle& Proof);

	/**
	 * Generate proof bundle ONLY from executed blind tests
	 * This is the real proof - not theatre.
	 * @param ExecutedResults - Results from RunExecutedBlindTest calls
	 * @param OutputPath - Path to save the proof JSON
	 * @return True if proof was generated and saved
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Evaluation")
	static bool GenerateRealProof(
		const TArray<FExecutedBlindTestResult>& ExecutedResults,
		const FString& OutputPath);

private:
	/** Generate a recipe from description (no templates) */
	static FVFXRecipe GenerateRecipeFromDescription(
		const FString& Category,
		const FString& StyleDescription,
		const TMap<FString, FString>& Inputs,
		const FVFXHeuristicsDB& Heuristics,
		const FEditDeltaDataset& LearningData);

	/** Run standard test cases */
	static TArray<FTestCaseResult> RunStandardTests(
		const FVFXHeuristicsDB& Heuristics,
		const FEditDeltaDataset& LearningData);

	/** Classify a failure */
	static void ClassifyFailure(
		const FString& ErrorMessage,
		TMap<FString, FFailureTaxonomy>& TaxonomyMap);
};
