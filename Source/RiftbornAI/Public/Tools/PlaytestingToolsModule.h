// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FPlaytestingToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FPlaytestingToolsModule : public TToolModuleBase<FPlaytestingToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("PlaytestingTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_RunAIPlaytester(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RecordPlayerHeatmap(const FClaudeToolCall& Call);
};
