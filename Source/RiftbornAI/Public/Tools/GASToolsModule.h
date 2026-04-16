// Copyright RiftbornAI. All Rights Reserved.
// GAS (Gameplay Ability System) Tools Module
//
// Registers the supported runtime GAS surface. Planning/preset/codegen helpers
// are intentionally omitted until they are backed by working implementations.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Gameplay Ability System Tools Module
 *
 * Registers the supported GAS runtime tools:
 * - Tag management (add, get, set on actors)
 * - Asset creation (abilities, effects, attribute sets, tag tables)
 * - Asset discovery and assignment
 */
class RIFTBORNAI_API FGASToolsModule : public TToolModuleBase<FGASToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("GASTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
};
