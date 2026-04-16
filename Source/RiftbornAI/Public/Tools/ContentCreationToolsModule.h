// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FContentCreationToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FContentCreationToolsModule : public TToolModuleBase<FContentCreationToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ContentCreationTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_ScatterPropsWithContext(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_MakeTextureTileable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PaintLandscapeMask(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateAnimMontage(const FClaudeToolCall& Call);
};
