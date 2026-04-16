// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ArenaSpecAsset.generated.h"

/**
 * Ability type enum
 */
UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	PROJECTILE      UMETA(DisplayName = "Projectile"),
	TELEPORT        UMETA(DisplayName = "Teleport"),
	INSTANT_MELEE   UMETA(DisplayName = "Instant Melee"),
	DASH            UMETA(DisplayName = "Dash"),
	BUFF            UMETA(DisplayName = "Buff"),
	CHANNEL         UMETA(DisplayName = "Channel")
};

/**
 * Champion role enum
 */
UENUM(BlueprintType)
enum class EChampionRole : uint8
{
	DPS       UMETA(DisplayName = "DPS"),
	TANK      UMETA(DisplayName = "Tank"),
	BRUISER   UMETA(DisplayName = "Bruiser"),
	SUPPORT   UMETA(DisplayName = "Support"),
	MAGE      UMETA(DisplayName = "Mage")
};

/**
 * Base stats for a champion
 */
USTRUCT(BlueprintType)
struct FChampionBaseStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Mana = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HealthRegen = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ManaRegen = 10.0f;
};

/**
 * Ability cost (mana, health, etc.)
 */
USTRUCT(BlueprintType)
struct FAbilityCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	float Mana = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	float Health = 0.0f;
};

/**
 * Ability specification
 */
USTRUCT(BlueprintType)
struct FAbilitySpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FName AbilityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	EAbilityType Type = EAbilityType::PROJECTILE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float Cooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FAbilityCost Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float Range = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float Damage = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float CastTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	TArray<FName> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FString Description;
};

/**
 * Champion specification
 */
USTRUCT(BlueprintType)
struct FChampionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion")
	FName ChampionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion")
	EChampionRole Role = EChampionRole::DPS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion")
	FChampionBaseStats BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion")
	TArray<FName> AbilityIDs;  // References to FAbilitySpec by ID

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion")
	FString Description;
};

/**
 * Spawn point specification
 */
USTRUCT(BlueprintType)
struct FSpawnPointSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName SpawnID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 Team = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * Map specification
 */
USTRUCT(BlueprintType)
struct FArenaMapSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FName MapID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FVector2D Bounds = FVector2D(5000.0f, 5000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TArray<FSpawnPointSpec> SpawnPoints;
};

/**
 * Game systems configuration
 */
USTRUCT(BlueprintType)
struct FArenaSystemsSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
	FString DamageFormula = TEXT("flat");  // v0: no scaling

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
	bool bRespawnEnabled = false;  // first to die loses

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
	float RoundTimeSeconds = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
	bool bFriendlyFire = false;
};

/**
 * Complete Arena Specification (v0 - minimal vertical slice)
 * 
 * This DataAsset defines everything needed to generate a playable arena:
 * - 1 map with spawn points
 * - 2 champions with stats and abilities
 * - 4 abilities (2 per champion)
 * - Simple game rules (first death = loss)
 * 
 * The URiftbornArenaGenerator consumes this and generates:
 * - C++ champion classes
 * - C++ ability classes
 * - Blueprints
 * - Map with spawn points
 * - GameMode with win conditions
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UArenaSpecAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena Spec")
	FName GameID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena Spec")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena Spec")
	TArray<FArenaMapSpec> Maps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena Spec")
	TArray<FChampionSpec> Champions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena Spec")
	TArray<FAbilitySpec> Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena Spec")
	FArenaSystemsSpec Systems;

	// Helper functions (non-Blueprint versions for C++)
	FAbilitySpec* FindAbility(FName AbilityID);
	FChampionSpec* FindChampion(FName ChampionID);
	FArenaMapSpec* FindMap(FName MapID);

	// Blueprint-safe lookup functions
	UFUNCTION(BlueprintCallable, Category = "Arena Spec")
	bool GetAbility(FName AbilityID, FAbilitySpec& OutAbility);

	UFUNCTION(BlueprintCallable, Category = "Arena Spec")
	bool GetChampion(FName ChampionID, FChampionSpec& OutChampion);

	UFUNCTION(BlueprintCallable, Category = "Arena Spec")
	bool GetMap(FName MapID, FArenaMapSpec& OutMap);

	// Validation
	UFUNCTION(BlueprintCallable, Category = "Arena Spec")
	bool ValidateSpec(FString& OutError);

	// Export to JSON (for AI processing)
	UFUNCTION(BlueprintCallable, Category = "Arena Spec")
	FString ExportToJSON() const;

	// Import from JSON (for AI generation)
	UFUNCTION(BlueprintCallable, Category = "Arena Spec")
	bool ImportFromJSON(const FString& JSONString, FString& OutError);
};
