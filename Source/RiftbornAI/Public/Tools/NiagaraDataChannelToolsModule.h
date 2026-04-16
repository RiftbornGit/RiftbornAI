// Copyright RiftbornAI. All Rights Reserved.
// Niagara Data Channel tools for gameplay-to-VFX data streaming.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FNiagaraDataChannelToolsModule
 * 
 * Niagara Data Channel tools for gameplay-to-VFX data streaming.
 */
class RIFTBORNAI_API FNiagaraDataChannelToolsModule : public TToolModuleBase<FNiagaraDataChannelToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("NiagaraDataChannelTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_GetNiagaraDataChannelInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_WriteNiagaraDataChannelBatch(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ReadNiagaraDataChannelBatch(const FClaudeToolCall& Call);
};
