// AssetVisualIntelligenceTools.h - Agent tools for visual asset understanding

#pragma once

#include "CoreMinimal.h"

/**
 * Asset Visual Intelligence Tools
 * 
 * Provides agent tools for understanding and querying assets visually:
 * - analyze_asset: Analyze what an asset IS (chair, rock, weapon, etc.)
 * - find_assets_visual: Search by visual description ("find all wooden chairs")
 * - get_asset_description: Get detailed visual description of an asset
 * - find_similar_assets: Find visually similar assets
 * 
 * These tools use Vision LLMs (LLaVA, etc.) to understand asset content
 */
class RIFTBORNAI_API FAssetVisualIntelligenceTools
{
public:
    /** Register all visual intelligence tools with the agent registry */
    static void RegisterTools();
    
private:
    // Tool implementations
    static FString Tool_AnalyzeAsset(const TMap<FString, FString>& Params);
    static FString Tool_FindAssetsVisual(const TMap<FString, FString>& Params);
    static FString Tool_GetAssetDescription(const TMap<FString, FString>& Params);
    static FString Tool_FindSimilarAssets(const TMap<FString, FString>& Params);
    static FString Tool_GetAssetsByCategory(const TMap<FString, FString>& Params);
    static FString Tool_AnalyzeFolder(const TMap<FString, FString>& Params);
    static FString Tool_RecheckOllamaAvailability(const TMap<FString, FString>& Params);
    static FString Tool_ConfigureVisualIntelligence(const TMap<FString, FString>& Params);
};
