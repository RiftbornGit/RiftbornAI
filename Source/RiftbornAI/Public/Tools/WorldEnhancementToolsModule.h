// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FWorldEnhancementToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FWorldEnhancementToolsModule : public TToolModuleBase<FWorldEnhancementToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("WorldEnhancementTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_SculptVolumetricClouds(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_StringCablesBetweenPoints(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PopulateInterior(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_OptimizeWorldPartition(const FClaudeToolCall& Call);
};
