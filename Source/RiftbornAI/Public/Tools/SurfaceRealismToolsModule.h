// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FSurfaceRealismToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FSurfaceRealismToolsModule : public TToolModuleBase<FSurfaceRealismToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("SurfaceRealismTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_AutoPlaceReflectionProbes(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_DistributeSurfaceDecals(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PaintProceduralWear(const FClaudeToolCall& Call);
};
