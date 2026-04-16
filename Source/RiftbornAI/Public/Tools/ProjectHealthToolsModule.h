// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

class RIFTBORNAI_API FProjectHealthToolsModule : public TToolModuleBase<FProjectHealthToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ProjectHealth"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	static FClaudeToolResult Tool_ScanProjectHealth(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AuditBlueprintHealth(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AuditTextureMemory(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_FindUnusedAssets(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CheckNamingConventions(const FClaudeToolCall& Call);
};
