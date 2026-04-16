// Copyright RiftbornAI. All Rights Reserved.
// Architecture Tools Module — procedural building, staircase, and spline-based architecture generation.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FArchitectureToolsModule
 * 
 * Architecture Tools Module — procedural building, staircase, and spline-based architecture generation.
 */
class RIFTBORNAI_API FArchitectureToolsModule : public TToolModuleBase<FArchitectureToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ArchitectureTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	static FClaudeToolResult Tool_CreateBuildingFromFloorPlan(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateStaircase(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateSplineArchitecture(const FClaudeToolCall& Call);
};
