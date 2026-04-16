// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FPCGComposerToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FPCGComposerToolsModule : public TToolModuleBase<FPCGComposerToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("PCGComposerTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_ComposePCGGraph(const FClaudeToolCall& Call);
};
