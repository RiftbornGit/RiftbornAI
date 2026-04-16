// Copyright 2025 RiftbornAI. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SmartDebugger.generated.h"

/**
 * Issue severity level
 */
UENUM(BlueprintType)
enum class EDebugIssueSeverity : uint8
{
    Blocker     UMETA(DisplayName = "Blocker - Prevents game from running"),
    Critical    UMETA(DisplayName = "Critical - Major functionality broken"),
    Major       UMETA(DisplayName = "Major - Significant impact on gameplay"),
    Minor       UMETA(DisplayName = "Minor - Small issues"),
    Cosmetic    UMETA(DisplayName = "Cosmetic - Visual/polish issues"),
    Info        UMETA(DisplayName = "Info - Informational only")
};

/**
 * Category of detected issue
 */
UENUM(BlueprintType)
enum class EDebugIssueCategory : uint8
{
    Performance     UMETA(DisplayName = "Performance Issues"),
    Memory          UMETA(DisplayName = "Memory Issues"),
    Crash           UMETA(DisplayName = "Crash/Stability"),
    Gameplay        UMETA(DisplayName = "Gameplay Logic"),
    Physics         UMETA(DisplayName = "Physics Issues"),
    Animation       UMETA(DisplayName = "Animation Issues"),
    Audio           UMETA(DisplayName = "Audio Issues"),
    Rendering       UMETA(DisplayName = "Rendering Issues"),
    Network         UMETA(DisplayName = "Network Issues"),
    Input           UMETA(DisplayName = "Input Issues"),
    AI              UMETA(DisplayName = "AI/Behavior Issues"),
    Blueprint       UMETA(DisplayName = "Blueprint Issues"),
    Configuration   UMETA(DisplayName = "Configuration Issues"),
    AssetMissing    UMETA(DisplayName = "Missing Assets"),
    Other           UMETA(DisplayName = "Other Issues")
};

/**
 * Represents a detected issue with context and solutions
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDetectedIssue
{
    GENERATED_BODY()

    // Unique ID for this issue
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString IssueId = TEXT("");

    // Human-readable title
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Title;

    // Detailed description of the issue
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Description;

    // Issue severity
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    EDebugIssueSeverity Severity = EDebugIssueSeverity::Minor;

    // Issue category
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    EDebugIssueCategory Category = EDebugIssueCategory::Other;

    // Where the issue was detected (file, actor, component, etc.)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Location;

    // Relevant code/blueprint path if applicable
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString SourcePath;

    // Line number if applicable
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    int32 LineNumber = -1;

    // Stack trace if available
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> StackTrace;

    // Related log messages
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> RelatedLogs;

    // Suggested fix description
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString SuggestedFix;

    // Code snippet showing the fix
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString FixCodeSnippet;

    // Steps to reproduce
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> ReproSteps;

    // When the issue was detected
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FDateTime DetectedTime = FDateTime();

    // Whether this issue has been acknowledged
    UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
    bool bAcknowledged = false;

    // Whether a fix has been attempted
    UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
    bool bFixAttempted = false;
    
    FDetectedIssue() = default;
};

/**
 * Debug session tracking
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDebugSession
{
    GENERATED_BODY()

    // Session ID - initialized in constructor
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString SessionId = TEXT("");

    // When session started
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FDateTime StartTime = FDateTime();

    // When session ended
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FDateTime EndTime = FDateTime();

    // All issues detected during session
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FDetectedIssue> Issues;

    // Total errors encountered
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    int32 ErrorCount = 0;

    // Total warnings encountered
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    int32 WarningCount = 0;

    // Issues that were fixed
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    int32 FixedCount = 0;

    // Session notes
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> Notes;
    
    FDebugSession() = default;
};

/**
 * Pattern for detecting common issues
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FIssuePattern
{
    GENERATED_BODY()

    // Pattern name
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString PatternName;

    // Regex or keyword pattern to match
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString MatchPattern;

    // Category this pattern detects
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    EDebugIssueCategory Category = EDebugIssueCategory::Other;

    // Default severity for matches
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    EDebugIssueSeverity DefaultSeverity = EDebugIssueSeverity::Minor;

    // Description template (with placeholders)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString DescriptionTemplate;

    // Fix template
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString FixTemplate;
};

/**
 * Watchpoint for monitoring specific values
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDebugWatchpoint
{
    GENERATED_BODY()

    // Watchpoint ID - initialized in constructor
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString WatchId = TEXT("");

    // Name for this watchpoint
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Name = TEXT("");

    // Expression being watched
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Expression = TEXT("");

    // Current value
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString CurrentValue = TEXT("");

    // Previous value
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString PreviousValue = TEXT("");

    // Expected value range (min)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float ExpectedMin = 0.0f;

    // Expected value range (max)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float ExpectedMax = FLT_MAX;

    // Whether to break on change
    UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
    bool bBreakOnChange = false;

    // Whether to break on out-of-range
    UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
    bool bBreakOnOutOfRange = false;

    // Whether currently enabled
    UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
    bool bEnabled = true;

    // History of values
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> ValueHistory;
    
    FDebugWatchpoint() = default;
};

/**
 * Smart Debugger Engine - AI-assisted debugging for game development
 */
class RIFTBORNAI_API FSmartDebugger
{
public:
    static FSmartDebugger& Get(UWorld* World = nullptr);

    // Session management
    void StartDebugSession();
    void EndDebugSession();
    FDebugSession GetCurrentSession() const { return CurrentSession; }
    TArray<FDebugSession> GetSessionHistory() const { return SessionHistory; }

    // Issue detection
    TArray<FDetectedIssue> ScanForIssues(UWorld* World);
    TArray<FDetectedIssue> AnalyzeLogFile(const FString& InLogFilePath);
    TArray<FDetectedIssue> CheckBlueprintHealth(UBlueprint* Blueprint);
    TArray<FDetectedIssue> CheckActorHealth(AActor* Actor);
    
    // Real-time monitoring
    void EnableRealTimeMonitoring(bool bEnable);
    bool IsMonitoringEnabled() const { return bRealTimeMonitoring; }
    void SetMonitoringInterval(float Seconds) { MonitoringInterval = Seconds; }

    // Issue management
    void ReportIssue(const FDetectedIssue& Issue);
    TArray<FDetectedIssue> GetActiveIssues() const;
    TArray<FDetectedIssue> GetIssuesByCategory(EDebugIssueCategory Category) const;
    TArray<FDetectedIssue> GetIssuesBySeverity(EDebugIssueSeverity Severity) const;
    void AcknowledgeIssue(const FString& IssueId);
    void MarkIssueFixed(const FString& IssueId);
    void DismissIssue(const FString& IssueId);

    // Pattern-based detection
    void RegisterIssuePattern(const FIssuePattern& Pattern);
    void RemoveIssuePattern(const FString& PatternName);
    TArray<FIssuePattern> GetRegisteredPatterns() const { return RegisteredPatterns; }

    // Watchpoints
    FString AddWatchpoint(const FString& Name, const FString& Expression);
    void RemoveWatchpoint(const FString& WatchId);
    void UpdateWatchpoint(const FString& WatchId, const FString& NewValue);
    TArray<FDebugWatchpoint> GetWatchpoints() const { return Watchpoints; }

    // Fix suggestions
    FString GenerateFixSuggestion(const FDetectedIssue& Issue);
    FString GenerateCodeFix(const FDetectedIssue& Issue);
    bool AttemptAutoFix(const FString& IssueId);

    // Query interface for prompts
    FString QueryDebugger(const FString& Query);
    FString ExplainIssue(const FString& IssueId);
    FString GetDebugContext(AActor* Actor);

    // Performance profiling
    void StartProfiling(const FString& ProfileName);
    void EndProfiling(const FString& ProfileName);
    FString GetProfilingResults(const FString& ProfileName);

    // Export/reporting
    FString ExportIssuesAsJSON() const;
    FString GenerateDebugReport() const;
    void SaveReportToFile(const FString& FilePath);

private:
    FSmartDebugger() = default;

    void InitializePatterns();
    FDetectedIssue CreateIssueFromLog(const FString& LogLine);
    void ProcessLogLine(const FString& Line);
    void TickMonitoring();

    TWeakObjectPtr<UWorld> CachedWorld;
    FDebugSession CurrentSession;
    TArray<FDebugSession> SessionHistory;
    TArray<FDetectedIssue> ActiveIssues;
    TArray<FIssuePattern> RegisteredPatterns;
    TArray<FDebugWatchpoint> Watchpoints;

    bool bRealTimeMonitoring = false;
    float MonitoringInterval = 1.0f;
    FTimerHandle MonitoringTimerHandle;

    // Pattern matching cache
    TMap<FString, FIssuePattern> PatternCache;
};
