// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// EnvironmentFingerprint.h - Context fingerprinting for scoped regret decay
//
// Regret events are tagged with an environment fingerprint.
// Penalties decay or fork when context changes significantly.
//
// Extracted from RegretScope.h for modularity.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Misc/EngineVersion.h"
#include "EngineUtils.h"  // For TActorIterator
#include "Engine/World.h"
#include "NavigationSystem.h"

#include "RegretEnums.h"  // For ECausalSurface

// Forward declaration for Editor-only functionality
#if WITH_EDITOR
#include "Editor.h"
#endif

// =============================================================================
// CONTEXT FINGERPRINT - Regret must be scoped to its environment
// =============================================================================
//
// Problem: A tool that failed in Map A, Version X should not be penalized
// forever in Map B, Version Y with different constraints.
//
// Solution: Regret events are tagged with an environment fingerprint.
// Penalties decay or fork when context changes significantly.
//
// FINGERPRINT COMPONENTS:
// - Engine version hash (major breaking changes)
// - Plugin version hash (capability changes)
// - Map signature (level-specific quirks)
// - World type (PIE vs Editor vs Standalone)
// - Lighting profile (affects visual tools)
// - Actor topology hash (structural changes)
//
// DECAY RULES:
// - Same context: full penalty applies
// - Compatible context (same engine, different map): 50% penalty
// - Incompatible context (different engine version): 0% penalty (fresh start)
// =============================================================================

/**
 * Environment fingerprint for context-scoped regret.
 * Penalties decay based on context similarity.
 */
struct FEnvironmentFingerprint
{
    // Engine/Plugin versioning
    FString EngineVersion;          // e.g., "5.7.1"
    int32 EngineChangelist = 0;     // CL number for precise matching
    FString PluginVersion;          // e.g., "1.0.0"
    int32 PluginVersionNumber = 0;  // Numeric for comparison

    // World context
    FString MapName;                // Current map name
    FString MapPackagePath;         // Full package path
    uint32 MapContentHash = 0;      // Hash of map content (actor count, types)
    FString WorldType;              // "Editor", "PIE", "Game"

    // Structural fingerprint
    int32 ActorCount = 0;           // Actor topology indicator
    uint32 ActorTypeHash = 0;       // Hash of actor class distribution

    // Lighting/rendering context (affects visual tool behavior)
    FString LightingScenario;       // Current lighting profile
    bool bHasNavMesh = false;       // Navigation context

    /** Generate fingerprint from current world state */
    static FEnvironmentFingerprint CaptureCurrentEnvironment()
    {
        FEnvironmentFingerprint Fp;

        // Engine version
        Fp.EngineVersion = FEngineVersion::Current().ToString();
        Fp.EngineChangelist = FEngineVersion::Current().GetChangelist();

        // Plugin version - would need to be injected from module
        Fp.PluginVersion = TEXT("1.0.0");  // Default, override from module
        Fp.PluginVersionNumber = 1;

        // World context (must be on game thread, editor only)
#if WITH_EDITOR
        if (GEditor)
        {
            UWorld* World = GEditor->GetEditorWorldContext().World();
            if (World)
            {
                Fp.MapName = World->GetMapName();
                Fp.MapPackagePath = World->GetOutermost()->GetName();
                Fp.WorldType = World->WorldType == EWorldType::Editor ? TEXT("Editor") :
                               World->WorldType == EWorldType::PIE ? TEXT("PIE") :
                               World->WorldType == EWorldType::Game ? TEXT("Game") : TEXT("Other");

                // Count actors for topology hash
                int32 Count = 0;
                uint32 TypeHash = 0;
                for (TActorIterator<AActor> It(World); It; ++It)
                {
                    Count++;
                    TypeHash ^= GetTypeHash(It->GetClass()->GetFName());
                }
                Fp.ActorCount = Count;
                Fp.ActorTypeHash = TypeHash;

                // Check for navmesh
                Fp.bHasNavMesh = World->GetNavigationSystem() != nullptr;
            }
        }
#endif

        return Fp;
    }

    /** Calculate similarity score with another fingerprint (0.0 = incompatible, 1.0 = identical) */
    float GetSimilarityScore(const FEnvironmentFingerprint& Other) const
    {
        float Score = 0.0f;
        float TotalWeight = 0.0f;

        // Engine version match (critical - different engine = different behavior)
        const float EngineWeight = 3.0f;
        TotalWeight += EngineWeight;
        if (EngineChangelist == Other.EngineChangelist)
        {
            Score += EngineWeight;  // Exact match
        }
        else if (EngineVersion.Left(3) == Other.EngineVersion.Left(3))
        {
            Score += EngineWeight * 0.7f;  // Same major.minor
        }
        // Different major version = 0 points

        // Plugin version match (important for capability changes)
        const float PluginWeight = 2.0f;
        TotalWeight += PluginWeight;
        if (PluginVersionNumber == Other.PluginVersionNumber)
        {
            Score += PluginWeight;
        }
        else if (FMath::Abs(PluginVersionNumber - Other.PluginVersionNumber) <= 1)
        {
            Score += PluginWeight * 0.5f;  // Adjacent versions
        }

        // Map match (significant - different maps have different constraints)
        const float MapWeight = 2.0f;
        TotalWeight += MapWeight;
        if (MapName == Other.MapName)
        {
            Score += MapWeight;
        }
        else if (MapPackagePath.Contains(Other.MapPackagePath) || Other.MapPackagePath.Contains(MapPackagePath))
        {
            Score += MapWeight * 0.3f;  // Related maps
        }

        // World type match (moderate - PIE vs Editor can differ)
        const float WorldWeight = 1.0f;
        TotalWeight += WorldWeight;
        if (WorldType == Other.WorldType)
        {
            Score += WorldWeight;
        }

        // Actor topology (rough indicator of structural similarity)
        const float TopologyWeight = 1.0f;
        TotalWeight += TopologyWeight;
        if (ActorTypeHash == Other.ActorTypeHash)
        {
            Score += TopologyWeight;  // Same actor distribution
        }
        else if (FMath::Abs(ActorCount - Other.ActorCount) < 100)
        {
            Score += TopologyWeight * 0.5f;  // Similar scale
        }

        return TotalWeight > 0.0f ? (Score / TotalWeight) : 0.0f;
    }

    /** Is this context compatible enough for penalty to apply? */
    bool IsCompatibleWith(const FEnvironmentFingerprint& Other) const
    {
        // Incompatible = different major engine version
        if (EngineVersion.Left(3) != Other.EngineVersion.Left(3))
        {
            return false;
        }

        // Otherwise compatible (penalty may decay but still applies)
        return true;
    }

    /**
     * Get penalty decay factor based on context similarity (0.0 to 1.0).
     * LEGACY: Does not respect causal surface. Use GetCausalDecayFactor instead.
     */
    float GetPenaltyDecayFactor(const FEnvironmentFingerprint& Other) const
    {
        if (!IsCompatibleWith(Other))
        {
            return 0.0f;  // Incompatible = no penalty carries over
        }

        float Similarity = GetSimilarityScore(Other);

        // Decay curve: identical context = 1.0, minimum compatible = 0.2
        // y = 0.2 + 0.8 * similarity
        return 0.2f + 0.8f * Similarity;
    }

    /**
     * Get penalty decay factor respecting causal surface.
     * This is the anti-laundering version.
     *
     * Key insight: If failure is INVARIANT (doesn't depend on context),
     * then context changes should NOT reduce penalty. No laundering.
     */
    float GetCausalDecayFactor(const FEnvironmentFingerprint& Other, ECausalSurface CausalSurface) const
    {
        // INVARIANT failures never decay - this is the anti-laundering rule
        if (CausalSurface == ECausalSurface::Invariant || CausalSurface == ECausalSurface::Unknown)
        {
            // Still check version compatibility (different engine = truly different system)
            if (!IsCompatibleWith(Other))
            {
                return 0.0f;  // New engine version = fresh start even for invariant
            }
            return 1.0f;  // No decay - penalty persists regardless of map/topology changes
        }

        // For context-dependent failures, only decay based on RELEVANT factors
        switch (CausalSurface)
        {
            case ECausalSurface::MapDependent:
            {
                // Only map similarity matters
                if (MapName == Other.MapName)
                {
                    return 1.0f;  // Same map = full penalty
                }
                else if (MapPackagePath.Contains(Other.MapPackagePath) ||
                         Other.MapPackagePath.Contains(MapPackagePath))
                {
                    return 0.5f;  // Related map = partial penalty
                }
                return 0.2f;  // Different map = minimal penalty (might be legitimately fixed)
            }

            case ECausalSurface::TopologyDependent:
            {
                // Actor topology matters
                if (ActorTypeHash == Other.ActorTypeHash)
                {
                    return 1.0f;  // Same topology = full penalty
                }
                else if (FMath::Abs(ActorCount - Other.ActorCount) < 50)
                {
                    return 0.7f;  // Similar topology
                }
                return 0.3f;  // Very different topology
            }

            case ECausalSurface::VersionDependent:
            {
                // Engine/plugin version matters
                if (EngineChangelist == Other.EngineChangelist &&
                    PluginVersionNumber == Other.PluginVersionNumber)
                {
                    return 1.0f;  // Same version = full penalty
                }
                else if (PluginVersionNumber != Other.PluginVersionNumber)
                {
                    return 0.3f;  // Different plugin version = might be fixed
                }
                return 0.5f;  // Same plugin, different engine minor
            }

            case ECausalSurface::WorldTypeDependent:
            {
                // PIE vs Editor matters
                if (WorldType == Other.WorldType)
                {
                    return 1.0f;  // Same world type = full penalty
                }
                return 0.2f;  // Different world type = might not apply
            }

            case ECausalSurface::MultiFactorDependent:
            {
                // Use full similarity score (legacy behavior)
                return GetPenaltyDecayFactor(Other);
            }

            default:
                return 1.0f;  // Conservative: no decay
        }
    }

    /** Serialize to JSON */
    TSharedPtr<FJsonObject> ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject);

        Obj->SetStringField(TEXT("engine_version"), EngineVersion);
        Obj->SetNumberField(TEXT("engine_changelist"), EngineChangelist);
        Obj->SetStringField(TEXT("plugin_version"), PluginVersion);
        Obj->SetNumberField(TEXT("plugin_version_number"), PluginVersionNumber);
        Obj->SetStringField(TEXT("map_name"), MapName);
        Obj->SetStringField(TEXT("map_package_path"), MapPackagePath);
        Obj->SetNumberField(TEXT("map_content_hash"), MapContentHash);
        Obj->SetStringField(TEXT("world_type"), WorldType);
        Obj->SetNumberField(TEXT("actor_count"), ActorCount);
        Obj->SetNumberField(TEXT("actor_type_hash"), ActorTypeHash);
        Obj->SetStringField(TEXT("lighting_scenario"), LightingScenario);
        Obj->SetBoolField(TEXT("has_navmesh"), bHasNavMesh);

        return Obj;
    }

    /** Deserialize from JSON */
    static FEnvironmentFingerprint FromJson(const TSharedPtr<FJsonObject>& Obj)
    {
        FEnvironmentFingerprint Fp;

        Obj->TryGetStringField(TEXT("engine_version"), Fp.EngineVersion);

        double Cl = 0;
        Obj->TryGetNumberField(TEXT("engine_changelist"), Cl);
        Fp.EngineChangelist = static_cast<int32>(Cl);

        Obj->TryGetStringField(TEXT("plugin_version"), Fp.PluginVersion);

        double Pv = 0;
        Obj->TryGetNumberField(TEXT("plugin_version_number"), Pv);
        Fp.PluginVersionNumber = static_cast<int32>(Pv);

        Obj->TryGetStringField(TEXT("map_name"), Fp.MapName);
        Obj->TryGetStringField(TEXT("map_package_path"), Fp.MapPackagePath);

        double Mch = 0;
        Obj->TryGetNumberField(TEXT("map_content_hash"), Mch);
        Fp.MapContentHash = static_cast<uint32>(Mch);

        Obj->TryGetStringField(TEXT("world_type"), Fp.WorldType);

        double Ac = 0;
        Obj->TryGetNumberField(TEXT("actor_count"), Ac);
        Fp.ActorCount = static_cast<int32>(Ac);

        double Ath = 0;
        Obj->TryGetNumberField(TEXT("actor_type_hash"), Ath);
        Fp.ActorTypeHash = static_cast<uint32>(Ath);

        Obj->TryGetStringField(TEXT("lighting_scenario"), Fp.LightingScenario);
        Obj->TryGetBoolField(TEXT("has_navmesh"), Fp.bHasNavMesh);

        return Fp;
    }

    /** Create a fingerprint hash for quick comparison */
    uint32 GetQuickHash() const
    {
        uint32 Hash = 0;
        Hash ^= GetTypeHash(EngineChangelist);
        Hash ^= GetTypeHash(PluginVersionNumber);
        Hash ^= GetTypeHash(MapName);
        Hash ^= ActorTypeHash;
        return Hash;
    }
};
