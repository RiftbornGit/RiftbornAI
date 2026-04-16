// Copyright RiftbornAI. All Rights Reserved.
// Control Rig Tools Module — IK rigs, retargeting, Control Rig graph manipulation

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Control Rig Tools Module
 *
 * Provides tools for animation rigging:
 * - create_ik_rig: Create an IK Rig asset from a skeleton
 * - create_ik_retargeter: Create an IK Retargeter between two rigs
 * - get_rig_info: Inspect IK Rig goals/chains/solvers or IK Retargeter source-target/op state
 */
class RIFTBORNAI_API FControlRigToolsModule : public TToolModuleBase<FControlRigToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ControlRigTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateIKRig(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateIKRetargeter(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetRigInfo(const FClaudeToolCall& Call);
};
