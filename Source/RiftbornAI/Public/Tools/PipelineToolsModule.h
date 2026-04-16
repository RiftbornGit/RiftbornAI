// Copyright RiftbornAI. All Rights Reserved.
// Pipeline Tools — auto-LOD, cinematic sequence generation, blueprint prefab creation.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FPipelineToolsModule
 * 
 * Pipeline Tools — auto-LOD, cinematic sequence generation, blueprint prefab creation.
 */
class RIFTBORNAI_API FPipelineToolsModule : public TToolModuleBase<FPipelineToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("PipelineTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool 3: Auto-generate LODs for a static mesh
	static FClaudeToolResult Tool_GenerateLODs(const FClaudeToolCall& Call);

	// Tool 4: Create a cinematic level sequence from a camera path description
	static FClaudeToolResult Tool_CreateCinematic(const FClaudeToolCall& Call);

	// Tool 9: Turn a group of actors into a reusable Blueprint prefab
	static FClaudeToolResult Tool_CreateBlueprintFromActors(const FClaudeToolCall& Call);
};
