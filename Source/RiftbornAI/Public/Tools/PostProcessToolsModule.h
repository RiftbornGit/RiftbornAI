// Copyright RiftbornAI. All Rights Reserved.
// Post-Processing Tools Module — PP volumes, bloom, DoF, exposure, color grading

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Post-Processing Tools Module
 *
 * Provides tools for visual post-processing:
 * - create_post_process_volume: Create a PP volume (bounded or infinite)
 * - set_bloom: Configure bloom intensity, threshold, size
 * - set_depth_of_field: Configure DoF (bokeh, Gaussian, cinematic)
 * - set_exposure: Configure auto-exposure, fixed, histogram settings
 * - set_color_grading: Color correction, white balance, saturation, contrast, LUT
 * - set_ambient_occlusion: Screen-space AO settings
 * - set_motion_blur: Motion blur amount and max velocity
 * - get_post_process_info: Query text or JSON curated PP settings on a volume
 * - set_chromatic_aberration: Chromatic aberration intensity
 * - set_lens_flare: Lens flare intensity, threshold, and bokeh size
 * - set_vignette: Vignette intensity
 * - set_local_exposure: Local exposure highlight/shadow balancing controls
 */
class RIFTBORNAI_API FPostProcessToolsModule : public TToolModuleBase<FPostProcessToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("PostProcessTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreatePostProcessVolume(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMotionBlur(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetPostProcessInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetChromaticAberration(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLensFlare(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetLocalExposure(const FClaudeToolCall& Call);

private:
    static APostProcessVolume* GetOrCreatePPV(const FString& Label);
};
