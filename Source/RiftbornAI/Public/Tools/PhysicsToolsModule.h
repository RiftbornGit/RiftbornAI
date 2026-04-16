// PhysicsToolsModule.h
// Physics simulation and collision tools for RiftbornAI
// Tools: set_physics_enabled, add_physics_constraint, set_collision_profile,
//        apply_force, apply_impulse, set_mass, get_physics_info

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Physics Tools Module
 * 
 * Provides tools for physics simulation and collision:
 * - set_physics_enabled: Enable/disable physics simulation on an actor
 * - add_physics_constraint: Add a physics constraint between actors
 * - set_collision_profile: Set collision profile/channel on an actor
 * - apply_force: Apply continuous force to an actor
 * - apply_impulse: Apply instant impulse to an actor
 * - set_mass: Set the mass of a physics body
 * - get_physics_info: Inspect primary-component physics state including mass, damping, COM, bounds, inertia, wake state, and collision metadata
 */
class RIFTBORNAI_API FPhysicsToolsModule : public TToolModuleBase<FPhysicsToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("PhysicsTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_SetPhysicsEnabled(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddPhysicsConstraint(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetCollisionProfile(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyForce(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyImpulse(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMass(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetPhysicsInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnPhysicsActor(const FClaudeToolCall& Call);

private:
    // Helper to get primitive component
    static UPrimitiveComponent* GetPrimitiveComponent(AActor* Actor);
};
