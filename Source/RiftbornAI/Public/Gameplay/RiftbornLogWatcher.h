// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Logging/LogMacros.h"
#include "Logging/MessageLog.h"
#include "RiftbornLogWatcher.generated.h"

/**
 * Log entry captured from UE output log
 * Named FRiftbornWatcherLogEntry to avoid conflict with FRiftbornLogEntry in RiftbornLogger.h
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornWatcherLogEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Logs")
    FString Category;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Logs")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Logs")
    FString Verbosity;  // "Error", "Warning", "Log", "Verbose"

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Logs")
    double Timestamp = 0.0;  // Seconds since session start

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Logs")
    int32 FrameNumber = 0;

    FString ToJson() const;
};

/**
 * Build event for tracking Live Coding / compilation
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornBuildEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Build")
    FString EventType;  // "started", "succeeded", "failed", "warning"

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Build")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Build")
    double Timestamp = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Build")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Build")
    int32 WarningCount = 0;

    FString ToJson() const;
};

/**
 * Alert entry captured from output log warnings/errors or watched Message Log deltas.
 * Sequence ids are monotonic and can be used as a cursor for incremental polling.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornLogAlert
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Alerts")
    int64 SequenceId = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Alerts")
    FString Source; // "output_log" or "message_log"

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Alerts")
    FString LogName;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Alerts")
    FString Category;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Alerts")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Alerts")
    FString Severity; // "Error", "Warning", "Info"

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Alerts")
    double Timestamp = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Riftborn|Alerts")
    int32 FrameNumber = 0;

    FString ToJson() const;
};

/**
 * Log Watcher Subsystem
 * 
 * Captures UE output logs in real-time and provides:
 * - Circular buffer of recent logs (configurable size)
 * - Filtering by category, verbosity
 * - Build/compile event tracking
 * - HTTP endpoint for external tools
 * - Alert callbacks for specific patterns
 * 
 * This runs as an EngineSubsystem so it's available during PIE and editor.
 */
UCLASS()
class RIFTBORNAI_API URiftbornLogWatcher : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Static access
    static URiftbornLogWatcher* Get();

    // ========== Log Capture ==========
    
    /** Get recent logs as JSON array */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Logs")
    static FString GetRecentLogsAsJson(int32 MaxCount = 100, bool bClear = false);

    /** Get logs filtered by category */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Logs")
    static FString GetLogsByCategoryAsJson(const FString& Category, int32 MaxCount = 50);

    /** Get only error/warning logs */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Logs")
    static FString GetErrorsAsJson(int32 MaxCount = 50);

    /** Get messages from a specific Message Log category (e.g., "PIE", "BlueprintLog", "LoadErrors") */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Logs")
    static FString GetMessageLogAsJson(const FString& LogName, int32 MaxCount = 100);

    /** Get counts from a named Message Log (e.g., "PIE", "BlueprintLog", "LoadErrors", "AssetCheck", "MapCheck") */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Logs")
    static FString GetMessageLogCountsAsJson(const FString& LogName);

    /**
     * Get alert deltas since a cursor across output-log warnings/errors and watched Message Log categories.
     *
     * @param SinceCursor - Return alerts with sequence id greater than this cursor. 0 returns buffered history.
     * @param MaxCount - Maximum number of alerts to return.
     * @param MinSeverity - "Info", "Warning", or "Error".
     * @param bIncludeOutputLog - Include output-log warnings/errors.
     * @param MessageLogFilter - Optional comma-separated Message Log names to include.
     */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Alerts")
    static FString GetAlertsSinceCursorAsJson(int64 SinceCursor = 0, int32 MaxCount = 100, const FString& MinSeverity = TEXT("Warning"), bool bIncludeOutputLog = true, const FString& MessageLogFilter = TEXT(""));

    /** Get the latest alert cursor emitted by the watcher. */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Alerts")
    static int64 GetLatestAlertCursor();

    /**
     * Inject a test message into a named Message Log category.
     * 
     * SECURITY: This function is protected by multiple gates:
     * 1. RUNTIME: Requires RIFTBORN_TEST_MODE=1 environment variable
     * 2. IMPLEMENTATION: In shipping builds, immediately returns false
     * 
     * Used for adversarial testing of the verification pipeline.
     * 
     * @param LogName - The Message Log category (e.g., "AssetCheck", "LoadErrors", "MapCheck", "BlueprintLog")
     * @param Message - The message text to inject
     * @param Severity - "Error", "Warning", or "Info" (default: "Error")
     * @return true if injection succeeded, false if test mode disabled or invalid params
     */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Testing")
    static bool InjectTestMessage(const FString& LogName, const FString& Message, const FString& Severity = TEXT("Error"));

    /**
     * Clear a test message log category used by InjectTestMessage.
     *
     * SECURITY: Same gates as InjectTestMessage. Intended only for runtime-cert cleanup.
     */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Testing")
    static bool ClearTestMessageLog(const FString& LogName);

    /** Clear all captured logs */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Logs")
    static void ClearLogs();

    // ========== Build Events ==========

    /** Get recent build events as JSON */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Build")
    static FString GetBuildEventsAsJson(int32 MaxCount = 20, bool bClear = false);

    /** Check if a build is currently in progress */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Build")
    static bool IsBuildInProgress();

    /** Get last build result: "success", "failed", "unknown" */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Build")
    static FString GetLastBuildResult();

    // ========== Alerts ==========

    /** Check if any errors occurred since last check */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Alerts")
    static bool HasNewErrors();

    /** Get error count since session start */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Alerts")
    static int32 GetErrorCount();

    /** Get warning count since session start */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Alerts")
    static int32 GetWarningCount();

    // ========== Status ==========

    /** Get overall status as JSON */
    UFUNCTION(BlueprintCallable, Category = "Riftborn|Status")
    static FString GetStatusAsJson();

private:
    struct FMessageLogWatchState
    {
        TSharedPtr<class IMessageLogListing> Listing;
        FDelegateHandle ChangedHandle;
        int32 SeenCount = 0;
        FString LastMessageSignature;
    };

    // Log output device for capturing logs
    class FLogOutputDevice : public FOutputDevice
    {
    public:
        URiftbornLogWatcher* Owner = nullptr;
        virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
    };

    TUniquePtr<FLogOutputDevice> LogDevice;

    // Circular buffer for logs
    static constexpr int32 MaxLogEntries = 1000;
    static TArray<FRiftbornWatcherLogEntry> LogBuffer;
    static int32 LogBufferHead;
    static FCriticalSection LogBufferLock;

    // Build events
    static TArray<FRiftbornBuildEvent> BuildEvents;
    static bool bBuildInProgress;
    static FString LastBuildResult;
    static FCriticalSection BuildLock;

    // Incremental alert feed
    static constexpr int32 MaxAlertEntries = 2048;
    static TArray<FRiftbornLogAlert> AlertBuffer;
    static int32 AlertBufferHead;
    static int64 NextAlertSequence;
    static FCriticalSection AlertBufferLock;

    // Counters
    static int32 TotalErrorCount;
    static int32 TotalWarningCount;
    static int32 ErrorCountAtLastCheck;
    static double SessionStartTime;

    // Internal methods
    void OnLogMessage(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category);
    void DetectBuildEvents(const FString& Message, const FName& Category, ELogVerbosity::Type Verbosity);
    void AppendAlert(const FString& Source, const FString& LogName, const FString& Category, const FString& Message, const FString& Severity, double Timestamp, int32 FrameNumber);

#if WITH_EDITOR
    void BindWatchedMessageLog(const FName& LogName);
    void ResetWatchedMessageLogBaselines();
    void OnMessageLogChanged(FName LogName);
    static FString BuildMessageSignature(const TSharedRef<class FTokenizedMessage>& Message);

    TMap<FName, FMessageLogWatchState> MessageLogWatchStates;
#endif

    // Delegate handles for build events
    FDelegateHandle CompileStartHandle;
    FDelegateHandle CompileEndHandle;
};
