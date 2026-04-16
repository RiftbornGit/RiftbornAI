// Copyright RiftbornAI. All Rights Reserved.
// Actor Organization Helpers - Unique labels + automatic folder placement

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Editor.h"
#include "RiftbornLog.h"

/**
 * Shared helpers for actor organization in the World Outliner.
 *
 * Two problems this solves:
 * 1. Spawned actors never get placed into Outliner folders → scene chaos
 * 2. Duplicate actor labels → find_actor_by_label confusion
 *
 * Every spawn/create tool should call:
 *   - MakeUniqueActorLabel() before SetActorLabel()
 *   - AssignActorToFolder()  after spawning
 */
namespace ActorOrganization
{
    /**
     * Given a desired label, return a unique variant that doesn't collide
     * with any existing actor label in the world.
     *
     * Examples:
     *   "MyCube"   → "MyCube"        (if unique)
     *   "MyCube"   → "MyCube_2"      (if "MyCube" already exists)
     *   "MyCube"   → "MyCube_3"      (if "MyCube" and "MyCube_2" exist)
     */
    inline FString MakeUniqueActorLabel(UWorld* World, const FString& DesiredLabel)
    {
        if (!World || DesiredLabel.IsEmpty())
        {
            return DesiredLabel;
        }

        // Collect all existing labels in a set for O(1) lookup
        TSet<FString> ExistingLabels;
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor)
            {
                FString Label = Actor->GetActorLabel();
                if (!Label.IsEmpty())
                {
                    ExistingLabels.Add(Label);
                }
            }
        }

        // If the desired label is already unique, use it
        if (!ExistingLabels.Contains(DesiredLabel))
        {
            return DesiredLabel;
        }

        // Otherwise, find the next available suffix
        for (int32 Suffix = 2; Suffix < 10000; ++Suffix)
        {
            FString Candidate = FString::Printf(TEXT("%s_%d"), *DesiredLabel, Suffix);
            if (!ExistingLabels.Contains(Candidate))
            {
                RIFTBORN_LOG(Log, TEXT("ActorOrganization: Label '%s' already exists, using '%s'"), *DesiredLabel, *Candidate);
                return Candidate;
            }
        }

        // Fallback (should never happen)
        return FString::Printf(TEXT("%s_%d"), *DesiredLabel, FMath::RandRange(10000, 99999));
    }

    /**
     * Derive an appropriate World Outliner folder from the tool name or actor class.
     * Returns folder like "Geometry", "Lighting", "Gameplay", etc.
     */
    inline FString DeriveFolder(const FString& ToolName, const FString& ClassName = TEXT(""))
    {
        // Tool-name based mapping (most specific)
        if (ToolName.Contains(TEXT("light")) || ToolName.Contains(TEXT("lighting")))
            return TEXT("Lighting");
        if (ToolName.Contains(TEXT("floor")) || ToolName.Contains(TEXT("wall")) || ToolName.Contains(TEXT("cover")) || ToolName.Contains(TEXT("geometry")))
            return TEXT("Geometry");
        if (ToolName.Contains(TEXT("player_start")) || ToolName.Contains(TEXT("target_point")))
            return TEXT("Gameplay/SpawnPoints");
        if (ToolName.Contains(TEXT("enemy")) || ToolName.Contains(TEXT("ai_")))
            return TEXT("Gameplay/AI");
        if (ToolName.Contains(TEXT("character")) || ToolName.Contains(TEXT("third_person")))
            return TEXT("Gameplay/Characters");
        if (ToolName.Contains(TEXT("niagara")) || ToolName.Contains(TEXT("particle")))
            return TEXT("Effects");
        if (ToolName.Contains(TEXT("projectile")))
            return TEXT("Gameplay/Projectiles");
        if (ToolName.Contains(TEXT("navigation")) || ToolName.Contains(TEXT("navmesh")))
            return TEXT("Navigation");
        if (ToolName.Contains(TEXT("fog")) || ToolName.Contains(TEXT("atmosphere")))
            return TEXT("Atmosphere");
        if (ToolName.Contains(TEXT("static_mesh")) || ToolName.Contains(TEXT("mesh_actor")))
            return TEXT("Geometry");

        // Class-name fallback
        if (!ClassName.IsEmpty())
        {
            if (ClassName.Contains(TEXT("Light")))
                return TEXT("Lighting");
            if (ClassName.Contains(TEXT("StaticMesh")))
                return TEXT("Geometry");
            if (ClassName.Contains(TEXT("Camera")))
                return TEXT("Cameras");
            if (ClassName.Contains(TEXT("PlayerStart")))
                return TEXT("Gameplay/SpawnPoints");
            if (ClassName.Contains(TEXT("TargetPoint")))
                return TEXT("Gameplay/SpawnPoints");
            if (ClassName.Contains(TEXT("Character")) || ClassName.Contains(TEXT("Pawn")))
                return TEXT("Gameplay/Characters");
            if (ClassName.Contains(TEXT("Fog")) || ClassName.Contains(TEXT("Atmosphere")))
                return TEXT("Atmosphere");
            if (ClassName.Contains(TEXT("Sky")))
                return TEXT("Lighting");
        }

        // Default
        return TEXT("Actors");
    }

    /**
     * Assign an actor to a World Outliner folder.
     *
     * @param Actor      The actor to organize
     * @param Folder     Explicit folder path (e.g., "Lighting/Sun"). If empty, auto-derives.
     * @param ToolName   The tool that created this actor (used for auto-derive)
     */
    inline void AssignActorToFolder(AActor* Actor, const FString& ExplicitFolder, const FString& ToolName = TEXT(""))
    {
        if (!Actor) return;

        FString FolderPath = ExplicitFolder;
        if (FolderPath.IsEmpty())
        {
            // Auto-derive from tool name and class
            FolderPath = DeriveFolder(ToolName, Actor->GetClass()->GetName());
        }

        Actor->SetFolderPath(FName(*FolderPath));
        RIFTBORN_LOG(Verbose, TEXT("ActorOrganization: '%s' → folder '%s'"), *Actor->GetActorLabel(), *FolderPath);
    }

    /**
     * Collect all folder paths currently used in the world.
     * Returns a map of FolderPath → actor count.
     */
    inline TMap<FString, int32> GetFolderSummary(UWorld* World)
    {
        TMap<FString, int32> Folders;
        if (!World) return Folders;

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor || Actor->IsA<AWorldSettings>()) continue;

            FString FolderStr = Actor->GetFolderPath().ToString();
            if (FolderStr.IsEmpty())
            {
                FolderStr = TEXT("(root)");
            }
            Folders.FindOrAdd(FolderStr)++;
        }
        return Folders;
    }
}
