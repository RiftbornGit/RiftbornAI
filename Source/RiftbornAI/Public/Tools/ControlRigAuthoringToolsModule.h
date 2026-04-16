// Copyright RiftbornAI. All Rights Reserved.
// Control Rig and IK Retarget authoring tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FControlRigAuthoringToolsModule
 * 
 * Control Rig and IK Retarget authoring tools.
 */
class RIFTBORNAI_API FControlRigAuthoringToolsModule : public TToolModuleBase<FControlRigAuthoringToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ControlRigAuthoringTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateControlRigAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateControlRigFromSkeletalMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetControlRigBlueprintInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddControlRigBone(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddControlRigNull(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddControlRigControl(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddControlRigVariableNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddControlRigBranchNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddControlRigCommentNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_LinkControlRigPins(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetControlRigPinDefault(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetIKRetargetRoot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddIKRetargetChain(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AutoMapRetargetChains(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateRetargetPose(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetCurrentRetargetPose(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetRetargetPoseRotationOffset(const FClaudeToolCall& Call);
};
