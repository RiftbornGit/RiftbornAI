// Copyright RiftbornAI. All Rights Reserved.
// WebResearchToolsModule - grounded public-web search and fetch tools

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Web research tools for grounded online research.
 * These tools fetch real public web content through curl and return
 * structured results that the copilot can cite and inspect.
 */
class RIFTBORNAI_API FWebResearchToolsModule : public TToolModuleBase<FWebResearchToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("WebResearchTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	static FClaudeToolResult Tool_SearchWeb(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_FetchWebpage(const FClaudeToolCall& Call);
};
