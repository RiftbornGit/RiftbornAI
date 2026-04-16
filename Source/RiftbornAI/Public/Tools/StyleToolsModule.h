// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FStyleToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FStyleToolsModule : public TToolModuleBase<FStyleToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("StyleTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_CreateDayNightCycle(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ApplyPostProcessStyle(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ExtractColorPalette(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_EnforceColorPalette(const FClaudeToolCall& Call);
};
