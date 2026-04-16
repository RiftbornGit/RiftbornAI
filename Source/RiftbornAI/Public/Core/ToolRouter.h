// Copyright RiftbornAI. All Rights Reserved.
// Tool Router - "Prefrontal Cortex" for intelligent tool selection

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "ClaudeToolUse.h"  // For EAgentProfile

/**
 * Tool categories for semantic grouping - uint64 for 33+ categories
 */
enum class EToolCategory : uint64
{
	None = 0,
	
	// Core editing (original)
	LevelEditing    = 1ULL << 0,   // spawn, move, delete, get_actors
	Blueprints      = 1ULL << 1,   // create_bp, compile, add_node, connect
	FileSystem      = 1ULL << 2,   // read, write, list_dir, search
	Python          = 1ULL << 3,   // execute_python
	Materials       = 1ULL << 4,   // create_material, apply, set_parameter
	Assets          = 1ULL << 5,   // import, list_assets, create_asset
	PlayControl     = 1ULL << 6,   // play, stop, save_level
	CodeGen         = 1ULL << 7,   // generate_cpp, create_source
	
	// Visual/Effects
	Lighting        = 1ULL << 8,   // lights, post-process, exposure
	Audio           = 1ULL << 9,   // sound cues, audio components
	Niagara         = 1ULL << 10,  // particle systems, emitters
	Rendering       = 1ULL << 11,  // render targets, materials, post-process
	
	// Gameplay Systems
	Animation       = 1ULL << 12,  // montages, blendspaces, anim BPs
	AI              = 1ULL << 13,  // behavior trees, blackboard, EQS, perception
	Navigation      = 1ULL << 14,  // nav mesh, nav volumes
	GameplayTags    = 1ULL << 15,  // tag management
	Collision       = 1ULL << 16,  // collision presets, responses
	Replication     = 1ULL << 17,  // networking, RPCs
	
	// UI
	UI              = 1ULL << 18,  // widgets, UMG
	
	// Sequences/Cinematics
	Sequencer       = 1ULL << 19,  // level sequences, tracks
	
	// World Building
	LevelStreaming  = 1ULL << 20,  // streaming levels, world partition
	Landscape       = 1ULL << 21,  // landscape, foliage
	
	// Data Management
	DataAssets      = 1ULL << 22,  // data assets, data tables
	
	// Editor Tools
	EditorUtility   = 1ULL << 23,  // blutilities, editor widgets
	Testing         = 1ULL << 24,  // automation tests
	SourceControl   = 1ULL << 25,  // git, perforce
	
	// Project Management
	Project         = 1ULL << 26,  // project settings, plugins, config
	
	// BATCH 9: Advanced Categories
	AIAgent         = 1ULL << 27,  // autonomous agent tasks, workflows
	HotReload       = 1ULL << 28,  // live coding, blueprint recompilation
	Debugging       = 1ULL << 29,  // PIE, breakpoints, object inspection
	Cooking         = 1ULL << 30,  // asset cooking, packaging
	Localization    = 1ULL << 31,  // string tables, cultures, i18n
	Online          = 1ULL << 32,  // sessions, matchmaking, online subsystem
	
	All = 0xFFFFFFFFFFFFFFFFULL
};
ENUM_CLASS_FLAGS(EToolCategory);

/**
 * Result of tool routing decision
 */
struct FToolRoutingResult
{
	/** Which tool categories should be loaded */
	EToolCategory Categories = EToolCategory::None;
	
	/** Specific tool names to include (optional, empty = all from categories) */
	TArray<FString> SpecificTools;
	
	/** Reasoning from the router (for debugging/display) */
	FString Reasoning;
	
	/** Whether routing succeeded */
	bool bSuccess = false;
	
	/** How many tokens the routing call used */
	int32 RoutingTokens = 0;
};

/**
 * FToolRouter - Lightweight "prefrontal cortex" that decides which tools are needed
 * 
 * Uses Claude Haiku (~$0.25/1M tokens) to analyze user queries and select
 * only the relevant tool categories before the main Opus call.
 * 
 * Cost savings: 51 tools × ~200 tokens = ~10K tokens saved per request
 * Routing cost: ~200 tokens input + ~50 tokens output = ~$0.0001 per route
 */
class RIFTBORNAI_API FToolRouter
{
public:
	FToolRouter();
	
	/**
	 * Route a user query to determine which tools are needed
	 * @param UserQuery - The user's message/request
	 * @param OnComplete - Callback with routing result
	 */
	void RouteQuery(
		const FString& UserQuery,
		TFunction<void(const FToolRoutingResult& Result)> OnComplete
	);
	
	/**
	 * Quick local routing using keyword matching (no API call)
	 * Less accurate but free and instant
	 */
	FToolRoutingResult RouteQueryLocal(const FString& UserQuery);
	
	/**
	 * Get tool names for a category
	 */
	static TArray<FString> GetToolsForCategory(EToolCategory Category);
	
	/**
	 * Get all tools for combined categories
	 */
	static TArray<FString> GetToolsForCategories(EToolCategory Categories);
	
	/**
	 * Get tools filtered by categories AND agent profile (profile-aware version)
	 * This is the preferred method when you have a router instance
	 */
	TArray<FString> GetToolsForCategoriesWithProfile(EToolCategory Categories) const;
	
	/** Set API key (should be same key as main client) */
	void SetAPIKey(const FString& InAPIKey) { APIKey = InAPIKey; }
	
	/** Set the active agent profile for tool filtering */
	void SetAgentProfile(EAgentProfile InProfile) { ActiveProfile = InProfile; }
	EAgentProfile GetAgentProfile() const { return ActiveProfile; }
	
	/** Enable/disable Haiku routing (if disabled, uses local keyword matching) */
	void SetHaikuRoutingEnabled(bool bEnabled) { bUseHaikuRouting = bEnabled; }
	bool IsHaikuRoutingEnabled() const { return bUseHaikuRouting; }
	
	/** Get total routing tokens used this session */
	int32 GetSessionRoutingTokens() const { return SessionRoutingTokens.GetValue(); }

private:
	void OnRoutingResponseReceived(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bWasSuccessful,
		TFunction<void(const FToolRoutingResult&)> OnComplete
	);
	
	FToolRoutingResult ParseRoutingResponse(const FString& JsonResponse);
	EToolCategory ParseCategoryFromString(const FString& CategoryStr);
	
	FString APIKey;
	FString HaikuEndpoint;
	FString HaikuModel;
	
	bool bUseHaikuRouting = true;
	FThreadSafeCounter SessionRoutingTokens;  // Atomic: written from HTTP callback threads, read from game thread
	EAgentProfile ActiveProfile = EAgentProfile::CodingAgent;  // Default to CodingAgent
	
	FHttpModule* HttpModule;
	
	// Category -> Tool name mappings
	static TMap<EToolCategory, TArray<FString>> CategoryToolMap;
	static void InitializeCategoryMap();
	
	// Keyword patterns for local routing
	static TMap<FString, EToolCategory> KeywordPatterns;
	static void InitializeKeywordPatterns();
};
