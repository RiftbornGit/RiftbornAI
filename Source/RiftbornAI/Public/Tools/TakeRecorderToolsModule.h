// Copyright RiftbornAI. All Rights Reserved.
// Thin Take Recorder controls.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FTakeRecorderToolsModule
 * 
 * Thin Take Recorder controls.
 */
class RIFTBORNAI_API FTakeRecorderToolsModule : public TToolModuleBase<FTakeRecorderToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("TakeRecorderTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_SetTakeRecorderTargetSequence(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddTakeRecorderActorSource(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_StartTakeRecording(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_StopTakeRecording(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectTakeRecorderState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetTakeRecorderInfo(const FClaudeToolCall& Call);
};
