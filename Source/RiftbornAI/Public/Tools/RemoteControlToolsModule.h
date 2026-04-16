// Copyright RiftbornAI. All Rights Reserved.
// Remote Control tools for exposing live editor/world controls.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FRemoteControlToolsModule
 * 
 * Remote Control tools for exposing live editor/world controls.
 */
class RIFTBORNAI_API FRemoteControlToolsModule : public TToolModuleBase<FRemoteControlToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("RemoteControlTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateRemoteControlPreset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetRemoteControlPresetInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExposeActorToRemoteControl(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExposePropertyToRemoteControl(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExposeFunctionToRemoteControl(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyRemoteControlColorWheelDelta(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyRemoteControlColorGradingDelta(const FClaudeToolCall& Call);
};
