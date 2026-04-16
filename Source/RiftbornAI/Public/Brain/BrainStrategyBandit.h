// BrainStrategyBandit.h
// Real multi-armed bandit for strategy selection per task family.
// Replaces the "STUB (brain reasoning removed)" stub.
//
// How it works:
//   * Each task family (code, tdm, scene_edit, world_edit, visual_edit, ...)
//     carries a list of candidate FBrainStrategy entries.
//   * SelectStrategy(family) returns the strategy with the highest UCB1 score,
//     giving unselected strategies an exploration boost.
//   * RecordOutcome(family, strategy_id, bSuccess, reward) updates stats so
//     future selections converge on the strategies that actually work for
//     this family.
//   * State persists to Saved/RiftbornAI/strategy_bandit.json so learning
//     survives editor restarts.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

namespace BrainStrategies
{
	namespace Code
	{
		static const FString TestFirst = TEXT("code.test_first");
		static const FString BottomUp  = TEXT("code.bottom_up");
	}
	namespace TDM
	{
		static const FString IncrementalTest = TEXT("tdm.incremental_test");
	}
}

/**
 * A single strategy entry — the "arm" of the bandit. TimesSelected and
 * Successes / TotalReward accumulate across sessions via persistence.
 */
struct RIFTBORNAI_API FBrainStrategy
{
	FString StrategyId;
	FString Name;
	FString Description;
	int32   TimesSelected = 0;
	int32   Successes     = 0;
	float   TotalReward   = 0.0f;

	/** UCB1 exploration bonus. TotalFamilySelections is N across all arms. */
	float GetUCBScore(int32 TotalFamilySelections) const;

	/** Hit rate over all times this strategy was picked. 0 for untried. */
	float GetSuccessRate() const;

	/** Mean reward across all times this strategy was picked. */
	float GetMeanReward() const;
};

/**
 * Process-global bandit. Thread-safe for read/write — RecordOutcome can run
 * from the agent tick thread while UI reads stats.
 */
class RIFTBORNAI_API FBrainStrategyBandit
{
public:
	static FBrainStrategyBandit& Get();

	/** Register / update a candidate strategy for a task family. Idempotent
	 *  on StrategyId — re-registering updates Name and Description without
	 *  clobbering the learned stats. */
	void RegisterStrategy(const FString& Family, const FBrainStrategy& Strategy);

	/** Pick the highest-UCB1 strategy for the family. Auto-registers the
	 *  "default" strategy if the family has no candidates yet so callers
	 *  always get a valid choice. */
	FBrainStrategy SelectStrategy(const FString& Family);

	/** Record the outcome of an executed strategy. bSuccess flips the
	 *  success counter; Reward is added to TotalReward (typical range 0..1). */
	void RecordOutcome(const FString& Family, const FString& StrategyId, bool bSuccess, float Reward = 1.0f);

	/** Read-only view of all strategies for a family (sorted by UCB desc). */
	TArray<FBrainStrategy> GetStrategies(const FString& Family) const;

	/** Load persisted state (called automatically on first Get()). */
	void LoadFromDisk();

	/** Force-flush state to disk. RecordOutcome flushes every 10 updates; this
	 *  is for operators / tests that want guaranteed persistence. */
	void SaveToDisk();

private:
	FBrainStrategyBandit();

	void SeedDefaults();
	FString GetPersistencePath() const;

	mutable FCriticalSection Lock;
	TMap<FString, TArray<FBrainStrategy>> StrategiesByFamily;
	int32 UpdatesSinceSave = 0;
	bool  bLoaded = false;
};
