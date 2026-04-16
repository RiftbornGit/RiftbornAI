// Copyright RiftbornAI. All Rights Reserved.
// Shared utilities for AI providers - Project context, tool filtering, workflow templates

#pragma once

#include "CoreMinimal.h"

/**
 * Utility functions shared across all AI providers (Ollama, Claude, OpenAI, Gemini)
 * These ensure consistent behavior for:
 * - Project context injection (open blueprints, level state)
 * - Tool filtering based on query
 * - Workflow templates for multi-step operations
 */
namespace ProviderUtils
{
    /**
     * Build dynamic project context block for system prompt.
     * Results are cached for 2 seconds to avoid repeated actor iteration + Asset Registry queries.
     * Includes:
     * - Current level name and actor counts
     * - Currently open blueprints in editor
     * - Project blueprint assets
     */
    FString BuildProjectContextBlock();
    
    /**
     * Invalidate the project context cache.
     * Call when the level changes significantly (new actor spawned, blueprint created, etc.)
     */
    void InvalidateProjectContextCache();
    
    /**
     * Build scene context block with spatial awareness for system prompts.
     * Provides detailed scene understanding that all providers can use, including:
     * - Actor inventory with positions and mesh names (up to 25 actors)
     * - Spatial analysis (floor detection, lighting, player start, navmesh)
     * - Scene bounds and size
     * - Editor selection state and viewport camera position
     *
     * Results cached for 2 seconds (same TTL as project context).
     * This is the provider-accessible equivalent of SRiftbornCopilotPanel::GatherSceneContext().
     */
    FString BuildSceneContextBlock();
    
    /**
     * Invalidate the scene context cache.
     * Call when actors are spawned/deleted/moved or selection changes.
     */
    void InvalidateSceneContextCache();
    
    /**
     * Force reload of the UE knowledge reference file (Config/ue_knowledge.md).
     * Call after editing the knowledge file to pick up changes without restarting.
     */
    void InvalidateUEKnowledgeCache();
    
    /**
     * Determine model size tier from model name string.
     * Used to adjust prompt verbosity for small vs large models.
     * @return 0 = small (≤7B), 1 = medium (8B-32B), 2 = large (>32B or cloud)
     */
    int32 GetModelSizeTier(const FString& ModelName);
    
    /**
     * Build enhanced system prompt with project context and workflow templates.
     * @param BasePrompt - The base system prompt to enhance
     * @return Enhanced prompt with context and workflows
     */
    FString BuildEnhancedSystemPrompt(const FString& BasePrompt);
    
    /**
     * Filter tools based on query relevance.
     * Reduces tool count for local models by only providing relevant tools.
     * @param Query - The user's query
     * @return Array of tool names relevant to the query
     */
    TArray<FString> FilterToolsForQuery(const FString& Query);
    
    /**
     * Get the default system prompt for Rift AI.
     * @return Default system prompt
     */
    FString GetDefaultRiftSystemPrompt();

    // ========================================================================
    // RETRY UTILITIES - Exponential Backoff for Transient Failures
    // ========================================================================

    /**
     * Check if an HTTP error code is retryable.
     * Retryable: 429 (rate limit), 500, 502, 503, 504 (server errors)
     * Not retryable: 400, 401, 403, 404 (client errors)
     */
    bool IsRetryableError(int32 HttpCode);

    /**
     * Calculate delay for exponential backoff.
     * @param AttemptNumber - Current attempt (0-indexed)
     * @param BaseDelaySeconds - Initial delay (default 1.0s)
     * @param MaxDelaySeconds - Maximum delay cap (default 30.0s)
     * @return Delay in seconds with jitter
     */
    float CalculateBackoffDelay(int32 AttemptNumber, float BaseDelaySeconds = 1.0f, float MaxDelaySeconds = 30.0f);

    /**
     * Retry configuration for HTTP requests
     */
    struct FRetryConfig
    {
        int32 MaxRetries = 3;
        float BaseDelaySeconds = 1.0f;
        float MaxDelaySeconds = 30.0f;
        bool bRetryOnTimeout = true;

        FRetryConfig() = default;
    };

    /**
     * Get default retry configuration
     */
    FRetryConfig GetDefaultRetryConfig();

    /**
     * Compact a verbose tool result into a concise one-line state delta.
     * Ported from FClaudeProvider::CompactToolResult for cross-provider use.
     * Reduces context window consumption by ~3-5x for tool-heavy sessions.
     * @param ToolName - The tool that produced the result
     * @param RawResult - The full raw result string
     * @return A compact summary like "[OK] spawn_actor: BP_MyActor_C"
     */
    FString CompactToolResult(const FString& ToolName, const FString& RawResult);

    /**
     * Clean raw LLM response text for display.
     * Strips common artifacts: tool format prefixes, markdown code block wrappers,
     * [CLARIFICATION_REQUEST] tags, excessive whitespace, repeated emoji prefixes.
     * All providers should use this before displaying responses.
     * @param RawResponse - The raw text from the LLM
     * @return Cleaned text suitable for display
     */
    FString CleanLLMResponse(const FString& RawResponse);

} // namespace ProviderUtils
