// AssetToolsModule.h
// Asset management tools for RiftbornAI
// Tools: delete_asset, rename_asset, duplicate_asset, move_asset, find_assets, get_asset_info

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Riftborn Asset Tools Module
 * 
 * Provides tools for managing Unreal Engine assets:
 * - delete_asset: Remove assets from project
 * - rename_asset: Rename assets with reference updates
 * - duplicate_asset: Clone assets to new locations
 * - move_asset: Relocate assets between folders
 * - find_assets: Search assets by name/type/path
 * - get_asset_info: Get detailed asset metadata
 * 
 * NOTE: Named FRiftbornAssetToolsModule to avoid collision with UE's FAssetToolsModule
 */
class RIFTBORNAI_API FRiftbornAssetToolsModule : public TToolModuleBase<FRiftbornAssetToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("AssetTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_DeleteAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RenameAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DuplicateAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_MoveAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_FindAssets(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetAssetInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AnalyzeAssetDependencies(const FClaudeToolCall& Call);
};
