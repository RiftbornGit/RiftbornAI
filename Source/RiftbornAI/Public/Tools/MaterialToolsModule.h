// MaterialToolsModule.h
// Material management tools for RiftbornAI
// Tools: create_material, create_material_instance, set_actor_material, get_material_info, set_actor_color

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Material Tools Module
 * 
 * Provides tools for creating and managing materials:
 * - create_material: Create a new material asset
 * - create_material_instance: Create material instance from parent
 * - set_actor_material: Apply material to an actor's mesh or decal component
 * - get_material_info: Inspect authored material state, including domain, blend, shading, usage, expressions, textures, static parameters, and instance overrides
 * - set_material_parameter: Set parameter on material instance
 * - set_actor_color: Set an actor's color (high-level convenience tool)
 */
class RIFTBORNAI_API FMaterialToolsModule : public TToolModuleBase<FMaterialToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("MaterialTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_CreateMaterial(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateMaterialInstance(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetActorMaterial(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetMaterialInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMaterialParameter(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetActorColor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreatePBRMaterial(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateFoliageMaterialPreset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMaterialBlendMode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AssignMeshMaterial(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateSilpomMaterial(const FClaudeToolCall& Call);

};
