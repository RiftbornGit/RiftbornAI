// Copyright RiftbornAI. All Rights Reserved.
// PCG (Procedural Content Generation) Tools Module

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * PCG Tools Module
 * Provides tools for procedural content generation with UE5's PCG framework.
 * 
 * Tools included:
 * - create_pcg_graph: Create a new PCG Graph asset
 * - spawn_pcg_volume: Spawn a PCG volume in the level and bind a graph
 * - execute_pcg: Request PCG generation and report task state
 * - get_pcg_info: Get info about a PCG graph
 * - list_pcg_graphs: List all PCG graphs in project
 * - add_pcg_node: Add a node to a PCG graph
 * - connect_pcg_nodes: Connect two nodes in a PCG graph
 * - remove_pcg_node: Remove a node from a PCG graph
 * - duplicate_pcg_graph: Duplicate an existing PCG graph
 */
class RIFTBORNAI_API FPCGToolsModule : public TToolModuleBase<FPCGToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("PCGTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreatePCGGraph(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnPCGVolume(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExecutePCG(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetPCGInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListPCGGraphs(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddPCGNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConnectPCGNodes(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RemovePCGNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DuplicatePCGGraph(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateGeometryFoundryGraph(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnGeometryFoundryVolume(const FClaudeToolCall& Call);
};
