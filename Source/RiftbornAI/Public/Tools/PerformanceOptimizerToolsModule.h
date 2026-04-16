// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FPerformanceOptimizerToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FPerformanceOptimizerToolsModule : public TToolModuleBase<FPerformanceOptimizerToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("PerformanceOptimizerTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_AutoInstanceOptimizer(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_BakeMeshImpostor(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AutoLODMaterial(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PrefetchTextureStreaming(const FClaudeToolCall& Call);
};
