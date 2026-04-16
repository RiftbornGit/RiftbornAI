// Copyright Epic Games, Inc. All Rights Reserved.
// Universal Game Specification - Source of Truth for All Game Types

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameSpec.generated.h"

// ============================================================================
// PRIMITIVES - Genre-Agnostic Building Blocks
// ============================================================================

/**
 * Entity semantic tags - not genre-specific, just roles
 */
UENUM(BlueprintType)
enum class EEntityTag : uint8
{
	Player,
	Champion,
	Minion,
	NPC,
	Boss,
	Objective,
	Projectile,
	Custom
};

/**
 * Stat definition - any numeric resource
 */
USTRUCT(BlueprintType)
struct FStatDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FName Name;              // "Health", "Mana", "Stamina"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float BaseValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float MinValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float MaxValue = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float RegenRate = 0.f;
};

/**
 * Ability/Action definition
 */
USTRUCT(BlueprintType)
struct FAbilityDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FName Id;                // "Fireball"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FName> Tags;      // "MAGIC", "PROJECTILE", "DASH"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float Cooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float Cost = 0.f;        // v0: single resource

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FName CostResource;      // "Mana"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float Range = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FString TargetingModel;  // "Self", "Unit", "Point", "Area"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FString EffectScript;    // high-level description for now; compiler/LLM interprets

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float Damage = 0.f;      // v0: simple flat damage

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float CastTime = 0.f;
};

/**
 * Entity definition - can be champion, minion, player, NPC, anything
 */
USTRUCT(BlueprintType)
struct FEntityDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FName Id;                // "FireMage"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	TArray<EEntityTag> Tags; // Champion, Minion, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FStatDef> Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FName> Abilities; // references to FAbilityDef::Id

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FName AIBehavior;        // optional for NPCs
};

/**
 * Map/Level definition
 */
USTRUCT(BlueprintType)
struct FMapDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FName Id;                // "Arena_Basic"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FString LayoutType;      // "Symmetric", "Asymmetric", "Lane", "Room"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FVector2D Bounds = FVector2D(10000, 10000);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	int32 NumTeams = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FVector> TeamSpawns;  // One spawn point per team

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	bool bGenerateNavMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	bool bGenerateCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FString TerrainStyle;    // "Flat", "Arena", "Natural" - hint for generator
};

/**
 * Win condition types
 */
UENUM(BlueprintType)
enum class EWinCondition : uint8
{
	EliminateEnemies,
	CaptureObjective,
	ScoreThreshold,
	TimeLimit,
	Custom
};

/**
 * Match flow phases
 */
UENUM(BlueprintType)
enum class EMatchPhase : uint8
{
	Lobby,              // Pre-match, champion select
	Countdown,          // 3..2..1.. start
	InProgress,         // Round active
	RoundEnd,           // Winner declared, brief pause
	MatchEnd,           // All rounds complete
	PostMatch           // Show results, return to menu
};

/**
 * Camera profile for generated PlayerController
 */
UENUM(BlueprintType)
enum class ECameraProfile : uint8
{
	ThirdPerson,        // Behind shoulder
	TopDown,            // MOBA/RTS style
	FirstPerson,        // FPS
	Isometric,          // Diablo-style
	Custom
};

/**
 * Input scheme template
 */
UENUM(BlueprintType)
enum class EInputScheme : uint8
{
	MOBA,               // QWER abilities, right-click move
	Arena,              // 1234 abilities, WASD move
	FPS,                // Mouse aim, WASD, number keys
	Custom
};

/**
 * Game rules and match flow
 */
USTRUCT(BlueprintType)
struct FGameRulesDef
{
	GENERATED_BODY()

	// Win conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Win Conditions")
	EWinCondition WinCondition = EWinCondition::EliminateEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Win Conditions")
	int32 RoundsToWin = 1;  // Best-of-1, Best-of-3, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Win Conditions")
	int32 ScoreToWin = 100;  // For ScoreThreshold mode

	// Respawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	bool bRespawnEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	float RespawnDelay = 5.0f;

	// Timing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float RoundTimeSeconds = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float CountdownSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float RoundEndDelay = 3.0f;  // Pause after round end before next round

	// Teams
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
	bool bFriendlyFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
	int32 NumTeams = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
	int32 PlayersPerTeam = 1;

	// Combat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FString DamageFormula = TEXT("flat");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bHasShields = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bHasCriticals = false;

	// Camera & Input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera & Input")
	ECameraProfile CameraProfile = ECameraProfile::ThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera & Input")
	EInputScheme InputScheme = EInputScheme::Arena;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera & Input")
	float CameraDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera & Input")
	float CameraPitch = -45.0f;
};

// ============================================================================
// UNIVERSAL GAME SPECIFICATION - Source of Truth
// ============================================================================

/**
 * UGameSpecAsset - The ONLY source of truth for what game is being built
 * 
 * This is NOT genre-specific. Arena, FPS, RPG, etc. are just different
 * configurations of these primitives.
 * 
 * The compiler reads this and generates the UE project.
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UGameSpecAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Meta
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	FName GameID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	TArray<FName> Tags; // Arena, FPS, RPG, etc.

	// Core definitions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entities")
	TArray<FEntityDef> Entities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<FAbilityDef> Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
	TArray<FMapDef> Maps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	FGameRulesDef Rules;

	// Lookups (non-UFUNCTION - UHT doesn't allow struct pointers)
	FEntityDef* FindEntity(FName EntityID);
	FAbilityDef* FindAbility(FName AbilityID);
	FMapDef* FindMap(FName MapID);
	
	UFUNCTION(BlueprintCallable, Category = "Game Spec")
	bool GetEntity(FName EntityID, FEntityDef& OutEntity);
	
	UFUNCTION(BlueprintCallable, Category = "Game Spec")
	bool GetAbility(FName AbilityID, FAbilityDef& OutAbility);
	
	UFUNCTION(BlueprintCallable, Category = "Game Spec")
	bool GetMap(FName MapID, FMapDef& OutMap);

	// Validation
	UFUNCTION(BlueprintCallable, Category = "Game Spec")
	bool Validate(FString& OutError);

	// JSON I/O for LLM
	UFUNCTION(BlueprintCallable, Category = "Game Spec")
	FString ExportToJSON() const;

	UFUNCTION(BlueprintCallable, Category = "Game Spec")
	bool ImportFromJSON(const FString& JSONString, FString& OutError);
};
