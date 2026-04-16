// Copyright RiftbornAI. All Rights Reserved.
// RiftbornSandboxGuardian.h - Prevents superintelligent agent from damaging itself
//
// THE PROBLEM:
// An AI agent with broad project access can:
// 1. Delete its own plugin files
// 2. Corrupt its configuration
// 3. Create infinite loops of self-modification
// 4. Delete critical assets the user needs
//
// THE SOLUTION:
// A guardian layer that enforces:
// - Protected paths (plugin source, configs)
// - Action blast radius limits
// - Session budgets
// - Immutable core files

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

/**
 * Result of a guardian policy check
 */
struct FGuardianCheckResult
{
    bool bAllowed = true;
    FString Reason;
    FString ViolatedPolicy;
    
    static FGuardianCheckResult Allow()
    {
        return FGuardianCheckResult{true, TEXT(""), TEXT("")};
    }
    
    static FGuardianCheckResult Deny(const FString& Policy, const FString& InReason)
    {
        return FGuardianCheckResult{false, InReason, Policy};
    }
};

/**
 * Session budget tracking
 */
struct FSessionBudget
{
    // Limits
    int32 MaxFilesPerSession = 50;
    int32 MaxAssetsPerSession = 100;
    int32 MaxDeletesPerSession = 10;
    int32 MaxActionsPerSession = 500;
    
    // Current usage
    int32 FilesTouched = 0;
    int32 AssetsTouched = 0;
    int32 DeletesPerformed = 0;
    int32 ActionsPerformed = 0;
    
    bool CanTouchFile() const { return FilesTouched < MaxFilesPerSession; }
    bool CanTouchAsset() const { return AssetsTouched < MaxAssetsPerSession; }
    bool CanDelete() const { return DeletesPerformed < MaxDeletesPerSession; }
    bool CanPerformAction() const { return ActionsPerformed < MaxActionsPerSession; }
    
    void RecordFileTouched() { FilesTouched++; ActionsPerformed++; }
    void RecordAssetTouched() { AssetsTouched++; ActionsPerformed++; }
    void RecordDelete() { DeletesPerformed++; ActionsPerformed++; }
    void RecordAction() { ActionsPerformed++; }
    
    FString GetUsageString() const
    {
        return FString::Printf(TEXT("Files: %d/%d, Assets: %d/%d, Deletes: %d/%d, Actions: %d/%d"),
            FilesTouched, MaxFilesPerSession,
            AssetsTouched, MaxAssetsPerSession,
            DeletesPerformed, MaxDeletesPerSession,
            ActionsPerformed, MaxActionsPerSession);
    }
};

/**
 * Action classification for policy enforcement
 */
enum class EActionDanger : uint8
{
    Safe,           // Read-only, non-destructive
    Moderate,       // Writes but reversible
    Dangerous,      // Potentially destructive
    Catastrophic    // Could break the project/plugin
};

/**
 * FRiftbornSandboxGuardian
 * 
 * The immune system of RiftbornAI - prevents self-destruction.
 * 
 * Core principles:
 * 1. Plugin source is SACRED - never touch it
 * 2. Config files are PROTECTED - modifications need elevation
 * 3. User project files are ALLOWED but limited
 * 4. Session budgets prevent runaway damage
 */
class RIFTBORNAI_API FRiftbornSandboxGuardian
{
public:
    static FRiftbornSandboxGuardian& Get();
    
    // =========================================================================
    // PATH VALIDATION
    // =========================================================================
    
    /** Check if a path can be read */
    FGuardianCheckResult CanRead(const FString& Path);
    
    /** Check if a path can be written/modified */
    FGuardianCheckResult CanWrite(const FString& Path);
    
    /** Check if a path can be deleted */
    FGuardianCheckResult CanDelete(const FString& Path);
    
    /** Check if an asset path can be modified */
    FGuardianCheckResult CanModifyAsset(const FString& AssetPath);
    
    // =========================================================================
    // ACTION VALIDATION
    // =========================================================================
    
    /** Check if an action is allowed */
    FGuardianCheckResult CanPerformAction(const FString& ActionName, const TMap<FString, FString>& Args);
    
    /** Get danger level of an action */
    EActionDanger GetActionDangerLevel(const FString& ActionName);
    
    /** Check if we're within session budget */
    bool IsWithinBudget() const;
    
    // =========================================================================
    // SESSION MANAGEMENT
    // =========================================================================
    
    /** Start a new session (resets budgets) */
    void StartSession();
    
    /** End current session */
    void EndSession();
    
    /** Get current session budget */
    const FSessionBudget& GetSessionBudget() const { return CurrentBudget; }
    
    /** Record that a file was touched */
    void RecordFileTouched(const FString& Path);
    
    /** Record that an asset was touched */
    void RecordAssetTouched(const FString& AssetPath);
    
    /** Record that a delete occurred */
    void RecordDelete(const FString& Path);
    
    // =========================================================================
    // CONFIGURATION
    // =========================================================================
    
    /** Add a protected path pattern (regex) */
    void AddProtectedPath(const FString& Pattern);
    
    /** Add a forbidden action */
    void AddForbiddenAction(const FString& ActionName);
    
    /** Set budget limits */
    void SetBudgetLimits(int32 MaxFiles, int32 MaxAssets, int32 MaxDeletes, int32 MaxActions);
    
    /** Enable/disable guardian (for testing only!) */
    void SetEnabled(bool bEnabled);
    
    /** Is guardian enabled? */
    bool IsEnabled() const { return bGuardianEnabled; }
    
private:
    FRiftbornSandboxGuardian();
    void InitializeDefaultProtections();
    
    bool IsPathProtected(const FString& Path) const;
    bool IsActionForbidden(const FString& ActionName) const;
    FString NormalizePath(const FString& Path) const;
    
    // Protected path patterns (these cannot be written/deleted)
    TArray<FString> ProtectedPathPatterns;
    
    // Forbidden actions (never allowed)
    TSet<FString> ForbiddenActions;
    
    // Action danger classifications
    TMap<FString, EActionDanger> ActionDangerLevels;
    
    // Current session budget
    FSessionBudget CurrentBudget;
    
    // Files touched this session (for reporting)
    TSet<FString> SessionFiles;
    TSet<FString> SessionAssets;
    
    // State
    bool bGuardianEnabled = true;
    bool bInSession = false;
    
    // Critical section for thread safety
    mutable FCriticalSection GuardianLock;
};

// ============================================================================
// CONVENIENCE MACROS
// ============================================================================

/** Check path access and return error if denied */
#define GUARDIAN_CHECK_READ(Path) \
    { \
        FGuardianCheckResult __result = FRiftbornSandboxGuardian::Get().CanRead(Path); \
        if (!__result.bAllowed) \
        { \
            return FClaudeToolResult::Failure(FString::Printf(TEXT("Access denied: %s"), *__result.Reason)); \
        } \
    }

#define GUARDIAN_CHECK_WRITE(Path) \
    { \
        FGuardianCheckResult __result = FRiftbornSandboxGuardian::Get().CanWrite(Path); \
        if (!__result.bAllowed) \
        { \
            return FClaudeToolResult::Failure(FString::Printf(TEXT("Write denied: %s"), *__result.Reason)); \
        } \
    }

#define GUARDIAN_CHECK_DELETE(Path) \
    { \
        FGuardianCheckResult __result = FRiftbornSandboxGuardian::Get().CanDelete(Path); \
        if (!__result.bAllowed) \
        { \
            return FClaudeToolResult::Failure(FString::Printf(TEXT("Delete denied: %s"), *__result.Reason)); \
        } \
    }

#define GUARDIAN_CHECK_ACTION(ActionName, Args) \
    { \
        FGuardianCheckResult __result = FRiftbornSandboxGuardian::Get().CanPerformAction(ActionName, Args); \
        if (!__result.bAllowed) \
        { \
            return FClaudeToolResult::Failure(FString::Printf(TEXT("Action denied: %s"), *__result.Reason)); \
        } \
    }

#define GUARDIAN_CHECK_BUDGET() \
    { \
        if (!FRiftbornSandboxGuardian::Get().IsWithinBudget()) \
        { \
            return FClaudeToolResult::Failure(TEXT("Session budget exceeded. Start a new session to continue.")); \
        } \
    }
