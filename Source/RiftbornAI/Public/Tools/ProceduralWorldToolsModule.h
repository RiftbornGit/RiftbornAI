// Copyright RiftbornAI. All Rights Reserved.
// Procedural World Tools Module — city blocks, rivers, biome painting, physics sandbox.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FProceduralWorldToolsModule
 * 
 * Procedural World Tools Module — city blocks, rivers, biome painting, physics sandbox.
 */
class RIFTBORNAI_API FProceduralWorldToolsModule : public TToolModuleBase<FProceduralWorldToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ProceduralWorldTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool 6: Create a river (landscape channel + AWaterBodyRiver)
	static FClaudeToolResult Tool_CreateProceduralRiver(const FClaudeToolCall& Call);

	// Tool 9: Paint a biome region (multi-layer landscape + foliage + atmosphere)
	static FClaudeToolResult Tool_PaintBiome(const FClaudeToolCall& Call);

	// Tool 4: Generate a grid of roads + buildings in one call
	static FClaudeToolResult Tool_CreateCityBlock(const FClaudeToolCall& Call);

	// Tool 10: Run a PIE physics sim and snap actors to their rest positions
	static FClaudeToolResult Tool_PhysicsSettle(const FClaudeToolCall& Call);

	// ── Settlement generation ──────────────────────────────────────────

	// Tool 11: Generate an organic settlement (village / town / city)
	static FClaudeToolResult Tool_CreateSettlement(const FClaudeToolCall& Call);

	// Internal layout algorithms (ProceduralWorld_SettlementLayout.cpp)
	static struct FSettlementLayout GenerateSettlementLayout(
		UWorld* World, const FVector2D& Center, uint8 StyleIndex,
		int32 NumBuildings, float RoadWidth, float LotDepth, FRandomStream& Rng);
	static float SampleTerrainHeight(UWorld* World, const FVector2D& XY);
	static float SampleTerrainSlope(UWorld* World, const FVector2D& XY);

	// Roof generation (ProceduralWorld_SettlementRoof.cpp)
	static class AStaticMeshActor* GenerateAndPlaceRoof(
		UWorld* World, const TArray<FVector2D>& Outline,
		float WallHeight, float FloorZ, float RoofPitchDeg,
		const FString& AssetName, bool bNanite);
};

// ── Settlement data structures ─────────────────────────────────────────

/** Style presets for settlement generation. */
enum class ESettlementStyle : uint8
{
	MedievalVillage = 0,   // Organic, small, central well, winding paths
	MarketTown      = 1,   // Larger, central market square, radiating streets
	CityDistrict    = 2,   // Dense, wider roads, taller buildings
	FantasyHamlet   = 3,   // Tiny, very organic, few buildings, greenery
};

/** Role assigned to each building lot. */
enum class EBuildingRole : uint8
{
	Residential,
	Commercial,
	Public,
	Landmark,
};

/** One segment of the generated road network. */
struct FSettlementRoadSegment
{
	FVector2D Start;
	FVector2D End;
	float Width   = 400.0f;
	int32 Depth   = 0;         // 0 = main, 1 = secondary, 2 = alley
	bool bIsRadial = true;
};

/** One buildable lot with its polygon outline. */
struct FSettlementLot
{
	TArray<FVector2D> Outline;          // 4+ point polygon in world XY
	EBuildingRole Role = EBuildingRole::Residential;
	float Area               = 0.0f;   // cm^2
	float DistanceFromCenter = 0.0f;   // cm
	int32 AdjacentRoadIndex  = -1;     // index into FSettlementLayout::Roads
};

/** Complete settlement layout: roads + lots. */
struct FSettlementLayout
{
	TArray<FSettlementRoadSegment> Roads;
	TArray<FSettlementLot> Lots;
	FVector2D CenterPoint = FVector2D::ZeroVector;
	float Radius = 0.0f;
};
