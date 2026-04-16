// Copyright RiftbornAI. All Rights Reserved.
// PerformanceMetrics.h - Tool execution metrics, diagnostics, and performance tracking

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"

/**
 * Individual metric for a single tool execution
 */
struct RIFTBORNAI_API FToolExecutionMetric
{
    // Tool identification
    FString ToolName;
    FString ToolCategory;
    
    // Timing
    double StartTime = 0.0;
    double EndTime = 0.0;
    double DurationMs = 0.0;
    
    // Result
    bool bSuccess = false;
    FString ErrorCategory;
    
    // Context
    int32 InputSizeBytes = 0;
    int32 OutputSizeBytes = 0;
    FString SessionId;
    
    // Calculate duration
    void Complete(bool bWasSuccessful, const FString& Error = TEXT(""))
    {
        EndTime = FPlatformTime::Seconds();
        DurationMs = (EndTime - StartTime) * 1000.0;
        bSuccess = bWasSuccessful;
        ErrorCategory = Error;
    }
};

/**
 * Aggregated statistics for a tool
 */
struct RIFTBORNAI_API FToolStatistics
{
    FString ToolName;
    
    // Counts
    int64 TotalExecutions = 0;
    int64 SuccessCount = 0;
    int64 FailureCount = 0;
    
    // Timing
    double TotalTimeMs = 0.0;
    double MinTimeMs = DBL_MAX;
    double MaxTimeMs = 0.0;
    double AvgTimeMs = 0.0;
    
    // P50, P90, P99 (requires sample collection)
    double P50Ms = 0.0;
    double P90Ms = 0.0;
    double P99Ms = 0.0;
    
    // Error breakdown
    TMap<FString, int32> ErrorCategories;
    
    // Data throughput
    int64 TotalInputBytes = 0;
    int64 TotalOutputBytes = 0;
    
    // Success rate
    float GetSuccessRate() const
    {
        return TotalExecutions > 0 ? (float)SuccessCount / (float)TotalExecutions : 0.0f;
    }
    
    // Add a sample
    void AddSample(const FToolExecutionMetric& Metric)
    {
        TotalExecutions++;
        if (Metric.bSuccess)
        {
            SuccessCount++;
        }
        else
        {
            FailureCount++;
            ErrorCategories.FindOrAdd(Metric.ErrorCategory)++;
        }
        
        TotalTimeMs += Metric.DurationMs;
        MinTimeMs = FMath::Min(MinTimeMs, Metric.DurationMs);
        MaxTimeMs = FMath::Max(MaxTimeMs, Metric.DurationMs);
        AvgTimeMs = TotalTimeMs / TotalExecutions;
        
        TotalInputBytes += Metric.InputSizeBytes;
        TotalOutputBytes += Metric.OutputSizeBytes;
    }
};

/**
 * Session-level metrics
 */
struct RIFTBORNAI_API FSessionMetrics
{
    FString SessionId;
    FDateTime StartTime;
    FDateTime EndTime;
    
    // Tool usage
    int32 TotalToolCalls = 0;
    int32 SuccessfulCalls = 0;
    int32 FailedCalls = 0;
    
    // Timing
    double TotalDurationMs = 0.0;
    
    // LLM interactions (if tracked)
    int32 LLMCalls = 0;
    int32 TokensIn = 0;
    int32 TokensOut = 0;
    
    // Tool breakdown
    TMap<FString, int32> ToolUsageCounts;
    
    // Most used tools
    TArray<TPair<FString, int32>> GetTopTools(int32 Limit = 10) const
    {
        TArray<TPair<FString, int32>> Sorted;
        for (const auto& Pair : ToolUsageCounts)
        {
            Sorted.Add(TPair<FString, int32>(Pair.Key, Pair.Value));
        }
        Sorted.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B) {
            return A.Value > B.Value;
        });
        if (Sorted.Num() > Limit)
        {
            Sorted.SetNum(Limit);
        }
        return Sorted;
    }
};

/**
 * Performance alert threshold
 */
struct RIFTBORNAI_API FPerformanceAlert
{
    FString AlertType;
    FString Message;
    FString ToolName;
    FDateTime Timestamp;
    double Value;
    double Threshold;
};

/**
 * Singleton performance metrics collector and analyzer
 * 
 * Usage:
 *     // Start timing a tool
 *     FToolExecutionMetric Metric = FPerformanceMetrics::Get().StartToolExecution("spawn_actor");
 *     
 *     // ... execute tool ...
 *     
 *     // Complete timing
 *     FPerformanceMetrics::Get().CompleteToolExecution(Metric, bSuccess, ErrorMessage);
 *     
 *     // Get statistics
 *     FToolStatistics Stats = FPerformanceMetrics::Get().GetToolStatistics("spawn_actor");
 */
class RIFTBORNAI_API FPerformanceMetrics
{
public:
    static FPerformanceMetrics& Get();
    
    // ========================================
    // Tool Execution Tracking
    // ========================================
    
    /**
     * Start tracking a tool execution
     * @return Metric struct with start time populated
     */
    FToolExecutionMetric StartToolExecution(const FString& ToolName, const FString& Category = TEXT(""));
    
    /**
     * Complete a tool execution tracking
     * @param Metric The metric from StartToolExecution
     * @param bSuccess Whether the tool succeeded
     * @param ErrorCategory Optional error category if failed
     */
    void CompleteToolExecution(FToolExecutionMetric& Metric, bool bSuccess, const FString& ErrorCategory = TEXT(""));
    
    /**
     * Record a complete tool execution in one call
     */
    void RecordToolExecution(const FString& ToolName, double DurationMs, bool bSuccess, 
                              const FString& ErrorCategory = TEXT(""), int32 InputBytes = 0, int32 OutputBytes = 0);
    
    // ========================================
    // Statistics Retrieval
    // ========================================
    
    /**
     * Get statistics for a specific tool
     */
    FToolStatistics GetToolStatistics(const FString& ToolName) const;
    
    /**
     * Get statistics for all tools
     */
    TMap<FString, FToolStatistics> GetAllToolStatistics() const;
    
    /**
     * Get top N slowest tools by average time
     */
    TArray<TPair<FString, double>> GetSlowestTools(int32 Limit = 10) const;
    
    /**
     * Get tools with lowest success rates
     */
    TArray<TPair<FString, float>> GetUnreliableTools(int32 Limit = 10) const;
    
    /**
     * Get most frequently used tools
     */
    TArray<TPair<FString, int64>> GetMostUsedTools(int32 Limit = 10) const;
    
    // ========================================
    // Session Management
    // ========================================
    
    /**
     * Start a new metrics session
     */
    FString StartSession();
    
    /**
     * End the current session
     */
    void EndSession();
    
    /**
     * Get current session metrics
     */
    FSessionMetrics GetCurrentSessionMetrics() const;
    
    /**
     * Get historical session metrics
     */
    TArray<FSessionMetrics> GetSessionHistory(int32 Limit = 10) const;
    
    // ========================================
    // Alerts and Thresholds
    // ========================================
    
    /**
     * Set performance threshold for a tool (triggers alert if exceeded)
     */
    void SetPerformanceThreshold(const FString& ToolName, double MaxDurationMs);
    
    /**
     * Set success rate threshold (triggers alert if below)
     */
    void SetSuccessRateThreshold(const FString& ToolName, float MinSuccessRate);
    
    /**
     * Get pending alerts
     */
    TArray<FPerformanceAlert> GetAlerts(bool bClearAfterRead = true);
    
    // ========================================
    // Diagnostics
    // ========================================
    
    /**
     * Generate a full diagnostics report as JSON
     */
    FString GenerateDiagnosticsReport() const;
    
    /**
     * Generate a human-readable summary
     */
    FString GenerateSummary() const;
    
    /**
     * Export metrics to CSV
     */
    bool ExportToCSV(const FString& FilePath) const;
    
    /**
     * Reset all metrics
     */
    void Reset();
    
    // ========================================
    // LLM Tracking (Optional)
    // ========================================
    
    /**
     * Record an LLM call
     */
    void RecordLLMCall(int32 InputTokens, int32 OutputTokens, double DurationMs);
    
    // Constructor is public for MakeUnique, but use Get() for access
    FPerformanceMetrics();

private:
    void CheckThresholds(const FToolExecutionMetric& Metric);
    void AddAlert(const FString& Type, const FString& Message, const FString& ToolName, double Value, double Threshold);
    
    // Tool statistics
    TMap<FString, FToolStatistics> ToolStats;
    mutable FCriticalSection StatsLock;
    
    // Recent samples for percentile calculation
    TMap<FString, TArray<double>> RecentSamples;
    static constexpr int32 MaxSamplesPerTool = 1000;
    
    // Session tracking
    FSessionMetrics CurrentSession;
    TArray<FSessionMetrics> SessionHistory;
    bool bSessionActive = false;
    
    // Thresholds
    TMap<FString, double> DurationThresholds;
    TMap<FString, float> SuccessRateThresholds;
    
    // Alerts
    TArray<FPerformanceAlert> PendingAlerts;
    mutable FCriticalSection AlertsLock;
    
    // Singleton
    static TUniquePtr<FPerformanceMetrics> Instance;
};

/**
 * RAII helper for automatic tool timing
 */
class RIFTBORNAI_API FScopedToolMetric
{
public:
    FScopedToolMetric(const FString& ToolName, const FString& Category = TEXT(""))
        : Metric(FPerformanceMetrics::Get().StartToolExecution(ToolName, Category))
        , bCompleted(false)
    {
    }
    
    ~FScopedToolMetric()
    {
        if (!bCompleted)
        {
            Complete(false, TEXT("ScopeExitWithoutCompletion"));
        }
    }
    
    void Complete(bool bSuccess, const FString& ErrorCategory = TEXT(""))
    {
        if (!bCompleted)
        {
            FPerformanceMetrics::Get().CompleteToolExecution(Metric, bSuccess, ErrorCategory);
            bCompleted = true;
        }
    }
    
    void SetInputSize(int32 Bytes) { Metric.InputSizeBytes = Bytes; }
    void SetOutputSize(int32 Bytes) { Metric.OutputSizeBytes = Bytes; }
    
private:
    FToolExecutionMetric Metric;
    bool bCompleted;
};

/**
 * Macro for easy scoped metrics
 * Usage: SCOPED_TOOL_METRIC("my_tool");
 *        ... do work ...
 *        COMPLETE_TOOL_METRIC(bSuccess, "error_category");
 */
#define SCOPED_TOOL_METRIC(ToolName) \
    FScopedToolMetric _ScopedMetric_##__LINE__(TEXT(ToolName))

#define COMPLETE_TOOL_METRIC(Success, ErrorCategory) \
    _ScopedMetric_##__LINE__.Complete(Success, TEXT(ErrorCategory))
