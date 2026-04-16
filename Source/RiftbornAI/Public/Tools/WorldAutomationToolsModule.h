// Copyright RiftbornAI. All Rights Reserved.
// World Automation Tools — screenshot-to-level, smart navmesh, interior decoration,
// volumetric clouds, ecosystem simulation.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FWorldAutomationToolsModule
 * 
 * World Automation Tools — screenshot-to-level, smart navmesh, interior decoration,
 */
class RIFTBORNAI_API FWorldAutomationToolsModule : public TToolModuleBase<FWorldAutomationToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("WorldAutomationTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool 1: AI-analyze a reference image → build a level from the description
	static FClaudeToolResult Tool_ScreenshotToLevel(const FClaudeToolCall& Call);

	// Tool 5: Analyze walkable geometry → auto-generate navmesh with nav areas
	static FClaudeToolResult Tool_SmartNavmesh(const FClaudeToolCall& Call);

	// Tool 7: Analyze room geometry → auto-place contextual furniture
	static FClaudeToolResult Tool_DecorateInterior(const FClaudeToolCall& Call);

	// Tool 8: Configure volumetric clouds for a weather type
	static FClaudeToolResult Tool_SculptClouds(const FClaudeToolCall& Call);

	// Tool 10: Ecology-driven vegetation placement from biome parameters
	static FClaudeToolResult Tool_SimulateEcosystem(const FClaudeToolCall& Call);
};
