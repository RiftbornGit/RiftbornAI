// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FBlueprintWiringToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FBlueprintWiringToolsModule : public TToolModuleBase<FBlueprintWiringToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("BlueprintWiringTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_WireBlueprintFromDescription(const FClaudeToolCall& Call);
};
