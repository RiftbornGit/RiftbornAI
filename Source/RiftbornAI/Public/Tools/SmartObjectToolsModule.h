// Copyright RiftbornAI. All Rights Reserved.
// Smart Object tools for authored affordance networks.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FSmartObjectToolsModule
 * 
 * Smart Object tools for authored affordance networks.
 */
class RIFTBORNAI_API FSmartObjectToolsModule : public TToolModuleBase<FSmartObjectToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("SmartObjectTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_GetSmartObjectActorInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RegisterSmartObjectActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSmartObjectActorEnabled(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectSmartObjectActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSmartObjectSlotEnabled(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ClaimSmartObjectSlot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_OccupySmartObjectSlot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ReleaseSmartObjectSlot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SendSmartObjectSlotEvent(const FClaudeToolCall& Call);
};
