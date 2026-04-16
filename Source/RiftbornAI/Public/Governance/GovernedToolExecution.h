// GovernedToolExecution.h — Single-call governed tool dispatch
//
// USAGE: Replace FClaudeToolRegistry::Get().ExecuteTool(Call) with:
//        RiftbornExecuteGovernedTool(Call)
//
// This routes ALL tool execution through FExecEngine::ExecuteGovernedTool,
// enforcing contracts, undo-by-tier, PROOF-mode blocking, and audit trails.
//
// The ONLY code that should call FClaudeToolRegistry::Get().ExecuteTool directly
// is the governance layer itself (FExecEngine) and test code.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

struct FClaudeToolCall;
struct FClaudeToolResult;

/**
 * Execute a tool through the full governance pipeline.
 *
 * Enforces:
 *   1. Contract existence (PROOF mode blocks uncontracted tools)
 *   2. Undo-by-tier (Dangerous/Destructive blocked without undo support)
 *   3. Proof emission on rejection
 *   4. Audit trail via CallSource
 *
 * Returns a full FClaudeToolResult with governance fields populated.
 * On governance denial, bSuccess=false and ErrorMessage contains the rejection reason.
 *
 * @param Call  The tool call to execute (ToolName, Arguments, CallSource, etc.)
 * @return      Full result including governance metadata
 */
RIFTBORNAI_API FClaudeToolResult RiftbornExecuteGovernedTool(const FClaudeToolCall& Call);

/**
 * Async-aware variant. Runs the same budget gate + perf snapshot enrichment
 * as RiftbornExecuteGovernedTool, then dispatches via
 * FClaudeToolRegistry::ExecuteToolAsync. For tools registered with
 * RegisterTool (sync), OnComplete fires inline before this returns; for
 * tools registered with RegisterToolAsync, OnComplete fires when the
 * tool's async handler invokes its continuation.
 *
 * Use this from contexts that can yield control back to the editor between
 * tool dispatch and result arrival — primarily the agentic loop.
 *
 * @param Call        Tool call (same fields as the sync variant).
 * @param OnComplete  Fires exactly once on the game thread with the
 *                    final result (after perf-snapshot enrichment).
 */
DECLARE_DELEGATE_OneParam(FOnRiftbornGovernedToolComplete, FClaudeToolResult);
RIFTBORNAI_API void RiftbornExecuteGovernedToolAsync(const FClaudeToolCall& Call, FOnRiftbornGovernedToolComplete OnComplete);

/** Global read-only mode. When active, mutating tools are blocked at the governance layer. */
RIFTBORNAI_API void RiftbornSetReadOnlyMode(bool bReadOnly);
RIFTBORNAI_API bool RiftbornIsReadOnlyMode();
