// Copyright RiftbornAI. All Rights Reserved.
// Thin Chooser / ProxyTable evaluation tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FChooserToolsModule
 * 
 * Thin Chooser / ProxyTable evaluation tools.
 */
class RIFTBORNAI_API FChooserToolsModule : public TToolModuleBase<FChooserToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ChooserTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_GetChooserTableInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetProxyTableInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateChooserTable(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateChooserTableMulti(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateProxyTable(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateProxyAsset(const FClaudeToolCall& Call);
};
