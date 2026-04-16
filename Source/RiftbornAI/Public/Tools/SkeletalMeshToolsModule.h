// Copyright RiftbornAI. All Rights Reserved.
// Skeletal Mesh Tools Module — Mesh merging, sockets, bones, morph targets

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Skeletal Mesh Tools Module
 *
 * Provides tools for skeletal mesh manipulation:
 * - get_skeleton_info: Get bone hierarchy, socket list, morph targets
 * - add_socket: Add a socket to a skeleton at a bone
 * - remove_socket: Remove a socket from a skeleton
 * - set_socket_transform: Set socket relative transform
 * - get_morph_targets: List all morph targets on a skeletal mesh
 * - set_morph_target_weight: Set morph target weight on a skinned mesh component
 * - get_bone_transform: Get world/component-space transform of a bone
 * - set_skeletal_mesh_lod: Configure LOD screen sizes for skeletal meshes
 * - copy_skeleton_sockets: Copy all sockets from one skeleton to another
 */
class RIFTBORNAI_API FSkeletalMeshToolsModule : public TToolModuleBase<FSkeletalMeshToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("SkeletalMeshTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_GetSkeletonInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddSocket(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RemoveSocket(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSocketTransform(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetMorphTargets(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMorphTargetWeight(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetBoneTransform(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSkeletalMeshLOD(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CopySkeletonSockets(const FClaudeToolCall& Call);
};
