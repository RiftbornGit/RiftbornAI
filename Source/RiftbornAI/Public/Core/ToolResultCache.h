// Copyright RiftbornAI. All Rights Reserved.
// Tool Result Caching - Cache expensive tool operations

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Cached tool result entry
 */
struct FToolCacheEntry
{
	FString ToolName;
	FString ArgumentsHash;  // Hash of tool arguments
	FClaudeToolResult Result;
	FDateTime CachedAt;
	float TTLSeconds;  // Time-to-live
	
	bool IsExpired() const
	{
		float ElapsedSeconds = (FDateTime::Now() - CachedAt).GetTotalSeconds();
		return ElapsedSeconds > TTLSeconds;
	}
};

/**
 * Tool Result Cache
 * Caches results of expensive tool operations to reduce API latency and token usage
 */
class RIFTBORNAI_API FToolResultCache
{
public:
	static FToolResultCache& Get();

	// Check if a cached result exists and is valid
	bool TryGetCached(const FString& ToolName, const TMap<FString, FString>& Args, FClaudeToolResult& OutResult);

	// Store a result in cache
	void CacheResult(const FString& ToolName, const TMap<FString, FString>& Args, const FClaudeToolResult& Result);

	// Invalidate cache for specific tool or all
	void InvalidateTool(const FString& ToolName);
	void InvalidateAll();

	// Invalidate based on events
	void OnLevelChanged();
	void OnActorSpawned();
	void OnActorDeleted();
	void OnBlueprintCompiled();
	void OnAssetCreated();

	// Cache statistics
	int32 GetCacheHits() const { return CacheHits; }
	int32 GetCacheMisses() const { return CacheMisses; }
	float GetHitRate() const;
	void ResetStats();

	// Settings
	void SetEnabled(bool bEnable) { bCacheEnabled = bEnable; }
	bool IsEnabled() const { return bCacheEnabled; }
	void SetMaxEntries(int32 Max) { MaxCacheEntries = Max; }
	void SetDefaultTTL(float Seconds) { DefaultTTLSeconds = Seconds; }

	// Get cache info
	int32 GetEntryCount() const { return Cache.Num(); }
	SIZE_T GetMemoryUsage() const;

private:
	FToolResultCache();
	
	FString ComputeArgsHash(const TMap<FString, FString>& Args) const;
	FString MakeCacheKey(const FString& ToolName, const FString& ArgsHash) const;
	float GetTTLForTool(const FString& ToolName) const;
	void EvictExpiredEntries();
	void EvictOldestEntries(int32 CountToEvict);

	TMap<FString, FToolCacheEntry> Cache;
	
	// Per-tool TTL settings (some tools change more frequently)
	TMap<FString, float> ToolTTLOverrides;
	
	// Stats
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	
	// Settings
	bool bCacheEnabled = true;
	int32 MaxCacheEntries = 100;
	float DefaultTTLSeconds = 30.0f;
};

/**
 * Wrapper that adds caching to tool execution
 */
class RIFTBORNAI_API FCachingToolExecutor
{
public:
	// Execute tool with caching
	static FClaudeToolResult Execute(const FClaudeToolCall& ToolCall);

	// Check if a tool's results should be cached
	static bool IsCacheableTool(const FString& ToolName);

	// Tools that should NEVER be cached (have side effects)
	static bool HasSideEffects(const FString& ToolName);
};
