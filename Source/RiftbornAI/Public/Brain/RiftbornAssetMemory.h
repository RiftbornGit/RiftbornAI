// Copyright RiftbornAI. All Rights Reserved.
// RiftbornAssetMemory.h — persistent per-project memory of argument VALUES
// that have succeeded for a given tool.
//
// FBrainMetrics remembers success RATES per tool. This class remembers the
// concrete asset paths / actor labels / class names that actually worked,
// so when a tool fails with a hallucinated value, the error message can
// surface real values the agent has seen succeed IN THIS PROJECT across
// prior sessions. That's the bit that makes the copilot get smarter over
// time — per-user, per-project asset knowledge compounding across runs.
//
// Storage: Saved/RiftbornAI/asset_memory.json
// Write cadence: debounced (at most once per DebounceSeconds) to avoid
//   hammering disk on high-throughput tool sessions.

#pragma once

#include "CoreMinimal.h"

/**
 * A single remembered "value that worked" for a (tool, arg) pair.
 */
struct RIFTBORNAI_API FAssetMemoryEntry
{
    FString Value;
    int32 SuccessCount = 0;
    FDateTime LastUsed;
};

/**
 * Project-scoped persistent memory of which specific argument values have
 * succeeded for each tool. Singleton, thread-safe reads, serialized writes.
 */
class RIFTBORNAI_API FRiftbornAssetMemory
{
public:
    static FRiftbornAssetMemory& Get();

    /** Record that a tool call succeeded with this specific arg value.
     *  Keying is (ToolName, ArgName) → ArgValue. Only called for args that
     *  are meaningful to remember (asset paths, class names, labels) —
     *  callers decide which args to feed in. */
    void RecordSuccess(const FString& ToolName, const FString& ArgName, const FString& ArgValue);

    /** Bulk-record every meaningful arg from a successful call. Filters out
     *  transient args (coords, intensity floats, etc.) and focuses on the
     *  ones worth remembering — paths, labels, class names, enums. */
    void RecordSuccessfulCall(const FString& ToolName, const TMap<FString, FString>& Args);

    /** Record that a tool call FAILED with this specific arg value. The
     *  counterpart to RecordSuccess — the memory learns not just what works
     *  but what persistently doesn't. Failed values that accumulate enough
     *  count get surfaced in the system prompt as guardrails ("don't try
     *  these again"), preventing the LLM-retries-hallucinated-path loop. */
    void RecordFailure(const FString& ToolName, const FString& ArgName, const FString& ArgValue);

    /** Bulk-record every meaningful failed arg from a failed call. Same
     *  filtering as RecordSuccessfulCall. */
    void RecordFailedCall(const FString& ToolName, const TMap<FString, FString>& Args);

    /** Return up to MaxResults historically-successful values for this
     *  (tool, arg) pair, ordered by SuccessCount DESC then LastUsed DESC.
     *  Empty if this project has no learned history for the pair. */
    TArray<FString> GetTopSuccesses(const FString& ToolName, const FString& ArgName, int32 MaxResults = 5) const;

    /** Return up to MaxResults values that have FAILED repeatedly for this
     *  (tool, arg) pair. Only returns values with FailureCount >= MinCount
     *  to avoid surfacing one-off typos. Ordered by FailureCount DESC. */
    TArray<FString> GetTopFailures(const FString& ToolName, const FString& ArgName,
                                   int32 MaxResults = 5, int32 MinCount = 3) const;

    /** Detect the most common directory / namespace prefixes across all
     *  successful values for a (tool, arg) pair. Returns at most MaxResults
     *  prefixes that account for at least MinShare of the successes.
     *  Empty when no strong convention exists yet. Example: after many
     *  scatter_foliage successes rooted in `/Game/Megascans/3D_Plants/`,
     *  this returns that prefix so the agent knows "this project keeps
     *  foliage meshes under that folder". */
    TArray<FString> DetectDirectoryConventions(const FString& ToolName, const FString& ArgName,
                                               int32 MaxResults = 3, float MinShare = 0.5f,
                                               int32 MinSampleSize = 3) const;

    /** Return the total number of distinct remembered (tool, arg, value)
     *  triplets. Useful for diagnostic / UI surfaces. */
    int32 GetMemorySize() const;

    /**
     * Build a compact, LLM-readable text block summarizing the top-N
     * historically-successful argument values per (tool, arg) pair. This is
     * the payload the agentic loop injects into the system prompt at session
     * start so the LLM's FIRST tool call already uses known-good values from
     * prior sessions — no cold-start hallucination round-trip needed.
     *
     * Bounded output: at most TopNPerKey entries per pair, at most
     * MaxTotalLines lines overall. Returns empty string if the memory has
     * nothing relevant yet (fresh install / first session).
     *
     * Format example:
     *   ## Project-known working values (from prior successful tool calls)
     *   scatter_foliage.mesh_path:
     *     /Game/Megascans/3D_Plants/SM_Fern_01.SM_Fern_01 (12x)
     *     /Game/Megascans/3D_Plants/SM_Oak_01.SM_Oak_01 (8x)
     *   set_actor_material.material_path:
     *     /Game/Materials/M_Grass.M_Grass (15x)
     */
    FString BuildProjectKnownValuesContext(int32 TopNPerKey = 5, int32 MaxTotalLines = 40) const;

    /** Flush any pending in-memory changes to disk immediately. Normally
     *  debounced; callers can force a flush on session end / shutdown. */
    void FlushIfDirty();

    /** Wipe the project's memory. Intended for user-initiated "start over"
     *  actions; never call automatically. */
    void Reset();

    /** Full persistence path for diagnostics. */
    static FString GetStorePath();

private:
    FRiftbornAssetMemory() = default;

    void LoadFromDisk();
    void SaveToDisk();

    // Decide whether an argument is worth remembering. Paths, class names,
    // labels = yes. Coordinates, colors, intensities = no.
    static bool IsArgWorthRemembering(const FString& ArgName, const FString& ArgValue);

    // Storage — key is "ToolName::ArgName", value is the list of remembered
    // values with their counts. Small per-tool lists are faster to sort +
    // rotate than a single giant map, and per-tool writes localize dirty
    // tracking.
    mutable FCriticalSection StoreLock;
    TMap<FString, TArray<FAssetMemoryEntry>> Store;

    // Parallel store for values that have FAILED. Same key shape as Store.
    // Kept separate rather than as a flag on FAssetMemoryEntry so the top-N
    // query for each axis stays O(N) instead of needing a filter pass.
    TMap<FString, TArray<FAssetMemoryEntry>> FailureStore;

    bool bLoaded = false;
    bool bDirty = false;
    double LastSaveSeconds = 0.0;

    // Per-tool cap. Prevents a runaway project from growing the memory file
    // unboundedly — we keep the TopN most-valuable entries per (tool, arg).
    static constexpr int32 MaxEntriesPerKey = 32;
    static constexpr double DebounceSeconds = 5.0;
};
