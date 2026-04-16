// Copyright RiftbornAI. All Rights Reserved.
// Project Asset Index - Semantic asset discovery for natural language understanding

#pragma once

#include "CoreMinimal.h"

/**
 * Asset entry with semantic metadata for NLU
 */
struct RIFTBORNAI_API FIndexedAsset
{
	FString Path;           // Full UE path: /Game/Characters/BP_Warrior
	FString Name;           // Display name: BP_Warrior
	FString AssetType;      // Blueprint, Material, Texture, etc.
	FString ParentClass;    // For Blueprints: Character, GameModeBase, etc.
	TArray<FString> Tags;   // Semantic tags: warrior, melee, character, champion
	TArray<FString> Aliases;// Alternative names: tank, fighter, melee_character
	
	bool MatchesQuery(const FString& Query) const;
};

/**
 * Categorized asset collection
 */
struct RIFTBORNAI_API FAssetCategory
{
	FString Name;           // GameModes, Characters, Weapons, UI, etc.
	TArray<FIndexedAsset> Assets;
	TArray<FString> Keywords; // Trigger words for this category
};

/**
 * Project Asset Index
 * Provides semantic asset discovery for AI natural language understanding.
 * 
 * Example usage:
 *   User: "Set game mode to use the warrior character"
 *   AI queries: ResolveAsset("warrior character", "Character")
 *   Returns: /Game/Champions/BP_Champion_Warrior
 */
class RIFTBORNAI_API FProjectAssetIndex
{
public:
	static FProjectAssetIndex& Get();
	
	// =========================================================================
	// INITIALIZATION
	// =========================================================================
	
	/** Scan project and build index (call once on startup) */
	void BuildIndex();
	
	/** Refresh a specific asset category */
	void RefreshCategory(const FString& Category);
	
	/** Clear and rebuild entire index */
	void Rebuild();
	
	/** Check if index is built */
	bool IsIndexed() const { return bIsIndexed; }
	
	// =========================================================================
	// SEMANTIC RESOLUTION
	// =========================================================================
	
	/** 
	 * Resolve natural language to asset path
	 * @param Query - Natural language like "warrior", "priest character", "arena gamemode"
	 * @param TypeHint - Optional type hint: "Blueprint", "Material", "GameMode", "Character"
	 * @return Best matching asset path or empty string
	 */
	FString ResolveAsset(const FString& Query, const FString& TypeHint = TEXT("")) const;
	
	/**
	 * Find all matching assets for a query
	 * @param Query - Search query
	 * @param TypeHint - Optional type filter
	 * @param MaxResults - Maximum results to return
	 * @return Array of matching assets sorted by relevance
	 */
	TArray<FIndexedAsset> FindAssets(const FString& Query, const FString& TypeHint = TEXT(""), int32 MaxResults = 10) const;
	
	/**
	 * Get all assets of a specific type
	 * @param AssetType - Blueprint, GameMode, Character, Pawn, etc.
	 */
	TArray<FIndexedAsset> GetAssetsByType(const FString& AssetType) const;
	
	/**
	 * Check if an asset exists by name/path
	 */
	bool AssetExists(const FString& NameOrPath) const;
	
	// =========================================================================
	// CONTEXT FOR AI
	// =========================================================================
	
	/**
	 * Get asset context for AI system prompt
	 * Provides a summary of available assets for the AI to understand
	 */
	FString GetAssetContextForAI() const;
	
	/**
	 * Get detailed context for a specific query
	 * Returns relevant assets based on query keywords
	 */
	FString GetRelevantAssetContext(const FString& UserQuery) const;
	
	/**
	 * Get a compact list of available assets by category
	 * Optimized for token efficiency
	 */
	FString GetCompactAssetList() const;
	
	// =========================================================================
	// SEMANTIC ALIASES
	// =========================================================================
	
	/**
	 * Add custom alias for an asset
	 * @param AssetPath - The actual asset path
	 * @param Alias - The alias to add (e.g., "tank" for warrior)
	 */
	void AddAlias(const FString& AssetPath, const FString& Alias);
	
	/**
	 * Add semantic tags to an asset
	 * @param AssetPath - The actual asset path
	 * @param Tags - Tags to add (e.g., "melee", "damage", "cc")
	 */
	void AddTags(const FString& AssetPath, const TArray<FString>& Tags);
	
	/**
	 * Load custom aliases from config file
	 */
	void LoadAliasesFromConfig();
	
	/**
	 * Save current aliases to config file
	 */
	void SaveAliasesToConfig();
	
	// =========================================================================
	// DIRECT ACCESS
	// =========================================================================
	
	const TArray<FAssetCategory>& GetCategories() const { return Categories; }
	const TMap<FString, FIndexedAsset>& GetAllAssets() const { return AssetMap; }
	
	// Statistics
	int32 GetTotalAssetCount() const { return AssetMap.Num(); }
	int32 GetBlueprintCount() const;
	int32 GetGameModeCount() const;
	int32 GetCharacterCount() const;

private:
	FProjectAssetIndex();
	
	void RegisterEnginePrimitives();
	void IndexBlueprints();
	void IndexMaterials();
	void IndexTextures();
	void IndexMeshes();
	void IndexDataAssets();
	void IndexWidgets();
	void IndexInputActions();
	
	void AddAssetToIndex(const FIndexedAsset& Asset);
	void InferSemanticTags(FIndexedAsset& Asset);
	FString ExtractParentClass(const struct FAssetData& AssetData) const;
	
	int32 CalculateMatchScore(const FIndexedAsset& Asset, const FString& Query, const FString& TypeHint) const;
	
	TArray<FAssetCategory> Categories;
	TMap<FString, FIndexedAsset> AssetMap;  // Path -> Asset
	TMap<FString, FString> AliasMap;        // Alias -> Path
	
	bool bIsIndexed = false;
	FDateTime LastIndexTime;
};
