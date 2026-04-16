// Copyright RiftbornAI. All Rights Reserved.
// Dynamic model discovery — fetches available models from provider APIs.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

/** A single model discovered from a provider API. */
struct FDiscoveredModel
{
	FString Id;           // e.g. "gpt-5.4", "claude-sonnet-4-6"
	FString DisplayName;  // e.g. "GPT-5 Mini", "Claude Sonnet 4"
	FString Provider;     // "openai", "anthropic", "google", "ollama"
	int32   ContextWindow = 0;
};

/**
 * Fetches and caches available models from AI provider APIs.
 * Thread-safe. Call RefreshAllAsync() to fetch in background;
 * read results with GetCachedModels(). Falls back to hardcoded
 * defaults if network fetch fails.
 */
class RIFTBORNAI_API FModelDiscovery
{
public:
	/** Fetch models from all configured providers on a background thread. */
	static void RefreshAllAsync();

	/** Blocking fetch — call from background threads only. */
	static void RefreshAllSync();

	/** Get cached models for one provider. Empty if not yet fetched. */
	static TArray<FDiscoveredModel> GetCachedModels(const FString& Provider);

	/** Get all cached models across all providers. */
	static TArray<FDiscoveredModel> GetAllCachedModels();

	/** True while a background refresh is running. */
	static bool IsRefreshing();

	/** Seconds since last successful refresh. Returns MAX_FLT if never. */
	static float SecondsSinceLastRefresh();

	/** Wipe the cache (e.g. when API keys change). */
	static void ClearCache();

private:
	// Per-provider fetch (run on any thread — no game-thread dependency)
	static TArray<FDiscoveredModel> FetchOpenAIModels(const FString& ApiKey);
	static TArray<FDiscoveredModel> FetchAnthropicModels(const FString& ApiKey);
	static TArray<FDiscoveredModel> FetchOllamaModels(const FString& Endpoint);
	static TArray<FDiscoveredModel> FetchGeminiModels(const FString& ApiKey);

	/** Synchronous HTTP GET via WinHTTP. Returns (bSuccess, ResponseBody). */
	static TPair<bool, FString> HttpGetSync(
		const FString& Url,
		const TMap<FString, FString>& Headers,
		float TimeoutSec = 10.f);

	static FCriticalSection CacheLock;
	static TMap<FString, TArray<FDiscoveredModel>> CachedModels;
	static double LastRefreshTimestamp; // FPlatformTime::Seconds()
	static TAtomic<bool> bRefreshing;
};
