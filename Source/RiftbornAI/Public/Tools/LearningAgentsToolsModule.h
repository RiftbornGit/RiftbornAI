// Copyright RiftbornAI. All Rights Reserved.
// Learning Agents inspection and imitation-training controls.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FLearningAgentsToolsModule
 * 
 * Learning Agents inspection and imitation-training controls.
 */
class RIFTBORNAI_API FLearningAgentsToolsModule : public TToolModuleBase<FLearningAgentsToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("LearningAgentsTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_InspectLearningAgentsManager(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLearningAgentsTrainerInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_MakeLearningAgentsImitationTrainer(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_BeginLearningAgentsTraining(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_IterateLearningAgentsTraining(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EndLearningAgentsTraining(const FClaudeToolCall& Call);
};
