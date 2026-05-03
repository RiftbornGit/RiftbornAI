// Copyright RiftbornAI. All Rights Reserved.
// Shared utilities for AI providers - Project context, tool filtering, workflow templates

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Utility functions shared across all AI providers (Ollama, Claude, OpenAI, Gemini)
 * These ensure consistent behavior for:
 * - Project context injection (open blueprints, level state)
 * - Tool filtering based on query
 * - Workflow templates for multi-step operations
 */
namespace ProviderUtils
{
    enum class ECopilotPromptProfile : uint8
    {
        Default,
        Chat,
        Plan,
        Act,
        Code,
        Bridge
    };

    struct FPromptBuildOptions
    {
        FString TaskContext;
        bool bIncludeUEKnowledge = true;
        bool bIncludeProjectRules = true;
        bool bIncludeSkillPacks = true;
        bool bIncludeProjectMemory = true;
        bool bIncludeRelevantHistory = true;
        bool bIncludeSceneContext = true;
        bool bIncludeRecipes = true;
        bool bIncludeCompletionInstruction = true;
    };

    struct FToolCatalogOptions
    {
        FString UserMessage;
        EAgentProfile AgentProfile = EAgentProfile::Unrestricted;
        int32 SelectionBudget = 0;
        int32 HardCap = 0;
        bool bLevelLoaded = false;
        bool bInPIE = false;
        int32 SelectedActorCount = 0;
        bool bReadOnlyOnly = false;
        bool bPreferCodeAuthoring = false;
        TArray<FString> AlwaysInclude;
    };

    struct FCopilotPromptProfileOptions
    {
        FString TaskContext;
        FString ToolCatalogSummary;
        FString AdditionalInstructions;
        bool bPlanFirst = false;
        bool bReadOnlyOnly = false;
    };

    struct FTaskDecisionProfile
    {
        bool bNeedsSceneInspectionBeforeMutation = false;
        bool bNeedsVisualObservationBeforeMutation = false;
        bool bNeedsVisualVerificationAfterMutation = false;
        bool bRequiresRuntimeVerification = false;
        bool bAvoidRuntimeVerificationUnlessExplicit = true;
        bool bPreferRecentActorContext = false;
        bool bLikelySingleTargetMutation = false;
        bool bPreferStopAfterPrimaryGoal = false;
        bool bAvoidSaveUnlessRequested = true;
        bool bFreshSceneScaffold = false;
        bool bExplicitSaveRequested = false;
        FString ReasonSummary;
    };

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
     * Build a categorized asset inventory block for system prompts. Scans
     * /Game/Megascans, /Game/Megaplants, /Game/Materials and the project
     * content root, groups assets by semantic category (trees, rocks, grass,
     * flowers, water, materials, foliage-generic, blueprints), and returns
     * a compact text block with up to N real object paths per category so
     * the LLM can reference concrete assets instead of guessing paths.
     * Results cached for 30s (asset registry is relatively stable).
     */
    FString BuildAssetInventoryBlock();

    /** Invalidate the asset inventory cache (e.g., after importing assets). */
    void InvalidateAssetInventoryCache();

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
     * @param Options - Additional context and section toggles
     * @return Enhanced prompt with context and workflows
     */
    FString BuildEnhancedSystemPrompt(const FString& BasePrompt, const FPromptBuildOptions& Options);

    /**
     * Build enhanced system prompt with default options.
     * @param BasePrompt - The base system prompt to enhance
     * @return Enhanced prompt with context and workflows
     */
    FString BuildEnhancedSystemPrompt(const FString& BasePrompt);

    /**
     * Build a deterministic tool catalog from the canonical selector path.
     * Uses the configured ToolSelector first, then falls back to the default selector,
     * then finally to the full registry if selection is unavailable.
     */
    TArray<FClaudeTool> BuildToolCatalog(const FToolCatalogOptions& Options);

    /** Build a deterministic list of tool names from the canonical selector path. */
    TArray<FString> BuildToolCatalogNames(const FToolCatalogOptions& Options);

    /** Render the selected tool catalog into prompt-friendly bullet text. */
    FString BuildToolCatalogSummary(const TArray<FClaudeTool>& Tools, int32 MaxTools = 64);

    /** Build a profile-specific copilot system prompt from the shared base identity. */
    FString BuildCopilotSystemPrompt(ECopilotPromptProfile Profile, const FCopilotPromptProfileOptions& Options = FCopilotPromptProfileOptions());

    /** Detect requests that should bias toward the Unreal C++ authoring lane. */
    bool IsCodeAuthoringTask(const FString& UserMessage);
    /** Detect requests that require grounded online research. */
    bool IsResearchHeavyTask(const FString& UserMessage);
    /** Detect traversal or climbing mechanic requests that need runtime proof. */
    bool IsTraversalMechanicTask(const FString& UserMessage);
    /** Detect broad game-building requests that should be milestone-driven. */
    bool IsBroadGameCreationTask(const FString& UserMessage);
    /** Detect requests that need honest animation-source discovery. */
    bool IsAnimationSourcingTask(const FString& UserMessage);
    /** Build a shared decision profile for observation, runtime proof, persistence, and follow-up targeting. */
    FTaskDecisionProfile AnalyzeTaskDecisionProfile(const FString& UserMessage);
    /** Detect requests that explicitly require PIE, playtesting, or runtime proof. */
    bool IsRuntimeVerificationTask(const FString& UserMessage);

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
