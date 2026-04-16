// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FAIBehaviorToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FAIBehaviorToolsModule : public TToolModuleBase<FAIBehaviorToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("AIBehaviorTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_CreateBehaviorTree(const FClaudeToolCall& Call);
};
