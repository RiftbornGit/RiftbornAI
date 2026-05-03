// Copyright RiftbornAI. All Rights Reserved.
// SkillPackLoader.h - Loads authorable markdown skill/workflow packs for prompt injection.

#pragma once

#include "CoreMinimal.h"
#include "Core/ClaudeToolUse_Types.h"

struct RIFTBORNAI_API FSkillPackEntry
{
	FString SourceLayer;
	FString Filename;
	FString Name;
	FString Kind = TEXT("skill");
	FString Summary;
	FString Body;
	FString Specialist;
	EAgentProfile Profile = EAgentProfile::EditorAssistant;
	TArray<FString> WhenPhrases;
	int32 Priority = 0;

	FString GetStableKey() const;
	bool IsValid() const;
};

/**
 * Loads authorable markdown prompt packs from layered directories:
 * - bundled plugin packs under Config/SkillPacks
 * - project packs under Config/RiftbornAI/Skills (configurable)
 *
 * Packs use lightweight frontmatter and are injected into prompts only when
 * they match the current task. This is intentionally additive to project
 * rules and learned skills, not a replacement for either system.
 */
class RIFTBORNAI_API FSkillPackLoader
{
public:
	static FSkillPackLoader& Get();

	/** Build a compact prompt section containing the best-matching skill packs. */
	FString BuildRelevantPromptSection(const FString& TaskContext);

	/** Build a prompt section for explicit skill-pack keys or specialist aliases. */
	FString BuildPromptSectionForPackKeys(
		const TArray<FString>& PackKeys,
		const FString& SectionTitle = TEXT("Relevant Skill Packs"));

	/** Resolve a named specialist alias to a base profile and system-prompt addendum. */
	bool TryResolveSpecialistAlias(
		const FString& Alias,
		EAgentProfile& OutBaseProfile,
		FString& OutAddendum,
		FString* OutDisplayName = nullptr);

	/** Return the loaded catalog for UI / MCP discovery. */
	TArray<FSkillPackEntry> GetLoadedSkillPacks();

	/** Invalidate disk cache so packs reload on next access. */
	void InvalidateCache();

	/** Resolved bundled pack directory inside the plugin. */
	FString GetResolvedBundledSkillPacksDirectory() const;

	/** Resolved per-project pack directory. */
	FString GetResolvedProjectSkillPacksDirectory() const;

	/** Resolved user/studio pack directory. */
	FString GetResolvedUserSkillPacksDirectory() const;

	/** Number of cached packs. */
	int32 GetLoadedSkillPackCount();

private:
	FSkillPackLoader() = default;

	void EnsureLoaded();
	void LoadFromDisk();

	FCriticalSection CacheLock;
	bool bCacheValid = false;
	TArray<FSkillPackEntry> CachedSkillPacks;
};
