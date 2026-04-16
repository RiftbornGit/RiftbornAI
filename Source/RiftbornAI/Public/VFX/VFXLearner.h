// Copyright Hivemind Studios. All Rights Reserved.
// VFXLearner.h - Connects NiagaraEnv RL loop to VFX learning system

#pragma once

#include "CoreMinimal.h"
#include "NiagaraProbeHarness.h"
#include "VFXEditDelta.h"
#include "VFXLearner.generated.h"

/**
 * Action-Outcome pair for learning
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVFXExperience
{
    GENERATED_BODY()
    
    // State before action
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FNiagaraObservation StateBefore;
    
    // Action taken
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FNiagaraAction Action;
    
    // State after action
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FNiagaraObservation StateAfter;
    
    // Actual reward received
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Reward = 0.0f;
    
    // Was action successful?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSuccess = false;
    
    // Timestamp
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FDateTime Timestamp;
};

/**
 * Action prediction with confidence
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVFXActionPrediction
{
    GENERATED_BODY()
    
    // Predicted action
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FNiagaraAction Action;
    
    // Expected reward
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float ExpectedReward = 0.0f;
    
    // Confidence (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Confidence = 0.0f;
    
    // Predicted success probability
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float SuccessProbability = 0.0f;
};

/**
 * Learning statistics
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVFXLearnerStats
{
    GENERATED_BODY()
    
    // Total experiences collected
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 TotalExperiences = 0;
    
    // Experiences per action type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, int32> ExperiencesPerPrimitive;
    
    // Average reward per action type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, float> AverageRewardPerPrimitive;
    
    // Success rate per action type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, float> SuccessRatePerPrimitive;
    
    // Total episodes completed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 EpisodesCompleted = 0;
    
    // Average episode reward
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float AverageEpisodeReward = 0.0f;
    
    // Best episode reward
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float BestEpisodeReward = 0.0f;
    
    // Targets achieved count
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 TargetsAchieved = 0;
};

/**
 * Exploration strategy
 */
UENUM(BlueprintType)
enum class EExplorationStrategy : uint8
{
    // Pure greedy (always pick best predicted action)
    Greedy,
    
    // Epsilon-greedy (random with probability epsilon)
    EpsilonGreedy,
    
    // Upper Confidence Bound (optimistic exploration)
    UCB,
    
    // Thompson Sampling (Bayesian exploration)
    Thompson,
    
    // Random (for baseline/testing)
    Random
};

/**
 * VFXLearner - Bridges NiagaraEnv RL loop to learning
 * 
 * This class:
 * 1. Collects experiences from NiagaraEnv
 * 2. Builds action-value estimates
 * 3. Suggests actions based on exploration strategy
 * 4. Integrates with VFXEditDelta for pattern learning
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UVFXLearner : public UObject
{
    GENERATED_BODY()
    
public:
    UVFXLearner();
    
    // ============================================================
    // EXPERIENCE COLLECTION
    // ============================================================
    
    /**
     * Record an experience (observation → action → reward)
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    void RecordExperience(const FVFXExperience& Experience);
    
    /**
     * Record from step result directly
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    void RecordStep(
        const FNiagaraObservation& StateBefore,
        const FNiagaraAction& Action,
        const FNiagaraStepResult& Result);
    
    /**
     * Record entire episode
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    void RecordEpisode(const FNiagaraEpisode& Episode);
    
    // ============================================================
    // ACTION SELECTION
    // ============================================================
    
    /**
     * Get best action for current state
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    FVFXActionPrediction GetBestAction(
        const FNiagaraObservation& CurrentState,
        const TArray<FNiagaraAction>& ValidActions) const;
    
    /**
     * Get action using exploration strategy
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    FVFXActionPrediction SelectAction(
        const FNiagaraObservation& CurrentState,
        const TArray<FNiagaraAction>& ValidActions);
    
    /**
     * Get ranked list of actions with predictions
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    TArray<FVFXActionPrediction> RankActions(
        const FNiagaraObservation& CurrentState,
        const TArray<FNiagaraAction>& ValidActions) const;
    
    // ============================================================
    // VALUE ESTIMATION
    // ============================================================
    
    /**
     * Estimate value of an action from current state
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    float EstimateActionValue(
        const FNiagaraObservation& State,
        const FNiagaraAction& Action) const;
    
    /**
     * Estimate action success probability
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    float EstimateSuccessProbability(
        const FNiagaraObservation& State,
        const FNiagaraAction& Action) const;
    
    /**
     * Get upper confidence bound for action
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    float GetUCBValue(
        const FNiagaraObservation& State,
        const FNiagaraAction& Action,
        float ExplorationConstant = 1.41f) const;
    
    // ============================================================
    // LEARNING
    // ============================================================
    
    /**
     * Update value estimates from experience buffer
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    void UpdateFromBuffer(int32 BatchSize = 32);
    
    /**
     * Decay exploration rate
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    void DecayExploration(float DecayRate = 0.995f);
    
    /**
     * Convert experiences to EditDelta format for pattern learning
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    void ExportToEditDeltaDataset(FEditDeltaDataset& OutDataset) const;
    
    /**
     * Import learned patterns from EditDelta dataset
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    void ImportFromEditDeltaDataset(const FEditDeltaDataset& Dataset);
    
    // ============================================================
    // STATISTICS
    // ============================================================
    
    /**
     * Get learning statistics
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    FVFXLearnerStats GetStats() const;
    
    /**
     * Get experience count
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    int32 GetExperienceCount() const { return ExperienceBuffer.Num(); }
    
    // ============================================================
    // PERSISTENCE
    // ============================================================
    
    /**
     * Save learner state to file
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    bool Save(const FString& FilePath) const;
    
    /**
     * Load learner state from file
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    bool Load(const FString& FilePath);
    
    /**
     * Reset all learning
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    void Reset();
    
    // ============================================================
    // CONFIGURATION
    // ============================================================
    
    /** Exploration strategy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    EExplorationStrategy Strategy = EExplorationStrategy::EpsilonGreedy;
    
    /** Epsilon for epsilon-greedy (probability of random action) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float Epsilon = 0.1f;
    
    /** Learning rate for value updates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float LearningRate = 0.1f;
    
    /** Discount factor for future rewards */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float DiscountFactor = 0.99f;
    
    /** Maximum experience buffer size */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 MaxBufferSize = 10000;
    
    /** UCB exploration constant */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float UCBConstant = 1.41f;
    
protected:
    // Experience replay buffer
    UPROPERTY()
    TArray<FVFXExperience> ExperienceBuffer;
    
    // Action-value estimates (keyed by action hash)
    UPROPERTY()
    TMap<FString, float> ActionValues;
    
    // Action counts (for UCB)
    UPROPERTY()
    TMap<FString, int32> ActionCounts;
    
    // Total action selections
    UPROPERTY()
    int32 TotalSelections = 0;
    
    // Completed episodes
    UPROPERTY()
    TArray<float> EpisodeRewards;
    
private:
    // Hash an action for lookup
    FString HashAction(const FNiagaraAction& Action) const;
    
    // Hash state-action pair
    FString HashStateAction(const FNiagaraObservation& State, const FNiagaraAction& Action) const;
    
    // Get primitive name from enum
    FString GetPrimitiveName(ENiagaraPrimitive Primitive) const;
    
    // Compute state features for similarity
    TArray<float> ExtractStateFeatures(const FNiagaraObservation& State) const;
    
    // Find similar experiences
    TArray<const FVFXExperience*> FindSimilarExperiences(
        const FNiagaraObservation& State,
        const FNiagaraAction& Action,
        int32 MaxCount = 10) const;
    
    // Sample from experience buffer
    TArray<FVFXExperience> SampleBatch(int32 BatchSize) const;
};

/**
 * VFXLearner utilities
 */
UCLASS()
class RIFTBORNAI_API UVFXLearnerUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    /**
     * Create a learner with default config
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    static UVFXLearner* CreateDefaultLearner();
    
    /**
     * Convert NiagaraAction to EditDelta type
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    static EEditDeltaType ActionToEditDeltaType(ENiagaraPrimitive Primitive);
    
    /**
     * Serialize experience to JSON
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    static FString ExperienceToJson(const FVFXExperience& Experience);
    
    /**
     * Serialize stats to JSON
     */
    UFUNCTION(BlueprintCallable, Category = "VFXLearner")
    static FString StatsToJson(const FVFXLearnerStats& Stats);
};
