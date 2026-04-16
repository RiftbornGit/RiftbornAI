// AsyncToolExecutor.h
// Non-blocking tool execution for governed tool loops
//
// Gap #1 Fix: Editor tool users will not tolerate editor freezes. This system ensures
// that tool execution never blocks the editor for more than one frame budget.
//
// Architecture:
// - Tools marked bGameThreadRequired=false run on a background thread pool
// - Tools requiring game thread are executed with frame-budget awareness:
//   each tool gets up to FrameBudgetMs before the system yields to let
//   the editor tick. Long-running game-thread tools still block their
//   individual execution, but the system pumps Slate between tool calls.
// - A "progress pump" ticks Slate/render between consecutive game-thread
//   tool calls so the editor never appears frozen during multi-tool sequences.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "HAL/CriticalSection.h"
#include "Async/Future.h"

// Forward declarations
struct FClaudeToolCall;
struct FClaudeToolResult;
class FClaudeToolRegistry;

/**
 * Result from async tool execution, including timing metadata
 */
struct RIFTBORNAI_API FAsyncToolResult
{
    FClaudeToolResult ToolResult;
    double ExecutionTimeMs = 0.0;
    bool bRanOnGameThread = true;
    bool bEditorWasPumped = false;  // Whether Slate was ticked during execution
};

/**
 * Delegate for when an async tool completes
 */
DECLARE_DELEGATE_OneParam(FOnAsyncToolExecutionComplete, const FAsyncToolResult& /* Result */);

/**
 * FAsyncToolExecutor
 * 
 * Ensures tool execution never freezes the editor:
 * 
 * 1. Read-only tools (bGameThreadRequired=false) run on FRunnable thread pool
 * 2. Game-thread tools execute synchronously but pump Slate between calls
 * 3. Long multi-tool sequences have editor responsiveness guarantees
 * 
 * Usage:
 *   FAsyncToolExecutor& Executor = FAsyncToolExecutor::Get();
 *   Executor.ExecuteToolAsync(Call, FOnAsyncToolExecutionComplete::CreateLambda([](const FAsyncToolResult& R) {
 *       // Handle result (always called on game thread)
 *   }));
 * 
 * Thread Safety:
 * - ExecuteToolAsync can be called from any thread
 * - Callback is ALWAYS invoked on the game thread
 * - Internal state protected by FCriticalSection
 */
class RIFTBORNAI_API FAsyncToolExecutor
{
public:
    static FAsyncToolExecutor& Get();

    /**
     * Execute a tool call without blocking the editor.
     * 
     * If the tool is thread-safe (bGameThreadRequired=false), it runs on a
     * background thread. Otherwise, it executes on the game thread.
     * 
     * The callback is ALWAYS called on the game thread.
     * 
     * @param Call - The tool call to execute
     * @param OnComplete - Called when execution finishes (game thread)
     */
    void ExecuteToolAsync(const FClaudeToolCall& Call, FOnAsyncToolExecutionComplete OnComplete);

    /**
     * Execute a tool synchronously on the current thread.
     * For game-thread tools, this is functionally identical to direct execution
     * but tracks timing and thread metadata.
     * 
     * @param Call - The tool call to execute
     * @return Result with timing metadata
     */
    FAsyncToolResult ExecuteToolSync(const FClaudeToolCall& Call);

    /**
     * Pump the editor between consecutive tool calls.
     * Call this between game-thread tool executions to maintain responsiveness.
     * Ticks Slate, processes deferred render commands, and allows UI to update.
     * 
     * Safe to call frequently — will skip if called within MinPumpIntervalMs.
     */
    static void PumpEditorBetweenTools();

    /**
     * Check if a tool can run off the game thread.
     * Uses the tool's bGameThreadRequired flag from the registry.
     * Falls back to game thread for unknown tools (safety first).
     */
    static bool CanRunOffGameThread(const FString& ToolName);

    // =========================================================================
    // Configuration
    // =========================================================================

    /** Maximum time (ms) between editor pumps during multi-tool sequences.
     *  Default: 33ms (two frames at 60fps) */
    float MaxTimeBetweenPumpsMs = 33.0f;

    /** Minimum interval between Slate pumps to avoid overhead. Default: 8ms */
    float MinPumpIntervalMs = 8.0f;

    /** Whether async background execution is enabled. If false, all tools run
     *  on game thread (safe fallback). Default: true */
    bool bEnableBackgroundExecution = true;

    /** Maximum concurrent background tool executions. Default: 2 */
    int32 MaxConcurrentBackgroundTools = 2;

    // =========================================================================
    // Metrics (read-only, for diagnostics)
    // =========================================================================

    /** Total tools executed via this system */
    int32 TotalToolsExecuted = 0;

    /** Tools that ran on background threads */
    int32 BackgroundToolsExecuted = 0;

    /** Number of times the editor was pumped between tools */
    int32 EditorPumpCount = 0;

    /** Longest single tool execution on game thread (ms) */
    double LongestGameThreadToolMs = 0.0;

    /** Name of the longest game-thread tool (for diagnostics) */
    FString LongestGameThreadToolName;

private:
    FAsyncToolExecutor() = default;
    ~FAsyncToolExecutor() = default;

    // Non-copyable
    FAsyncToolExecutor(const FAsyncToolExecutor&) = delete;
    FAsyncToolExecutor& operator=(const FAsyncToolExecutor&) = delete;

    /** Execute the governed tool and wrap result with metadata */
    static FAsyncToolResult ExecuteGovernedToolInternal(const FClaudeToolCall& Call, bool bOnGameThread);

    /** Thread protection for metrics and concurrent background count */
    FCriticalSection MetricsLock;

    /** Current number of in-flight background tool executions */
    std::atomic<int32> CurrentBackgroundTools{0};

    /** Last time we pumped the editor (for throttling) */
    double LastPumpTimeSeconds = 0.0;
};
