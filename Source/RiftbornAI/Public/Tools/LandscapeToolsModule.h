// Copyright RiftbornAI. All Rights Reserved.
// Landscape Tools Module - Terrain creation and manipulation

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Landscape Tools Module
 *
 * Provides tools for terrain and landscape manipulation:
 * - create_landscape: Create a new landscape/terrain actor
 * - sculpt_landscape: Apply sculpting operations (raise/lower/flatten/smooth)
 * - paint_landscape_layer: Paint material layers on landscape
 * - draw_landscape_path: Rasterize a polyline into a weight-map layer (roads, trails, rivers) with optional height delta
 * - import_heightmap: Import a square raw16 heightmap into an existing landscape
 * - get_landscape_info: Inspect landscape bounds, topology, materials, and registered target layers
 * - add_landscape_layer: Add a new paint layer to landscape material
 * - set_landscape_material: Apply a landscape material
 */
class RIFTBORNAI_API FLandscapeToolsModule : public TToolModuleBase<FLandscapeToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("LandscapeTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateLandscape(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SculptLandscape(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SculptRidge(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SculptDuneCrescent(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SculptNoise(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PaintLandscapeLayer(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DrawLandscapePath(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ImportHeightmap(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLandscapeInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddLandscapeLayer(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLandscapeMaterial(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ScatterCanopyTrees(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ScatterFoliage(const FClaudeToolCall& Call);

private:
    static class ALandscape* FindLandscapeByLabel(const FString& Label);
    static class ALandscapeProxy* FindLandscapeProxyByLabel(const FString& Label);
};
