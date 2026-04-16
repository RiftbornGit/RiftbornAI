// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FGamePolishToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FGamePolishToolsModule : public TToolModuleBase<FGamePolishToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("GamePolishTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_AddGameJuice(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PlaceAmbientSoundscape(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ComposeAnimStateMachine(const FClaudeToolCall& Call);
};
