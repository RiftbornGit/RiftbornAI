// Copyright 2026 RiftbornAI. All Rights Reserved.
//
// NetworkingClassifier — Phase 7 of the judgment layer.
//
// Classifies an actor/Blueprint by its likely networking role BEFORE it
// ships, so noisy or mis-owned replication shows up in review instead of
// at runtime. Pure classification: reads parent-class ancestry + bReplicates
// flag (UE 5.7 AActor has `uint8 bReplicates:1;` and `uint8 bNetLoadOnClient:1;`).
//
// Complements existing post-hoc tooling:
//   Private/Tools/IrisReplicationToolsModule.cpp — Iris replication policy ops
//   Private/Tools/Audit_NetReplication.cpp       — passive CPF_Net property scan
// Neither of those runs at creation time; this one does.
//
// Used by: (future) Tool_CreateBlueprint / Tool_SpawnActor hooks to tag newly
// created assets with their replication classification in the semantic graph
// (SemanticGraphDB.set_tag(pkg, "replication_mode", <role>)).
//
// Classification is intentionally conservative — unknown parents return
// "ambiguous" rather than guessing.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR || !UE_BUILD_SHIPPING
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#endif

/** Coarse networking role of a class. Matches typical UE multiplayer patterns. */
enum class ENetworkingRole : uint8
{
    /** Unclassifiable — unknown parent chain or non-AActor class. */
    Ambiguous = 0,

    /** Server-owned and replicated to clients. Bulk of gameplay state. */
    ServerAuthReplicated,

    /** Server-only. Never exists on clients (AGameMode, most subsystems). */
    ServerOnly,

    /** Client-predicted, server-authoritative (APawn/ACharacter typically). */
    ClientPredicted,

    /** Client-only. No replication, no server counterpart (AHUD, AMyPlayerCameraManager). */
    ClientOnly,

    /** Cosmetic — existence is local, no gameplay-relevant state to replicate. */
    CosmeticLocal
};

inline const TCHAR* NetworkingRoleToString(ENetworkingRole R)
{
    switch (R)
    {
        case ENetworkingRole::Ambiguous:            return TEXT("ambiguous");
        case ENetworkingRole::ServerAuthReplicated: return TEXT("server_auth_replicated");
        case ENetworkingRole::ServerOnly:           return TEXT("server_only");
        case ENetworkingRole::ClientPredicted:      return TEXT("client_predicted");
        case ENetworkingRole::ClientOnly:           return TEXT("client_only");
        case ENetworkingRole::CosmeticLocal:        return TEXT("cosmetic_local");
        default:                                    return TEXT("unknown");
    }
}

/** Reasoning attached to a classification. */
struct FNetworkingClassification
{
    ENetworkingRole Role = ENetworkingRole::Ambiguous;
    FString Reason;
    FString MatchedParent;   // which ancestor class triggered the match
    bool bReplicatesFlag = false;
};

class FNetworkingClassifier
{
public:
    /**
     * Classify by class-name string alone (e.g. "Character", "PlayerController",
     * "StaticMeshActor"). Used when we don't have a UClass* handy — e.g. when
     * classifying a tool-call payload before the Blueprint is instantiated.
     */
    static FNetworkingClassification ClassifyByClassName(const FString& ClassNameIn)
    {
        FNetworkingClassification R;
        const FString Name = NormalizeClassName(ClassNameIn);

        if (Name.IsEmpty())
        {
            R.Reason = TEXT("empty class name");
            return R;
        }

        // Order matters: most specific matches first.
        struct FMap { const TCHAR* Token; ENetworkingRole Role; const TCHAR* Reason; };
        static const FMap Map[] =
        {
            // Controllers — client-side prediction vs server-side AI.
            { TEXT("PlayerController"),    ENetworkingRole::ClientPredicted,      TEXT("APlayerController owns local input + replicates possession") },
            { TEXT("AIController"),        ENetworkingRole::ServerOnly,           TEXT("AAIController runs server-only") },

            // Game flow.
            { TEXT("GameMode"),            ENetworkingRole::ServerOnly,           TEXT("AGameMode exists only on server") },
            { TEXT("GameState"),           ENetworkingRole::ServerAuthReplicated, TEXT("AGameState replicates shared match state") },
            { TEXT("PlayerState"),         ENetworkingRole::ServerAuthReplicated, TEXT("APlayerState replicates per-player info") },

            // Pawn / Character — client-predicted movement.
            { TEXT("Character"),           ENetworkingRole::ClientPredicted,      TEXT("ACharacter uses client prediction via CharacterMovement") },
            { TEXT("Pawn"),                ENetworkingRole::ClientPredicted,      TEXT("APawn is typically client-predicted") },

            // UI / camera — client-only.
            { TEXT("HUD"),                 ENetworkingRole::ClientOnly,           TEXT("AHUD is local to the owning client") },
            { TEXT("PlayerCameraManager"), ENetworkingRole::ClientOnly,           TEXT("APlayerCameraManager is local") },
            { TEXT("UserWidget"),          ENetworkingRole::ClientOnly,           TEXT("UUserWidget is client-local UI") },
            { TEXT("WidgetComponent"),     ENetworkingRole::ClientOnly,           TEXT("UWidgetComponent renders locally") },

            // Physics / gameplay-visible world actors — default replicated if derived by designer.
            { TEXT("PhysicsVolume"),       ENetworkingRole::ServerAuthReplicated, TEXT("APhysicsVolume affects gameplay physics and replicates") },
            { TEXT("TriggerVolume"),       ENetworkingRole::ServerAuthReplicated, TEXT("ATriggerVolume fires gameplay events") },
            { TEXT("Volume"),              ENetworkingRole::ServerAuthReplicated, TEXT("AVolume-derived — usually server gameplay") },

            // Cosmetic.
            { TEXT("DecalActor"),          ENetworkingRole::CosmeticLocal,        TEXT("ADecalActor is typically cosmetic") },
            { TEXT("EmitterActor"),        ENetworkingRole::CosmeticLocal,        TEXT("Niagara/emitter actors are typically cosmetic") },
            { TEXT("NiagaraActor"),        ENetworkingRole::CosmeticLocal,        TEXT("ANiagaraActor is typically cosmetic") },
            { TEXT("Atmospheric"),         ENetworkingRole::CosmeticLocal,        TEXT("atmospheric actors are local") },
            { TEXT("SkyAtmosphere"),       ENetworkingRole::CosmeticLocal,        TEXT("sky/atmosphere actors are local") },
            { TEXT("SkyLight"),            ENetworkingRole::CosmeticLocal,        TEXT("sky light is local") },
            { TEXT("DirectionalLight"),    ENetworkingRole::CosmeticLocal,        TEXT("directional light is local") },
            { TEXT("PointLight"),          ENetworkingRole::CosmeticLocal,        TEXT("point light is local") },
            { TEXT("SpotLight"),           ENetworkingRole::CosmeticLocal,        TEXT("spot light is local") },
            { TEXT("RectLight"),           ENetworkingRole::CosmeticLocal,        TEXT("rect light is local") },
            { TEXT("PostProcessVolume"),   ENetworkingRole::CosmeticLocal,        TEXT("post-process volume is local") },
            { TEXT("ExponentialHeightFog"),ENetworkingRole::CosmeticLocal,        TEXT("fog actor is local") },
            { TEXT("BackgroundBlur"),      ENetworkingRole::CosmeticLocal,        TEXT("UI background is local") },

            // Static mesh actor — cosmetic unless explicitly replicated.
            { TEXT("StaticMeshActor"),     ENetworkingRole::CosmeticLocal,        TEXT("AStaticMeshActor doesn't replicate by default") },

            // Fallback for generic Actor — ambiguous. Let caller decide.
            { TEXT("Actor"),               ENetworkingRole::Ambiguous,            TEXT("Plain AActor — replication depends on bReplicates + designer intent") }
        };

        for (const FMap& Entry : Map)
        {
            if (Name.Contains(Entry.Token, ESearchCase::IgnoreCase))
            {
                R.Role = Entry.Role;
                R.Reason = Entry.Reason;
                R.MatchedParent = Entry.Token;
                return R;
            }
        }

        R.Reason = FString::Printf(TEXT("No known ancestor matched name '%s'"), *Name);
        return R;
    }

#if WITH_EDITOR || !UE_BUILD_SHIPPING
    /**
     * Classify a UClass* — walks the parent chain so "ABP_MyHero_C → ACharacter
     * → APawn → AActor" reaches ACharacter and returns ClientPredicted.
     * Also peeks the CDO's bReplicates flag for refinement.
     */
    static FNetworkingClassification ClassifyClass(UClass* Cls)
    {
        FNetworkingClassification R;
        if (!Cls)
        {
            R.Reason = TEXT("null UClass");
            return R;
        }

        // Walk the parent chain matching known tokens.
        for (UClass* Parent = Cls; Parent; Parent = Parent->GetSuperClass())
        {
            const FString PName = Parent->GetName();
            FNetworkingClassification Sub = ClassifyByClassName(PName);
            if (Sub.Role != ENetworkingRole::Ambiguous)
            {
                R = Sub;
                break;
            }
        }

        // Refine with bReplicates flag on the CDO if available.
        if (const AActor* CDO = Cast<AActor>(Cls->GetDefaultObject(false)))
        {
            R.bReplicatesFlag = CDO->GetIsReplicated();
            if (R.Role == ENetworkingRole::Ambiguous)
            {
                R.Role = R.bReplicatesFlag ? ENetworkingRole::ServerAuthReplicated
                                           : ENetworkingRole::CosmeticLocal;
                R.Reason = R.bReplicatesFlag
                    ? TEXT("bReplicates=true on CDO — treating as server-auth replicated")
                    : TEXT("bReplicates=false on CDO — treating as cosmetic/local");
            }
        }
        else if (R.Role == ENetworkingRole::Ambiguous)
        {
            R.Reason = FString::Printf(
                TEXT("Class '%s' is not an AActor ancestor and no known parent matched"),
                *Cls->GetName());
        }

        return R;
    }

    /** Classify a Blueprint asset via its parent class. */
    static FNetworkingClassification ClassifyBlueprint(UBlueprint* BP)
    {
        if (!BP)
        {
            FNetworkingClassification R;
            R.Reason = TEXT("null Blueprint");
            return R;
        }
        return ClassifyClass(BP->GeneratedClass ? BP->GeneratedClass : BP->ParentClass);
    }
#endif

private:
    /** Strip "ABP_", "BP_", trailing "_C", etc. so "BP_MyHero_C" hits "MyHero" heuristics. */
    static FString NormalizeClassName(const FString& In)
    {
        FString Out = In;
        Out.TrimStartAndEndInline();
        // Strip common prefixes.
        if (Out.StartsWith(TEXT("BP_"))) Out.RightChopInline(3);
        else if (Out.StartsWith(TEXT("ABP_"))) Out.RightChopInline(4);
        // Strip trailing "_C".
        if (Out.EndsWith(TEXT("_C"))) Out.LeftChopInline(2);
        return Out;
    }
};
