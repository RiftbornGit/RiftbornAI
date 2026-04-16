// Copyright RiftbornAI. All Rights Reserved.
// Procedural Geometry Tools Module — SDF caves, L-system trees, SDF boolean architecture.
// Requires RIFTBORN_WITH_GEOMETRY_SCRIPTING=1 (GeometryCore).

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FProceduralGeometryToolsModule
 * 
 * Procedural Geometry Tools Module — SDF caves, L-system trees, SDF boolean architecture.
 */
class RIFTBORNAI_API FProceduralGeometryToolsModule : public TToolModuleBase<FProceduralGeometryToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ProceduralGeometryTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool 2: Cave/overhang generator (SDF composition + Marching Cubes)
	static FClaudeToolResult Tool_CreateCave(const FClaudeToolCall& Call);

	// Tool 4: Procedural tree via L-system + sweep
	static FClaudeToolResult Tool_CreateProceduralTree(const FClaudeToolCall& Call);

	// Tool 8: SDF boolean architecture (smooth blending volumes)
	static FClaudeToolResult Tool_CreateSDFArchitecture(const FClaudeToolCall& Call);
};
