// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "ProductionHardening.h"

/**
 * Tool parameter definition
 */
struct RIFTBORNAI_API FToolParameter
{
    FString Name;
    FString Description;
    bool bRequired;
    FString Type;  // string, int, float, bool, array
    FString DefaultValue;
    
    FToolParameter() : bRequired(false) {}
    FToolParameter(const FString& InName, const FString& InDesc, bool bInRequired, const FString& InType)
        : Name(InName), Description(InDesc), bRequired(bInRequired), Type(InType)
    {}
};

/**
 * Tool execution result
 */
struct RIFTBORNAI_API FToolResult
{
    bool bSuccess;
    FString Output;
    FString ErrorMessage;
    
    FToolResult() : bSuccess(false) {}
    
    static FToolResult Success(const FString& InOutput)
    {
        FToolResult Result;
        Result.bSuccess = true;
        Result.Output = InOutput;
        return Result;
    }
    
    // Overload for JSON object results - serializes to string
    static FToolResult Success(TSharedPtr<FJsonObject> JsonResult)
    {
        FToolResult Result;
        Result.bSuccess = true;
        if (JsonResult.IsValid())
        {
            TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result.Output);
            FJsonSerializer::Serialize(JsonResult.ToSharedRef(), Writer);
        }
        return Result;
    }
    
    static FToolResult Failure(const FString& Error)
    {
        FToolResult Result;
        Result.bSuccess = false;
        Result.ErrorMessage = Error;
        return Result;
    }
    
    FString ToJson() const
    {
        TSharedRef<FJsonObject> JsonObj = MakeShared<FJsonObject>();
        JsonObj->SetBoolField(TEXT("success"), bSuccess);
        if (bSuccess)
        {
            JsonObj->SetStringField(TEXT("output"), Output);
        }
        else
        {
            JsonObj->SetStringField(TEXT("error"), ErrorMessage);
        }
        
        FString OutputString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
        FJsonSerializer::Serialize(JsonObj, Writer);
        return OutputString;
    }
};

/**
 * Base class for agent tools
 * Tools inherit from this to provide specific functionality to the agent
 */
class RIFTBORNAI_API FAgentTool
{
public:
    virtual ~FAgentTool() = default;
    
    /** Get the unique name of this tool */
    virtual FString GetName() const = 0;
    
    /** Get human-readable description of what this tool does */
    virtual FString GetDescription() const = 0;
    
    /** Get the parameters this tool accepts */
    virtual TArray<FToolParameter> GetParameters() const = 0;
    
    /** Get the permission tier required to execute this tool */
    virtual EToolPermissionTier GetPermissionTier() const { return EToolPermissionTier::ReadOnly; }
    
    /** Execute the tool with given parameters */
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) = 0;
    
    /** Build JSON schema for this tool (for Claude API) */
    TSharedPtr<FJsonObject> BuildToolJson() const
    {
        TSharedRef<FJsonObject> ToolObj = MakeShared<FJsonObject>();
        ToolObj->SetStringField(TEXT("name"), GetName());
        ToolObj->SetStringField(TEXT("description"), GetDescription());
        ToolObj->SetStringField(TEXT("permission_tier"), ToolPermissionTierToString(GetPermissionTier()));
        
        // Build input schema
        TSharedRef<FJsonObject> InputSchema = MakeShared<FJsonObject>();
        InputSchema->SetStringField(TEXT("type"), TEXT("object"));
        
        TSharedRef<FJsonObject> Properties = MakeShared<FJsonObject>();
        TArray<TSharedPtr<FJsonValue>> Required;
        
        for (const FToolParameter& Param : GetParameters())
        {
            TSharedRef<FJsonObject> PropObj = MakeShared<FJsonObject>();
            PropObj->SetStringField(TEXT("type"), Param.Type);
            PropObj->SetStringField(TEXT("description"), Param.Description);
            
            if (!Param.DefaultValue.IsEmpty())
            {
                PropObj->SetStringField(TEXT("default"), Param.DefaultValue);
            }
            
            Properties->SetObjectField(Param.Name, PropObj);
            
            if (Param.bRequired)
            {
                Required.Add(MakeShared<FJsonValueString>(Param.Name));
            }
        }
        
        InputSchema->SetObjectField(TEXT("properties"), Properties);
        InputSchema->SetArrayField(TEXT("required"), Required);
        
        ToolObj->SetObjectField(TEXT("input_schema"), InputSchema);
        
        return ToolObj;
    }
};

/**
 * Registry for agent tools
 * Manages the legacy agent-only tool surface.
 *
 * This registry is intentionally isolated from the governed/public
 * FClaudeToolRegistry execution path. Do not route provider, bridge,
 * controller, or tool-discovery surfaces through it unless the architecture
 * is intentionally being unified.
 */
class RIFTBORNAI_API FAgentToolRegistry
{
public:
    static FAgentToolRegistry& Get()
    {
        static FAgentToolRegistry Instance;
        return Instance;
    }

    /** Register a tool instance */
    void RegisterTool(const FString& Name, TSharedPtr<FAgentTool> Tool)
    {
        if (Tool.IsValid())
        {
            FScopeLock Lock(&ToolsLock);
            Tools.Add(Name, Tool);
            UE_LOG(LogTemp, Log, TEXT("[AgentToolRegistry] Registered tool: %s"), *Name);
        }
    }

    /** Register a tool instance (auto-extracts name from tool) */
    void RegisterTool(TSharedPtr<FAgentTool> Tool)
    {
        if (Tool.IsValid())
        {
            RegisterTool(Tool->GetName(), Tool);
        }
    }

    /** Unregister a tool */
    void UnregisterTool(const FString& Name)
    {
        FScopeLock Lock(&ToolsLock);
        Tools.Remove(Name);
    }
    
    /** Get a tool by name */
    TSharedPtr<FAgentTool> GetTool(const FString& Name) const
    {
        FScopeLock Lock(&ToolsLock);
        const TSharedPtr<FAgentTool>* Found = Tools.Find(Name);
        return Found ? *Found : nullptr;
    }

    /** Get all registered tools (returns copy for thread safety) */
    TMap<FString, TSharedPtr<FAgentTool>> GetAllTools() const
    {
        FScopeLock Lock(&ToolsLock);
        return Tools;
    }

    /** Execute a tool by name with permission tier checking */
    FToolResult ExecuteTool(const FString& Name, const TMap<FString, FString>& Parameters, UWorld* World,
                            EToolPermissionTier MaxAllowedTier = EToolPermissionTier::Administrative)
    {
        TSharedPtr<FAgentTool> Tool = GetTool(Name);
        if (!Tool.IsValid())
        {
            return FToolResult::Failure(FString::Printf(TEXT("Tool not found: %s"), *Name));
        }
        
        // Check permission tier
        EToolPermissionTier ToolTier = Tool->GetPermissionTier();
        if (static_cast<uint8>(ToolTier) > static_cast<uint8>(MaxAllowedTier))
        {
            return FToolResult::Failure(FString::Printf(
                TEXT("Permission denied: tool '%s' requires tier %s but max allowed is %s"),
                *Name,
                *ToolPermissionTierToString(ToolTier),
                *ToolPermissionTierToString(MaxAllowedTier)));
        }
        
        // Block destructive by default (unless explicitly allowed)
        if (RiftbornProductionFlags::BLOCK_DESTRUCTIVE_BY_DEFAULT && 
            ToolTier >= EToolPermissionTier::Destructive &&
            MaxAllowedTier < EToolPermissionTier::Destructive)
        {
            return FToolResult::Failure(FString::Printf(
                TEXT("Destructive tool '%s' blocked by production safety flag"),
                *Name));
        }
        
        return Tool->Execute(Parameters, World);
    }
    
    /** Build JSON array of all tools for Claude API */
    TArray<TSharedPtr<FJsonValue>> BuildToolsJson() const
    {
        FScopeLock Lock(&ToolsLock);
        TArray<TSharedPtr<FJsonValue>> ToolsArray;
        for (const auto& Pair : Tools)
        {
            if (Pair.Value.IsValid())
            {
                ToolsArray.Add(MakeShared<FJsonValueObject>(Pair.Value->BuildToolJson()));
            }
        }
        return ToolsArray;
    }

    /** Get count of registered tools */
    int32 GetToolCount() const
    {
        FScopeLock Lock(&ToolsLock);
        return Tools.Num();
    }

    /** Get all tool names */
    TArray<FString> GetToolNames() const
    {
        FScopeLock Lock(&ToolsLock);
        TArray<FString> Names;
        Tools.GetKeys(Names);
        return Names;
    }
    
private:
    FAgentToolRegistry() = default;
    
    TMap<FString, TSharedPtr<FAgentTool>> Tools;
    mutable FCriticalSection ToolsLock;
};
