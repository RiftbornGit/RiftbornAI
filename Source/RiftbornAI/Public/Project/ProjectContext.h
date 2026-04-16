// Copyright RiftbornAI. All Rights Reserved.
// Project Context - Automatic context injection for smarter AI responses

#pragma once

#include "CoreMinimal.h"

/**
 * Project context snapshot for AI prompts
 */
struct RIFTBORNAI_API FProjectContextData
{
	// Project info
	FString ProjectName;
	FString EnginVersion;
	TArray<FString> EnabledPlugins;

	// Current level
	FString CurrentLevelName;
	FString CurrentLevelPath;
	int32 ActorCount = 0;
	TArray<FString> ActorSummary;  // Top actor classes with counts

	// Selection
	TArray<FString> SelectedActors;  // Names of selected actors
	FString SelectedActorDetails;    // Details of primary selection

	// Open assets
	TArray<FString> OpenBlueprints;
	FString FocusedBlueprintPath;

	// Recent errors
	TArray<FString> CompilationErrors;
	TArray<FString> BlueprintErrors;

	// Project structure summary
	TArray<FString> SourceModules;
	TArray<FString> ContentFolders;
	int32 BlueprintCount = 0;
	int32 CppClassCount = 0;

	// Convert to compact string for injection
	FString ToContextString() const;
	
	// Get only the most relevant bits (for token efficiency)
	FString ToMinimalContextString() const;
};

/**
 * Project Context Gatherer
 * Collects relevant context from the editor state
 */
class RIFTBORNAI_API FProjectContextGatherer
{
public:
	static FProjectContextGatherer& Get();

	// Gather full context (expensive, ~100ms)
	FProjectContextData GatherFullContext();

	// Gather minimal context (fast, ~10ms)
	FProjectContextData GatherMinimalContext();

	// Specific gatherers
	void GatherLevelContext(FProjectContextData& Context);
	void GatherSelectionContext(FProjectContextData& Context);
	void GatherOpenAssetsContext(FProjectContextData& Context);
	void GatherErrorContext(FProjectContextData& Context);
	void GatherProjectStructure(FProjectContextData& Context);

	// Cache management
	void InvalidateCache();
	bool IsCacheValid() const;
	
	// Get cached or refresh
	const FProjectContextData& GetCachedContext();

	// Settings
	void SetCacheDuration(float Seconds) { CacheDurationSeconds = Seconds; }
	void SetAutoRefresh(bool bEnable) { bAutoRefreshEnabled = bEnable; }

private:
	FProjectContextGatherer();

	FProjectContextData CachedContext;
	FDateTime CacheTimestamp;
	float CacheDurationSeconds = 5.0f;  // Cache for 5 seconds
	bool bAutoRefreshEnabled = true;
};

/**
 * Simplified interface for SCodexUI
 */
class RIFTBORNAI_API FProjectContext
{
public:
	FProjectContext() {}
	
	FString GatherContext() const
	{
		return FProjectContextGatherer::Get().GatherMinimalContext().ToContextString();
	}
	
	FString GetContextString() const
	{
		return FProjectContextGatherer::Get().GetCachedContext().ToContextString();
	}
};

/**
 * Context Injection Strategies
 */
enum class EContextInjectionMode : uint8
{
	None,           // No context injection
	Minimal,        // Just level + selection
	Standard,       // Level + selection + errors
	Full,           // Everything including project structure
	Smart           // Dynamically choose based on query
};

/**
 * Smart context analyzer - determines what context is relevant for a query
 */
class RIFTBORNAI_API FSmartContextAnalyzer
{
public:
	// Analyze query and return relevant context
	static FString GetRelevantContext(const FString& UserQuery, EContextInjectionMode Mode = EContextInjectionMode::Smart);

	// Check if query needs specific context types
	static bool NeedsLevelContext(const FString& Query);
	static bool NeedsSelectionContext(const FString& Query);
	static bool NeedsBlueprintContext(const FString& Query);
	static bool NeedsErrorContext(const FString& Query);
	static bool NeedsProjectStructure(const FString& Query);
};
