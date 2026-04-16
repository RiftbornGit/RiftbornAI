// Copyright RiftbornAI. All Rights Reserved.
// ProceduralGenerator - Creates dungeons, terrain, and levels from descriptions

#pragma once

#include "CoreMinimal.h"

/**
 * Room types for procedural dungeon generation
 */
UENUM(BlueprintType)
enum class EProceduralRoomType : uint8
{
	Normal,
	Start,
	Boss,
	Shop,
	Treasure,
	Secret,
	Corridor,
	Arena,
	Puzzle,
	Rest
};

/**
 * Dungeon generation algorithms
 */
UENUM(BlueprintType)
enum class EDungeonAlgorithm : uint8
{
	BSP,           // Binary Space Partitioning
	RandomWalk,    // Drunkard's walk
	CellularAutomata,
	RoomAndCorridor,
	WaveFunctionCollapse,
	Prefab         // Hand-placed room prefabs
};

/**
 * A single room in the dungeon
 */
struct FDungeonRoom
{
	int32 RoomId = 0;
	EProceduralRoomType Type = EProceduralRoomType::Normal;
	FIntPoint Position;      // Grid position
	FIntPoint Size;          // Room dimensions
	TArray<int32> ConnectedRooms;
	
	// Content
	int32 EnemyCount = 0;
	int32 TreasureCount = 0;
	bool bHasShop = false;
	bool bHasBoss = false;
	FString PrefabPath;      // Optional prefab to use
};

/**
 * Connection between rooms
 */
struct FDungeonCorridor
{
	int32 FromRoom;
	int32 ToRoom;
	TArray<FIntPoint> Path;
	int32 Width = 2;
	bool bHasDoor = true;
};

/**
 * Complete dungeon layout
 */
struct FDungeonLayout
{
	FString Name;
	int32 Seed = 0;
	FIntPoint GridSize;
	
	TArray<FDungeonRoom> Rooms;
	TArray<FDungeonCorridor> Corridors;
	
	int32 StartRoomId = 0;
	int32 BossRoomId = -1;
	
	// Metadata
	int32 FloorNumber = 1;
	FString Theme;           // "cave", "castle", "forest"
	float DifficultyMultiplier = 1.0f;
};

/**
 * Dungeon generation parameters
 */
struct FDungeonParams
{
	EDungeonAlgorithm Algorithm = EDungeonAlgorithm::BSP;
	int32 Seed = 0;          // 0 = random
	
	// Size
	FIntPoint GridSize = FIntPoint(50, 50);
	int32 MinRooms = 5;
	int32 MaxRooms = 12;
	FIntPoint MinRoomSize = FIntPoint(4, 4);
	FIntPoint MaxRoomSize = FIntPoint(10, 10);
	
	// Room distribution
	float ShopChance = 0.15f;
	float TreasureChance = 0.2f;
	float SecretChance = 0.1f;
	bool bGuaranteeBossRoom = true;
	
	// Content
	int32 BaseEnemiesPerRoom = 3;
	float EnemyScalePerFloor = 0.2f;
	
	// Style
	FString Theme = TEXT("dungeon");
	int32 CorridorWidth = 2;
};

/**
 * Result of terrain generation
 */
struct FTerrainResult
{
	bool bSuccess = false;
	FString HeightmapPath;
	TArray<FString> MaterialLayers;
	FString SetupInstructions;
	FString ErrorMessage;
};

/**
 * Terrain generation parameters
 */
struct FTerrainParams
{
	FIntPoint Size = FIntPoint(1009, 1009);  // Landscape-friendly size
	float MinHeight = -100.0f;
	float MaxHeight = 500.0f;
	
	// Noise
	float NoiseScale = 0.01f;
	int32 Octaves = 4;
	float Persistence = 0.5f;
	float Lacunarity = 2.0f;
	
	// Features
	bool bHasMountains = true;
	bool bHasRivers = false;
	bool bHasLakes = false;
	float FlatAreaPercent = 0.3f;
	
	// Biomes
	TArray<FString> Biomes;  // "grass", "desert", "snow"
};

/**
 * Prop placement result
 */
struct FPropPlacement
{
	FString MeshPath;
	FVector Location;
	FRotator Rotation;
	FVector Scale = FVector::OneVector;
	FString Category;  // "tree", "rock", "building"
};

/**
 * Prop distribution parameters
 */
struct FPropParams
{
	FString Category;
	int32 Count = 10;
	float MinSpacing = 100.0f;
	float MaxSlope = 30.0f;
	bool bAlignToSurface = true;
	float RandomScaleMin = 0.8f;
	float RandomScaleMax = 1.2f;
};

/**
 * Procedural Generator - The world creation engine
 */
class RIFTBORNAI_API FProceduralGenerator
{
public:
	static FProceduralGenerator& Get();
	
	// ========================================================================
	// DUNGEON GENERATION
	// ========================================================================
	
	/**
	 * Generate a dungeon layout from parameters
	 */
	FDungeonLayout GenerateDungeon(const FDungeonParams& Params);
	
	/**
	 * Generate a dungeon from natural language
	 */
	FDungeonLayout GenerateDungeonFromDescription(const FString& Description);
	
	/**
	 * Convert dungeon layout to spawn instructions
	 */
	FString GetDungeonSpawnInstructions(const FDungeonLayout& Layout);
	
	/**
	 * Generate C++ code for runtime dungeon generation
	 */
	FString GenerateDungeonGeneratorCode(const FDungeonParams& Params);
	
	// ========================================================================
	// TERRAIN GENERATION
	// ========================================================================
	
	/**
	 * Generate terrain heightmap data
	 */
	FTerrainResult GenerateTerrain(const FTerrainParams& Params);
	
	/**
	 * Generate terrain from natural language
	 */
	FTerrainResult GenerateTerrainFromDescription(const FString& Description);
	
	// ========================================================================
	// PROP PLACEMENT
	// ========================================================================
	
	/**
	 * Generate prop placements for an area
	 */
	TArray<FPropPlacement> GeneratePropPlacements(const FPropParams& Params, const FBox& Area);
	
	/**
	 * Scatter props from description
	 */
	TArray<FPropPlacement> ScatterPropsFromDescription(const FString& Description, const FBox& Area);

private:
	// BSP dungeon generation
	FDungeonLayout GenerateBSP(const FDungeonParams& Params);
	void BSPSplit(TArray<FIntRect>& Partitions, const FIntRect& Area, int32 Depth, int32 MaxDepth, const FDungeonParams& Params);
	
	// Room and corridor generation
	FDungeonLayout GenerateRoomAndCorridor(const FDungeonParams& Params);
	FDungeonCorridor ConnectRooms(const FDungeonRoom& A, const FDungeonRoom& B, int32 Width);
	
	// Random walk (cave-like)
	FDungeonLayout GenerateRandomWalk(const FDungeonParams& Params);
	
	// Room type assignment
	void AssignRoomTypes(FDungeonLayout& Layout, const FDungeonParams& Params);
	void PopulateRooms(FDungeonLayout& Layout, const FDungeonParams& Params);
	
	// Terrain helpers
	TArray<float> GenerateHeightmap(const FTerrainParams& Params);
	float PerlinNoise(float X, float Y, int32 Octaves, float Persistence, float Lacunarity);
	
	// Parsing
	FDungeonParams ParseDungeonDescription(const FString& Description);
	FTerrainParams ParseTerrainDescription(const FString& Description);
	
	// Random
	FRandomStream Random;
};
