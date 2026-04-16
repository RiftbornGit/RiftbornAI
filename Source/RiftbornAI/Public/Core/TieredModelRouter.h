// Copyright RiftbornAI. All Rights Reserved.
// TieredModelRouter - Routes queries to appropriate models based on complexity
// Gap #2: Latency optimization — fast model for simple queries, strong model for complex ones

#pragma once

#include "CoreMinimal.h"
#include "IAIProvider.h"

/**
 * Query complexity tier — determines which model handles the request.
 * The key insight: 80% of copilot queries are simple (move actor, get info, set property)
 * and can be handled by a fast model in <2s. Only 20% need the big guns.
 */
enum class EQueryTier : uint8
{
	/** Fast tier: simple queries, single-tool, known patterns. Target: <2s TTFT */
	Fast,
	
	/** Standard tier: multi-step plans, some ambiguity. Target: <5s TTFT */
	Standard,
	
	/** Heavy tier: complex reasoning, code generation, multi-file. Target: <15s TTFT */
	Heavy
};

/**
 * Classification result for a user query
 */
struct FQueryClassification
{
	EQueryTier Tier = EQueryTier::Standard;
	float Confidence = 0.0f;
	FString Reason;
	
	/** Matched keyword pattern (if Fast tier) */
	FString MatchedPattern;
};

/**
 * Model assignment for a tier
 */
struct FTierModelConfig
{
	/** Provider to use (Ollama, Claude, OpenAI, Gemini) */
	FString ProviderName;
	
	/** Model name within the provider */
	FString ModelName;
	
	/** Temperature override (-1 = use default) */
	float Temperature = -1.0f;
	
	/** Max tokens override (-1 = use default) */
	int32 MaxTokens = -1;
	
	/** Expected TTFT in seconds (for monitoring) */
	float ExpectedTTFTSeconds = 5.0f;
};

/**
 * Tiered Model Router
 * 
 * Routes user queries to the most appropriate model based on complexity classification.
 * This is the #1 latency optimization — most queries don't need a $0.05 Opus call.
 * 
 * Classification is done locally (zero-cost, <1ms) using keyword patterns and heuristics.
 * No LLM call needed for routing — that would defeat the purpose.
 * 
 * Tier assignment:
 *   Fast:     Single-tool queries, property gets/sets, navigation, info requests
 *   Standard: Multi-step but predictable tasks, common patterns
 *   Heavy:    Code generation, complex reasoning, ambiguous intent, multi-file edits
 */
class RIFTBORNAI_API FTieredModelRouter
{
public:
	static FTieredModelRouter& Get();
	
	/**
	 * Classify a user query into a complexity tier.
	 * This is a LOCAL classification — no LLM call, no cost, <1ms.
	 * 
	 * @param UserQuery The raw user message
	 * @param ToolCount Number of tools the LLM would see (more tools = heavier)
	 * @param ConversationTurnCount Number of turns in current conversation
	 * @return Classification with tier, confidence, and reason
	 */
	FQueryClassification ClassifyQuery(
		const FString& UserQuery,
		int32 ToolCount = 0,
		int32 ConversationTurnCount = 0) const;
	
	/**
	 * Get the model configuration for a tier.
	 * Falls back to Standard config if tier-specific isn't set.
	 */
	FTierModelConfig GetModelForTier(EQueryTier Tier) const;
	
	/**
	 * Set custom model config for a tier.
	 */
	void SetModelForTier(EQueryTier Tier, const FTierModelConfig& Config);
	
	/**
	 * Auto-configure tiers based on available providers.
	 * Detects what's available (Ollama models, API keys) and assigns optimally.
	 */
	void AutoConfigure();
	
	/**
	 * Get a human-readable name for the tier
	 */
	static FString TierToString(EQueryTier Tier);
	
	/**
	 * Record actual latency for a tier (for monitoring and auto-tuning)
	 */
	void RecordLatency(EQueryTier Tier, float ActualTTFTSeconds, bool bSuccess);
	
	/**
	 * Get average latency for a tier
	 */
	float GetAverageLatency(EQueryTier Tier) const;
	
	/**
	 * Is the router configured and ready?
	 */
	bool IsConfigured() const { return bIsConfigured; }
	
private:
	FTieredModelRouter();
	
	/** Initialize default keyword patterns */
	void InitializePatterns();
	
	/** Check if query matches Fast-tier patterns */
	bool MatchesFastPattern(const FString& Query, FString& OutMatchedPattern) const;
	
	/** Check if query requires Heavy-tier processing */
	bool RequiresHeavyProcessing(const FString& Query, int32 ToolCount) const;
	
	/** Model configs per tier */
	TMap<EQueryTier, FTierModelConfig> TierConfigs;
	
	/** Fast-tier keyword patterns (lowercase) */
	TArray<FString> FastPatterns;
	
	/** Heavy-tier keyword patterns (lowercase) */
	TArray<FString> HeavyPatterns;
	
	/** Latency tracking per tier */
	struct FLatencyStats
	{
		TArray<float> RecentLatencies;  // Last 50
		float TotalLatency = 0.0f;
		int32 Count = 0;
		int32 Failures = 0;
		
		void Record(float Latency, bool bSuccess)
		{
			if (RecentLatencies.Num() >= 50)
			{
				TotalLatency -= RecentLatencies[0];
				RecentLatencies.RemoveAt(0);
			}
			RecentLatencies.Add(Latency);
			TotalLatency += Latency;
			Count++;
			if (!bSuccess) Failures++;
		}
		
		float GetAverage() const
		{
			return Count > 0 ? TotalLatency / RecentLatencies.Num() : 0.0f;
		}
	};
	TMap<EQueryTier, FLatencyStats> LatencyStats;
	
	bool bIsConfigured = false;
	
	mutable FCriticalSection RouterLock;
};
