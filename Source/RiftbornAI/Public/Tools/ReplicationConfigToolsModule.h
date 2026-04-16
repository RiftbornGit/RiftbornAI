// Copyright RiftbornAI. All Rights Reserved.
// Non-Iris replication configuration tools.
// Works on any actor regardless of whether the Iris replication system is active.
// Complements IrisReplicationToolsModule (which requires RIFTBORN_WITH_IRIS).

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

class RIFTBORNAI_API FReplicationConfigToolsModule : public TToolModuleBase<FReplicationConfigToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ReplicationConfigTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	static FClaudeToolResult Tool_SetActorReplicationConfig(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_StartMultiClientPIE(const FClaudeToolCall& Call);
};
