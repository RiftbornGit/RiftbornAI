// Copyright RiftbornAI. All Rights Reserved.
// Asset Search Tools - MCP wrappers for FProjectAssetIndex

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FAssetSearchToolsModule
 *
 * Thin MCP wrappers around FProjectAssetIndex for semantic
 * asset discovery, single-best resolution, and index rebuild.
 *
 * Tools:
 * - search_project_assets   Find assets by natural language query
 * - resolve_asset           Single best-match path resolution
 * - rebuild_asset_index     Force full re-index of project assets
 */
class RIFTBORNAI_API FAssetSearchToolsModule : public TToolModuleBase<FAssetSearchToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("AssetSearch"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_SearchProjectAssets(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ResolveAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RebuildAssetIndex(const FClaudeToolCall& Call);

    // Renders the editor thumbnail for an asset (cached if available, else
    // synthesized) and writes it to disk as a PNG so the AI can SEE what the
    // asset looks like before placing it.
    static FClaudeToolResult Tool_GetAssetThumbnail(const FClaudeToolCall& Call);

    // Reference-aware asset lookup. Takes a description and (when available)
    // consults the session's cached reference scene graph to enrich the
    // query with biome / style / palette hints, then ranks candidates.
    static FClaudeToolResult Tool_FindAssetMatch(const FClaudeToolCall& Call);

    // Meta tool: semantic search over the plugin's 800+ tool registry. Lets
    // an AI agent ask "what tools help with X?" and get a short ranked list
    // with names + descriptions + params, instead of paying for the full
    // tool catalogue every turn. Wraps FClaudeToolRegistry::SuggestToolsForIntent.
    static FClaudeToolResult Tool_SearchToolsByPurpose(const FClaudeToolCall& Call);
};
