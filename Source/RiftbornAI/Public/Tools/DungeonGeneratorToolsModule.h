// DungeonGeneratorToolsModule.h
// Procedural dungeon generation using Binary Space Partitioning (BSP)
// Generates room layouts, corridors, spawns geometry, adds lighting and navmesh

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

struct FDungeonRoom
{
	FIntRect Bounds; // Grid-space bounds
	FVector WorldCenter; // World-space center
	FVector WorldExtent; // World-space half-extents
	int32 RoomIndex;
	FString Label;
};

struct FDungeonCorridor
{
	FVector Start;
	FVector End;
	float Width;
	int32 FromRoom;
	int32 ToRoom;
};

struct FBSPNode
{
	FIntRect Bounds;
	TSharedPtr<FBSPNode> Left;
	TSharedPtr<FBSPNode> Right;
	int32 RoomIndex = -1; // -1 = internal node, >=0 = leaf with room

	bool IsLeaf() const { return !Left.IsValid() && !Right.IsValid(); }
};

/**
 * FDungeonGeneratorToolsModule
 * 
 * Procedural dungeon generation using Binary Space Partitioning (BSP)
 */
class RIFTBORNAI_API FDungeonGeneratorToolsModule : public TToolModuleBase<FDungeonGeneratorToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("DungeonGenerator"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// generate_dungeon: Full BSP dungeon with rooms, corridors, lighting
	static FClaudeToolResult Tool_GenerateDungeon(const FClaudeToolCall& Call);

	// get_dungeon_layout: Returns room/corridor data without spawning
	static FClaudeToolResult Tool_GetDungeonLayout(const FClaudeToolCall& Call);

private:
	// BSP algorithm
	static TSharedPtr<FBSPNode> BuildBSPTree(FIntRect Bounds, int32 Depth, int32 MaxDepth, int32 MinRoomSize, FRandomStream& Rng);
	static void CollectLeaves(const TSharedPtr<FBSPNode>& Node, TArray<TSharedPtr<FBSPNode>>& OutLeaves);
	static void GenerateRooms(const TArray<TSharedPtr<FBSPNode>>& Leaves, TArray<FDungeonRoom>& OutRooms, int32 MinRoomSize, int32 RoomPadding, float CellSize, FVector Origin, FRandomStream& Rng);
	static void GenerateCorridors(const TSharedPtr<FBSPNode>& Node, const TArray<FDungeonRoom>& Rooms, TArray<FDungeonCorridor>& OutCorridors, float CorridorWidth, float CellSize, FVector Origin);
	static FVector GetRoomCenter(const TSharedPtr<FBSPNode>& Node, const TArray<FDungeonRoom>& Rooms);

	// World spawning
	static void SpawnRoomGeometry(UWorld* World, const FDungeonRoom& Room, float WallHeight, const FString& FloorMesh, const FString& WallMesh, TArray<AActor*>& OutActors);
	static void SpawnCorridorGeometry(UWorld* World, const FDungeonCorridor& Corridor, float WallHeight, TArray<AActor*>& OutActors);
	static void SpawnRoomLighting(UWorld* World, const FDungeonRoom& Room, float WallHeight);
};
