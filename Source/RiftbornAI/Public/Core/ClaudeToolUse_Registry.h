// ClaudeToolUse_Registry.h
// Extracted from ClaudeToolUse.h: FClaudeToolRegistry class

#pragma once

#include "CoreMinimal.h"

// Forward declarations — full types in ClaudeToolUse.h
struct FClaudeTool;
struct FClaudeToolCall;
struct FClaudeToolResult;
enum class EToolRisk : uint8;
enum class EToolVisibility : uint8;
typedef FClaudeToolResult (*FClaudeToolFunction)(const FClaudeToolCall&);
typedef FString (*FLegacyToolFunction)(const TMap<FString, FString>&);

/**
 * Claude Tool Registry - Manages available tools for AI function calling
 */
class RIFTBORNAI_API FClaudeToolRegistry
{
public:
	static FClaudeToolRegistry& Get();

	// Register a tool with its execution handler (new format)
	void RegisterTool(const FClaudeTool& Tool, FOnExecuteTool ExecutionHandler);

	// Register a tool with an ASYNC execution handler. The handler must call
	// the supplied FOnAsyncToolComplete continuation exactly once on the
	// game thread when the result is ready. The dispatch path returns to
	// the caller immediately after invoking the handler — the editor stays
	// responsive while the tool's latent work (GPU readback, subagent
	// turn, etc.) is in flight. See FOnExecuteToolAsync for the contract.
	void RegisterToolAsync(const FClaudeTool& Tool, FOnExecuteToolAsync ExecutionHandler);

	// Register a tool with legacy function signature (backward compatibility)
	// Automatically wraps the legacy function to convert arguments and results
	void RegisterLegacyTool(const FClaudeTool& Tool, FLegacyToolFunction LegacyFunction);

	// Unregister a tool
	void UnregisterTool(const FString& ToolName);

	/** True if the tool was registered via RegisterToolAsync. Aliases resolve. */
	bool IsToolAsync(const FString& ToolName) const;

	// Clear all registered tools and handlers (useful after Hot Reload to refresh function pointers)
	void Reset();
	
	// === Tool Name Aliasing ===
	
	/** Register an alias for a tool name. When alias is called, canonical tool is executed. */
	void RegisterToolAlias(const FString& AliasName, const FString& CanonicalName);
	
	/** Resolve a tool name through aliases. Returns canonical name if aliased, or original if not. */
	FString ResolveToolName(const FString& ToolName) const;
	
	/** Initialize built-in aliases for known tool name variations */
	void InitializeDefaultAliases();

	// Check if a tool exists
	bool HasTool(const FString& ToolName) const { return RegisteredTools.Contains(ResolveToolName(ToolName)); }

	// Get a tool definition (for schema validation) — resolves aliases
	const FClaudeTool* GetTool(const FString& ToolName) const 
	{ 
		return RegisteredTools.Find(ResolveToolName(ToolName)); 
	}
	
	// Alias for GetTool for code consistency (kept for backward compatibility)
	const FClaudeTool* FindTool(const FString& ToolName) const 
	{ 
		return GetTool(ToolName); 
	}

	// Get all registered tools
	const TMap<FString, FClaudeTool>& GetAllTools() const { return RegisteredTools; }
	
	// Get mutable access to all registered tools.
	// SECURITY NOTE: This should ONLY be called during initialization (before
	// tools are served to clients). Post-initialization mutation would allow
	// overwriting tool handlers to inject malicious behavior.
	// TODO: Replace with explicit SetToolRiskTier() API and remove mutable access.
	TMap<FString, FClaudeTool>& GetAllToolsMutable() { return RegisteredTools; }
	
	// Get all tool names as sorted array (deterministic ordering from TMap)
	TArray<FString> GetAllToolNames() const
	{
		TArray<FString> Names;
		RegisteredTools.GetKeys(Names);
		Names.Sort();
		return Names;
	}
	
	// Get only PUBLIC tools (for LLM exposure)
	TArray<FClaudeTool> GetPublicTools() const;
	
	// Get tools filtered by visibility
	TArray<FClaudeTool> GetToolsByVisibility(EToolVisibility Visibility) const;
	
	// Get tools filtered by category
	TArray<FClaudeTool> GetToolsByCategory(const FString& Category) const;
	
	// Get count of tools by visibility
	int32 GetToolCountByVisibility(EToolVisibility Visibility) const;
	
	// === AGENT PROFILE FILTERING ===
	
	// Get tools available for a specific agent profile
	TArray<FClaudeTool> GetToolsForProfile(EAgentProfile Profile) const;
	
	// Get maximum allowed risk level for a profile
	static EToolRisk GetMaxRiskForProfile(EAgentProfile Profile);
	
	// Check if a tool is allowed for a given profile
	bool IsToolAllowedForProfile(const FString& ToolName, EAgentProfile Profile) const;
	
	// Build tools JSON for a specific agent profile
	TArray<TSharedPtr<FJsonValue>> BuildToolsJsonForProfile(EAgentProfile Profile) const;
	
	// Build tools JSON filtered by profile AND specific tool names (for router integration)
	TArray<TSharedPtr<FJsonValue>> BuildToolsJsonFilteredWithProfile(
		const TArray<FString>& ToolNames, 
		EAgentProfile Profile
	) const;

	// Execute a tool call (synchronous). For tools registered via
	// RegisterToolAsync this falls back to a GT-blocking wait on the
	// continuation — kept for back-compat with legacy non-agentic call
	// sites; the agentic loop itself uses ExecuteToolAsync directly.
	FClaudeToolResult ExecuteTool(const FClaudeToolCall& ToolCall);

	// Execute a tool call asynchronously. OnComplete fires exactly once on
	// the game thread when the result is ready. Sync tools complete
	// synchronously (OnComplete fires inline before this returns); async
	// tools schedule their continuation and return immediately so the
	// editor can continue rendering frames.
	void ExecuteToolAsync(const FClaudeToolCall& ToolCall, FOnAsyncToolComplete OnComplete);
	
	// =========================================================================
	// GOVERNANCE INSTRUMENTATION (2026-02-01 Choke Point)
	// Counter increments on EVERY tool execution attempt (pass or blocked).
	// CI tests assert this counter increments to verify all paths hit the choke point.
	// =========================================================================
	static uint64 GetGovernanceCheckCount();
	static void ResetGovernanceCheckCount();  // For testing only
	
	// Validate tool call arguments against schema
	// Returns empty string if valid, error message if invalid
	FString ValidateToolCall(FClaudeToolCall& ToolCall) const;

	// Build tools array for Claude API request (all tools)
	TArray<TSharedPtr<FJsonValue>> BuildToolsJson() const;
	
	// Build tools array filtered to specific tool names
	TArray<TSharedPtr<FJsonValue>> BuildToolsJsonFiltered(const TArray<FString>& ToolNames) const;
	
	// Build tools array for PUBLIC tools only (default for LLM)
	TArray<TSharedPtr<FJsonValue>> BuildPublicToolsJson() const;

	// Parse tool calls from Claude response
	static TArray<FClaudeToolCall> ParseToolCalls(const FString& JsonResponse);
	
	// === TOKEN OPTIMIZATION ===
	
	/** Build COMPRESSED tool schema (minimal JSON, ~40% smaller) */
	TArray<TSharedPtr<FJsonValue>> BuildCompressedToolsJson(const TArray<FString>& ToolNames) const;
	
	/** Get cached schema for a tool (lazy loaded) */
	TSharedPtr<FJsonObject> GetCachedToolSchema(const FString& ToolName) const;
	
	/** Clear schema cache (call after tool registration changes) */
	void ClearSchemaCache();
	
	/** Estimate token count for tools array */
	int32 EstimateToolsTokenCount(const TArray<FString>& ToolNames) const;

	// === CONFIRMATION SYSTEM ===
	// Tools with NeedsConfirmation policy are blocked until confirmed
	
	/** Check if a tool call requires confirmation before execution */
	bool RequiresConfirmation(const FString& ToolName, EToolRisk Risk, bool bSafeModeEnabled) const;
	
	/** Create a pending confirmation token for a blocked tool call */
	FRiftbornConfirmationToken CreateConfirmationToken(
		const FClaudeToolCall& ToolCall, 
		const FString& RequestId,
		const FString& SessionId,
		const FString& Reason);
	
	/** Validate and consume a confirmation token, returning the original tool call if valid */
	bool ValidateAndConsumeToken(
		const FString& Token, 
		const FString& RequestId,
		FClaudeToolCall& OutToolCall,
		FString& OutError);

	/** Consume a confirmation token after args/tool validation (single-use enforcement) */
	bool ConsumeConfirmationToken(const FString& Token, FString& OutError);
	
	/** Get pending confirmation by token (read-only copy) */
	bool GetPendingConfirmation(const FString& Token, FRiftbornConfirmationToken& OutConfirmation) const;
	
	/** Clear expired confirmations (call periodically) */
	void PruneExpiredConfirmations();
	
	/** Get count of pending confirmations */
	int32 GetPendingConfirmationCount() const { FScopeLock Lock(&ConfirmationLock); return PendingConfirmations.Num(); }

	// === TOOL SUGGESTION SYSTEM ===
	
	/** Suggest tools based on natural language intent. Returns tool names sorted by relevance. */
	TArray<FString> SuggestToolsForIntent(const FString& Intent, int32 MaxResults = 5) const;
	
	/** Get related tools (tools that work well together or can substitute each other) */
	TArray<FString> GetRelatedTools(const FString& ToolName) const;
	
	/** Get tools by semantic group (what the user is trying to accomplish) */
	TArray<FString> GetToolsForTask(const FString& TaskKeyword) const;

	// === SESSION TOOL FILTER (Brain Integration) ===
	
	/**
	 * Set a session-scoped tool filter.
	 * When set, BuildToolsJson() and GetPublicTools() will only return tools in this list.
	 * This allows the brain to pre-select tools BEFORE the provider call.
	 * 
	 * @param ToolNames - Tools to allow (empty = no filter, all public tools)
	 * @param ToolConfidence - Optional confidence per tool (for logging)
	 */
	void SetSessionToolFilter(
		const TArray<FString>& ToolNames,
		const TMap<FString, float>& ToolConfidence = TMap<FString, float>()
	);
	
	/** Clear the session tool filter (reverts to all public tools) */
	void ClearSessionToolFilter();
	
	/** Check if a session filter is active */
	bool HasSessionToolFilter() const { return bSessionFilterActive; }
	
	/** Get current session filter (for diagnostics) */
	const TArray<FString>& GetSessionToolFilter() const { return SessionToolFilter; }

private:
	FClaudeToolRegistry() = default;
	
	/** Internal prune - caller must hold ConfirmationLock */
	void PruneExpiredConfirmationsInternal();

	TMap<FString, FClaudeTool> RegisteredTools;
	TMap<FString, FOnExecuteTool> ToolHandlers;
	TMap<FString, FOnExecuteToolAsync> AsyncToolHandlers;
	TMap<FString, FLegacyToolFunction> LegacyToolFunctions;
	
	// Tool name aliasing: maps alias -> canonical name
	// e.g. "set_game_mode_override" -> "set_gamemode_override"
	TMap<FString, FString> ToolAliases;

	// Schema cache for lazy loading
	mutable TMap<FString, TSharedPtr<FJsonObject>> CachedSchemas;
	mutable TMap<FString, TSharedPtr<FJsonObject>> CachedCompressedSchemas;
	mutable bool bSchemaCacheDirty = true;

	// Pending confirmation tokens: Token -> ConfirmationData
	// Single-use tokens that bind a blocked tool call to a confirm action
	// THREAD SAFETY: Protected by ConfirmationLock (HTTP server is multi-threaded)
	mutable FCriticalSection ConfirmationLock;
	mutable TMap<FString, FRiftbornConfirmationToken> PendingConfirmations;
	
	// Session tool filter (brain integration)
	// When active, only tools in this list are returned by BuildToolsJson/GetPublicTools
	bool bSessionFilterActive = false;
	TArray<FString> SessionToolFilter;
	TMap<FString, float> SessionToolConfidence;
	
	// Confirmation token expiry duration (default 5 minutes)
	static constexpr double ConfirmationExpirySeconds = 300.0;
};
