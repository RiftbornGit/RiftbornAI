// Copyright RiftbornAI. All Rights Reserved.
// Data Registry inspection and registration tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FDataRegistryToolsModule
 * 
 * Data Registry inspection and registration tools.
 */
class RIFTBORNAI_API FDataRegistryToolsModule : public TToolModuleBase<FDataRegistryToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("DataRegistryTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_GetDataRegistryInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListDataRegistryIds(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetDataRegistryIdDisplayText(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateDataRegistryCurve(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RegisterDataRegistryAsset(const FClaudeToolCall& Call);
};
