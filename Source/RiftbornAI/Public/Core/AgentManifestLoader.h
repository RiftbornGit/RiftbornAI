// Copyright RiftbornAI. All Rights Reserved.
// AgentManifestLoader.h - Loads authored specialist-agent manifests for subagents.

#pragma once

#include "CoreMinimal.h"
#include "Core/ClaudeToolUse_Types.h"

struct RIFTBORNAI_API FAgentManifestEntry
{
	FString SourceLayer;
	FString Filename;
	FString Name;
	FString Alias;
	FString Summary;
	FString Body;
	EAgentProfile Profile = EAgentProfile::EditorAssistant;
	TArray<FString> WhenPhrases;
	TArray<FString> DefaultSkillPacks;
	TArray<FString> ToolInclude;
	TArray<FString> ToolExclude;
	FString PreferredProvider;
	int32 MaxIterations = 0;
	float TimeoutSeconds = 0.0f;
	bool bIncludeSceneContext = true;
	bool bReadOnlyBias = false;
	bool bVerificationBias = false;
	FString CompletionStyle;
	int32 Priority = 0;

	FString GetStableKey() const;
	bool IsValid() const;
};

struct RIFTBORNAI_API FResolvedAgentManifest
{
	FString DisplayName;
	FString Alias;
	FString Summary;
	EAgentProfile Profile = EAgentProfile::EditorAssistant;
	FString SystemPromptAddendum;
	FString PreferredProvider;
	int32 MaxIterations = 0;
	float TimeoutSeconds = 0.0f;
	bool bIncludeSceneContext = true;
	bool bReadOnlyBias = false;
	bool bVerificationBias = false;
	FString CompletionStyle;
	TArray<FString> ToolInclude;
	TArray<FString> ToolExclude;
	TArray<FString> DefaultSkillPacks;
};

/**
 * Loads authored agent manifests from layered markdown/frontmatter directories:
 * - bundled plugin manifests under Config/Agents
 * - project manifests under Config/RiftbornAI/Agents
 *
 * Agent manifests turn specialist aliases into a first-class product layer:
 * base profile, prompt addendum, default skill packs, and provider bias.
 */
class RIFTBORNAI_API FAgentManifestLoader
{
public:
	static FAgentManifestLoader& Get();

	/** Resolve an authored agent alias or display name into concrete subagent options. */
	bool TryResolveAgentAlias(const FString& AliasOrName, FResolvedAgentManifest& OutResolved);

	/** Return the loaded authored-agent catalog for UI / MCP discovery. */
	TArray<FAgentManifestEntry> GetLoadedAgentManifests();

	/** Build a compact one-line-per-agent summary for tool descriptions or prompts. */
	FString BuildCatalogSummary(int32 MaxAgents = 8, int32 MaxChars = 800);

	/** Build a composer-ready starter prompt that routes work through a named authored agent. */
	bool TryBuildStarterPrompt(const FString& AliasOrName, FString& OutPrompt);

	/** Suggest the best authored agent for the given task context. */
	bool TrySuggestAgentForTask(const FString& TaskContext, FResolvedAgentManifest& OutResolved);

	/** Invalidate disk cache so manifests reload on next access. */
	void InvalidateCache();

	/** Resolved bundled manifest directory inside the plugin. */
	FString GetResolvedBundledAgentsDirectory() const;

	/** Resolved per-project manifest directory. */
	FString GetResolvedProjectAgentsDirectory() const;

	/** Resolved user/studio manifest directory. */
	FString GetResolvedUserAgentsDirectory() const;

	/** Number of cached manifests. */
	int32 GetLoadedAgentManifestCount();

private:
	FAgentManifestLoader() = default;

	void EnsureLoaded();
	void LoadFromDisk();

	FCriticalSection CacheLock;
	bool bCacheValid = false;
	TArray<FAgentManifestEntry> CachedAgentManifests;
};
