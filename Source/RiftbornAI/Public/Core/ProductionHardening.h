// Copyright RiftbornAI. All Rights Reserved.
// ProductionHardening.h - Security and abuse prevention constants
//
// This file defines hardened limits and policies.
// All values are auditable and tunable via config.

#pragma once

#include "CoreMinimal.h"

// =============================================================================
// HTTP ENDPOINT HARDENING
// =============================================================================

namespace RiftbornSecurity
{
    // Request size limits
    static constexpr int32 MAX_REQUEST_BODY_BYTES = 1024 * 1024;  // 1MB max
    static constexpr int32 MAX_ARGUMENTS_JSON_BYTES = 64 * 1024;  // 64KB max for tool args
    static constexpr int32 MAX_TOOL_NAME_LENGTH = 128;            // Reasonable tool name
    static constexpr int32 MAX_GOAL_ID_LENGTH = 64;               // UUID + margin
    
    // Rate limiting
    static constexpr int32 RATE_LIMIT_REQUESTS_PER_SECOND = 10;   // Per client
    static constexpr int32 RATE_LIMIT_BURST_SIZE = 20;            // Burst allowance
    static constexpr int32 RATE_LIMIT_WINDOW_SECONDS = 60;        // Window for tracking
    
    // Timeout limits
    static constexpr int32 MIN_TIMEOUT_MS = 100;                  // Minimum allowed
    static constexpr int32 MAX_TIMEOUT_MS = 300000;               // 5 minutes max
    static constexpr int32 DEFAULT_TIMEOUT_MS = 30000;            // 30 seconds default
    
    // Tool name validation (alphanumeric, underscore, hyphen only)
    inline bool IsValidToolNameChar(TCHAR c)
    {
        return FChar::IsAlnum(c) || c == '_' || c == '-';
    }
    
    inline bool IsValidToolName(const FString& Name)
    {
        if (Name.IsEmpty() || Name.Len() > MAX_TOOL_NAME_LENGTH)
        {
            return false;
        }
        for (TCHAR c : Name)
        {
            if (!IsValidToolNameChar(c))
            {
                return false;
            }
        }
        return true;
    }
}

// =============================================================================
// TOOL PERMISSION TIERS
// =============================================================================

enum class EToolPermissionTier : uint8
{
    ReadOnly,      // Query state, no modifications
    Mutating,      // Modify actors, properties
    Destructive,   // Delete assets, actors
    Administrative // System-level operations
};

inline FString ToolPermissionTierToString(EToolPermissionTier Tier)
{
    switch (Tier)
    {
        case EToolPermissionTier::ReadOnly: return TEXT("ReadOnly");
        case EToolPermissionTier::Mutating: return TEXT("Mutating");
        case EToolPermissionTier::Destructive: return TEXT("Destructive");
        case EToolPermissionTier::Administrative: return TEXT("Administrative");
    }
    return TEXT("Unknown");
}

// =============================================================================
// PRODUCTION SAFETY FLAGS
// =============================================================================

namespace RiftbornProductionFlags
{
    // These flags control safety behaviors in production
    
    /** Block destructive tools by default (require explicit enable) */
    static constexpr bool BLOCK_DESTRUCTIVE_BY_DEFAULT = true;
    
    /** Require explicit lossy adaptation permission */
    static constexpr bool REQUIRE_EXPLICIT_LOSSY_ALLOW = true;
    
    /** Log all tool executions to audit trail */
    static constexpr bool AUDIT_ALL_EXECUTIONS = true;
    
    /** Refuse unknown fields in JSON requests */
    static constexpr bool REJECT_UNKNOWN_JSON_FIELDS = true;
    
    /** Validate tool names against registry */
    static constexpr bool VALIDATE_TOOL_REGISTRY = true;
    
    /** Enable request size enforcement */
    static constexpr bool ENFORCE_REQUEST_LIMITS = true;
    
    /** Enable rate limiting */
    static constexpr bool ENFORCE_RATE_LIMITS = true;   // Enabled for production safety
}

// =============================================================================
// REFUSAL REASON CODES
// =============================================================================

enum class ERefusalReason : uint8
{
    None,
    TrustPenaltyExceeded,
    NoAlternativeAvailable,
    LossyAdaptationDisallowed,
    ToolNotInRegistry,
    PermissionDenied,
    RateLimitExceeded,
    RequestTooLarge,
    InvalidToolName,
    InvalidArguments,
    GoalHalted,
    SystemError
};

inline FString RefusalReasonToString(ERefusalReason Reason)
{
    switch (Reason)
    {
        case ERefusalReason::None: return TEXT("None");
        case ERefusalReason::TrustPenaltyExceeded: return TEXT("TrustPenaltyExceeded");
        case ERefusalReason::NoAlternativeAvailable: return TEXT("NoAlternativeAvailable");
        case ERefusalReason::LossyAdaptationDisallowed: return TEXT("LossyAdaptationDisallowed");
        case ERefusalReason::ToolNotInRegistry: return TEXT("ToolNotInRegistry");
        case ERefusalReason::PermissionDenied: return TEXT("PermissionDenied");
        case ERefusalReason::RateLimitExceeded: return TEXT("RateLimitExceeded");
        case ERefusalReason::RequestTooLarge: return TEXT("RequestTooLarge");
        case ERefusalReason::InvalidToolName: return TEXT("InvalidToolName");
        case ERefusalReason::InvalidArguments: return TEXT("InvalidArguments");
        case ERefusalReason::GoalHalted: return TEXT("GoalHalted");
        case ERefusalReason::SystemError: return TEXT("SystemError");
    }
    return TEXT("Unknown");
}

// Human-readable explanations for refusals
inline FString GetRefusalExplanation(ERefusalReason Reason, const FString& Context = TEXT(""))
{
    switch (Reason)
    {
        case ERefusalReason::None:
            return TEXT("No refusal");
            
        case ERefusalReason::TrustPenaltyExceeded:
            return FString::Printf(
                TEXT("Tool '%s' is blocked because its trust penalty exceeds threshold (3.0). "
                     "This happens when a tool repeatedly produces unexpected outcomes. "
                     "To unblock: reset penalty via /agent/reset-penalty or wait for decay."),
                *Context);
            
        case ERefusalReason::NoAlternativeAvailable:
            return FString::Printf(
                TEXT("Tool '%s' is blocked and no equivalent tool is available. "
                     "The agent cannot substitute because no other tool implements the same capability. "
                     "To resolve: provide alternative tools in request, or reset the blocked tool's penalty."),
                *Context);
            
        case ERefusalReason::LossyAdaptationDisallowed:
            return FString::Printf(
                TEXT("Substituting '%s' would lose semantic data and lossy adaptation is not permitted. "
                     "Set allow_lossy=true in request to permit, or use a more expressive tool."),
                *Context);
            
        case ERefusalReason::ToolNotInRegistry:
            return FString::Printf(
                TEXT("Tool '%s' is not found in the registry. This tool is not registered. "
                     "To unblock: check spelling of the tool name or register the tool in C++ if it should exist."),
                *Context);
            
        case ERefusalReason::PermissionDenied:
            return FString::Printf(
                TEXT("Tool '%s' requires permission tier '%s' which is not currently enabled."),
                *Context, *Context);  // Context should include tier info
            
        case ERefusalReason::RateLimitExceeded:
            return TEXT("Request rate limit exceeded. Wait before sending more requests.");
            
        case ERefusalReason::RequestTooLarge:
            return TEXT("Request body exceeds size limit. Reduce argument size.");
            
        case ERefusalReason::InvalidToolName:
            return FString::Printf(
                TEXT("Tool name '%s' contains invalid characters or is empty. "
                     "Valid tool names use only alphanumeric, underscore, or hyphen characters. "
                     "To unblock: provide a valid tool name."),
                *Context);
            
        case ERefusalReason::InvalidArguments:
            return FString::Printf(
                TEXT("Arguments validation failed for tool '%s'. "
                     "Required fields may be missing or have invalid values. "
                     "To unblock: provide all required fields and include valid JSON arguments. "
                     "Add the missing required fields to your request."),
                *Context);
            
        case ERefusalReason::GoalHalted:
            return TEXT("This goal has been halted by policy. No further steps will be executed.");
            
        case ERefusalReason::SystemError:
            return FString::Printf(TEXT("Internal system error: %s"), *Context);
    }
    return TEXT("Unknown refusal reason");
}

// =============================================================================
// SEMANTIC LOSS TRACKING
// =============================================================================

struct FSemanticLossRecord
{
    FString FromTool;
    FString ToTool;
    TArray<FString> DroppedFields;
    FDateTime Timestamp;
    bool bWasAllowed = false;

    TSharedPtr<FJsonObject> ToJson() const
    {
        TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
        Json->SetStringField(TEXT("from_tool"), FromTool);
        Json->SetStringField(TEXT("to_tool"), ToTool);
        Json->SetBoolField(TEXT("was_allowed"), bWasAllowed);
        Json->SetStringField(TEXT("timestamp"), Timestamp.ToIso8601());

        TArray<TSharedPtr<FJsonValue>> DroppedArray;
        for (const FString& Field : DroppedFields)
        {
            DroppedArray.Add(MakeShared<FJsonValueString>(Field));
        }
        Json->SetArrayField(TEXT("dropped_fields"), DroppedArray);

        return Json;
    }
};

// =============================================================================
// AGENTIC LOOP LIMITS
// =============================================================================

namespace RiftbornAgentLimits
{
    /**
     * Maximum iterations for agentic tool loops.
     * Safety limit to prevent infinite loops in AI-driven tool chains.
     * Each iteration = 1 LLM call + potential tool execution.
     * At 20 iterations with ~5s per iteration = ~100s max per task.
     *
     * Configurable via DefaultRiftbornAI.ini: MaxAgenticIterations
     */
    static constexpr int32 MAX_AGENTIC_ITERATIONS = 20;

    /**
     * Conversation history messages to retain after pruning.
     * Keeps approximately 5 user/assistant exchanges for context.
     * Balance: More messages = better context but higher token cost/latency.
     *
     * Used by all providers (Claude, OpenAI, Gemini, Ollama).
     * Configurable via DefaultRiftbornAI.ini: MaxHistoryMessages
     */
    static constexpr int32 PRUNE_KEEP_LAST_N = 10;

    /**
     * Minimum sample size for pattern learning.
     * Patterns with fewer samples are considered unreliable.
     */
    static constexpr int32 MIN_PATTERN_SAMPLES = 3;

    /**
     * Maximum number of duplicate tool calls before breaking loop.
     * Prevents infinite loops where AI keeps calling same tool with same args.
     */
    static constexpr int32 MAX_DUPLICATE_TOOL_CALLS = 2;
}

// =============================================================================
// TOKEN LIMITS
// =============================================================================

namespace RiftbornTokenLimits
{
    /**
     * Default maximum tokens for LLM response generation.
     * Claude supports 4K-100K context; this limits response size.
     *
     * Constructor default for ClaudeProvider (matches high-capability models).
     * Can be overridden via DefaultRiftbornAI.ini: MaxResponseTokens
     */
    static constexpr int32 DEFAULT_MAX_TOKENS = 8192;

    /**
     * INI config fallback value (legacy compatibility).
     * Some older configs use 4096 - this provides fallback behavior.
     */
    static constexpr int32 INI_FALLBACK_MAX_TOKENS = 4096;

    /**
     * Maximum tokens to consider in context window for history pruning.
     * When history exceeds this, aggressive pruning kicks in.
     */
    static constexpr int32 MAX_CONTEXT_TOKENS = 32768;
}

// =============================================================================
// TIMEOUT LIMITS
// =============================================================================

namespace RiftbornTimeoutLimits
{
    /**
     * Default request timeout for LLM API calls in seconds.
     * Long timeout (120s) allows for complex reasoning tasks.
     *
     * Constructor default for all providers.
     * Can be overridden via DefaultRiftbornAI.ini: RequestTimeoutSeconds
     */
    static constexpr float DEFAULT_REQUEST_TIMEOUT_S = 120.0f;

    /**
     * Minimum request timeout (prevents misconfiguration).
     * Setting timeout below this will be clamped up.
     */
    static constexpr float MIN_REQUEST_TIMEOUT_S = 5.0f;

    /**
     * Maximum request timeout (prevents hung requests).
     * Setting timeout above this will be clamped down.
     */
    static constexpr float MAX_REQUEST_TIMEOUT_S = 300.0f;

    /**
     * Default tool execution timeout in milliseconds.
     * Tools that take longer than this are considered failed.
     */
    static constexpr int32 DEFAULT_TOOL_TIMEOUT_MS = 30000;

    /**
     * Python bridge connection timeout in seconds.
     * How long to wait when establishing connection to Python daemon.
     */
    static constexpr float BRIDGE_CONNECT_TIMEOUT_S = 5.0f;

    /**
     * Python bridge I/O timeout in seconds.
     * How long to wait for responses from Python daemon.
     */
    static constexpr float BRIDGE_IO_TIMEOUT_S = 30.0f;
}
