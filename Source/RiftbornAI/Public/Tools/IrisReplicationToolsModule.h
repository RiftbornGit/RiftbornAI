// Copyright RiftbornAI. All Rights Reserved.
// Thin Iris replication policy tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FIrisReplicationToolsModule
 * 
 * Thin Iris replication policy tools.
 */
class RIFTBORNAI_API FIrisReplicationToolsModule : public TToolModuleBase<FIrisReplicationToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("IrisReplicationTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateReplicationGroup(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddActorToReplicationGroup(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetReplicationGroupFilterStatus(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetActorCullDistanceOverride(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ClearActorCullDistanceOverride(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetActorStaticPriority(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ForceActorNetUpdate(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectActorReplication(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetActorReplicationInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetReplicationGroupInfo(const FClaudeToolCall& Call);
};
