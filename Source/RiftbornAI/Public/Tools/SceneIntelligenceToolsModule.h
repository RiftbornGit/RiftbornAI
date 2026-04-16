// Copyright RiftbornAI. All Rights Reserved.
// Scene Intelligence Tools Module
//
// Provides ground-truth structured scene queries for AI vision enhancement:
// - Frustum query: which actors are visible in the current camera view
// - Material audit: bulk material properties for all actors in one call
// - Animation state: current anim state for all skeletal mesh actors
// - Audio spatial: sound source positions, attenuation, and listener-relative data

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FSceneIntelligenceToolsModule
 * 
 * Scene Intelligence Tools Module
 */
class RIFTBORNAI_API FSceneIntelligenceToolsModule : public TToolModuleBase<FSceneIntelligenceToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("SceneIntelligence"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	static FClaudeToolResult Tool_FrustumQuery(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_MaterialAudit(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AnimationState(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AudioSpatial(const FClaudeToolCall& Call);

};
