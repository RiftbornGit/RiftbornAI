// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornWeaponComponent.generated.h"

UENUM(BlueprintType)
enum class ERiftbornWeaponType : uint8
{
	Hitscan		UMETA(DisplayName = "Hitscan"),
	Projectile	UMETA(DisplayName = "Projectile"),
	Melee		UMETA(DisplayName = "Melee"),
	Beam		UMETA(DisplayName = "Beam")
};

UENUM(BlueprintType)
enum class ERiftbornFireMode : uint8
{
	SemiAuto	UMETA(DisplayName = "Semi-Automatic"),
	FullAuto	UMETA(DisplayName = "Full-Automatic"),
	Burst		UMETA(DisplayName = "Burst Fire"),
	Charge		UMETA(DisplayName = "Charge")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32, CurrentAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponDamageDealt, float, Damage, AActor*, Target);

/**
 * Weapon component handling firing, reloading, and ammunition.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftbornWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornWeaponComponent();

	// Weapon configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponName = "DefaultWeapon";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	ERiftbornWeaponType WeaponType = ERiftbornWeaponType::Hitscan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	ERiftbornFireMode FireMode = ERiftbornFireMode::SemiAuto;

	// Damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Damage")
	float BaseDamage = 25.0f;  // Damage per shot

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Damage")
	float HeadshotMultiplier = 2.0f;  // Multiplier for headshots

	// Firing stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Firing")
	float FireRate = 600.0f;  // Rounds per minute

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Firing")
	int32 BurstCount = 3;  // Shots per burst (if burst mode)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Firing")
	float BurstDelay = 0.05f;  // Delay between burst shots

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Firing")
	float ChargeTime = 1.0f;  // Time to fully charge (if charge mode)

	// Ammunition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MagazineSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 ReserveAmmo = 120;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MaxReserveAmmo = 180;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	bool bInfiniteAmmo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	float ReloadTime = 2.0f;

	// Accuracy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Accuracy")
	float BaseSpread = 1.0f;  // Base spread in degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Accuracy")
	float MaxSpread = 5.0f;  // Max spread when firing continuously

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Accuracy")
	float SpreadIncreasePerShot = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Accuracy")
	float SpreadRecoveryRate = 3.0f;  // Degrees per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Accuracy")
	float AimDownSightsSpreadMultiplier = 0.5f;

	// Range (for hitscan/melee)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Range")
	float MaxRange = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Range")
	float MeleeRange = 200.0f;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsReloading = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsFiring = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentSpread = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentChargePercent = 0.0f;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponFired OnWeaponFired;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnReloadStarted OnReloadStarted;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnReloadFinished OnReloadFinished;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponDamageDealt OnWeaponDamageDealt;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFiring();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFiring();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartReload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void CancelReload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AddAmmo(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanReload() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetTimeBetweenShots() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FVector GetSpreadDirection(const FVector& BaseDirection) const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentSpread(bool bAiming) const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FTimerHandle FireTimerHandle;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle BurstTimerHandle;
	
	int32 BurstShotsRemaining = 0;
	float LastFireTime = 0.0f;
	bool bWantsToFire = false;
	
	// Combat start delay system - prevents tick-order first-shot advantage
	float GameStartTime = 0.0f;           // When BeginPlay was called
	float CombatStartDelay = 0.15f;       // Base delay (each weapon adds 0-0.25s random)

	void FireOnce();
	void FinishReload();
	void ConsumeAmmo();
	void UpdateSpread(float DeltaTime);
};
