// Copyright RiftbornAI. All Rights Reserved.
// World Partition Tools Module
//
// Manages World Partition data layers, streaming sources, HLOD,
// level instancing, and One File Per Actor (OFPA) for large open worlds.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * World Partition Tools Module
 *
 * Provides tools for UE5 World Partition system:
 * - list_data_layers: List all data layers in the current world
 * - create_data_layer: Create a new data layer
 * - set_actor_data_layer: Assign an actor to a data layer
 * - configure_streaming_source: Configure a streaming source component
 * - get_world_partition_info: Inspect World Partition streaming, data-layer, HLOD, and actor-count state
 * - create_hlod_layer: Create an HLOD layer asset with type, cell size, and loading range
 * - configure_level_instance: Create/configure a level instance
 */
class RIFTBORNAI_API FWorldPartitionToolsModule : public TToolModuleBase<FWorldPartitionToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("WorldPartitionTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_ListDataLayers(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateDataLayer(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetActorDataLayer(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConfigureStreamingSource(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetWorldPartitionInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateHLODLayer(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConfigureLevelInstance(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectWorldPartitionLiveState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateCityStateDataLayers(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetCityStateRuntime(const FClaudeToolCall& Call);
};
