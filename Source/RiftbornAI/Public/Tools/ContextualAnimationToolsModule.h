// Copyright RiftbornAI. All Rights Reserved.
// Contextual Animation binding and authored-state inspection tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FContextualAnimationToolsModule
 * 
 * Contextual Animation binding and authored-state inspection tools.
 */
class RIFTBORNAI_API FContextualAnimationToolsModule : public TToolModuleBase<FContextualAnimationToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ContextualAnimationTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_GetContextualAnimSceneAssetInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateContextualAnimBindingsForTwoActors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CalculateContextualAnimWarpPoints(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ActivateContextualAnimWarpTargets(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectContextualAnimBindings(const FClaudeToolCall& Call);
};
