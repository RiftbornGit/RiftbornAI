// Copyright RiftbornAI. All Rights Reserved.
// Lighting Tools Module — Create, configure, and query lights
//
// Every scene needs lights. This module provides scripted control over
// common UE5 lighting operations: point, spot, directional, rect lights,
// sky lights, Lumen settings, light channels, and related helpers.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Lighting Tools Module
 *
 * Provides tools for scene lighting:
 * - spawn_light: Spawn a light actor (Point, Spot, Directional, Rect, Sky)
 * - set_light_properties: Configure intensity, color, attenuation, temperature
 * - set_light_mobility: Set static/stationary/movable mobility
 * - get_light_info: Query light properties
 * - list_lights: List all lights in the scene
 * - set_light_channel: Configure light channel membership
 * - create_hdri_backdrop: Spawn and configure an HDRI Backdrop actor
 * - setup_photoreal_forest_lighting: Legacy compatibility name for creating or retuning a managed HDRI + sun + fog + post-process forest lighting stack
 * - set_skylight_cubemap: Assign cubemap to a sky light
 * - set_lumen_settings: Configure Lumen GI settings on a post-process volume
 * - batch_set_light_property: Set a property on multiple lights at once
 */
class RIFTBORNAI_API FLightingToolsModule : public TToolModuleBase<FLightingToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("LightingTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_SpawnLight(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLightMobility(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLightInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListLights(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLightChannel(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateHDRIBackdrop(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetupPhotorealForestLighting(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSkylightCubemap(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLumenSettings(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_BatchSetLightProperty(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetupOutdoorLighting(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyForestAtmospherePreset(const FClaudeToolCall& Call);

};
