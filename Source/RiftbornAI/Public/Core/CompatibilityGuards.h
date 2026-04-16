// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Runtime/Launch/Resources/Version.h"

/**
 * Compatibility guards for optional plugin dependencies
 * 
 * Use these macros and functions to safely check for optional modules
 * before using their types or APIs.
 */

// =============================================================================
// Module Availability Checks
// =============================================================================

/**
 * Check if a module is loaded and available
 */
inline bool IsModuleAvailable(const TCHAR* ModuleName)
{
    FModuleManager& ModuleManager = FModuleManager::Get();
    const FName ModuleFName(ModuleName);

    if (ModuleManager.IsModuleLoaded(ModuleFName) || ModuleManager.IsModuleSafeToUse(ModuleFName))
    {
        return true;
    }

    if (!ModuleManager.ModuleExists(ModuleName))
    {
        return false;
    }

    for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
    {
        for (const FModuleDescriptor& Descriptor : Plugin->GetDescriptor().Modules)
        {
            if (Descriptor.Name == ModuleFName)
            {
                return true;
            }
        }
    }

    return false;
}

// Common optional modules
inline bool IsNiagaraAvailable()
{
    return IsModuleAvailable(TEXT("Niagara"));
}

inline bool IsPCGAvailable()
{
    return IsModuleAvailable(TEXT("PCG"));
}

inline bool IsPythonAvailable()
{
    return IsModuleAvailable(TEXT("PythonScriptPlugin"));
}

inline bool IsEnhancedInputAvailable()
{
    return IsModuleAvailable(TEXT("EnhancedInput"));
}

inline bool IsOnlineSubsystemAvailable()
{
    return IsModuleAvailable(TEXT("OnlineSubsystem"));
}

inline bool IsGameplayAbilitiesAvailable()
{
    return IsModuleAvailable(TEXT("GameplayAbilities"));
}

// =============================================================================
// Guarded Tool Registration Macro
// =============================================================================

/**
 * Register a tool only if a module is available
 * Usage:
 *   REGISTER_TOOL_IF_MODULE_AVAILABLE(TEXT("Niagara"), Tool, Handler);
 */
#define REGISTER_TOOL_IF_MODULE_AVAILABLE(ModuleName, ToolVar, Handler) \
    do { \
        if (IsModuleAvailable(ModuleName)) { \
            Registry.RegisterTool(ToolVar, Handler); \
            RegisteredToolCount++; \
        } else { \
            UE_LOG(LogRiftbornAI, Verbose, TEXT("Skipping tool '%s' - module '%s' not available"), *ToolVar.Name, ModuleName); \
        } \
    } while(0)

/**
 * Register Niagara tool only if Niagara is available
 */
#define REGISTER_NIAGARA_TOOL(ToolVar, Handler) \
    REGISTER_TOOL_IF_MODULE_AVAILABLE(TEXT("Niagara"), ToolVar, Handler)

/**
 * Register PCG tool only if PCG is available
 */
#define REGISTER_PCG_TOOL(ToolVar, Handler) \
    REGISTER_TOOL_IF_MODULE_AVAILABLE(TEXT("PCG"), ToolVar, Handler)

/**
 * Register Python tool only if Python scripting is available
 */
#define REGISTER_PYTHON_TOOL(ToolVar, Handler) \
    REGISTER_TOOL_IF_MODULE_AVAILABLE(TEXT("PythonScriptPlugin"), ToolVar, Handler)

// =============================================================================
// Safe Tool Result for Unavailable Features
// =============================================================================

/**
 * Create a standardized error result for unavailable features
 */
inline FClaudeToolResult MakeUnavailableModuleResult(const FString& ToolUseId, const FString& ModuleName)
{
    FClaudeToolResult Result;
    Result.ToolUseId = ToolUseId;
    Result.bSuccess = false;
    Result.Result = FString::Printf(
        TEXT("{\"error\": \"Module '%s' is not available. Enable it in your project's .uproject file or Editor > Plugins.\"}"),
        *ModuleName
    );
    return Result;
}

/**
 * Guard macro for tool implementations - returns error if module not available
 * Usage at start of tool function:
 *   GUARD_MODULE_AVAILABLE(TEXT("Niagara"), Call);
 */
#define GUARD_MODULE_AVAILABLE(ModuleName, CallVar) \
    do { \
        if (!IsModuleAvailable(ModuleName)) { \
            return MakeUnavailableModuleResult(CallVar.ToolUseId, ModuleName); \
        } \
    } while(0)

// =============================================================================
// Version Compatibility
// =============================================================================

/**
 * Get the engine version for compatibility checks
 */
inline int32 GetEngineMinorVersion()
{
    return ENGINE_MINOR_VERSION;
}

inline int32 GetEngineMajorVersion()
{
    return ENGINE_MAJOR_VERSION;
}

/**
 * Check if we're on at least a specific engine version
 */
inline bool IsEngineVersionAtLeast(int32 Major, int32 Minor)
{
    if (ENGINE_MAJOR_VERSION > Major) return true;
    if (ENGINE_MAJOR_VERSION < Major) return false;
    return ENGINE_MINOR_VERSION >= Minor;
}

/**
 * Guard for UE 5.7+ specific features
 */
#define GUARD_UE57_FEATURE(CallVar, FeatureName) \
    do { \
        if (!IsEngineVersionAtLeast(5, 7)) { \
            FClaudeToolResult Result; \
            Result.ToolUseId = CallVar.ToolUseId; \
            Result.bSuccess = false; \
            Result.Result = FString::Printf(TEXT("{\"error\": \"Feature '%s' requires Unreal Engine 5.7 or later\"}"), TEXT(FeatureName)); \
            return Result; \
        } \
    } while(0)

// =============================================================================
// Editor-Only Guards
// =============================================================================

/**
 * Guard for editor-only functionality
 */
#define GUARD_EDITOR_ONLY(CallVar) \
    do { \
        if (!GIsEditor || !GEditor) { \
            FClaudeToolResult Result; \
            Result.ToolUseId = CallVar.ToolUseId; \
            Result.bSuccess = false; \
            Result.Result = TEXT("{\"error\": \"This tool is only available in the Unreal Editor\"}"); \
            return Result; \
        } \
    } while(0)

/**
 * Guard for PIE (Play In Editor) context
 */
#define GUARD_NOT_IN_PIE(CallVar) \
    do { \
        if (GEditor && GEditor->IsPlayingSessionInEditor()) { \
            FClaudeToolResult Result; \
            Result.ToolUseId = CallVar.ToolUseId; \
            Result.bSuccess = false; \
            Result.Result = TEXT("{\"error\": \"This tool cannot be used during Play In Editor. Stop the game first.\"}"); \
            return Result; \
        } \
    } while(0)
