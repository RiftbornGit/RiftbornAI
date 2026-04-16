// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FCinematicComposerToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FCinematicComposerToolsModule : public TToolModuleBase<FCinematicComposerToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("CinematicComposerTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_ComposeCinematicSequence(const FClaudeToolCall& Call);
};
