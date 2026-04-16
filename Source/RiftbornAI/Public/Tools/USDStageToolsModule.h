// Copyright RiftbornAI. All Rights Reserved.
// USD stage tools for live twin and staged import workflows.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FUSDStageToolsModule
 * 
 * USD stage tools for live twin and staged import workflows.
 */
class RIFTBORNAI_API FUSDStageToolsModule : public TToolModuleBase<FUSDStageToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("USDStageTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_SpawnUSDStageActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetUSDStageActorInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetUSDStageRootLayer(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConfigureUSDStageImport(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetUSDStageTime(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectUSDStageActor(const FClaudeToolCall& Call);
};
