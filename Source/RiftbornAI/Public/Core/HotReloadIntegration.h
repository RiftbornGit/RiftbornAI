// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Async/Future.h"
#include "HAL/PlatformProcess.h"

class FJsonObject;

/**
 * Hot Reload Integration
 * Enables the AI agent to trigger C++ compilation and hot reload
 * after writing/modifying C++ source files.
 */

RIFTBORNAI_API DECLARE_LOG_CATEGORY_EXTERN(LogHotReload, Log, All);

/**
 * Result of a compilation attempt
 */
struct RIFTBORNAI_API FCompilationResult
{
    bool bHasResult = false;
    bool bCompleted = false;
    bool bSuccess = false;
    FString Output;
    TArray<FString> Errors;
    TArray<FString> Warnings;
    float CompilationTimeSeconds = 0.0f;
    FString CompilationMethod;
    FDateTime StartedAtUtc;
    FDateTime CompletedAtUtc;
    int32 ProcessReturnCode = 0;

    // Parse build output into structured errors
    void ParseBuildOutput(const FString& RawOutput);
};

/**
 * Compilation error with file location
 */
struct RIFTBORNAI_API FBuildCompileError
{
    FString FilePath;
    int32 Line = 0;
    int32 Column = 0;
    FString Message;
    bool bIsError = true; // false = warning

    FString ToString() const
    {
        return FString::Printf(TEXT("%s(%d,%d): %s: %s"),
            *FilePath, Line, Column,
            bIsError ? TEXT("error") : TEXT("warning"),
            *Message);
    }
};

/**
 * Hot Reload Integration Manager
 */
class RIFTBORNAI_API FHotReloadIntegration
{
public:
    static FHotReloadIntegration& Get();

    /** Initialize with project path */
    void Initialize(const FString& InProjectPath);

    /** Check if Live Coding is available */
    bool IsLiveCodingAvailable() const;

    /** Trigger Live Coding rebuild (editor hot reload) */
    FCompilationResult TriggerLiveCoding();

    /** Run UnrealBuildTool for full module rebuild */
    FCompilationResult RunUBT(const FString& ModuleName, const FString& Configuration = TEXT("Development"));

    /** Parse compilation errors from build output */
    TArray<FBuildCompileError> ParseErrors(const FString& BuildOutput) const;

    /** Generate fix suggestions for a compilation error */
    FString SuggestFix(const FBuildCompileError& Error) const;

    /** Wait for any running compilation to complete */
    void WaitForCompilation();

    /** Is compilation currently in progress? */
    bool IsCompiling() const { return bIsCompiling; }

    /** Get raw output from the last compilation (if any) */
    FString GetLastCompilationOutput() const { return LastCompilationResult.Output; }

    /** Get the last compilation result (if any) */
    const FCompilationResult& GetLastCompilationResult() const { return LastCompilationResult; }

    /** Get cached structured compilation errors from the last UBT run */
    const TArray<FBuildCompileError>& GetCachedErrors() const { return CachedErrors; }

    /** Build a structured compilation status snapshot for tools/control-plane state */
    TSharedPtr<FJsonObject> BuildCompilationStatusJson(int32 MaxMessages = 10) const;

    // Tool implementations
    static FClaudeToolResult Tool_TriggerLiveCoding(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RunUBT(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetCompilationErrors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_FixCompilationError(const FClaudeToolCall& Call);

    /** Register all hot reload tools */
    void RegisterTools();

private:
    FHotReloadIntegration() = default;

    FString ProjectPath;
    FString ProjectName;
    FString EnginePath;
    bool bIsCompiling = false;

    FCompilationResult LastCompilationResult;
    TArray<FBuildCompileError> CachedErrors;

    // Execute a process and capture output
    bool ExecuteProcess(const FString& Executable, const FString& Arguments,
                       FString& OutOutput, int32& OutReturnCode, float TimeoutSeconds = 300.0f);

    // Find UBT path
    FString GetUBTPath() const;

    // Get target name from module
    FString GetTargetName() const;
};
