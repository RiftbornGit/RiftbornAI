// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "Misc/DateTime.h"

/**
 * Histogram bucket for latency tracking
 */
struct RIFTBORNAI_API FRiftbornHistogram
{
	/** Bucket boundaries in milliseconds */
	TArray<double> Buckets;
	
	/** Count in each bucket */
	TArray<int64> Counts;
	
	/** Total sum of all values */
	double Sum;
	
	/** Total count */
	int64 Count;
	
	/** Min/Max values */
	double Min;
	double Max;
	
	FRiftbornHistogram();
	
	/** Initialize with default latency buckets (5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000 ms) */
	void InitializeLatencyBuckets();
	
	/** Record a value */
	void Record(double Value);
	
	/** Get percentile value (0-100) */
	double GetPercentile(double Percentile) const;
	
	/** Reset all values */
	void Reset();
};

/**
 * Counter metric (monotonically increasing)
 */
struct RIFTBORNAI_API FRiftbornCounter
{
	int64 Value;
	
	FRiftbornCounter() : Value(0) {}
	
	void Inc() { ++Value; }
	void Add(int64 Delta) { Value += Delta; }
	int64 Get() const { return Value; }
	void Reset() { Value = 0; }
};

/**
 * Gauge metric (can go up or down)
 */
struct RIFTBORNAI_API FRiftbornGauge
{
	double Value;
	
	FRiftbornGauge() : Value(0.0) {}
	
	void Set(double NewValue) { Value = NewValue; }
	void Inc() { ++Value; }
	void Dec() { --Value; }
	void Add(double Delta) { Value += Delta; }
	double Get() const { return Value; }
	void Reset() { Value = 0.0; }
};

/**
 * Time window for rate calculations
 */
struct RIFTBORNAI_API FRiftbornRateWindow
{
	TArray<TPair<FDateTime, int64>> Samples;
	FTimespan WindowDuration;
	
	FRiftbornRateWindow(FTimespan InDuration = FTimespan::FromMinutes(1));
	
	void AddSample(int64 Value);
	double GetRate() const; // per second
	void Cleanup();
};

/**
 * Centralized metrics collection for RiftbornAI
 * 
 * Tracked metrics:
 * - AI requests (total, by provider)
 * - AI request duration (histogram)
 * - AI tokens used (by provider)
 * - Bridge commands (total, by command)
 * - Bridge latency (histogram)
 * - Actions (total, by result)
 * - Snapshot size
 * - Active connections
 */
class RIFTBORNAI_API FRiftbornMetrics
{
public:
	/** Get singleton instance */
	static FRiftbornMetrics& Get();
	
	/** Shutdown */
	static void Shutdown();
	
	// ==========================================================================
	// AI Metrics
	// ==========================================================================
	
	/** Record AI request */
	void RecordAIRequest(const FString& Provider, double DurationMs, bool bSuccess);
	
	/** Record AI tokens used */
	void RecordAITokens(const FString& Provider, int32 InputTokens, int32 OutputTokens);
	
	/** Get AI request count by provider */
	int64 GetAIRequestCount(const FString& Provider = TEXT("")) const;
	
	/** Get AI request rate (per minute) */
	double GetAIRequestRate(const FString& Provider = TEXT("")) const;
	
	/** Get AI latency percentile */
	double GetAILatencyPercentile(double Percentile) const;
	
	/** Get AI error rate (0-1) */
	double GetAIErrorRate(const FString& Provider = TEXT("")) const;
	
	/** Get total tokens used by provider */
	int64 GetAITokensUsed(const FString& Provider) const;
	
	// ==========================================================================
	// Bridge Metrics
	// ==========================================================================
	
	/** Record bridge command */
	void RecordBridgeCommand(const FString& Command, double DurationMs, bool bSuccess);
	
	/** Get bridge command count by command type */
	int64 GetBridgeCommandCount(const FString& Command = TEXT("")) const;
	
	/** Get bridge latency percentile */
	double GetBridgeLatencyPercentile(double Percentile) const;
	
	/** Get bridge error rate */
	double GetBridgeErrorRate() const;
	
	// ==========================================================================
	// Action Metrics
	// ==========================================================================
	
	/** Record action execution */
	void RecordAction(const FString& ActionType, double DurationMs, bool bSuccess, bool bRolledBack = false);
	
	/** Get action count by type */
	int64 GetActionCount(const FString& ActionType = TEXT("")) const;
	
	/** Get action success rate */
	double GetActionSuccessRate() const;
	
	/** Get action rollback rate */
	double GetActionRollbackRate() const;
	
	// ==========================================================================
	// Snapshot Metrics
	// ==========================================================================
	
	/** Record snapshot operation */
	void RecordSnapshot(int64 SizeBytes, double DurationMs);
	
	/** Get current snapshot size */
	int64 GetSnapshotSizeBytes() const;
	
	/** Get total snapshots taken */
	int64 GetSnapshotCount() const;
	
	// ==========================================================================
	// Connection Metrics
	// ==========================================================================
	
	/** Set active connection count */
	void SetActiveConnections(int32 Count);
	
	/** Get active connection count */
	int32 GetActiveConnections() const;
	
	/** Set pool size */
	void SetConnectionPoolSize(int32 Size);
	
	/** Get pool size */
	int32 GetConnectionPoolSize() const;
	
	// ==========================================================================
	// Export & Reset
	// ==========================================================================
	
	/** Export all metrics as JSON */
	FString ExportJson() const;
	
	/** Reset all metrics */
	void Reset();
	
	/** Get uptime in seconds */
	double GetUptimeSeconds() const;

	// Destructor must be public for TUniquePtr
	~FRiftbornMetrics() = default;
	
private:
	FRiftbornMetrics();
	
	FRiftbornMetrics(const FRiftbornMetrics&) = delete;
	FRiftbornMetrics& operator=(const FRiftbornMetrics&) = delete;
	
	// AI metrics
	TMap<FString, FRiftbornCounter> AIRequestCounts;
	TMap<FString, FRiftbornCounter> AIErrorCounts;
	TMap<FString, FRiftbornRateWindow> AIRequestRates;
	TMap<FString, FRiftbornCounter> AIInputTokens;
	TMap<FString, FRiftbornCounter> AIOutputTokens;
	FRiftbornHistogram AILatency;
	
	// Bridge metrics
	TMap<FString, FRiftbornCounter> BridgeCommandCounts;
	FRiftbornCounter BridgeErrorCount;
	FRiftbornHistogram BridgeLatency;
	
	// Action metrics
	TMap<FString, FRiftbornCounter> ActionCounts;
	FRiftbornCounter ActionSuccessCount;
	FRiftbornCounter ActionFailureCount;
	FRiftbornCounter ActionRollbackCount;
	
	// Snapshot metrics
	FRiftbornCounter SnapshotCount;
	FRiftbornGauge SnapshotSizeBytes;
	
	// Connection metrics
	FRiftbornGauge ActiveConnections;
	FRiftbornGauge ConnectionPoolSize;
	
	// Startup time for uptime calculation
	FDateTime StartupTime;
	
	// Thread safety
	mutable FCriticalSection CriticalSection;
	
	// Singleton
	static TUniquePtr<FRiftbornMetrics> Instance;
};

// Convenience macros
#define RIFTBORN_METRICS_AI_REQUEST(Provider, DurationMs, Success) \
	FRiftbornMetrics::Get().RecordAIRequest(TEXT(Provider), DurationMs, Success)

#define RIFTBORN_METRICS_BRIDGE_CMD(Command, DurationMs, Success) \
	FRiftbornMetrics::Get().RecordBridgeCommand(TEXT(Command), DurationMs, Success)

#define RIFTBORN_METRICS_ACTION(ActionType, DurationMs, Success, RolledBack) \
	FRiftbornMetrics::Get().RecordAction(TEXT(ActionType), DurationMs, Success, RolledBack)
