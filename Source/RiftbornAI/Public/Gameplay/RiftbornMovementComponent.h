// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornMovementComponent.generated.h"

UENUM(BlueprintType)
enum class ERiftbornMovementState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Walking		UMETA(DisplayName = "Walking"),
	Running		UMETA(DisplayName = "Running"),
	Sprinting	UMETA(DisplayName = "Sprinting"),
	Crouching	UMETA(DisplayName = "Crouching"),
	Jumping		UMETA(DisplayName = "Jumping"),
	Falling		UMETA(DisplayName = "Falling"),
	Sliding		UMETA(DisplayName = "Sliding"),
	WallRunning	UMETA(DisplayName = "Wall Running"),
	Dashing		UMETA(DisplayName = "Dashing")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChanged, ERiftbornMovementState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlideStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, CurrentStamina);

/**
 * Enhanced movement component for arena shooter mechanics.
 * Supports sprinting, sliding, dashing, and wall running.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftbornMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornMovementComponent();

	// Base speeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float RunSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float SprintSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float CrouchSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float SlideSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float WallRunSpeed = 800.0f;

	// Jump settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float JumpForce = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	int32 MaxJumps = 2;  // Double jump support

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float WallJumpForce = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float WallJumpAngle = 45.0f;  // Degrees away from wall

	// Dash settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
	float DashDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
	float DashDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
	float DashCooldown = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
	int32 MaxDashCharges = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
	float DashChargeRegenTime = 3.0f;

	// Slide settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float MinSpeedToSlide = 400.0f;

	// Wall run settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun")
	float WallRunMinSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun")
	float WallRunMaxDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|WallRun")
	float WallRunGravityScale = 0.3f;

	// Stamina (for sprinting)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float CurrentStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float SprintStaminaCost = 15.0f;  // Per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float StaminaRegenRate = 20.0f;  // Per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float StaminaRegenDelay = 1.0f;  // Delay before regen starts

	// Current state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ERiftbornMovementState CurrentState = ERiftbornMovementState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 JumpsRemaining = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 DashCharges = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsCrouching = false;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnMovementStateChanged OnMovementStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnDashStarted OnDashStarted;

	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnSlideStarted OnSlideStarted;

	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnStaminaChanged OnStaminaChanged;

	// Input functions
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprinting();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprinting();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartCrouching();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopCrouching();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool TryJump();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool TryDash(const FVector& Direction);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool TrySlide();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void OnLanded();

	// Query functions
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetCurrentMaxSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool CanSprint() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool CanDash() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool CanSlide() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool CanJump() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetStaminaPercent() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FTimerHandle DashCooldownHandle;
	FTimerHandle DashChargeRegenHandle;
	FTimerHandle SlideCooldownHandle;
	FTimerHandle SlideEndHandle;

	float LastStaminaUseTime = 0.0f;
	bool bCanDash = true;
	bool bCanSlide = true;

	void SetMovementState(ERiftbornMovementState NewState);
	void UpdateStamina(float DeltaTime);
	void ResetDashCooldown();
	void RegenDashCharge();
	void EndSlide();
	void ResetSlideCooldown();
};
