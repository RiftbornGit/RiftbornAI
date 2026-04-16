// Copyright RiftbornAI. All Rights Reserved.
// EnvironmentSteward.h - Level 4: Proactive Environment Monitoring and Health Management

#pragma once

#include "CoreMinimal.h"
#include "Expectation.h"
#include "AgentPlan.h"

class UWorld;

/**
 * Health status categories
 */
enum class EHealthStatus : uint8
{
    Healthy,        // All metrics within normal range
    Warning,        // Some metrics approaching thresholds
    Degraded,       // Metrics outside normal range, intervention needed
    Critical,       // System in failure state
    Unknown         // Cannot assess health
};

/**
 * Trend direction for metrics
 */
enum class ETrendDirection : uint8
{
    Improving,      // Getting better
    Stable,         // No significant change
    Declining,      // Getting worse
    Volatile,       // Unpredictable swings
    Unknown         // Insufficient data
};

/**
 * FHealthMetric - Single tracked health indicator
 * 
 * Tracks a specific aspect of environment health over time:
 * - Actor count stability
 * - Tool success rates
 * - Expectation violation rates
 * - Response latencies
 * - Memory/resource usage
 */
struct RIFTBORNAI_API FHealthMetric
{
    /** Unique metric identifier */
    FName MetricId;
    
    /** Human-readable name */
    FString DisplayName;
    
    /** Current value */
    float CurrentValue = 0.0f;
    
    /** Historical values (ring buffer, last N samples) */
    TArray<float> History;
    
    /** Timestamps for history entries */
    TArray<FDateTime> HistoryTimestamps;
    
    /** Maximum history size */
    static constexpr int32 MaxHistorySize = 100;
    
    /** Normal range - values outside trigger warnings */
    float NormalMin = 0.0f;
    float NormalMax = 1.0f;
    
    /** Critical thresholds - values outside are critical */
    float CriticalMin = -0.5f;
    float CriticalMax = 1.5f;
    
    /** Trend detection window (last N samples) */
    int32 TrendWindow = 10;
    
    /** Current trend */
    ETrendDirection Trend = ETrendDirection::Unknown;
    
    /** Current status based on value */
    EHealthStatus Status = EHealthStatus::Unknown;
    
    /** Decay rate per hour for old values (exponential decay) */
    float DecayRate = 0.1f;

    /** Whether lower values are better (affects trend interpretation) */
    bool bLowerIsBetter = false;
    
    FHealthMetric() = default;
    
    FHealthMetric(FName InId, const FString& InName, float InMin, float InMax, bool bInLowerIsBetter = false)
        : MetricId(InId)
        , DisplayName(InName)
        , NormalMin(InMin)
        , NormalMax(InMax)
        , CriticalMin(InMin - (InMax - InMin) * 0.5f)
        , CriticalMax(InMax + (InMax - InMin) * 0.5f)
        , bLowerIsBetter(bInLowerIsBetter)
    {
    }
    
    /** Record a new value */
    void RecordValue(float Value)
    {
        CurrentValue = Value;
        
        // Add to history
        History.Add(Value);
        HistoryTimestamps.Add(FDateTime::UtcNow());
        
        // Trim history if too large
        while (History.Num() > MaxHistorySize)
        {
            History.RemoveAt(0);
            HistoryTimestamps.RemoveAt(0);
        }
        
        UpdateStatus();
        UpdateTrend();
    }
    
    /** Get average of recent values */
    float GetRecentAverage(int32 Count = 10) const
    {
        if (History.Num() == 0) return 0.0f;
        
        float Sum = 0.0f;
        int32 N = FMath::Min(Count, History.Num());
        for (int32 i = History.Num() - N; i < History.Num(); i++)
        {
            Sum += History[i];
        }
        return Sum / N;
    }
    
    /** Get standard deviation of recent values */
    float GetRecentStdDev(int32 Count = 10) const
    {
        if (History.Num() < 2) return 0.0f;
        
        float Mean = GetRecentAverage(Count);
        float SumSq = 0.0f;
        int32 N = FMath::Min(Count, History.Num());
        for (int32 i = History.Num() - N; i < History.Num(); i++)
        {
            float Diff = History[i] - Mean;
            SumSq += Diff * Diff;
        }
        return FMath::Sqrt(SumSq / N);
    }
    
    /** Calculate trend over window */
    void UpdateTrend()
    {
        if (History.Num() < TrendWindow)
        {
            Trend = ETrendDirection::Unknown;
            return;
        }
        
        // Simple linear regression over trend window
        int32 Start = History.Num() - TrendWindow;
        float SumX = 0.0f, SumY = 0.0f, SumXY = 0.0f, SumXX = 0.0f;
        
        for (int32 i = 0; i < TrendWindow; i++)
        {
            float X = static_cast<float>(i);
            float Y = History[Start + i];
            SumX += X;
            SumY += Y;
            SumXY += X * Y;
            SumXX += X * X;
        }
        
        float N = static_cast<float>(TrendWindow);
        float Slope = (N * SumXY - SumX * SumY) / (N * SumXX - SumX * SumX);
        float StdDev = GetRecentStdDev(TrendWindow);
        
        // Classify trend based on slope relative to std dev
        float SlopeThreshold = StdDev * 0.1f;  // Significant if slope > 10% of std dev per sample
        
        if (FMath::Abs(Slope) < SlopeThreshold)
        {
            Trend = ETrendDirection::Stable;
        }
        else if (Slope > 0)
        {
            // Positive slope - good or bad depending on metric
            Trend = bLowerIsBetter ? ETrendDirection::Declining : ETrendDirection::Improving;
        }
        else
        {
            Trend = bLowerIsBetter ? ETrendDirection::Improving : ETrendDirection::Declining;
        }
        
        // Check for volatility (high std dev relative to mean)
        float Mean = GetRecentAverage(TrendWindow);
        if (Mean != 0.0f && StdDev / FMath::Abs(Mean) > 0.5f)
        {
            Trend = ETrendDirection::Volatile;
        }
    }
    
    /** Update status based on current value */
    void UpdateStatus()
    {
        if (CurrentValue < CriticalMin || CurrentValue > CriticalMax)
        {
            Status = EHealthStatus::Critical;
        }
        else if (CurrentValue < NormalMin || CurrentValue > NormalMax)
        {
            Status = EHealthStatus::Degraded;
        }
        else
        {
            // Check if approaching thresholds
            float Range = NormalMax - NormalMin;
            float WarningMargin = Range * 0.2f;
            if (CurrentValue < NormalMin + WarningMargin || CurrentValue > NormalMax - WarningMargin)
            {
                Status = EHealthStatus::Warning;
            }
            else
            {
                Status = EHealthStatus::Healthy;
            }
        }
    }
    
    /** Predict future value based on trend */
    float PredictValue(int32 StepsAhead = 10) const
    {
        if (History.Num() < TrendWindow) return CurrentValue;
        
        // Linear extrapolation based on recent trend
        int32 Start = History.Num() - TrendWindow;
        float SumX = 0.0f, SumY = 0.0f, SumXY = 0.0f, SumXX = 0.0f;
        
        for (int32 i = 0; i < TrendWindow; i++)
        {
            float X = static_cast<float>(i);
            float Y = History[Start + i];
            SumX += X;
            SumY += Y;
            SumXY += X * Y;
            SumXX += X * X;
        }
        
        float N = static_cast<float>(TrendWindow);
        float Slope = (N * SumXY - SumX * SumY) / (N * SumXX - SumX * SumX);
        float Intercept = (SumY - Slope * SumX) / N;
        
        return Intercept + Slope * (TrendWindow + StepsAhead);
    }
    
    /** Time to critical if declining */
    int32 StepsUntilCritical() const
    {
        if (Trend != ETrendDirection::Declining || History.Num() < TrendWindow)
        {
            return -1;  // Not declining or insufficient data
        }
        
        // Binary search for when prediction hits critical
        for (int32 Steps = 1; Steps <= 1000; Steps++)
        {
            float Predicted = PredictValue(Steps);
            if (Predicted < CriticalMin || Predicted > CriticalMax)
            {
                return Steps;
            }
        }
        return -1;  // Won't hit critical in foreseeable future
    }
};

/**
 * FDegradationAlert - Detected degradation that needs attention
 */
struct RIFTBORNAI_API FDegradationAlert
{
    /** Which metric is degrading */
    FName MetricId;
    
    /** Current status */
    EHealthStatus Status;
    
    /** Trend direction */
    ETrendDirection Trend;
    
    /** Predicted steps until critical (if applicable) */
    int32 StepsUntilCritical = -1;
    
    /** Suggested intervention (tool call) */
    FString SuggestedIntervention;
    
    /** Arguments for intervention */
    FString InterventionArgs;
    
    /** Severity score (0-1, higher = more urgent) */
    float Severity = 0.0f;

    /** Baseline value (if available) */
    float BaselineValue = 0.0f;

    /** Baseline standard deviation (if available) */
    float BaselineStdDev = 0.0f;

    /** Was this flagged as an anomaly relative to baseline? */
    bool bIsAnomaly = false;

    /** Explanation for anomaly detection */
    FString AnomalyReason;
    
    /** When this alert was created */
    FDateTime CreatedAt;
    
    /** Has this alert been addressed? */
    bool bAddressed = false;
    
    FDegradationAlert()
    {
        CreatedAt = FDateTime::UtcNow();
    }
};

/**
 * FProactiveGoal - Goal generated by the steward to maintain health
 */
struct RIFTBORNAI_API FProactiveGoal
{
    /** What triggered this goal */
    FDegradationAlert TriggerAlert;
    
    /** The goal to achieve */
    FAgentGoal Goal;
    
    /** Priority (higher = more urgent) */
    float Priority = 0.0f;

    /** Estimated intervention cost (0-1) */
    float EstimatedCost = 0.0f;

    /** Expected benefit (0-1) */
    float ExpectedBenefit = 0.0f;
    
    /** Created at */
    FDateTime CreatedAt;
    
    /** Deadline (if applicable) */
    FDateTime Deadline;
    
    /** Has been executed? */
    bool bExecuted = false;

    /** Is currently in progress? */
    bool bInProgress = false;
    
    /** Did execution succeed? */
    bool bSucceeded = false;
    
    FProactiveGoal()
    {
        CreatedAt = FDateTime::UtcNow();
    }
};

/**
 * FTrustAssessment - Self-assessment of tool reliability and uncertainty
 */
struct RIFTBORNAI_API FTrustAssessment
{
    /** Overall confidence (0-1) */
    float Confidence = 0.0f;

    /** Uncertainty estimate (0-1, higher = more uncertain) */
    float Uncertainty = 1.0f;

    /** Expected failure rate (0-1) */
    float ExpectedFailureRate = 1.0f;

    /** Should we escalate to human confirmation? */
    bool bEscalate = false;

    /** Explanation for the assessment */
    FString Explanation;

    /** Risk factors that drove the decision */
    TArray<FString> RiskFactors;
};

/**
 * FEnvironmentSteward - Level 4: Proactive Environment Health Manager
 * 
 * The steward continuously monitors environment health:
 * 1. Tracks multiple health metrics over time
 * 2. Detects degradation trends before they become failures
 * 3. Generates proactive goals to maintain health
 * 4. Balances short-term success vs long-term stability
 * 
 * This is the transition from REACTIVE to PROACTIVE:
 * - Level 3: Execute plans when given goals
 * - Level 4: Generate goals based on observed degradation
 */
class RIFTBORNAI_API FEnvironmentSteward
{
public:
    static FEnvironmentSteward& Get()
    {
        static FEnvironmentSteward Instance;
        return Instance;
    }
    
    /** Initialize steward with default metrics */
    void Initialize();

    /** Tick steward on a cadence (updates metrics, baselines, goals) */
    void Tick(UWorld* World = nullptr);
    
    /** Update all metrics from current environment state */
    void UpdateMetrics(UWorld* World = nullptr);
    
    /** Register a new health metric to track */
    void RegisterMetric(const FHealthMetric& Metric);
    
    /** Record a value for a specific metric */
    void RecordMetricValue(FName MetricId, float Value);
    
    /** Get current health status (worst of all metrics) */
    EHealthStatus GetOverallHealth() const;
    
    /** Get health of a specific metric */
    EHealthStatus GetMetricHealth(FName MetricId) const;
    
    /** Check all metrics for degradation, generate alerts */
    TArray<FDegradationAlert> CheckForDegradation();
    
    /** Generate proactive goals from current alerts */
    TArray<FProactiveGoal> GenerateProactiveGoals();
    
    /** Get highest priority pending goal */
    FProactiveGoal* GetNextProactiveGoal();

    /** Mark goal as in progress */
    bool MarkGoalInProgress(const FGuid& GoalId);
    
    /** Mark a goal as executed with result */
    void RecordGoalOutcome(const FProactiveGoal& Goal, bool bSuccess);

    /** Mark goal outcome by GoalId */
    bool RecordGoalOutcomeById(const FGuid& GoalId, bool bSuccess);
    
    /** Get intervention suggestion for a degrading metric */
    FString GetInterventionForMetric(FName MetricId) const;

    /** Assess trust and uncertainty for a set of tools */
    FTrustAssessment AssessTrustForTools(const TArray<FString>& ToolNames) const;

    /** Should we escalate based on trust assessment? */
    bool ShouldEscalate(const FTrustAssessment& Assessment) const;

    /** Validate current world state against baseline */
    bool ValidateWorldState(UWorld* World, FString& OutSummary, bool bRebaseline = false);
    
    /** Save steward state to disk (persistence) */
    bool SaveState(const FString& FilePath);
    
    /** Load steward state from disk */
    bool LoadState(const FString& FilePath);
    
    /** Get all metrics for inspection */
    const TMap<FName, FHealthMetric>& GetAllMetrics() const { return Metrics; }
    
    /** Get all pending alerts */
    const TArray<FDegradationAlert>& GetPendingAlerts() const { return PendingAlerts; }
    
    /** Get all proactive goals (pending and completed) */
    const TArray<FProactiveGoal>& GetProactiveGoals() const { return ProactiveGoals; }
    
    /** Get success rate of proactive interventions */
    float GetInterventionSuccessRate() const;

    /** Do we have an established baseline? */
    bool HasBaseline() const { return BaselineValues.Num() > 0; }

    /** Do we have a world baseline? */
    bool HasWorldBaseline() const { return bHasWorldBaseline; }

    /** Get baseline value for a metric (0 if not available) */
    float GetBaselineValue(FName MetricId) const;
    
    /** How many steps ahead to predict for early warning */
    int32 PredictionHorizon = 20;
    
    /** Minimum severity to generate alert */
    float AlertSeverityThreshold = 0.3f;
    
    /** Maximum number of concurrent proactive goals */
    int32 MaxConcurrentGoals = 3;

    /** Steward update cadence */
    float UpdateIntervalSeconds = 10.0f;

    /** Baseline sampling window */
    int32 BaselineSampleWindow = 20;

    /** Standard deviation multiplier for anomalies */
    float BaselineStdDevThreshold = 2.0f;

    /** How often to refresh world baseline (hours) */
    float WorldBaselineRefreshHours = 24.0f;

    /** Minimum priority to auto-execute proactive goals */
    float ProactivePriorityThreshold = 0.35f;

    /** Benefit-to-cost threshold to accept a proactive goal */
    float CostBenefitThreshold = 0.8f;

    /** Confidence threshold for escalation */
    float ConfidenceEscalationThreshold = 0.4f;

    /** Surprise threshold for escalation */
    float SurpriseEscalationThreshold = 0.6f;

    /** Pattern mining cadence */
    float PatternMiningIntervalSeconds = 300.0f;

    /** Experience half-life for forgetting (hours) */
    float ExperienceHalfLifeHours = 48.0f;
    
private:
    FEnvironmentSteward() = default;
    
    /** All tracked health metrics */
    TMap<FName, FHealthMetric> Metrics;
    
    /** Current pending alerts */
    TArray<FDegradationAlert> PendingAlerts;
    
    /** Proactive goals (queue) */
    TArray<FProactiveGoal> ProactiveGoals;
    
    /** Intervention mappings: metric -> suggested tool */
    TMap<FName, TPair<FString, FString>> InterventionMap;
    
    /** Initialize default intervention mappings */
    void InitializeInterventionMap();

    /** Update baseline values when healthy */
    void UpdateBaseline();

    /** Check if a metric is anomalous relative to baseline */
    bool IsAnomaly(const FHealthMetric& Metric, float& OutDeviation, float& OutBaseline, float& OutStdDev) const;
    
    /** Calculate severity from metric state */
    float CalculateSeverity(const FHealthMetric& Metric) const;

    /** Estimate intervention cost */
    float EstimateInterventionCost(const FString& ToolName) const;

    /** Estimate intervention benefit */
    float EstimateInterventionBenefit(const FHealthMetric& Metric, const FDegradationAlert& Alert) const;
    
    /** Create goal from alert */
    FProactiveGoal CreateGoalFromAlert(const FDegradationAlert& Alert);

    /** Apply learned patterns from trajectories */
    void ApplyExperiencePatterns();
    
    /** Intervention success tracking */
    int32 TotalInterventions = 0;
    int32 SuccessfulInterventions = 0;

    /** Baseline values for anomaly detection */
    TMap<FName, float> BaselineValues;
    TMap<FName, float> BaselineStdDev;
    FDateTime LastBaselineUpdate;

    /** World baseline snapshot */
    FWorldStateDigest WorldBaseline;
    bool bHasWorldBaseline = false;
    FDateTime LastWorldBaselineUpdate;

    /** Tick tracking */
    double LastUpdateSeconds = 0.0;
    double LastPatternMiningSeconds = 0.0;

    /** Initialization guard */
    bool bInitialized = false;
};

/**
 * Standard health metrics tracked by the steward
 */
namespace HealthMetrics
{
    // Tool success rate (0-1)
    static const FName ToolSuccessRate(TEXT("tool_success_rate"));
    
    // Expectation violation rate (0-1, lower is better)
    static const FName ExpectationViolationRate(TEXT("expectation_violation_rate"));
    
    // Average tool trust (0-1, higher is better)
    static const FName AverageToolTrust(TEXT("average_tool_trust"));
    
    // Actor count stability (variance in actor counts)
    static const FName ActorCountStability(TEXT("actor_count_stability"));
    
    // Plan success rate (0-1)
    static const FName PlanSuccessRate(TEXT("plan_success_rate"));
    
    // Response latency (ms, lower is better)
    static const FName ResponseLatency(TEXT("response_latency"));
    
    // Blocked tool count (lower is better)
    static const FName BlockedToolCount(TEXT("blocked_tool_count"));
    
    // Claim calibration precision (0-1, higher is better)
    static const FName ClaimPrecision(TEXT("claim_precision"));
}
