// SubagentRunner.h
// Spawns a scoped child FAgenticLoopRunner with a filtered tool set and a
// fresh conversation, runs it to completion, and returns a structured
// result. Provider-agnostic — same behaviour regardless of whether the
// parent is using Claude, OpenAI, or Ollama (capabilities match; only the
// underlying model's reasoning quality differs).
//
// Intended to be called synchronously from a tool handler on the game
// thread; internally pumps the HTTP manager so provider callbacks can
// land while we wait.
//
// Recursion guard: nested spawn_subagent calls are bounded. If the
// recursion depth exceeds MaxDepth (default 2) the runner refuses to
// spawn and returns a failure with a clear reason.

#pragma once

#include "CoreMinimal.h"
#include "Core/ClaudeToolUse_Types.h"   // EAgentProfile

struct RIFTBORNAI_API FSubagentResult
{
	bool     bSuccess = false;
	FString  FinalResponse;
	FString  ErrorMessage;

	int32    Iterations      = 0;
	int32    ToolCallsMade   = 0;
	int64    InputTokens     = 0;
	int64    OutputTokens    = 0;
	float    DurationSeconds = 0.0f;

	/** Tool names invoked by the sub-agent, in order (deduped only trivially). */
	TArray<FString> ToolsUsed;

	/** For caller observability — how deep the nesting was. */
	int32    Depth = 0;

	/** JSON summary suitable for returning to the parent LLM. */
	FString ToJsonString() const;
};

struct RIFTBORNAI_API FSubagentOptions
{
	EAgentProfile Profile = EAgentProfile::EditorAssistant;
	FString       Task;
	int32         MaxIterations = 6;
	float         TimeoutSeconds = 120.0f;

	/** If true, the scene perception JSON is prepended to the task so the
	 *  sub-agent sees the current world without needing its own probe. */
	bool bIncludeSceneContext = true;

	/** Optional additional system prompt addendum (e.g. "you are scouting,
	 *  do not mutate state"). Merged with the provider's default. */
	FString SystemPromptAddendum;
};

class RIFTBORNAI_API FSubagentRunner
{
public:
	/** Blocking synchronous run. Safe to call from a tool handler on the
	 *  game thread — internally pumps the HTTP manager + Slate while
	 *  waiting. Prefer RunAsync for new code so the editor stays responsive. */
	static FSubagentResult RunSync(const FSubagentOptions& Options);

	/** Async variant — does NOT block the calling thread. The continuation
	 *  fires on the game thread when the sub-agent completes (success,
	 *  failure, or timeout). Use this from async tool handlers so the
	 *  editor keeps presenting frames while the sub-agent's HTTP turns
	 *  are in flight. */
	DECLARE_DELEGATE_OneParam(FOnSubagentComplete, FSubagentResult);
	static void RunAsync(const FSubagentOptions& Options, FOnSubagentComplete OnComplete);

	/** Runtime-configurable cap on nesting depth. Default 2 (parent → child
	 *  → grandchild stops). Increase carefully. */
	static int32 GetMaxRecursionDepth();
	static void  SetMaxRecursionDepth(int32 NewDepth);

	/** Current depth of the game thread's spawn stack — incremented when a
	 *  sub-agent begins, decremented when it finishes. Used by the tool to
	 *  reject recursive spawns above the limit. */
	static int32 GetCurrentDepth();
};
