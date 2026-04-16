// Copyright RiftbornAI. All Rights Reserved.
// Transform Tools Module - Actor manipulation tools

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Transform Tools Module
 * Provides tools for manipulating actor transforms (position, rotation, scale).
 * 
 * Tools included:
 * - move_actor: Move an actor to a new location
 * - rotate_actor: Rotate an actor
 * - scale_actor: Scale an actor uniformly or non-uniformly
 * - set_actor_transform: Set complete transform (location, rotation, scale)
 * - duplicate_actor: Duplicate an actor with offset
 * - select_actor: Select an actor in the editor
 * - focus_actor: Focus viewport on an actor
 * - orbit_actor: Position the viewport camera around an actor
 */
class RIFTBORNAI_API FTransformToolsModule : public TToolModuleBase<FTransformToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("TransformTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_MoveActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RotateActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ScaleActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetActorTransform(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DuplicateActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SelectActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_FocusActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_OrbitActor(const FClaudeToolCall& Call);
    
private:
    // Helper functions to resolve live editor actors from grounded ids or labels.
    static AActor* FindActorByPath(const FString& ActorPath);
    static AActor* ResolveActorReference(const FString& ActorId, const FString& Label, FString& OutResolutionSource);
};
