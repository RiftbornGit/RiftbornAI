// Copyright RiftbornAI. All Rights Reserved.
// Chaos Destruction Tools Module
//
// Tools for UE5 Chaos Destruction system: geometry collections,
// fracture, clustering, debris tuning, and runtime destruction.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Chaos Destruction Tools Module
 *
 * Provides tools for UE5 Chaos physics destruction:
 * - create_geometry_collection: Create a destructible geometry collection from static mesh
 * - fracture_mesh: Apply fracture pattern to a geometry collection
 * - configure_clustering: Set supported clustering asset parameters
 * - set_damage_threshold: Configure break strain thresholds on a GC component
 * - get_destruction_info: Get live Geometry Collection and field-system status
 * - enable_destruction: Enable/disable Chaos destruction on an actor
 * - configure_debris: Set supported debris removal properties
 */
class RIFTBORNAI_API FChaosToolsModule : public TToolModuleBase<FChaosToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ChaosTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateGeometryCollection(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_FractureMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConfigureClustering(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetDamageThreshold(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetDestructionInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EnableDestruction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConfigureDebris(const FClaudeToolCall& Call);
};
