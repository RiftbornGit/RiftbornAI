// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * Tools for gameplay systems awareness - GAS abilities/attributes, AI behavior trees, blackboards
 */

// Tool: get_gas_abilities - Get GAS abilities on an actor
class RIFTBORNAI_API FGetGasAbilitiesTool : public FAgentTool
{
public:
    FGetGasAbilitiesTool();
    virtual FString GetName() const override { return TEXT("get_gas_abilities"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_gas_attributes - Get GAS attribute set values
class RIFTBORNAI_API FGetGasAttributesTool : public FAgentTool
{
public:
    FGetGasAttributesTool();
    virtual FString GetName() const override { return TEXT("get_gas_attributes"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_active_effects - Get active gameplay effects on actor
class RIFTBORNAI_API FGetActiveEffectsTool : public FAgentTool
{
public:
    FGetActiveEffectsTool();
    virtual FString GetName() const override { return TEXT("get_active_effects"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_gameplay_tags - Get gameplay tags on actor
class RIFTBORNAI_API FGetGameplayTagsTool : public FAgentTool
{
public:
    FGetGameplayTagsTool();
    virtual FString GetName() const override { return TEXT("get_gameplay_tags"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_behavior_tree_state - Get AI behavior tree state
class RIFTBORNAI_API FGetBehaviorTreeStateTool : public FAgentTool
{
public:
    FGetBehaviorTreeStateTool();
    virtual FString GetName() const override { return TEXT("get_behavior_tree_state"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_blackboard_values - Get AI blackboard key values
class RIFTBORNAI_API FGetBlackboardValuesTool : public FAgentTool
{
public:
    FGetBlackboardValuesTool();
    virtual FString GetName() const override { return TEXT("get_blackboard_values"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_ai_controllers - List all AI controllers and their pawns
class RIFTBORNAI_API FGetAIControllersTool : public FAgentTool
{
public:
    FGetAIControllersTool();
    virtual FString GetName() const override { return TEXT("get_ai_controllers"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_eqs_queries - Get Environment Query System results
class RIFTBORNAI_API FGetEQSQueriesTool : public FAgentTool
{
public:
    FGetEQSQueriesTool();
    virtual FString GetName() const override { return TEXT("get_eqs_queries"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration wrapper class
class RIFTBORNAI_API FGameplaySystemsAwarenessTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
