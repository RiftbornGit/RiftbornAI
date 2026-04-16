// Copyright RiftbornAI. All Rights Reserved.
// Neural morph and ML deformer asset tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FNeuralMorphToolsModule
 * 
 * Neural morph and ML deformer asset tools.
 */
class RIFTBORNAI_API FNeuralMorphToolsModule : public TToolModuleBase<FNeuralMorphToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("NeuralMorphTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateNeuralMorphAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConfigureNeuralMorphAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectNeuralMorphAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetNeuralMorphAssetInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AttachNeuralMorphComponent(const FClaudeToolCall& Call);
};
