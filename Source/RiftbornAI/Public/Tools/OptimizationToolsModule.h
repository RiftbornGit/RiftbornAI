// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FOptimizationToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FOptimizationToolsModule : public TToolModuleBase<FOptimizationToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("OptimizationTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_MergeStaticMeshes(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AuditPerformance(const FClaudeToolCall& Call);
};
