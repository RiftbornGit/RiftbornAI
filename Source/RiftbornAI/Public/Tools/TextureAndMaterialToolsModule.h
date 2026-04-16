// Copyright RiftbornAI. All Rights Reserved.
// Texture & Material Tools — procedural texture generation and custom HLSL materials.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FTextureAndMaterialToolsModule
 * 
 * Texture & Material Tools — procedural texture generation and custom HLSL materials.
 */
class RIFTBORNAI_API FTextureAndMaterialToolsModule : public TToolModuleBase<FTextureAndMaterialToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("TextureAndMaterialTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool 2: Generate tileable PBR texture from mathematical pattern
	static FClaudeToolResult Tool_GenerateProceduralTexture(const FClaudeToolCall& Call);

	// Tool 6: Create material with custom HLSL code expression
	static FClaudeToolResult Tool_CreateHLSLMaterial(const FClaudeToolCall& Call);

	// Tool 7: Create voxel terrain uber material (vertex-color layer selection, triplanar, POM)
	static FClaudeToolResult Tool_CreateVoxelTerrainMaterial(const FClaudeToolCall& Call);

	// Tool 8: Volumetric Material Engine — stochastic tiling, context rules, volumetric depth
	static FClaudeToolResult Tool_CreateVMEMaterial(const FClaudeToolCall& Call);
};
