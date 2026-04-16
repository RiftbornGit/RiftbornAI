// MaterialGraphToolsModule.h
// Low-level material graph manipulation + high-level expression presets.
// Allows building arbitrary material node graphs through tool calls.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Material Graph Tools Module
 *
 * Low-level graph manipulation:
 * - add_material_expression: Add any expression node type to a material graph
 * - connect_material_nodes: Wire expression outputs to inputs or material pins
 * - set_expression_value: Set constant/parameter values on expression nodes
 * - list_material_nodes: Read-only dump of all nodes in a material graph
 * - disconnect_material_node: Remove a connection
 * - remove_material_expression: Delete a node from the graph
 *
 * High-level presets (in MaterialGraphToolsModule_Presets.cpp):
 * - add_texture_sample: Add TextureSample + TextureCoordinate, auto-wire
 * - add_lerp_blend: Lerp between two nodes with configurable alpha
 * - add_uv_transform: TextureCoordinate with tiling and offset
 * - add_fresnel_effect: Fresnel with exponent, auto-wire to a pin
 * - add_world_position_offset: Simple vertex animation
 */
class RIFTBORNAI_API FMaterialGraphToolsModule : public TToolModuleBase<FMaterialGraphToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("MaterialGraphTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Low-level graph tools
    static FClaudeToolResult Tool_AddMaterialExpression(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConnectMaterialNodes(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetExpressionValue(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMaterialExpressionProperty(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddMaterialFunctionCall(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConnectMaterialAttributes(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EnableLandscapeAutoGrass(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListMaterialNodes(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DisconnectMaterialNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RemoveMaterialExpression(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DeleteMaterialExpression(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ReplaceMaterialExpression(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ReparentMaterialInstance(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_BatchCompileMaterials(const FClaudeToolCall& Call);

    // High-level presets (implemented in MaterialGraphToolsModule_Presets.cpp)
    static FClaudeToolResult Tool_AddTextureSample(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddLerpBlend(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddUVTransform(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddFresnelEffect(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddWorldPositionOffset(const FClaudeToolCall& Call);
};
