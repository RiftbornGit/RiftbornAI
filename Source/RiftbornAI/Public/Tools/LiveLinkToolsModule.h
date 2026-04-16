// Copyright RiftbornAI. All Rights Reserved.
// Thin Live Link inspection and routing controls.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FLiveLinkToolsModule
 * 
 * Thin Live Link inspection and routing controls.
 */
class RIFTBORNAI_API FLiveLinkToolsModule : public TToolModuleBase<FLiveLinkToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("LiveLinkTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_ListLiveLinkSubjects(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLiveLinkSubjectInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateLiveLinkFrame(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLiveLinkSubjectEnabled(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PauseLiveLinkSubject(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_UnpauseLiveLinkSubject(const FClaudeToolCall& Call);
};
