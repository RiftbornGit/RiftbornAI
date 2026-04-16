// Copyright RiftbornAI. All Rights Reserved.
// Foliage Tools Module - Vegetation and foliage placement

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Foliage Tools Module
 *
 * Provides tools for placing and managing vegetation/foliage:
 * - create_foliage_type: Create a foliage type asset from a static mesh
 * - paint_foliage: Paint foliage instances in an area
 * - remove_foliage: Remove foliage from an area
 * - get_foliage_density: Get foliage count/density statistics
 * - set_foliage_properties: Configure foliage density, scale, collision
 * - list_foliage_types: List available foliage type assets
 */
class RIFTBORNAI_API FFoliageToolsModule : public TToolModuleBase<FFoliageToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("FoliageTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_PaintFoliage(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GroundFoliageToLandscape(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetFoliageDensity(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateLandscapeGrassType(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddGrassVariety(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateProceduralFoliageSpawner(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnProceduralFoliageVolume(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ResimulateProceduralFoliage(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectProceduralFoliage(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetFoliageInstanceLifecycle(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PromoteFoliageToDynamicActors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetDynamicFoliageActorState(const FClaudeToolCall& Call);
private:
    static class AInstancedFoliageActor* GetFoliageActor();
};
