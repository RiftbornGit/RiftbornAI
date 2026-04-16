// Copyright RiftbornAI. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FProjectAuditToolsModule
 * 
 * Tool module providing specialized functionality.
 */
class RIFTBORNAI_API FProjectAuditToolsModule : public TToolModuleBase<FProjectAuditToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ProjectAuditTools"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	static FClaudeToolResult Tool_AnnotateNavmeshTactical(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AuditNetReplication(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AuditAccessibility(const FClaudeToolCall& Call);
};
