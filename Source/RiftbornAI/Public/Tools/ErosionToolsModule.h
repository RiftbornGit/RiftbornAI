// Copyright RiftbornAI. All Rights Reserved.
// Erosion Tools Module — hydraulic erosion simulation and non-destructive terrain stacks.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FErosionToolsModule
 * 
 * Erosion Tools Module — hydraulic erosion simulation and non-destructive terrain stacks.
 */
class RIFTBORNAI_API FErosionToolsModule : public TToolModuleBase<FErosionToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ErosionTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool 1: Particle-based hydraulic erosion on landscape heightmap
	static FClaudeToolResult Tool_HydraulicErosion(const FClaudeToolCall& Call);

	// Tool 7: Non-destructive terrain operation stack
	static FClaudeToolResult Tool_ApplyTerrainStack(const FClaudeToolCall& Call);
};
