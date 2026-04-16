// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FMeshPipelineToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FMeshPipelineToolsModule : public TToolModuleBase<FMeshPipelineToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("MeshPipelineTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_GenerateCollisionMesh(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AutoLODChain(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ForgeTextureFromMaterial(const FClaudeToolCall& Call);
};
