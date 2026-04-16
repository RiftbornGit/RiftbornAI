// Copyright 2025 RiftbornAI. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameBalanceAnalyzer.generated.h"

/**
 * Type of balance metric being measured
 */
UENUM(BlueprintType)
enum class EBalanceMetricType : uint8
{
    Damage              UMETA(DisplayName = "Damage Output"),
    Health              UMETA(DisplayName = "Health/Survivability"),
    Speed               UMETA(DisplayName = "Movement Speed"),
    AttackSpeed         UMETA(DisplayName = "Attack Speed"),
    Range               UMETA(DisplayName = "Attack Range"),
    Cost                UMETA(DisplayName = "Resource Cost"),
    Cooldown            UMETA(DisplayName = "Cooldown Time"),
    AreaOfEffect        UMETA(DisplayName = "Area of Effect"),
    DamagePerSecond     UMETA(DisplayName = "DPS"),
    TimeToKill          UMETA(DisplayName = "Time to Kill"),
    WinRate             UMETA(DisplayName = "Win Rate"),
    PickRate            UMETA(DisplayName = "Pick/Usage Rate"),
    Utility             UMETA(DisplayName = "Utility Value"),
    Efficiency          UMETA(DisplayName = "Resource Efficiency"),
    Custom              UMETA(DisplayName = "Custom Metric")
};

/**
 * Balance concern severity
 */
UENUM(BlueprintType)
enum class EBalanceSeverity : uint8
{
    Broken      UMETA(DisplayName = "Broken - Game-breaking imbalance"),
    Severe      UMETA(DisplayName = "Severe - Major imbalance"),
    Moderate    UMETA(DisplayName = "Moderate - Noticeable imbalance"),
    Minor       UMETA(DisplayName = "Minor - Slight imbalance"),
    Optimal     UMETA(DisplayName = "Optimal - Well balanced")
};

/**
 * A single balance metric for an entity
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBalanceMetric
{
    GENERATED_BODY()

    // What this metric measures
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    EBalanceMetricType MetricType = EBalanceMetricType::Custom;

    // Custom metric name if type is Custom
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString CustomName;

    // The actual value
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float Value = 0.0f;

    // Normalized value (0-1 scale relative to others)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float NormalizedValue = 0.0f;

    // Expected/ideal value for balance
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float ExpectedValue = 0.0f;

    // Deviation from expected
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float Deviation = 0.0f;

    // Weight of this metric in overall balance calculation
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float Weight = 1.0f;
};

/**
 * Balance data for a single entity (character, weapon, ability, etc.)
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FEntityBalance
{
    GENERATED_BODY()

    // Entity identifier (name, ID, etc.)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString EntityId;

    // Display name
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString DisplayName;

    // Entity type (Character, Weapon, Ability, Item, etc.)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString EntityType;

    // All metrics for this entity
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FBalanceMetric> Metrics;

    // Overall balance score (0-100, 50 = perfectly balanced)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float BalanceScore = 50.0f;

    // Power level relative to average
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float PowerLevel = 1.0f;

    // How this compares to intended role
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString RoleAssessment;

    // Severity of any balance issues
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    EBalanceSeverity Severity = EBalanceSeverity::Optimal;
};

/**
 * Comparison between two entities
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBalanceComparison
{
    GENERATED_BODY()

    // First entity
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Entity1;

    // Second entity
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Entity2;

    // Which metrics Entity1 is better at
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> Entity1Advantages;

    // Which metrics Entity2 is better at
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> Entity2Advantages;

    // Overall comparison verdict
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Verdict;

    // Power difference (positive = Entity1 stronger)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float PowerDifference = 0.0f;

    // Whether this matchup is fair
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    bool bFairMatchup = true;
};

/**
 * A balance concern/issue that needs attention
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBalanceConcern
{
    GENERATED_BODY()

    // Concern ID
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString ConcernId = TEXT("");

    // Title of the concern
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Title;

    // Detailed description
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Description;

    // Affected entities
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> AffectedEntities;

    // Severity
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    EBalanceSeverity Severity = EBalanceSeverity::Minor;

    // Suggested fix
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString SuggestedFix;

    // Specific number adjustments suggested
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TMap<FString, float> SuggestedAdjustments;

    // Impact if not fixed
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString ImpactDescription;

    // When detected
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FDateTime DetectedTime = FDateTime();

    FBalanceConcern() = default;
};

/**
 * Meta analysis for the entire game
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FGameMetaAnalysis
{
    GENERATED_BODY()

    // Time to kill analysis
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float AverageTTK = 0.0f;

    // TTK variance
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float TTKVariance = 0.0f;

    // Power creep index (higher = more power creep)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float PowerCreepIndex = 0.0f;

    // Diversity score (how many viable options)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float DiversityScore = 0.0f;

    // Skill ceiling assessment
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float SkillCeilingScore = 0.0f;

    // Accessibility score
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float AccessibilityScore = 0.0f;

    // Most dominant entity
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString DominantEntity;

    // Weakest entity
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString WeakestEntity;

    // Overall health score
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float OverallHealthScore = 0.0f;

    // Recommendations
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> Recommendations;
};

/**
 * Balance profile for a category of entities
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBalanceProfile
{
    GENERATED_BODY()

    // Category name (e.g., "Assault Rifles", "Tank Characters")
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString CategoryName;

    // All entities in this category
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FEntityBalance> Entities;

    // Average metrics for the category
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TMap<FString, float> AverageMetrics;

    // Standard deviation for metrics
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TMap<FString, float> MetricStdDev;

    // Outliers
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> Outliers;

    // Category health score
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float HealthScore = 0.0f;
};

/**
 * Game Balance Analyzer - game balance analysis and diagnostics
 */
class RIFTBORNAI_API FGameBalanceAnalyzer
{
public:
    static FGameBalanceAnalyzer& Get();

    // Entity management
    void RegisterEntity(const FEntityBalance& Entity);
    void UpdateEntityMetric(const FString& EntityId, EBalanceMetricType Type, float Value);
    void RemoveEntity(const FString& EntityId);
    FEntityBalance GetEntity(const FString& EntityId) const;
    TArray<FEntityBalance> GetAllEntities() const { return RegisteredEntities; }

    // Analysis
    FEntityBalance AnalyzeEntity(const FString& EntityId);
    FBalanceComparison CompareEntities(const FString& Entity1, const FString& Entity2);
    TArray<FBalanceConcern> AnalyzeAllBalance();
    FGameMetaAnalysis PerformMetaAnalysis();

    // Category analysis
    void CreateCategory(const FString& CategoryName);
    void AddEntityToCategory(const FString& EntityId, const FString& CategoryName);
    FBalanceProfile AnalyzeCategory(const FString& CategoryName);
    TArray<FBalanceProfile> GetAllCategories() const;

    // Recommendations
    TArray<FString> GetBalanceRecommendations(const FString& EntityId);
    TMap<FString, float> SuggestStatAdjustments(const FString& EntityId);
    FString ExplainBalance(const FString& EntityId);

    // Query interface
    FString QueryBalance(const FString& Query);
    TArray<FEntityBalance> FindOverpowered(float Threshold = 1.3f);
    TArray<FEntityBalance> FindUnderpowered(float Threshold = 0.7f);
    TArray<FBalanceComparison> FindUnfairMatchups();

    // Simulation
    float SimulateTTK(const FString& Attacker, const FString& Defender);
    float SimulateDPS(const FString& EntityId);
    float CalculateEfficiency(const FString& EntityId);

    // Export
    FString ExportBalanceReport() const;
    FString ExportAsJSON() const;
    void SaveToFile(const FString& FilePath);

    // Configuration
    void SetBalanceThreshold(float Threshold) { BalanceThreshold = Threshold; }
    void SetWeightForMetric(EBalanceMetricType Type, float Weight);

private:
    FGameBalanceAnalyzer() = default;

    void RecalculateNormalizedValues();
    void DetectOutliers(FBalanceProfile& Profile);
    float CalculateBalanceScore(const FEntityBalance& Entity);
    EBalanceSeverity DetermineSeverity(float Deviation);

    TArray<FEntityBalance> RegisteredEntities;
    TMap<FString, FBalanceProfile> Categories;
    TArray<FBalanceConcern> ActiveConcerns;

    TMap<EBalanceMetricType, float> MetricWeights;
    float BalanceThreshold = 0.2f; // 20% deviation is concerning
};
