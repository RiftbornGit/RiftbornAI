// Copyright RiftbornAI. All Rights Reserved.
// AI Infrastructure Tools Module - NavMesh and AI Perception

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FAIInfraToolsModule
 * 
 * AI Infrastructure Tools Module - NavMesh and AI Perception
 */
class RIFTBORNAI_API FAIInfraToolsModule : public TToolModuleBase<FAIInfraToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("AIInfraTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// NavMesh tools
	static FClaudeToolResult Tool_GetNavMeshStatus(const FClaudeToolCall& Call);

	// AI Perception tools
	static FClaudeToolResult Tool_ConfigureAIPerception(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetAIPerceptionInfo(const FClaudeToolCall& Call);

};
