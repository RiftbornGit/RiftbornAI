// Copyright RiftbornAI. All Rights Reserved.
// Mass Crowd tools for live crowd simulation control.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FMassCrowdToolsModule
 * 
 * Mass Crowd tools for live crowd simulation control.
 */
class RIFTBORNAI_API FMassCrowdToolsModule : public TToolModuleBase<FMassCrowdToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("MassCrowdTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_SpawnMassCrowdSpawner(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RunMassCrowdSpawner(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetMassCrowdLaneInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectMassCrowdState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMassCrowdLaneState(const FClaudeToolCall& Call);
};
