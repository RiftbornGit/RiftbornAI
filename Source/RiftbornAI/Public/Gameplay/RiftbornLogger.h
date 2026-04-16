// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "Misc/DateTime.h"
#include "Gameplay/RiftbornSettings.h"  // canonical ERiftbornLogLevel enum

/**
 * Structured log entry with all required fields
 */
struct RIFTBORNAI_API FRiftbornLogEntry
{
	/** Timestamp in ISO 8601 format */
	FDateTime Timestamp;
	
	/** Log level */
	ERiftbornLogLevel Level;
	
	/** Component name (Bridge, Agent, ActionRunner, etc.) */
	FString Component;
	
	/** Correlation ID for tracking related operations */
	FString ActionId;
	
	/** Main log message */
	FString Message;
	
	/** Operation duration in milliseconds (optional) */
	double DurationMs;
	
	/** Whether operation succeeded */
	bool bSuccess;
	
	/** Additional context as key-value pairs */
	TMap<FString, FString> Context;
	
	FRiftbornLogEntry()
		: Timestamp(FDateTime::UtcNow())
		, Level(ERiftbornLogLevel::Info)
		, DurationMs(-1.0)
		, bSuccess(true)
	{}
	
	/** Convert to JSON string for file output */
	FString ToJson() const;
	
	/** Get log level as string */
	static FString LevelToString(ERiftbornLogLevel InLevel);
	
	/** Get log level from string */
	static ERiftbornLogLevel StringToLevel(const FString& InString);
};

/**
 * RAII scope for timing operations
 */
class RIFTBORNAI_API FRiftbornLogScope
{
public:
	FRiftbornLogScope(const FString& InComponent, const FString& InMessage, const FString& InActionId = TEXT(""));
	~FRiftbornLogScope();
	
	/** Mark the operation as failed */
	void SetFailed(const FString& ErrorMessage = TEXT(""));
	
	/** Add context to the log entry */
	void AddContext(const FString& Key, const FString& Value);
	
	/** Set custom action ID (useful if generated after scope creation) */
	void SetActionId(const FString& InActionId);
	
private:
	FString Component;
	FString Message;
	FString ActionId;
	FString ErrorMessage;
	TMap<FString, FString> Context;
	double StartTime;
	bool bFailed;
};

/**
 * Centralized structured logging system for RiftbornAI
 * 
 * Features:
 * - JSON structured output
 * - Daily log rotation
 * - Configurable retention (default 7 days)
 * - Thread-safe
 * - UE_LOG integration
 */
class RIFTBORNAI_API FRiftbornLogger
{
public:
	/** Get singleton instance */
	static FRiftbornLogger& Get();
	
	/** Shutdown and flush all logs */
	static void Shutdown();
	
	/** Initialize logger with custom settings */
	void Initialize(const FString& LogDirectory = TEXT(""), int32 RetentionDays = 7);
	
	/** Log a structured entry */
	void Log(const FRiftbornLogEntry& Entry);
	
	/** Convenience methods */
	void Debug(const FString& Component, const FString& Message, const FString& ActionId = TEXT(""));
	void Info(const FString& Component, const FString& Message, const FString& ActionId = TEXT(""));
	void Warn(const FString& Component, const FString& Message, const FString& ActionId = TEXT(""));
	void Error(const FString& Component, const FString& Message, const FString& ActionId = TEXT(""));
	void Fatal(const FString& Component, const FString& Message, const FString& ActionId = TEXT(""));
	
	/** Log with timing */
	void LogWithDuration(const FString& Component, const FString& Message, double DurationMs, bool bSuccess, const FString& ActionId = TEXT(""));
	
	/** Set minimum log level (default: Info for file, Debug for in-memory) */
	void SetMinLevel(ERiftbornLogLevel Level) { MinLevel = Level; }
	ERiftbornLogLevel GetMinLevel() const { return MinLevel; }
	
	/** Enable/disable UE_LOG mirroring */
	void SetMirrorToUELog(bool bEnable) { bMirrorToUELog = bEnable; }
	
	/** Get recent log entries (for dashboard) */
	TArray<FRiftbornLogEntry> GetRecentEntries(int32 Count = 100, ERiftbornLogLevel MinLevelFilter = ERiftbornLogLevel::Debug) const;
	
	/** Get log file path for today */
	FString GetCurrentLogPath() const;
	
	/** Flush pending writes to disk */
	void Flush();
	
	/** Clean up old log files based on retention policy */
	void CleanupOldLogs();
	
	/** Generate unique action ID */
	static FString GenerateActionId();

	// Destructor must be public for TUniquePtr
	~FRiftbornLogger();
	
private:
	FRiftbornLogger();
	
	FRiftbornLogger(const FRiftbornLogger&) = delete;
	FRiftbornLogger& operator=(const FRiftbornLogger&) = delete;
	
	void WriteToFile(const FString& JsonLine);
	void RotateLogIfNeeded();
	void MirrorToUELog(const FRiftbornLogEntry& Entry);
	
	/** Log directory path */
	FString LogDirectory;
	
	/** Current log file handle */
	TUniquePtr<FArchive> LogFile;
	
	/** Current log file date (for rotation) */
	FDateTime CurrentLogDate;
	
	/** Minimum level for file output */
	ERiftbornLogLevel MinLevel;
	
	/** Number of days to keep logs */
	int32 RetentionDays;
	
	/** Whether to mirror to UE_LOG */
	bool bMirrorToUELog;
	
	/** In-memory ring buffer for recent entries */
	TArray<FRiftbornLogEntry> RecentEntries;
	static constexpr int32 MaxRecentEntries = 1000;
	
	/** Thread safety */
	mutable FCriticalSection CriticalSection;
	
	/** Whether initialized */
	bool bInitialized;
	
	/** Singleton instance */
	static TUniquePtr<FRiftbornLogger> Instance;
};

// Convenience macros for structured logging (different from RiftbornLog.h UE_LOG macros)
#define RIFTBORN_LOGGER_DEBUG(Component, Message) FRiftbornLogger::Get().Debug(TEXT(Component), Message)
#define RIFTBORN_LOGGER_INFO(Component, Message) FRiftbornLogger::Get().Info(TEXT(Component), Message)
#define RIFTBORN_LOGGER_WARN(Component, Message) FRiftbornLogger::Get().Warn(TEXT(Component), Message)
#define RIFTBORN_LOGGER_ERROR(Component, Message) FRiftbornLogger::Get().Error(TEXT(Component), Message)
#define RIFTBORN_LOGGER_FATAL(Component, Message) FRiftbornLogger::Get().Fatal(TEXT(Component), Message)

// Scoped timing macro
#define RIFTBORN_LOG_SCOPE(Component, Message) FRiftbornLogScope _RiftbornLogScope_(TEXT(Component), Message)
#define RIFTBORN_LOG_SCOPE_ID(Component, Message, ActionId) FRiftbornLogScope _RiftbornLogScope_(TEXT(Component), Message, ActionId)
