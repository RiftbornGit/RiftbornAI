// Copyright RiftbornAI. All Rights Reserved.
// Volumetric Effects Tools Module — Fog, clouds, sky atmosphere, height fog

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Volumetric Tools Module
 *
 * Provides tools for atmospheric/volumetric effects:
 * - create_exponential_height_fog: Spawn and configure EHF
 * - set_fog_properties: Density, color, falloff, inscattering
 * - create_volumetric_cloud: Spawn volumetric cloud component
 * - set_cloud_properties: Layer bottom/height, density, shape noise
 * - create_sky_atmosphere: Spawn sky atmosphere component
 * - set_atmosphere_properties: Rayleigh/Mie scattering, absorption
 * - create_sky_light: Spawn sky light with real-time capture
 * - get_volumetric_actor_info: Inspect cloud, local fog, and sky-atmosphere component state on an actor
 * - set_fog_volume: Create or update a native LocalFogVolume actor for localized fog pockets
 */
class RIFTBORNAI_API FVolumetricToolsModule : public TToolModuleBase<FVolumetricToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("VolumetricTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateVolumetricCloud(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetCloudProperties(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetVolumetricActorInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetFogVolume(const FClaudeToolCall& Call);

};
