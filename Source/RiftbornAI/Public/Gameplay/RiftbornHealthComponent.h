// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth, float, DeltaHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, KilledBy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevive);

/**
 * Health component for arena combat.
 * Attach to any actor to give it health and damage handling.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftbornHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornHealthComponent();

	/** Maximum health value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	/** Current health value */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth = 100.0f;

	/** Whether this entity is dead */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;

	/** Whether this entity can be revived after death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bCanRevive = true;

	/** Time before auto-respawn (0 = no auto-respawn, default for arena) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float AutoRespawnTime = 0.0f;

	/** Shield/armor value that absorbs damage first */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Shield = 0.0f;

	/** Maximum shield value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxShield = 100.0f;

	/** Damage multiplier (1.0 = normal, 2.0 = double damage taken) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float DamageMultiplier = 1.0f;

	/** Team ID for friendly fire checks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	int32 TeamId = 0;

	/** Whether friendly fire is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	bool bAllowFriendlyFire = false;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnRevive OnRevive;

	// Functions
	
	/** Apply damage to this entity. Returns actual damage dealt. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageAmount, AActor* DamageSource = nullptr, AActor* DamageCauser = nullptr);

	/** Heal this entity. Returns actual healing done. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float HealAmount);

	/** Kill this entity immediately */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Kill(AActor* Killer = nullptr);

	/** Revive this entity with specified health */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Revive(float HealthPercent = 1.0f);

	/** Get health as a percentage (0-1) */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	/** Get shield as a percentage (0-1) */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetShieldPercent() const;

	/** Check if entity is alive */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsAlive() const { return !bIsDead; }

	/** Set shield to a specific value */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetShield(float NewShield);

	/** Add to current shield */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float AddShield(float ShieldAmount);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentHealth();

	void HandleDeath(AActor* Killer);
	void StartAutoRespawnTimer();

	FTimerHandle RespawnTimerHandle;
};
