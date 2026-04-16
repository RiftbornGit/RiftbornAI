// Copyright RiftbornAI. All Rights Reserved.
// Chaos Mover tools for physics-native movement authoring.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FChaosMoverToolsModule
 * 
 * Chaos Mover tools for physics-native movement authoring.
 */
class RIFTBORNAI_API FChaosMoverToolsModule : public TToolModuleBase<FChaosMoverToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ChaosMoverTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_AddChaosCharacterMover(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetChaosMoverInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_QueueChaosMoverMode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_LaunchChaosMover(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_OverrideChaosMoverSettings(const FClaudeToolCall& Call);
};
