// Copyright RiftbornAI. All Rights Reserved.
// Movie Render Queue Tools Module — High-quality cinematic rendering

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Movie Render Queue Tools Module
 *
 * Provides tools for cinematic rendering:
 * - create_movie_pipeline: Create a UMoviePipelinePrimaryConfig for rendering
 * - set_render_output: Configure output path, file naming, resolution, and frame rate
 * - add_render_pass: Add supported render passes (currently beauty/deferred)
 * - set_anti_aliasing: Configure spatial/temporal AA samples and optional AA override
 * - render_sequence: Queue a level sequence for rendering
 * - get_render_status: Check current render progress
 * - set_console_variables_for_render: Add numeric console variable overrides to the MRQ config
 * - create_render_preset: Save/load render configuration presets
 */
class RIFTBORNAI_API FMovieRenderQueueToolsModule : public TToolModuleBase<FMovieRenderQueueToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("MovieRenderQueueTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateMoviePipeline(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetRenderOutput(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddRenderPass(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetAntiAliasing(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RenderSequence(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetRenderStatus(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetMoviePipelineInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetConsoleVarsForRender(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateRenderPreset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateMovieGraphConfig(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_QueueTrailerRenderJob(const FClaudeToolCall& Call);
};
