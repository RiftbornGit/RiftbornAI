// Copyright RiftbornAI. All Rights Reserved.
// Variant Manager tools for authored world-state switching.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FVariantManagerToolsModule
 * 
 * Variant Manager tools for authored world-state switching.
 */
class RIFTBORNAI_API FVariantManagerToolsModule : public TToolModuleBase<FVariantManagerToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("VariantManagerTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateLevelVariantSetsAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateLevelVariantSetsActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddVariantSet(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddVariant(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_BindActorToVariant(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CaptureVariantProperty(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLevelVariantSetsInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SwitchVariantByName(const FClaudeToolCall& Call);
};
