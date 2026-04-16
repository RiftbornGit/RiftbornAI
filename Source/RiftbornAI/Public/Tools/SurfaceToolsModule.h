// Copyright RiftbornAI. All Rights Reserved.
// Surface Tools Module — texture-to-displacement and material-from-photo tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FSurfaceToolsModule
 * 
 * Surface Tools Module — texture-to-displacement and material-from-photo tools.
 */
class RIFTBORNAI_API FSurfaceToolsModule : public TToolModuleBase<FSurfaceToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("SurfaceTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool 2: Texture → heightmap → Nanite displacement or POM material setup
	static FClaudeToolResult Tool_ApplySurfaceDepth(const FClaudeToolCall& Call);

	// Tool 7: Photo → PBR material (albedo, normal, roughness, height textures)
	static FClaudeToolResult Tool_CreateMaterialFromPhoto(const FClaudeToolCall& Call);
};
