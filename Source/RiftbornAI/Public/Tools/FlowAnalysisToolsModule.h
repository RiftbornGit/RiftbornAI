// Copyright RiftbornAI. All Rights Reserved.
// Player flow and spatial layout analysis tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

class RIFTBORNAI_API FFlowAnalysisToolsModule : public TToolModuleBase<FFlowAnalysisToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("FlowAnalysisTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	static FClaudeToolResult Tool_AnalyzePlayerFlow(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AuditCoverLayout(const FClaudeToolCall& Call);
};
