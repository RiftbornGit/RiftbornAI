// Copyright RiftbornAI. All Rights Reserved.
// ProjectRulesLoader.h - Loads per-project .md rule files for AI context injection
//
// Customers write .md files in Config/RiftbornAI/Rules/ to customize AI behavior.
// Rules are injected into the system prompt as project-specific constraints.
// The AI treats them as guidance (overridable by the user), not hardcoded law.

#pragma once

#include "CoreMinimal.h"

struct FProjectRule
{
	FString Filename;
	FString Content;
	int32 CharCount = 0;
};

/**
 * Singleton loader for per-project rule files.
 * Thread-safe, caches on first load, supports hot-reload via console command.
 */
class RIFTBORNAI_API FProjectRulesLoader
{
public:
	static FProjectRulesLoader& Get();

	/** Load (or return cached) rules and build the prompt section.
	 *  Returns empty string if disabled or no rules found. */
	FString BuildProjectRulesSection();

	/** Invalidate cache — rules will reload on next BuildProjectRulesSection call. */
	void InvalidateCache();

	/** Get the resolved absolute path to the rules directory. */
	FString GetResolvedRulesDirectory() const;

	/** Get count of currently loaded rules. */
	int32 GetLoadedRuleCount() const;

private:
	FProjectRulesLoader() = default;

	void LoadRulesFromDisk();

	FCriticalSection CacheLock;
	TArray<FProjectRule> CachedRules;
	bool bCacheValid = false;
	FString CachedPromptSection;
};
