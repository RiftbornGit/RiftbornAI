// Copyright RiftbornAI. All Rights Reserved.
// Chaos Field System tools for authored force and destruction painting.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FFieldSystemToolsModule
 * 
 * Chaos Field System tools for authored force and destruction painting.
 */
class RIFTBORNAI_API FFieldSystemToolsModule : public TToolModuleBase<FFieldSystemToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("FieldSystemTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_AddFieldSystemComponent(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetFieldSystemInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyLinearForceField(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyRadialForceField(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyUniformForceField(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddPersistentRadialFalloffField(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ResetFieldSystem(const FClaudeToolCall& Call);
};
