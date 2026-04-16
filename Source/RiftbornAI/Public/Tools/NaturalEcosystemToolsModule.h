// Copyright RiftbornAI. All Rights Reserved.
// Natural Ecosystem Tools — ecology-aware tree scatter, understory generation,
// seasonal variation, vine growth, ground debris, foliage interaction, deformation trails.
// All APIs verified against UE 5.7: AInstancedFoliageActor, FFoliageInfo,
// FLandscapeEditDataInterface, UCableComponent, UMaterialParameterCollection,
// UMaterialExpressionCollectionParameter, UTextureRenderTarget2D.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FNaturalEcosystemToolsModule
 * 
 * Natural Ecosystem Tools — ecology-aware tree scatter, understory generation,
 */
class RIFTBORNAI_API FNaturalEcosystemToolsModule : public TToolModuleBase<FNaturalEcosystemToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("NaturalEcosystem"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Ecology-aware tree placement: Poisson disk + slope/moisture/water proximity rules
	static FClaudeToolResult Tool_ScatterEcologyTrees(const FClaudeToolCall& Call);

	// Auto-generate understory plants based on canopy occlusion (ray cast up)
	static FClaudeToolResult Tool_GenerateUnderstory(const FClaudeToolCall& Call);

	// Seasonal color/density variation via MaterialParameterCollection
	static FClaudeToolResult Tool_ApplySeason(const FClaudeToolCall& Call);

	// Spawn vine cables climbing vertical surfaces (tree trunks, walls, rocks)
	static FClaudeToolResult Tool_GrowVines(const FClaudeToolCall& Call);

	// Scatter leaf litter, twigs, pinecones near existing tree bases
	static FClaudeToolResult Tool_ScatterGroundDebris(const FClaudeToolCall& Call);

	// MPC + WPO proximity bending: grass/foliage bends away from characters
	static FClaudeToolResult Tool_SetupFoliageInteraction(const FClaudeToolCall& Call);

	// Persistent RT2D trail system: snow paths, grass trails, tire tracks
	static FClaudeToolResult Tool_SetupDeformationTrails(const FClaudeToolCall& Call);
};
