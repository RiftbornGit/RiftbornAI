// Copyright RiftbornAI. All Rights Reserved.
// Cross-domain scene coherence auditing.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

class RIFTBORNAI_API FSceneCoherenceToolsModule : public TToolModuleBase<FSceneCoherenceToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("SceneCoherenceTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	static FClaudeToolResult Tool_AuditSceneCoherence(const FClaudeToolCall& Call);
};
