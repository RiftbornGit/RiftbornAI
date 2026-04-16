// Copyright RiftbornAI. All Rights Reserved.
// Destruction Tools Module — automatic breakable setup from static meshes.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FDestructionToolsModule
 * 
 * Destruction Tools Module — automatic breakable setup from static meshes.
 */
class RIFTBORNAI_API FDestructionToolsModule : public TToolModuleBase<FDestructionToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("DestructionTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	static FClaudeToolResult Tool_InspectDestructibleConversionReadiness(const FClaudeToolCall& Call);
	// Tool 5: Convert a static mesh actor into a breakable geometry collection
	static FClaudeToolResult Tool_MakeDestructible(const FClaudeToolCall& Call);
};
