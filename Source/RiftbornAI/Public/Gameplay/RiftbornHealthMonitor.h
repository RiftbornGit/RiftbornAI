// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "HAL/CriticalSection.h"
#include "RiftbornHealthMonitor.generated.h"

/**
 * Health status levels
 */
UENUM(BlueprintType)
enum class ERiftbornHealthStatus : uint8
{
	OK,
	Degraded,
	Down,
	Unknown
};

/**
 * Component health info
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornComponentHealth
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Name;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	ERiftbornHealthStatus Status = ERiftbornHealthStatus::Unknown;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Message;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FDateTime LastCheck = FDateTime();
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	float LatencyMs = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	int32 ErrorCount = 0;
};

/**
 * Overall system health
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornSystemHealth
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	ERiftbornHealthStatus Status = ERiftbornHealthStatus::Unknown;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString PythonVersion;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	float UptimeSeconds = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	int32 QueueDepth = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString LastError;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FDateTime LastErrorTime = FDateTime();
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TMap<FString, FRiftbornComponentHealth> Components;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	float ErrorRatePerMin = 0.0f;
	
	/** Is system healthy? */
	bool IsHealthy() const { return Status == ERiftbornHealthStatus::OK; }
	
	/** Get status as display string with emoji */
	FString GetStatusString() const;
	
	/** Get color for status (for UI) */
	FLinearColor GetStatusColor() const;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthStatusChanged, const FRiftbornSystemHealth&);

/**
 * Health monitor that periodically polls bridge health status.
 * 
 * Features:
 * - Polls every 30 seconds (configurable)
 * - Caches last known health
 * - Fires delegate on status changes
 * - Provides quick synchronous check
 */
class RIFTBORNAI_API FRiftbornHealthMonitor : public FTickableGameObject
{
public:
	/** Destructor */
	~FRiftbornHealthMonitor() = default;
	
	/** Get singleton instance */
	static FRiftbornHealthMonitor& Get();
	
	/** Shutdown */
	static void Shutdown();
	
	/** Set poll interval in seconds (default: 30) */
	void SetPollInterval(float Seconds) { PollIntervalSeconds = Seconds; }
	
	/** Get current cached health (no network call) */
	FRiftbornSystemHealth GetCachedHealth() const;
	
	/** Force immediate health check (async) */
	void CheckHealthAsync();
	
	/** Force immediate health check (blocking) */
	FRiftbornSystemHealth CheckHealthSync();
	
	/** Get whether bridge is healthy */
	bool IsHealthy() const;
	
	/** Get delegate fired when health status changes */
	FOnHealthStatusChanged& OnHealthStatusChanged() { return HealthStatusChangedDelegate; }
	
	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FRiftbornHealthMonitor, STATGROUP_Tickables); }
	virtual bool IsTickableWhenPaused() const override { return true; }
	
private:
	FRiftbornHealthMonitor();
	
	FRiftbornHealthMonitor(const FRiftbornHealthMonitor&) = delete;
	FRiftbornHealthMonitor& operator=(const FRiftbornHealthMonitor&) = delete;
	
	void ParseHealthResponse(const FString& JsonResponse);
	ERiftbornHealthStatus StringToStatus(const FString& StatusStr) const;
	
	/** Cached health status */
	FRiftbornSystemHealth CachedHealth;
	
	/** Previous status for change detection */
	ERiftbornHealthStatus PreviousStatus = ERiftbornHealthStatus::Unknown;
	
	/** Time since last poll */
	float TimeSinceLastPoll = 0.0f;
	
	/** Poll interval in seconds */
	float PollIntervalSeconds = 30.0f;
	
	/** Whether a poll is in progress */
	bool bPollInProgress = false;
	
	/** Thread safety */
	mutable FCriticalSection CriticalSection;
	
	/** Delegate fired when status changes */
	FOnHealthStatusChanged HealthStatusChangedDelegate;
	
	/** Singleton instance */
	static TUniquePtr<FRiftbornHealthMonitor> Instance;
};
