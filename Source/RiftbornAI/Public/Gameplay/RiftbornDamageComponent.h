// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornDamageComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRiftbornAIDamageDealt, AActor*, Target, float, DamageDealt, bool, bKilled);

/**
 * Damage dealing component for weapons and abilities.
 * Handles damage calculation, critical hits, and damage over time.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftbornDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornDamageComponent();

	/** Base damage amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 25.0f;

	/** Damage multiplier (applied on top of base damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageMultiplier = 1.0f;

	/** Critical hit chance (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Critical")
	float CriticalChance = 0.1f;

	/** Critical hit damage multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Critical")
	float CriticalMultiplier = 2.0f;

	/** Headshot damage multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float HeadshotMultiplier = 2.0f;

	/** Falloff start distance (damage starts reducing after this) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Falloff")
	float FalloffStartDistance = 1000.0f;

	/** Falloff end distance (minimum damage at this distance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Falloff")
	float FalloffEndDistance = 3000.0f;

	/** Minimum damage percentage at max falloff (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Falloff")
	float MinFalloffDamagePercent = 0.25f;

	/** Team ID for source (used in friendly fire checks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	int32 TeamId = 0;

	// Stats tracking
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 TotalHits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 TotalKills = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float TotalDamageDealt = 0.0f;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnRiftbornAIDamageDealt OnDamageDealt;

	// Functions

	/** Deal damage to a target actor. Returns damage dealt. */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	float DealDamage(AActor* Target, bool bIsHeadshot = false, bool bForceCrit = false);

	/** Deal damage with distance falloff. Returns damage dealt. */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	float DealDamageWithFalloff(AActor* Target, float Distance, bool bIsHeadshot = false);

	/** Calculate damage amount (for preview/UI) without applying */
	UFUNCTION(BlueprintPure, Category = "Damage")
	float CalculateDamage(bool bIsHeadshot = false, bool bIsCrit = false, float Distance = 0.0f) const;

	/** Roll for critical hit */
	UFUNCTION(BlueprintPure, Category = "Damage")
	bool RollCritical() const;

	/** Get damage falloff multiplier for a distance */
	UFUNCTION(BlueprintPure, Category = "Damage")
	float GetFalloffMultiplier(float Distance) const;

	/** Reset stats */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ResetStats();

protected:
	float ApplyDamageToTarget(AActor* Target, float FinalDamage);
};
