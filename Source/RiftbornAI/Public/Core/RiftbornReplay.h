// RiftbornReplay.h
// Determinism replay — records the agent's decisions so a later run can
// verify it reproduces the same tool-call sequence and result hashes.
//
// Layers:
//   * FRiftbornReplayRecorder — captures every tool call to JSONL on disk.
//     Paired with FRiftbornDeterminism so the seed is part of the record.
//   * FRiftbornReplayPlayer   — reads a recorded file back, sets the seed,
//     then verifies the same tool-call sequence arrives at the same result
//     hashes. Mismatches are reported without aborting.
//
// This is the scaffolding for "same prompt, same project → same result."
// Full replay (re-driving an LLM against the recorded conversation) is out
// of scope here; this tier catches determinism regressions at the tool
// dispatch layer, which is where non-determinism actually lives.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

struct FClaudeToolCall;
struct FClaudeToolResult;

/**
 * One recorded tool call — the minimum data needed to replay deterministically.
 * Stored as one JSON object per line (JSONL) for streaming writes.
 */
struct RIFTBORNAI_API FReplayEntry
{
	FDateTime Timestamp;
	FString   ToolName;
	FString   ToolUseId;
	TMap<FString, FString> Arguments;
	bool      bSuccess = false;
	FString   ResultHash;   // stable hash of the result payload (not the full result)
	FString   SemanticHash; // from FClaudeToolResult::SemanticHash (when present)
	int64     ExecDurationMs = 0;
};

/**
 * Recorder — call StartRecording("session.jsonl"), then AppendCall() from
 * the dispatch choke point. Stop writes the session seed as a trailer so
 * the player can set the seed before replaying.
 */
class RIFTBORNAI_API FRiftbornReplayRecorder
{
public:
	static FRiftbornReplayRecorder& Get();

	/** Begin recording to the given path. Overwrites existing file.
	 *  Records the current FRiftbornDeterminism session seed as the header. */
	bool StartRecording(const FString& FilePath);

	/** Stop recording. Flushes any buffered entries and writes a footer. */
	void StopRecording();

	/** True while a recording is in progress. */
	bool IsRecording() const;

	/** Append a single tool call + result. Called from FClaudeToolRegistry::ExecuteTool. */
	void AppendCall(const FClaudeToolCall& Call, const FClaudeToolResult& Result);

	/** Path currently being written to, or empty if not recording. */
	FString GetCurrentPath() const;

private:
	FRiftbornReplayRecorder() = default;

	mutable FCriticalSection Lock;
	bool    bRecording = false;
	FString CurrentPath;
	uint64  HeaderSeed = 0;
	int32   EntriesWritten = 0;
};

/**
 * Player — reads a recording and verifies a new run produces the same
 * tool-call sequence. No-ops (returns true / empty report) when the file
 * doesn't exist.
 */
struct RIFTBORNAI_API FReplayVerifyReport
{
	int32 EntriesTotal = 0;
	int32 EntriesMatched = 0;
	int32 ResultHashMismatches = 0;
	int32 SequenceMismatches = 0;
	TArray<FString> MismatchDetails;
	uint64 RecordedSeed = 0;
};

class RIFTBORNAI_API FRiftbornReplayPlayer
{
public:
	/** Load a recording. Returns true on success. */
	bool Load(const FString& FilePath);

	/** Apply the recorded session seed via FRiftbornDeterminism. Idempotent. */
	void ApplySeed() const;

	/** Number of entries available for replay verification. */
	int32 NumEntries() const { return Entries.Num(); }

	/** Verify that a freshly-recorded sequence matches this one. */
	FReplayVerifyReport Verify(const TArray<FReplayEntry>& Fresh) const;

	/** Direct read access for consumers (e.g. tests). */
	const TArray<FReplayEntry>& GetEntries() const { return Entries; }
	uint64 GetRecordedSeed() const { return RecordedSeed; }

private:
	uint64 RecordedSeed = 0;
	TArray<FReplayEntry> Entries;
};
