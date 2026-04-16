// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FAtmosphericRealismToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FAtmosphericRealismToolsModule : public TToolModuleBase<FAtmosphericRealismToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("AtmosphericRealismTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_PropagateWindGusts(const FClaudeToolCall& Call);
};
