// Copyright RiftbornAI. All Rights Reserved.
// Level Tools Module - Tools for actor/level manipulation

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Level Tools Module
 * Provides tools for working with actors and levels.
 * 
 * Tools:
 * - spawn_actor: Spawn an actor in the level
 * - get_level_actors: List level actors with structured inventory data
 * - delete_actor: Remove an actor from level
 * - delete_all_actors: Remove all actors from level (with filtering options)
 * - set_actor_property: Set property on actor
 * - get_actor_info: Get actor transform, bounds, tags, flags, and data-layer info
 * - duplicate_actor: Clone an actor
 * - save_level: Save the current level
 * - load_level: Load/open a level
 * - get_current_level: Inspect current world and streaming-level state
 */
class RIFTBORNAI_API FLevelToolsModule : public TToolModuleBase<FLevelToolsModule>
{
public:
    /** Module name for registration */
    static FString StaticModuleName() { return TEXT("LevelTools"); }
    
    /** Register all Level tools with the registry */
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // =========================================================================
    // Tool Implementations
    // =========================================================================
    
    /** Spawn an actor in the current level */
    static FClaudeToolResult Tool_SpawnActor(const FClaudeToolCall& Call);
    
    /** Internal implementation that runs on game thread */
    static FClaudeToolResult Tool_SpawnActor_Internal(const FClaudeToolCall& Call);
    
    /** Get list of all actors in the level */
    static FClaudeToolResult Tool_GetLevelActors(const FClaudeToolCall& Call);
    
    /** Delete an actor from the level */
    static FClaudeToolResult Tool_DeleteActor(const FClaudeToolCall& Call);
    
    /** Delete all actors from the level (optionally skip defaults or player-related) */
    static FClaudeToolResult Tool_DeleteAllActors(const FClaudeToolCall& Call);
    
    /** Set a property on an actor */
    static FClaudeToolResult Tool_SetActorProperty(const FClaudeToolCall& Call);
    
    /** Get detailed information about an actor */
    static FClaudeToolResult Tool_GetActorInfo(const FClaudeToolCall& Call);

    /** Get just the transform for an actor */
    static FClaudeToolResult Tool_GetActorTransform(const FClaudeToolCall& Call);

    /** Find actors near a point in the editor world */
    static FClaudeToolResult Tool_GetActorsInRadius(const FClaudeToolCall& Call);

    /** Duplicate/clone an actor */
    static FClaudeToolResult Tool_DuplicateActor(const FClaudeToolCall& Call);

    /** Spawn a StaticMeshActor with an explicit mesh assignment */
    static FClaudeToolResult Tool_CreateStaticMeshActor(const FClaudeToolCall& Call);
    
    /** Save the current level */
    static FClaudeToolResult Tool_SaveLevel(const FClaudeToolCall& Call);
    
    /** Load/open a level */
    static FClaudeToolResult Tool_LoadLevel(const FClaudeToolCall& Call);
    
    /** Get current level info */
    static FClaudeToolResult Tool_GetCurrentLevel(const FClaudeToolCall& Call);
    
    /** Create basic geometry (floor, wall, cube, etc) */
    static FClaudeToolResult Tool_CreateBasicGeometry(const FClaudeToolCall& Call);
    
    /** Find actor by label and return structured info */
    static FClaudeToolResult Tool_FindActorByLabel(const FClaudeToolCall& Call);
    
    /** Assert that an actor with given label exists (useful for verification) */
    static FClaudeToolResult Tool_AssertActorExists(const FClaudeToolCall& Call);

    /** Set the outliner folder path for an actor (organizes World Outliner) */
    static FClaudeToolResult Tool_SetActorFolder(const FClaudeToolCall& Call);

    /** Set the outliner folder for multiple actors matching a label pattern */
    static FClaudeToolResult Tool_BatchSetActorFolder(const FClaudeToolCall& Call);

    // =========================================================================
    // PIE Runtime Tools - Work in running PIE session
    // =========================================================================
    
    /** Spawn actor in PIE world (not editor) */
    static FClaudeToolResult Tool_SpawnActorPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnActorPIE_Internal(const FClaudeToolCall& Call);
    
    /** Set actor transform in PIE world */
    static FClaudeToolResult Tool_SetActorTransformPIE(const FClaudeToolCall& Call);
    
    /** Apply physics impulse to actor in PIE world */
    static FClaudeToolResult Tool_ApplyImpulsePIE(const FClaudeToolCall& Call);
    
    /** Destroy actor in PIE world (runtime cleanup) */
    static FClaudeToolResult Tool_DestroyActorPIE(const FClaudeToolCall& Call);
    
    /** Get actor info from PIE world (includes velocity) */
    static FClaudeToolResult Tool_GetActorInfoPIE(const FClaudeToolCall& Call);

private:
    /** Helper to find actor by scene-graph path/id */
    static AActor* FindActorByPath(const FString& ActorPath);
    /** Helper to resolve actor_id first, then label fallback */
    static AActor* ResolveActorReference(const FString& ActorId, const FString& Label, FString& OutResolutionSource);

    /** Helper to get EditorActorSubsystem */
    static class UEditorActorSubsystem* GetEditorActorSubsystem();
};
