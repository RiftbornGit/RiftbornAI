// ComponentToolsModule.h
// Component management tools for RiftbornAI
// Tools: add_component, remove_component, get_actor_components, set_component_property

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Component Tools Module
 * 
 * Provides tools for managing actor components:
 * - add_component: Add a component to an actor
 * - remove_component: Remove a component from an actor  
 * - get_actor_components: List all components on an actor
 * - set_component_property: Set a property value on a component
 * - get_component_property: Get a property value from a component
 */
class RIFTBORNAI_API FComponentToolsModule : public TToolModuleBase<FComponentToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ComponentTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_RemoveComponent(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetActorComponents(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetComponentProperty(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetComponentProperty(const FClaudeToolCall& Call);
    
private:
    // Helper to find actor by name
    static AActor* FindActorByName(UWorld* World, const FString& ActorName);
    // Helper to find actor by scene-graph id/path
    static AActor* FindActorByPath(UWorld* World, const FString& ActorPath);
    // Helper to resolve actor_id first, then actor_name fallback
    static AActor* ResolveActorReference(UWorld* World, const FString& ActorId, const FString& ActorName, FString& OutResolutionSource);
    // Helper to find component by name
    static UActorComponent* FindComponentByName(AActor* Actor, const FString& ComponentName);
    // Helper to find component by path/id
    static UActorComponent* FindComponentByPath(AActor* Actor, const FString& ComponentPath);
    // Helper to resolve component_id first, then component_name fallback
    static UActorComponent* ResolveComponentReference(AActor* Actor, const FString& ComponentId, const FString& ComponentName, FString& OutResolutionSource);
};
