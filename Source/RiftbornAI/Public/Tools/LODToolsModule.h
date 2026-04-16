// Copyright RiftbornAI. All Rights Reserved.
// LOD (Level of Detail) Tools Module
//
// Manages mesh LOD groups, screen-size thresholds, auto-generation,
// and actor LOD settings for performance optimization.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * LOD Tools Module
 *
 * Provides tools for Level of Detail management:
 * - get_mesh_lod_info: Get LOD details for a static/skeletal mesh
 * - set_lod_screen_size: Set screen-size threshold for an LOD level
 * - auto_generate_lods: Auto-generate LOD levels for a mesh using reduction settings
 * - remove_lod: Remove a specific LOD level from a mesh
 * - set_lod_count: Set the number of LOD levels
 * - get_lod_group_info: Get LOD group settings
 * - set_actor_lod_distance_factor: Override LOD distance factor on an actor
 * - batch_set_lod_group: Assign LOD group to multiple meshes
 * - list_lod_groups: List all available LOD groups
 */
class RIFTBORNAI_API FLODToolsModule : public TToolModuleBase<FLODToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("LODTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_GetMeshLODInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLODScreenSize(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AutoGenerateLODs(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RemoveLOD(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLODCount(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLODGroupInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetActorLODDistanceFactor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_BatchSetLODGroup(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListLODGroups(const FClaudeToolCall& Call);

};
