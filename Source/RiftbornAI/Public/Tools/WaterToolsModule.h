// Copyright RiftbornAI. All Rights Reserved.
// Water Tools Module - Water body creation and configuration

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

class AWaterBody;

/**
 * Water Tools Module
 *
 * Provides tools for creating and configuring water bodies:
 * - create_water_body: Create a water body (lake, river, ocean, custom)
 * - set_water_level: Adjust water surface elevation
 * - set_water_flow: Configure water flow direction and speed
 * - set_water_material: Apply water shader/material settings
 * - create_water_spline: Create a river path using spline
 * - get_water_info: Query water body properties
 */
class RIFTBORNAI_API FWaterToolsModule : public TToolModuleBase<FWaterToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("WaterTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateWaterBody(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWaterLevel(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWaterFlow(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWaterMaterial(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateWaterSpline(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetWaterInfo(const FClaudeToolCall& Call);

    // Advanced water tools — verified against UE 5.7 Water Plugin API
    static FClaudeToolResult Tool_EnsureWaterZone(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWaterZoneExtent(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ForceWaterRebuild(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWaterBodyAffectsLandscape(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetRiverPerPointWidth(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetRiverPerPointDepth(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddWaterBodyIsland(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddWaterExclusionVolume(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetOceanFloodHeight(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetWaterDiagnostics(const FClaudeToolCall& Call);

private:
    static AWaterBody* FindWaterBodyByLabel(const FString& Label);
};
