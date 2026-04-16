// Copyright RiftbornAI. All Rights Reserved.
// ClaudeToolUse_Types.h - Enums, structs, and delegates for Claude tool use system
// Split from ClaudeToolUse.h for file size discipline

#pragma once

#include "CoreMinimal.h"
#include "RiftbornGovernanceTypes.h"
#include "ClaimTypes.h"
#include "ClaudeToolUse_Types.generated.h"

/**
 * Tool parameter type for Claude function calling
 */
UENUM(BlueprintType)
enum class EClaudeToolParamType : uint8
{
	String UMETA(DisplayName = "String"),
	Integer UMETA(DisplayName = "Integer"),
	Number UMETA(DisplayName = "Number (Float)"),
	Boolean UMETA(DisplayName = "Boolean"),
	Array UMETA(DisplayName = "Array"),
	Object UMETA(DisplayName = "Object")
};

/**
 * Tool parameter definition
 */
USTRUCT(BlueprintType)
struct FClaudeToolParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	EClaudeToolParamType Type = EClaudeToolParamType::String;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	bool bRequired = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString DefaultValue;

	// For enum-like constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	TArray<FString> AllowedValues;
};

/** Build JSON Schema for a single tool parameter. */
RIFTBORNAI_API TSharedPtr<FJsonObject> RiftbornBuildToolParameterSchema(const FClaudeToolParameter& Param);

/** Build the `input_schema` / `parameters` object for a tool's parameters. */
RIFTBORNAI_API TSharedPtr<FJsonObject> RiftbornBuildToolInputSchema(const TArray<FClaudeToolParameter>& Parameters, bool bIncludeEmptyRequired = true);

/**
 * Tool visibility level - controls when tool is exposed to LLM
 */
UENUM(BlueprintType)
enum class EToolVisibility : uint8
{
	Public UMETA(DisplayName = "Public"),       // Visible to LLM by default
	Internal UMETA(DisplayName = "Internal"),   // Only used by other tools
	Deprecated UMETA(DisplayName = "Deprecated"), // Scheduled for removal
	Experimental UMETA(DisplayName = "Experimental") // Not stable
};

/**
 * Tool risk level - for safety filtering
 */
UENUM(BlueprintType)
enum class EToolRisk : uint8
{
	Safe UMETA(DisplayName = "Safe"),           // Read-only, no side effects
	Elevated UMETA(DisplayName = "Elevated"),   // Elevated risk but reversible
	Mutation UMETA(DisplayName = "Mutation"),   // Modifies project/assets
	Dangerous UMETA(DisplayName = "Dangerous"), // System/file access
	Destructive UMETA(DisplayName = "Destructive"), // Can delete/corrupt data
	Unknown UMETA(DisplayName = "Unknown")      // Risk not yet determined (default for governance)
};

/**
 * Tool cost hint - for routing optimization
 */
UENUM(BlueprintType)
enum class EToolCost : uint8
{
	Cheap UMETA(DisplayName = "Cheap"),       // Fast, no external calls
	Moderate UMETA(DisplayName = "Moderate"), // Normal operation
	Expensive UMETA(DisplayName = "Expensive") // Slow, external calls, LLM chain
};

/**
 * Agent profile - determines which tools are available per context
 * Each profile has different visibility, risk, and category constraints
 */
UENUM(BlueprintType)
enum class EAgentProfile : uint8
{
	/** Default coding agent - Safe/Mutation, core editing tools */
	CodingAgent UMETA(DisplayName = "Coding Agent"),

	/** Editor assistant - broader access, still no destructive ops */
	EditorAssistant UMETA(DisplayName = "Editor Assistant"),

	/** System/pipeline agent - can access build/package/profiling */
	SystemAgent UMETA(DisplayName = "System Agent"),

	/** Design agent - access to design_*, generate_*, preset tools */
	DesignAgent UMETA(DisplayName = "Design Agent"),

	/** Unrestricted - all PUBLIC tools (use with caution) */
	Unrestricted UMETA(DisplayName = "Unrestricted")
};

/**
 * Tool definition for Claude function calling
 */
USTRUCT(BlueprintType)
struct FClaudeTool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	TArray<FClaudeToolParameter> Parameters;

	// =========================================================================
	// SCHEMA VERSION — FClaudeTool public SDK evolution marker.
	// Bumped when fields are added, repurposed, or removed. Third-party tool
	// plugins should check this at registration to detect SDK drift between the
	// version they compiled against and the RiftbornAI core they link into.
	// History:
	//   1  — initial SDK surface (2026-04-15)
	// =========================================================================
	static constexpr int32 CurrentSchemaVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	int32 SchemaVersion = CurrentSchemaVersion;

	// === NEW: Tool Metadata for Routing/Filtering ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	EToolVisibility Visibility = EToolVisibility::Public;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	EToolRisk Risk = EToolRisk::Mutation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	EToolCost Cost = EToolCost::Moderate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	FString Category;  // e.g., "Engine", "Design", "System", "Internal"

	// =========================================================================
	// UNDO CONTRACT - Tools must declare their undo strategy at registration
	// =========================================================================
	// IMPORTANT: This field is MANDATORY for new tools. Tools without explicit
	// undo strategy default to NoUndo (honest about irreversibility).
	//
	// NoUndo (DEFAULT): Tool cannot be undone - most honest for mutation tools
	// InverseOperation: Tool has a registered inverse (e.g., delete_actor for spawn_actor)
	// Snapshot: Tool uses Unreal transaction system (editor undo works)
	// NotDeclared: Legacy tools - treated as NoUndo, logged as warning
	// =========================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Undo")
	EToolUndoStrategy UndoStrategy = EToolUndoStrategy::NotDeclared;

	// For InverseOperation strategy: name of the inverse tool
	// e.g., "spawn_actor" might have InverseTool="destroy_actor"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Undo")
	FString InverseToolName;

	// Timeout in milliseconds (0 = no timeout, default based on Cost)
	// Cheap: 5000ms, Moderate: 30000ms, Expensive: 120000ms
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	int32 TimeoutMs = 0;

	// Whether this tool requires Python bridge to execute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	bool bRequiresPython = false;

	// =========================================================================
	// ASYNC EXECUTION (Gap #1: Non-blocking tool execution)
	// =========================================================================
	// Tools that only READ data (no UObject mutation) can run on background
	// threads, keeping the editor responsive. Default is TRUE (game thread)
	// for safety — tools must explicitly opt-in to background execution.
	//
	// Safe for background: list_assets, resolve_asset, get_actor_info,
	//   file system reads, asset registry queries
	// MUST be game thread: spawn_actor, delete_actor, set_property,
	//   any tool that creates/modifies/destroys UObjects
	// =========================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Threading")
	bool bGameThreadRequired = true;

	// Get effective timeout based on Cost if TimeoutMs is 0
	int32 GetEffectiveTimeoutMs() const
	{
		if (TimeoutMs > 0) return TimeoutMs;
		switch (Cost)
		{
			case EToolCost::Cheap: return 5000;
			case EToolCost::Moderate: return 30000;
			case EToolCost::Expensive: return 120000;
			default: return 30000;
		}
	}

	// Convert to JSON schema for Claude API
	TSharedPtr<FJsonObject> ToJsonSchema() const;
};

/**
 * Tool call from Claude response
 */
USTRUCT(BlueprintType)
struct FClaudeToolCall
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Tool")
	FString ToolUseId;

	UPROPERTY(BlueprintReadOnly, Category = "Tool")
	FString ToolName;

	UPROPERTY(BlueprintReadOnly, Category = "Tool")
	TMap<FString, FString> Arguments;

	// Raw JSON for complex types
	FString RawArgumentsJson;

	// =========================================================================
	// GOVERNANCE FIELDS (2026-02-02)
	// These must be populated by the caller to prove authorization.
	// In PROOF mode, mutating tools require one of these to be valid.
	// =========================================================================

	/** Confirmation token from prior NeedsConfirmation response (single-use) */
	FString ConfirmationToken;

	/** Signed execution context from plan executor (proves step is part of authorized plan) */
	FString ExecCtxSignature;

	/** Execution context ID for correlation with plan/step */
	FString ExecCtxId;

	/** True if ExecCtx was fully validated by the caller before reaching the registry */
	bool bExecCtxValidated = false;

	/** True if this call was produced by /riftborn/confirm after token validation */
	bool bConfirmedByToken = false;

	/** Source of this call - for audit trail (e.g., "http_8766", "http_8767", "agent_step", "internal")
	 *  SECURITY (2026-02-20): Defaults to "unknown" — callers MUST set this explicitly.
	 *  In PROOF mode, "unknown" sources are BLOCKED for mutating tools.
	 *  In DEV mode, "unknown" sources are logged with warning + session taint.
	 */
	FString CallSource = TEXT("unknown");

	/** If true, caller is asserting internal/kernel privilege (still verified) */
	bool bClaimsKernelPrivilege = false;

	// =========================================================================
	// PLAN CONTEXT FIELDS (2026-02-02)
	// Required for mutating tools in PROOF mode to enable replay/audit.
	// Must be populated by governed execution lanes (agent/step, plans/execute).
	// =========================================================================

	/** Hash of the plan this tool call is part of (required for mutators in PROOF mode) */
	FString PlanHash;

	/** How the plan hash was computed (e.g., "md5_plan_contents", "sha256_plan_json") */
	FString PlanHashKind;

	/** If plan hash computation failed, this contains the error */
	FString PlanHashCanonicalizationError;

	/** Step index within the plan (required for mutators in PROOF mode) */
	int32 PlanStepIndex = -1;

	/** Plan ID for correlation */
	FString PlanId;

	/** Execution lane: "plan" (governed), "direct" (ad-hoc), "internal" */
	FString ExecutionLane;

	// =========================================================================
	// UNIFIED TIMELINE CORRELATION (P4 Fix 2026-02-04)
	// This ID links all events across C++, Python, and proof streams.
	// Must be propagated through all execution paths for full traceability.
	// =========================================================================

	/** Unified correlation ID - links this execution to intent/planning/evidence streams */
	FString CorrelationId;

	/** Timestamp when this call was created (UTC) */
	FDateTime CreatedAt;

	/** HTTP request ID that triggered this call (if from HTTP server) */
	FString HttpRequestId;

	/** Session ID for editor session correlation */
	FString EditorSessionId;

	// Execution tracking - populated after tool is executed
	UPROPERTY(BlueprintReadOnly, Category = "Tool")
	bool bExecuted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Tool")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Tool")
	FString ExecutionResult;
};

/**
 * Result of executing a tool
 */
USTRUCT(BlueprintType)
struct FClaudeToolResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	FString ToolUseId;

	/** Tool name that was executed (populated by executor) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	FString ToolName;

	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	FString Result;

	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	FString ErrorMessage;

	// Execution timing (populated by ExecuteTool)
	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	double ExecutionTimeMs = 0.0;

	// True if tool exceeded its timeout threshold
	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	bool bTimedOut = false;

	// The timeout that was applied (for debugging)
	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	int32 TimeoutThresholdMs = 0;

	// True if tool was BLOCKED due to trust penalty (policy enforcement)
	// When true, the tool was NOT executed - caller should select alternative
	UPROPERTY(BlueprintReadWrite, Category = "Tool")
	bool bBlockedByTrust = false;

	// Structured metadata for programmatic access (e.g., actor_path, actor_name)
	// Not serialized to JSON result string, but available for internal use
	TMap<FString, FString> Metadata;

	// Witness data for PROOF mode - state change evidence for contract verification
	// Tools populate this with their required witness keys (defined in tool contracts)
	TMap<FString, FString> Witness;

	// =========================================================================
	// GOVERNANCE FIELDS - Typed execution metadata for UI rendering
	// These fields are the source of truth - UI must NOT parse/infer from strings
	// =========================================================================

	/** Tool risk level from registry (default Unknown until populated) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Governance")
	EToolRisk ToolRisk = EToolRisk::Unknown;

	/** Policy decision: allowed, needs_confirmation, forbidden, failed_validation */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Governance")
	ERiftbornPolicyDecision PolicyDecision = ERiftbornPolicyDecision::Allowed;

	/** Human-readable reason for policy decision (empty if allowed) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Governance")
	FString PolicyReason;

	/** Backend-generated summary of what the tool did (NOT UI-inferred) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Governance")
	FString SummaryText;

	/** True if SummaryText is auto-generated fallback (audit: mutation tools should have real summaries) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Governance")
	bool bSummaryIsFallback = true;

	/** Structured mutation receipt JSON for before/after state, affected entities, and verification hints */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Mutation")
	FString ReceiptJson;

	/** True when the agent loop should perform a post-execution visual verification
	 *  (capture viewport + analyze) before accepting this result. Set automatically
	 *  for successful Mutation / Destructive tools; tools may opt out by setting
	 *  it false explicitly when their effect is non-visual (config, CVar, etc). */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Mutation")
	bool bVisionVerifyRecommended = false;

	// =========================================================================
	// UNDO INFO - Honest about reversibility
	// =========================================================================

	/** Whether this operation can be undone */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Undo")
	bool bCanUndo = false;

	/** Transaction/snapshot ID for undo (empty if no undo available) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Undo")
	FString UndoToken;

	/** Reversibility classification */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Undo")
	ERiftbornReversibility Reversibility = ERiftbornReversibility::Unknown;

	// =========================================================================
	// PROOF INFO - Dual-hash system for semantic + witness proofs
	// =========================================================================
	//
	// SemanticHash: Deterministic - same call with same result hashes identically
	//   Covers: tool, args (sorted), success, result_hash, risk
	//   Use for: equivalence checking, replay verification, caching
	//
	// WitnessHash: Event-specific - unique per execution instance
	//   Covers: semantic_hash + tool_use_id + timestamp + execution_ms
	//   Use for: audit trails, non-repudiation, evidence
	// =========================================================================

	/** Semantic hash - deterministic, excludes time (for equivalence checking) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Proof")
	FString SemanticHash;

	/** Witness hash - includes timestamps (for audit/evidence) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Proof")
	FString WitnessHash;

	/** Legacy ProofId field - now set to WitnessHash for backward compatibility */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Proof")
	FString ProofId;

	/** Local path to proof bundle (empty if no proof emitted) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Proof")
	FString ProofBundlePath;

	// =========================================================================
	// CLAIMS - Typed assertions about what this tool did (for verification)
	// =========================================================================

	/** Claims made by this tool execution (verified post-execution) */
	TArray<struct FToolClaim> Claims;

	/** Proof schema version (e.g., "v4", "trajectory_v1") */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Proof")
	FString ProofSchemaVersion;

	/** API endpoint to replay/fetch this execution (e.g., "/riftborn/jobs/{id}") */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Proof")
	FString ReplayPointer;

	// =========================================================================
	// FALLBACK TRACE - For proof reproducibility when alternatives are used
	// =========================================================================

	/** True if this tool was chosen as a fallback after another tool failed/was blocked */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Fallback")
	bool bIsFallback = false;

	/** Original tool that was attempted before this fallback (empty if not fallback) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Fallback")
	FString OriginalToolAttempted;

	/** Why the original tool failed or was blocked */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Fallback")
	FString FallbackReason;

	/** All alternatives that were offered (for replay verification) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Fallback")
	TArray<FString> AlternativesOffered;

	/** Response envelope schema version for client evolution */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Governance")
	FString ResponseSchemaVersion = TEXT("v1");

	/** Autonomy level required for this tool (L0-L6) */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Governance")
	int32 RequiredAutonomyLevel = 0;

	/** Current user autonomy level when tool was executed */
	UPROPERTY(BlueprintReadWrite, Category = "Tool|Governance")
	int32 CurrentAutonomyLevel = 0;

  // =========================================================================
  // SERIALIZATION
  // =========================================================================

  /** Canonical JSON serializer for all standard result fields.
   *  Does NOT include envelope fields (job_id, client_id, gates, receipt) —
   *  callers overlay those after calling ToJson().
   *  Includes: ok, tool, result/error, metadata, governance, undo, summary,
   *  proof, witness, schema_version, execution_time_ms. */
  TSharedPtr<FJsonObject> ToJson() const;

  // =========================================================================
  // CONVENIENCE FACTORY METHODS - For tool modules
  // =========================================================================

  /** Create a success result with the given message */
  static FClaudeToolResult Success(const FString& Message)
  {
    FClaudeToolResult R;
    R.bSuccess = true;
    R.Result = Message;
    return R;
  }

  /** Create a failure result with the given error message */
  static FClaudeToolResult Failure(const FString& Message)
  {
    FClaudeToolResult R;
    R.bSuccess = false;
    R.ErrorMessage = Message;
    R.Result = Message;
    return R;
  }
};

/**
 * Delegate for tool execution (new format)
 */
DECLARE_DELEGATE_RetVal_OneParam(FClaudeToolResult, FOnExecuteTool, const FClaudeToolCall&);

/**
 * Async tool dispatch (game-thread-friendly). Tools that would otherwise
 * block the game thread waiting for a GPU readback, an LLM round-trip, or
 * any other latent operation register via FClaudeToolRegistry::RegisterToolAsync
 * and use this delegate. The handler must invoke OnComplete (exactly once,
 * on the game thread) when the result is ready — the caller does NOT block
 * waiting for it; control returns to the editor so frames can present and
 * input lands. The continuation re-enters the agentic loop when ready.
 *
 * Any tool can be registered as either sync or async; the agentic loop
 * picks the right dispatch path automatically. Sync remains the default
 * because most tools are CPU-only on the GT and complete in microseconds —
 * for those, the extra ceremony of async would add latency without benefit.
 */
DECLARE_DELEGATE_OneParam(FOnAsyncToolComplete, FClaudeToolResult);
DECLARE_DELEGATE_TwoParams(FOnExecuteToolAsync, const FClaudeToolCall&, FOnAsyncToolComplete);

/**
 * Legacy tool function signature (for backward compatibility)
 * Tools using this signature return FString and take TMap<FString, FString> arguments
 */
typedef FString (*FLegacyToolFunction)(const TMap<FString, FString>&);
