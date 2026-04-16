// Copyright RiftbornAI. All Rights Reserved.
// Agent MCP Tools — thin wrappers around FRiftbornAgentCore for task management and skill queries.

#pragma once

#include "CoreMinimal.h"
#include "Tools/ToolModuleBase.h"

struct FClaudeToolCall;
struct FClaudeToolResult;
class FClaudeToolRegistry;

class RIFTBORNAI_API FAgentMCPToolsModule : public TToolModuleBase<FAgentMCPToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("AgentMCP"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_StartAgentTask(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetAgenticSession(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetAgentTask(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CancelAgentTask(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListAgentTasks(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_FindAgentSkills(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetProjectGraph(const FClaudeToolCall& Call);
};
