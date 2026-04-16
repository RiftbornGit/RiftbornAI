// Copyright RiftbornAI. All Rights Reserved.
// VFX Edit Delta Tracking - Phase 6 Implementation

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "VFXRecipe.h"
#include "VFXEditDelta.generated.h"

/**
 * Type of edit made
 */
UENUM(BlueprintType)
enum class EEditDeltaType : uint8
{
	ParamChanged,       // Parameter value changed
	ModuleAdded,        // Module added to emitter
	ModuleRemoved,      // Module removed
	EmitterAdded,       // Emitter added to system
	EmitterRemoved,     // Emitter removed
	RendererChanged,    // Renderer property changed
	MaterialChanged,    // Material swapped
	ConnectionChanged,  // Graph connection changed
	Unknown
};

/**
 * Single parameter change
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FParamChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString EmitterName;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ModuleName;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ParamPath;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ParamType;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString OldValue;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString NewValue;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float ChangePercent = 0.0f;  // Relative change for numeric values
};

/**
 * Single edit delta - what changed and when
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FEditDelta
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString DeltaId;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime Timestamp = FDateTime::UtcNow();

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	EEditDeltaType DeltaType = EEditDeltaType::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SystemPath;  // Which system was edited

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString EditDescription;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FParamChange> ParamChanges;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString BeforeSnapshotHash;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString AfterSnapshotHash;

	// Session info
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 EditSequenceNumber = 0;  // Order within session
};

/**
 * Causal hypothesis - what effect did this edit have?
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FCausalHypothesis
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString HypothesisId;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString DeltaId;  // Which edit this hypothesis is about

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ParamPath;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString HypothesisText;  // "Increasing Rate made fire denser"

	// Observed effects
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TMap<FString, float> EffectScores;  // "density" -> 0.8, "brightness" -> 0.3

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float Confidence = 0.5f;  // How sure we are

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 ObservationCount = 1;  // Times this pattern was seen
};

/**
 * Edit session - a sequence of related edits
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FEditSession
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime StartTime = FDateTime::UtcNow();

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime EndTime;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SystemPath;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FEditDelta> Deltas;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString InitialSnapshotHash;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString FinalSnapshotHash;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FCausalHypothesis> Hypotheses;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bSuccessfulSession = false;  // Did artist like the result?
};

/**
 * Edit delta dataset - all recorded edits
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FEditDeltaDataset
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Version = TEXT("1.0");

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FEditSession> Sessions;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FCausalHypothesis> AggregatedHypotheses;  // Cross-session patterns

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 TotalEdits = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 TotalSessions = 0;
};

/**
 * VFX Edit Tracker - Captures human VFX editing patterns
 * 
 * Phase 6: Editor-Introspective Learning
 * 
 * Key Principle: Learn from WHAT humans do, not WHAT they say.
 * - Track parameter changes as they happen
 * - Capture before/after snapshots
 * - Generate causal hypotheses about effects
 * - Build a dataset of "artist intuition"
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UVFXEditTracker : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Start a new edit session for a Niagara system
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static FString StartSession(UNiagaraSystem* System);

	/**
	 * Record an edit delta
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static void RecordDelta(
		const FString& SessionId,
		UNiagaraSystem* System,
		EEditDeltaType DeltaType,
		const TArray<FParamChange>& ParamChanges,
		const FString& Description);

	/**
	 * End an edit session
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static FEditSession EndSession(const FString& SessionId, bool bSuccessful);

	/**
	 * Compare two system states and generate deltas
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static TArray<FParamChange> ComputeDeltas(
		UNiagaraSystem* Before,
		UNiagaraSystem* After);

	/**
	 * Generate causal hypotheses from a delta
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static TArray<FCausalHypothesis> GenerateHypotheses(const FEditDelta& Delta);

	/**
	 * Aggregate hypotheses across sessions
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static TArray<FCausalHypothesis> AggregateHypotheses(const FEditDeltaDataset& Dataset);

	/**
	 * Save dataset to JSON
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static bool SaveDataset(const FEditDeltaDataset& Dataset, const FString& FilePath);

	/**
	 * Load dataset from JSON
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static bool LoadDataset(const FString& FilePath, FEditDeltaDataset& OutDataset);

	/**
	 * Get active sessions
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static TArray<FString> GetActiveSessions();

	/**
	 * Get session by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static FEditSession GetSession(const FString& SessionId);

	/**
	 * Learn recipe improvements from dataset
	 * Applies learned patterns to improve a recipe
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static FVFXRecipe ApplyLearnedPatterns(
		const FEditDeltaDataset& Dataset,
		const FVFXRecipe& Recipe);

	/**
	 * Get hypothesis quality score
	 * Based on observation count and consistency
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|EditTracker")
	static float GetHypothesisQuality(const FCausalHypothesis& Hypothesis);

private:
	/** Active sessions (in-memory) */
	static TMap<FString, FEditSession> ActiveSessions;

	/** Compute snapshot hash for a system state */
	static FString ComputeSnapshotHash(UNiagaraSystem* System);

	/** Extract all parameter values from a system */
	static TMap<FString, FString> ExtractAllParams(UNiagaraSystem* System);

	/** Map parameter change to likely visual effects */
	static TMap<FString, float> InferEffects(const FParamChange& Change);
};
