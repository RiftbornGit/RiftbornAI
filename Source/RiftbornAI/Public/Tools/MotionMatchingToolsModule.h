// Copyright RiftbornAI. All Rights Reserved.
// Pose Search and motion-matching asset authoring tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FMotionMatchingToolsModule
 * 
 * Pose Search and motion-matching asset authoring tools.
 */
class RIFTBORNAI_API FMotionMatchingToolsModule : public TToolModuleBase<FMotionMatchingToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("MotionMatchingTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreatePoseSearchDatabase(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddAnimationToPoseSearchDatabase(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_BuildPoseSearchDatabaseIndex(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetPoseSearchDatabaseInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectPoseSearchDatabase(const FClaudeToolCall& Call);
};
