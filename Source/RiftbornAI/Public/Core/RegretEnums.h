// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// RegretEnums.h - Enum definitions for regret scope, causal surface, and exploration tiers
//
// Extracted from RegretScope.h for modularity.

#pragma once

#include "CoreMinimal.h"

// =============================================================================
// REGRET SCOPE - How severe is this regret?
// =============================================================================

/**
 * Regret scope classification.
 * Determines how penalty should be applied and whether it should decay.
 */
enum class ERegretScope : uint8
{
    /** Immediate outcome failure - tool produced wrong result */
    Tactical,

    /** Multi-step plan regressed overall goal progress */
    Strategic,

    /** Action reduced future optionality (closed doors) */
    Optionality,

    /** Currently bad but necessary for future success - DO NOT PENALIZE */
    Deferred,

    /** Unknown/unclassified - default to Tactical for safety */
    Unknown
};

/**
 * Classification weights by scope.
 * Lower weight = less impact on trust penalty.
 */
inline float GetRegretScopeWeight(ERegretScope Scope)
{
    switch (Scope)
    {
        case ERegretScope::Tactical:     return 1.0f;   // Full penalty
        case ERegretScope::Strategic:    return 1.5f;   // Higher - plan-level failure is worse
        case ERegretScope::Optionality:  return 2.0f;   // Highest - closing doors is catastrophic
        case ERegretScope::Deferred:     return 0.0f;   // No penalty - acceptable sacrifice
        case ERegretScope::Unknown:      return 1.0f;   // Default to tactical
        default:                         return 1.0f;
    }
}

inline FString RegretScopeToString(ERegretScope Scope)
{
    switch (Scope)
    {
        case ERegretScope::Tactical:     return TEXT("tactical");
        case ERegretScope::Strategic:    return TEXT("strategic");
        case ERegretScope::Optionality:  return TEXT("optionality");
        case ERegretScope::Deferred:     return TEXT("deferred");
        case ERegretScope::Unknown:      return TEXT("unknown");
        default:                         return TEXT("unknown");
    }
}

inline ERegretScope StringToRegretScope(const FString& Str)
{
    if (Str.Equals(TEXT("tactical"), ESearchCase::IgnoreCase))     return ERegretScope::Tactical;
    if (Str.Equals(TEXT("strategic"), ESearchCase::IgnoreCase))    return ERegretScope::Strategic;
    if (Str.Equals(TEXT("optionality"), ESearchCase::IgnoreCase))  return ERegretScope::Optionality;
    if (Str.Equals(TEXT("deferred"), ESearchCase::IgnoreCase))     return ERegretScope::Deferred;
    return ERegretScope::Unknown;
}

// =============================================================================
// CAUSAL SURFACE - What context factors does this failure ACTUALLY depend on?
// =============================================================================
//
// Problem: Penalty decay based on fingerprint similarity allows "laundering" -
// agent changes map / spawns noise actors to reduce similarity and escape penalty.
//
// Solution: Tag each failure with its CAUSAL SURFACE - which context factors
// actually caused the failure. Decay only respects changes to relevant factors.

/**
 * Causal surface classification - what context does this failure depend on?
 */
enum class ECausalSurface : uint8
{
    /** Failure is truly invariant - penalty never decays regardless of context */
    Invariant,

    /** Failure depends on map/level structure */
    MapDependent,

    /** Failure depends on actor topology/layout */
    TopologyDependent,

    /** Failure depends on engine/plugin version */
    VersionDependent,

    /** Failure depends on world type (PIE vs Editor) */
    WorldTypeDependent,

    /** Failure depends on multiple factors - use full fingerprint */
    MultiFactorDependent,

    /** Unknown - conservative: treat as invariant */
    Unknown
};

inline FString CausalSurfaceToString(ECausalSurface Surface)
{
    switch (Surface)
    {
        case ECausalSurface::Invariant:           return TEXT("invariant");
        case ECausalSurface::MapDependent:        return TEXT("map_dependent");
        case ECausalSurface::TopologyDependent:   return TEXT("topology_dependent");
        case ECausalSurface::VersionDependent:    return TEXT("version_dependent");
        case ECausalSurface::WorldTypeDependent:  return TEXT("world_type_dependent");
        case ECausalSurface::MultiFactorDependent: return TEXT("multi_factor_dependent");
        case ECausalSurface::Unknown:             return TEXT("unknown");
        default:                                  return TEXT("unknown");
    }
}

inline ECausalSurface StringToCausalSurface(const FString& Str)
{
    if (Str.Equals(TEXT("invariant"), ESearchCase::IgnoreCase))           return ECausalSurface::Invariant;
    if (Str.Equals(TEXT("map_dependent"), ESearchCase::IgnoreCase))       return ECausalSurface::MapDependent;
    if (Str.Equals(TEXT("topology_dependent"), ESearchCase::IgnoreCase))  return ECausalSurface::TopologyDependent;
    if (Str.Equals(TEXT("version_dependent"), ESearchCase::IgnoreCase))   return ECausalSurface::VersionDependent;
    if (Str.Equals(TEXT("world_type_dependent"), ESearchCase::IgnoreCase)) return ECausalSurface::WorldTypeDependent;
    if (Str.Equals(TEXT("multi_factor_dependent"), ESearchCase::IgnoreCase)) return ECausalSurface::MultiFactorDependent;
    return ECausalSurface::Unknown;
}

/**
 * Classify failure mode into causal surface.
 * This is the policy that prevents laundering.
 */
inline ECausalSurface ClassifyFailureCausalSurface(const FString& FailureMode)
{
    // INVARIANT - these failures don't depend on context, can't be laundered
    if (FailureMode.Contains(TEXT("no_delta")) ||
        FailureMode.Contains(TEXT("no_change")) ||
        FailureMode.Contains(TEXT("success_but_no_effect")) ||
        FailureMode.Contains(TEXT("tool_lied")) ||
        FailureMode.Contains(TEXT("fabricated")) ||
        FailureMode.Contains(TEXT("returned_wrong_type")) ||
        FailureMode.Contains(TEXT("json_parse_error")) ||
        FailureMode.Contains(TEXT("parameter_invalid")))
    {
        return ECausalSurface::Invariant;
    }

    // MAP DEPENDENT - changing map might legitimately fix these
    if (FailureMode.Contains(TEXT("collision")) ||
        FailureMode.Contains(TEXT("spawn_blocked")) ||
        FailureMode.Contains(TEXT("navmesh")) ||
        FailureMode.Contains(TEXT("no_valid_location")) ||
        FailureMode.Contains(TEXT("lighting")) ||
        FailureMode.Contains(TEXT("level_bounds")))
    {
        return ECausalSurface::MapDependent;
    }

    // TOPOLOGY DEPENDENT - actor layout matters
    if (FailureMode.Contains(TEXT("actor_not_found")) ||
        FailureMode.Contains(TEXT("reference_invalid")) ||
        FailureMode.Contains(TEXT("component_missing")) ||
        FailureMode.Contains(TEXT("parent_deleted")))
    {
        return ECausalSurface::TopologyDependent;
    }

    // VERSION DEPENDENT - engine/plugin version matters
    if (FailureMode.Contains(TEXT("timeout")) ||
        FailureMode.Contains(TEXT("api_changed")) ||
        FailureMode.Contains(TEXT("deprecated")) ||
        FailureMode.Contains(TEXT("not_implemented")))
    {
        return ECausalSurface::VersionDependent;
    }

    // WORLD TYPE DEPENDENT - PIE vs Editor matters
    if (FailureMode.Contains(TEXT("pie_only")) ||
        FailureMode.Contains(TEXT("editor_only")) ||
        FailureMode.Contains(TEXT("runtime_only")))
    {
        return ECausalSurface::WorldTypeDependent;
    }

    // Default to invariant (conservative - no laundering possible)
    return ECausalSurface::Invariant;
}

// =============================================================================
// CANONICAL FORMULA HASH - Cross-Language Consistency Enforcement
// =============================================================================
//
// This hash is computed from ALL canonical constants and formulas.
// If ANY constant changes (scope weights, decay factors, thresholds),
// this hash changes, and the Python verifier will fail.
//
// NEVER change this hash manually. It is computed by _verify_proof_bundle.py.

/** Canonical formula hash for cross-language verification */
constexpr const char* FORMULA_HASH = "fe1a1928bcf421bb";

/** Get the formula hash for embedding in proof bundles */
inline FString GetFormulaHash()
{
    return FString(UTF8_TO_TCHAR(FORMULA_HASH));
}

// =============================================================================
// EXPLORATION TIER - Risk levels for exploration contracts
// =============================================================================

/**
 * Exploration contract types.
 * Different risk levels with different bounds.
 */
enum class EExplorationTier : uint8
{
    /** Low risk - try a different approach to same goal */
    Tactical,

    /** Medium risk - try a new tool or capability */
    Capability,

    /** High risk - try something the agent has never done */
    Novel,

    /** Experimental - deliberately testing failure modes */
    Experimental
};

inline float GetExplorationBudget(EExplorationTier Tier)
{
    switch (Tier)
    {
        case EExplorationTier::Tactical:      return 2.0f;   // 2 regrets before contract exhausted
        case EExplorationTier::Capability:    return 4.0f;   // 4 regrets
        case EExplorationTier::Novel:         return 6.0f;   // 6 regrets
        case EExplorationTier::Experimental:  return 10.0f;  // 10 regrets (testing mode)
        default:                              return 2.0f;
    }
}

inline FString ExplorationTierToString(EExplorationTier Tier)
{
    switch (Tier)
    {
        case EExplorationTier::Tactical:      return TEXT("tactical");
        case EExplorationTier::Capability:    return TEXT("capability");
        case EExplorationTier::Novel:         return TEXT("novel");
        case EExplorationTier::Experimental:  return TEXT("experimental");
        default:                              return TEXT("tactical");
    }
}
