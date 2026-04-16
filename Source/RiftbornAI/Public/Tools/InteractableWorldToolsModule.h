// Copyright RiftbornAI. All Rights Reserved.
// Interactable World Tools — resource mining, tree harvesting, terrain digging.
// Verified against UE 5.7: FGeometryCollectionEngineConversion::AppendStaticMesh,
// AGeometryCollectionActor, UGeometryCollectionComponent::SetDamageThreshold,
// AInstancedFoliageActor, FLandscapeEditDataInterface, Actor::Tags.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FInteractableWorldToolsModule
 * 
 * Interactable World Tools — resource mining, tree harvesting, terrain digging.
 */
class RIFTBORNAI_API FInteractableWorldToolsModule : public TToolModuleBase<FInteractableWorldToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("InteractableWorld"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Convert static mesh → pre-fractured Chaos destructible with HP, resource tags, debris config
	static FClaudeToolResult Tool_MakeResourceNode(const FClaudeToolCall& Call);

	// Promote foliage instances → cuttable dynamic actors with HP, fall physics, harvest tags
	static FClaudeToolResult Tool_MakeTreeHarvestable(const FClaudeToolCall& Call);

	// Lower landscape heightmap at a point to create dig holes (editor/PIE)
	static FClaudeToolResult Tool_DigTerrain(const FClaudeToolCall& Call);

	// Voxel dig: carve a 3D sphere into marching-cubes terrain, creating actual holes
	// with underground geometry. Punches landscape visibility hole + generates voxel mesh.
	static FClaudeToolResult Tool_DigTerrainVoxel(const FClaudeToolCall& Call);

	// Spawn a voxel stamp actor that shapes terrain non-destructively.
	static FClaudeToolResult Tool_SpawnVoxelStamp(const FClaudeToolCall& Call);

	// Debug a material: read shader compile errors, expression graph, pin connections.
	static FClaudeToolResult Tool_DebugMaterial(const FClaudeToolCall& Call);
};
