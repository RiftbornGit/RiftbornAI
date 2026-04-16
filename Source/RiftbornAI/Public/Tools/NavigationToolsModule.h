// Copyright RiftbornAI. All Rights Reserved.
// Navigation Tools Module — NavMesh, pathfinding, nav modifiers, nav links
//
// Navigation is fundamental to any game with AI. This module provides
// scripted access to common UE5 navigation operations.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Navigation Tools Module
 *
 * Provides tools for navigation:
 * - build_navmesh: Trigger navmesh generation / rebuild
 * - set_nav_agent_properties: Configure agent radius, height, step height
 * - add_nav_modifier_volume: Add a nav area modifier (walkable, blocked, custom)
 * - add_nav_link: Add a nav link proxy for jumps/teleports
 * - query_path: Find a path between two points
 * - get_navmesh_info: Get navmesh statistics and coverage
 * - set_navmesh_bounds: Create or resize nav mesh bounds volume
 */
class RIFTBORNAI_API FNavigationToolsModule : public TToolModuleBase<FNavigationToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("NavigationTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_BuildNavMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetNavAgentProperties(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddNavModifierVolume(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddNavLink(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_QueryPath(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetNavMeshInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetNavMeshBounds(const FClaudeToolCall& Call);

};
