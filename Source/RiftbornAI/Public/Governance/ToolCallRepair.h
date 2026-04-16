// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// ToolCallRepair - Schema-driven tool call self-repair

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Result of a repair attempt
 */
struct RIFTBORNAI_API FToolRepairResult
{
    /** Whether repair succeeded (all missing args were filled) */
    bool bSuccess = false;
    
    /** The repaired tool call with filled arguments */
    FClaudeToolCall RepairedCall;
    
    /** List of parameters that were auto-filled */
    TArray<FString> FilledParams;
    
    /** Sources used for filling (cached_context, query_tool, default) */
    TMap<FString, FString> FillSources;
    
    /** Why repair failed (if bSuccess is false) */
    FString FailureReason;
};

/**
 * Context cache for tool call repair - stores recent state
 */
struct RIFTBORNAI_API FToolRepairContext
{
    /** Last selected actor label/name */
    FString LastSelectedActor;
    
    /** Last spawned actor label */
    FString LastSpawnedActor;
    
    /** Last used class name */
    FString LastClassName;
    
    /** Current level name */
    FString CurrentLevelName;
    
    /** Last query results (for various tools) */
    TMap<FString, FString> CachedQueryResults;
    
    /** When context was last updated */
    FDateTime LastUpdated;
    
    /** Update from tool result metadata */
    void UpdateFromToolResult(const FClaudeToolResult& Result);
    
    /** Clear old cached data (older than specified seconds) */
    void PruneOldData(float MaxAgeSeconds = 300.0f);
};

/**
 * Tool Call Repair System
 * 
 * When a tool call fails validation (missing required params), this system:
 * 1. Parses the validation error to extract missing fields
 * 2. Attempts to fill them from cached context, query tools, or safe defaults
 * 3. Returns a repaired call that can be retried
 * 
 * This is SCHEMA-DRIVEN, not LLM-guess-driven.
 */
class RIFTBORNAI_API FToolCallRepair
{
public:
    static FToolCallRepair& Get();
    
    /**
     * Attempt to repair a tool call that failed validation
     * 
     * @param OriginalCall - The call that failed
     * @param ValidationError - The error message from ValidateToolCall
     * @return Repair result with success status and repaired call
     */
    FToolRepairResult AttemptRepair(const FClaudeToolCall& OriginalCall, const FString& ValidationError);
    
    /**
     * Update context from a successful tool result
     * Call this after every successful tool execution to maintain context
     */
    void UpdateContext(const FClaudeToolResult& Result);
    
    /**
     * Get the current repair context (for debugging/introspection)
     */
    const FToolRepairContext& GetContext() const { return Context; }
    
    /**
     * Clear all cached context
     */
    void ClearContext();
    
    /**
     * Set whether to allow query tools for repair (default: true)
     * If false, only uses cached context and defaults
     */
    void SetAllowQueryTools(bool bAllow) { bAllowQueryTools = bAllow; }
    
private:
    FToolCallRepair() = default;
    
    /** Parse validation error to extract missing parameter names */
    TArray<FString> ParseMissingParams(const FString& ValidationError);
    
    /** Try to fill a missing parameter from context */
    bool TryFillFromContext(const FString& ToolName, const FString& ParamName, FString& OutValue, FString& OutSource);
    
    /** Try to fill a missing parameter by running a query tool */
    bool TryFillFromQuery(const FString& ToolName, const FString& ParamName, FString& OutValue, FString& OutSource);
    
    /** Try to fill with a safe default value */
    bool TryFillWithDefault(const FString& ToolName, const FString& ParamName, const FClaudeToolParameter& ParamDef, FString& OutValue, FString& OutSource);
    
    /** Get the tool schema for introspection */
    const FClaudeTool* GetToolSchema(const FString& ToolName) const;
    
    FToolRepairContext Context;
    bool bAllowQueryTools = true;
    mutable FCriticalSection ContextLock;
};
