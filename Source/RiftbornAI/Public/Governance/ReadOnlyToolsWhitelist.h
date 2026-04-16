// Copyright Riftborn Studio. All Rights Reserved.
//
// Read-only tool classification with PROOF-mode-safe and DEV-mode variants.
//
// SECURITY CRITICAL (2026-02-02):
// Pattern-based classification (get_*, list_*, etc.) is a NAMING BYPASS VULNERABILITY.
// A tool named "get_delete_all_actors" would pass pattern checks but is mutating.
//
// Therefore:
//   - PROOF MODE: Use IsExplicitlySafe() - requires explicit declaration, no patterns
//   - DEV MODE: Can use IsReadOnlyTool() for convenience (patterns allowed)
//
// The pattern-based check exists ONLY for DEV mode convenience routing.
// It MUST NOT be used for PROOF mode authorization decisions.

#pragma once

#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "ClaudeToolUse.h"  // For FClaudeTool and EToolRisk
#include "ToolContract.h"   // For EContractRiskTier

/**
 * PROOF-MODE-SAFE: Returns true ONLY if tool has EXPLICIT safe declaration.
 *
 * This function is safe to use for authorization decisions in PROOF mode.
 * It does NOT use pattern matching - only explicit declarations count.
 *
 * A tool is explicitly safe if:
 *   1. EToolRisk::Safe declared in C++ registration, OR
 *   2. Contract exists with risk_tier SAFE or VERIFICATION
 *
 * @param ToolName The tool name to check
 * @param ToolDef The tool definition (may be null)
 * @param bHasContract Whether the tool has a contract
 * @param ContractRiskTier The contract's risk tier (if bHasContract is true)
 * @return true only if explicitly declared safe
 */
inline bool IsExplicitlySafe(
    const FString& ToolName,
    const FClaudeTool* ToolDef,
    bool bHasContract = false,
    EContractRiskTier ContractRiskTier = EContractRiskTier::Unknown)
{
    // Check 1: Explicit EToolRisk::Safe in C++ registration
    if (ToolDef && ToolDef->Risk == EToolRisk::Safe)
    {
        return true;
    }

    // Check 2: Contract with SAFE or VERIFICATION risk tier
    if (bHasContract)
    {
        if (ContractRiskTier == EContractRiskTier::Safe ||
            ContractRiskTier == EContractRiskTier::Verification)
        {
            return true;
        }
    }

    // No explicit declaration = not safe for PROOF mode purposes
    return false;
}

/**
 * DEV-MODE-ONLY: Pattern-based read-only check for convenience routing.
 *
 * WARNING: This function uses NAME PATTERNS which can be bypassed!
 * A tool named "get_delete_actors" would pass this check but is mutating.
 *
 * DO NOT use this for PROOF mode authorization. Use IsExplicitlySafe() instead.
 *
 * This exists for DEV mode convenience:
 *   - Allows quick iteration without contracts for read-only-looking tools
 *   - NOT a security boundary - just a heuristic for routing
 *
 * @param ToolName The tool name to check
 * @param ToolDef The tool definition (may be null)
 * @return true if tool appears read-only (explicit OR pattern-based)
 */
inline bool IsReadOnlyTool(const FString& ToolName, const FClaudeTool* ToolDef)
{
    // 1. If explicitly safe, always true
    if (ToolDef && ToolDef->Risk == EToolRisk::Safe)
    {
        return true;
    }

    // 2. Pattern matching (DEV MODE CONVENIENCE ONLY - NOT A SECURITY BOUNDARY)
    // These prefixes SUGGEST read-only but are NOT authoritative
    static const TArray<FString> ReadOnlyPrefixes = {
        TEXT("get_"),
        TEXT("list_"),
        TEXT("is_"),
        TEXT("has_"),
        TEXT("can_"),
        TEXT("find_"),
        TEXT("search_"),
        TEXT("check_"),
        TEXT("inspect_"),
        TEXT("query_"),
        TEXT("count_"),
        TEXT("diff_"),
        TEXT("compare_"),
        TEXT("validate_"),
    };

    // Exclusion patterns - known mutating patterns that look read-only
    // NOTE: This list can never be complete - that's why patterns are unsafe
    static const TArray<FString> MutatingPatterns = {
        TEXT("_and_set"),     // get_and_set is mutating
        TEXT("_or_create"),   // get_or_create is mutating
        TEXT("_and_delete"),  // find_and_delete is mutating
        TEXT("_and_modify"),  // get_and_modify is mutating
        TEXT("_and_update"),  // get_and_update is mutating
        TEXT("_and_remove"),  // find_and_remove is mutating
        TEXT("_and_fix"),     // check_and_fix is mutating
        TEXT("_and_repair"),  // validate_and_repair is mutating
        TEXT("_delete"),      // get_delete_* patterns
        TEXT("_destroy"),     // find_destroy_* patterns
        TEXT("_spawn"),       // get_spawn_* would be mutating
        TEXT("_create"),      // list_create_* would be mutating
    };

    // Check exclusions first - any match means NOT read-only
    for (const FString& Pattern : MutatingPatterns)
    {
        if (ToolName.Contains(Pattern))
        {
            return false;
        }
    }

    // Check prefixes
    for (const FString& Prefix : ReadOnlyPrefixes)
    {
        if (ToolName.StartsWith(Prefix))
        {
            return true;
        }
    }

    // 3. Legacy static whitelist (being phased out)
    static const TSet<FString> LegacyWhitelist = {
        TEXT("ping"),
        TEXT("resolve_asset"),
        TEXT("assert_actor_exists"),
    };

    return LegacyWhitelist.Contains(ToolName);
}

/**
 * Returns true if a tool name looks like it could be a misnamed mutator.
 * Used for CI adversarial testing.
 */
inline bool LooksLikeMisnamedMutator(const FString& ToolName)
{
    // Check if it starts with a read-only prefix
    static const TArray<FString> ReadOnlyPrefixes = {
        TEXT("get_"), TEXT("list_"), TEXT("is_"), TEXT("has_"),
        TEXT("find_"), TEXT("search_"), TEXT("check_"), TEXT("inspect_"),
    };

    bool bHasReadOnlyPrefix = false;
    for (const FString& Prefix : ReadOnlyPrefixes)
    {
        if (ToolName.StartsWith(Prefix))
        {
            bHasReadOnlyPrefix = true;
            break;
        }
    }

    if (!bHasReadOnlyPrefix)
    {
        return false;
    }

    // Check if it contains mutating keywords after the prefix
    static const TArray<FString> MutatingKeywords = {
        TEXT("delete"), TEXT("destroy"), TEXT("remove"), TEXT("spawn"),
        TEXT("create"), TEXT("modify"), TEXT("update"), TEXT("set"),
        TEXT("add"), TEXT("insert"), TEXT("write"), TEXT("save"),
    };

    for (const FString& Keyword : MutatingKeywords)
    {
        if (ToolName.Contains(Keyword))
        {
            return true; // e.g., "get_delete_actor" or "find_and_destroy"
        }
    }

    return false;
}
