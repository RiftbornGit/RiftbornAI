// Copyright RiftbornAI. All Rights Reserved.
//
// FClaudeCodeProvider — IAIProvider that uses the Claude Code CLI as a
// reasoning backend instead of direct Anthropic API calls.
//
// Why:
//   * Users with a Claude Pro/Team subscription can use that quota instead
//     of paying per-token API charges.
//   * Inherits Claude Code's prompt caching, agent orchestration, plan mode,
//     skill library, and todo discipline — none of which we'd otherwise get
//     for free.
//   * Same model surface (Opus 4.7 / Sonnet 4.6) — reasoning quality is
//     identical to the direct-API path.
//
// How it works:
//   1. We spawn `claude` (CLI) as a subprocess via FInteractiveProcess.
//   2. Claude Code runs in --print mode with --output-format stream-json so
//      we can parse incremental events.
//   3. Per invocation we generate a temp `mcp.json` that points Claude Code
//      at the project-bundled stdio MCP server (mcp-server/dist/index.js).
//      That MCP server speaks the full RiftbornAI tool surface — same one
//      the user runs in their normal Claude Code sessions — so tools route
//      back into the editor without us having to maintain a second wire
//      format alongside the existing HTTP bridge.
//   4. stdout is parsed line-by-line as JSONL; assistant text blocks fire
//      OnToken; tool_use blocks are surfaced for UI display only (Claude
//      Code is the one dispatching them via MCP, not us); the final result
//      event triggers OnComplete and records token usage.
//
// What this provider does NOT do:
//   * It does not call Anthropic directly. That's the whole point.
//   * It does not bypass Claude Code's tool dispatch — tools route through
//     the MCP server so governance / proof mode / trust scoring still apply
//     (the same FClaudeToolRegistry that backs the in-editor copilot).

#pragma once

#include "CoreMinimal.h"
#include "IAIProvider.h"

class FInteractiveProcess;

class RIFTBORNAI_API FClaudeCodeProvider
	: public IAIProvider
	, public TSharedFromThis<FClaudeCodeProvider>
{
public:
	FClaudeCodeProvider();
	virtual ~FClaudeCodeProvider() override;

	// ============================================================================
	// IAIProvider — Configuration
	// ============================================================================
	virtual void SetAPIKey(const FString& InAPIKey) override { /* unused — Claude Code owns auth */ }
	virtual void SetModel(const FString& InModel) override { Model = InModel; }
	virtual FString GetModel() const override { return Model; }
	virtual FString GetProviderName() const override { return TEXT("ClaudeCode"); }
	virtual bool IsConfigured() const override;

	virtual void SetSystemPrompt(const FString& Prompt) override { SystemPrompt = Prompt; }

	// ============================================================================
	// IAIProvider — Messaging
	// ============================================================================
	virtual void SendMessage(
		const FString& Message,
		TFunction<void(bool bSuccess, const FString& Response)> OnComplete) override;

	virtual void SendMessageWithTools(
		const FString& Message,
		const FString& RequestId,
		TFunction<void(bool bSuccess, const FString& Response, const FString& RequestId)> OnComplete,
		TFunction<void(const FString& Status)> OnProgress = nullptr) override;

	virtual void SendMessageStreaming(
		const FString& Message,
		const TArray<TSharedPtr<FJsonValue>>& Tools,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete) override;

	virtual void ContinueWithToolResult(
		const FString& ToolUseId,
		const FString& Result,
		const TArray<TSharedPtr<FJsonValue>>& Tools,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete) override;

	// ============================================================================
	// IAIProvider — State
	// ============================================================================
	virtual void ClearHistory() override;
	virtual void CancelRequest() override;
	virtual bool IsCancelled() const override { return bCancelled; }

	virtual const TArray<FClaudeToolCall>&    GetLastToolCalls() const override { return LastToolCalls; }
	virtual void                              ClearLastToolCalls() override { LastToolCalls.Reset(); }
	virtual TArray<FClaudeToolResult>         GetToolResultsForRequest(const FString& RequestId) const override;
	virtual void                              ClearToolResultsForRequest(const FString& RequestId) override;
	virtual const TArray<FClaudeToolResult>&  GetLastToolResults() const override { return EmptyResults; }
	virtual void                              ClearLastToolResults() override {}

	virtual int64 GetSessionInputTokens() const override  { return TotalInputTokens; }
	virtual int64 GetSessionOutputTokens() const override { return TotalOutputTokens; }
	virtual float EstimateSessionCost() const override;
	virtual void  ResetSessionTokens() override { TotalInputTokens = 0; TotalOutputTokens = 0; }

	// ============================================================================
	// Claude-Code-specific configuration
	// ============================================================================

	/** Override the path to the `claude` binary. Empty = auto-detect via PATH. */
	void SetClaudeCodePath(const FString& Path) { ClaudeCodePath = Path; }
	FString GetClaudeCodePath() const { return ClaudeCodePath; }

	/** Auto-detect the `claude` binary on PATH. Returns absolute path or empty. */
	static FString DetectClaudeCodeBinary();

	/** Quick check: is `claude` reachable + does `--version` succeed? */
	static bool IsClaudeCodeAvailable();

	/** Read the version Claude Code reports (best-effort, may be empty). */
	static FString GetClaudeCodeVersion();

private:
	// ============================================================================
	// Internals
	// ============================================================================

	/** Build the JSONL line buffer up until newline, then dispatch each line
	 *  as a single stream-json event. */
	void HandleProcessOutput(const FString& Chunk);

	/** Parse one stream-json line and route it to the appropriate callback. */
	void DispatchStreamEvent(const FString& JsonLine);

	/** Build an MCP config JSON pointing Claude Code at the running RiftbornAI
	 *  HTTP bridge. Written to a per-invocation temp file. */
	FString WriteMCPConfigForThisInvocation() const;

	/** Build the RiftbornAI system guidance text for this request. */
	FString BuildSystemPromptArgument(const FString& UserMessage) const;

	/** On Windows, write the request body to a temp file so cmd.exe never has
	 *  to carry large prompt/context blobs on the command line. */
	FString WritePromptInputForThisInvocation(const FString& Message, const FString& Prompt) const;

	/** Quote a string for safe inclusion in the command line on Windows. */
	static FString QuoteForCommandLine(const FString& In);

	/** Cleanly tear down the active process if any. */
	void EndCurrentInvocation();

	// Identity / config
	FString  Model;
	FString  ClaudeCodePath;     // empty = auto-detect each invocation
	FString  SystemPrompt;

	// Active invocation state
	TSharedPtr<FInteractiveProcess> Process;
	FString  StdoutLineBuffer;
	bool     bCancelled = false;
	bool     bRequestInProgress = false;
	FString  CurrentRequestId;
	FString  CurrentMcpConfigPath;  // we delete this on cleanup
	FString  CurrentPromptInputPath;
	FString  LastProcessDiagnostic;
	FString  LastRateLimitReason;   // populated when CLI emits rate_limit_event with status=rejected
	bool     bAssistantTextSeen = false;  // tracks whether real model output streamed

	// Active-invocation callbacks. Captured at SendMessageStreaming time.
	TFunction<void(const FString&)> OnTokenCallback;
	TFunction<void(const FString&, const TMap<FString, FString>&, const FString&)> OnToolCallCallback;
	TFunction<void(bool, const FString&)> OnCompleteCallback;

	// Conversation history (we maintain on our side because each `claude --print`
	// call is a fresh process and Claude Code's own session continuity isn't
	// used in this provider).
	TArray<TPair<FString, FString>> History;  // (role, content)

	// Last-call tool tracking — populated for UI display only (we don't dispatch).
	TArray<FClaudeToolCall>          LastToolCalls;

	// Per-request results (interface contract; mostly empty for this provider
	// because the MCP-side dispatch captures real results elsewhere).
	mutable FCriticalSection                              ToolResultsLock;
	TMap<FString, TArray<FClaudeToolResult>>              ToolResultsByRequest;
	static inline TArray<FClaudeToolResult>               EmptyResults;

	// Token / cost tracking from Claude Code's `result` event.
	int64    TotalInputTokens  = 0;
	int64    TotalOutputTokens = 0;
	double   TotalReportedCostUsd = 0.0;
};
