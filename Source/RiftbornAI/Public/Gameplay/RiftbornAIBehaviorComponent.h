// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornAIBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ERiftbornAIState : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	Patrolling		UMETA(DisplayName = "Patrolling"),
	Searching		UMETA(DisplayName = "Searching"),
	Chasing			UMETA(DisplayName = "Chasing"),
	Attacking		UMETA(DisplayName = "Attacking"),
	TakingCover		UMETA(DisplayName = "Taking Cover"),
	Retreating		UMETA(DisplayName = "Retreating"),
	Dead			UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ERiftbornAIAggression : uint8
{
	Passive		UMETA(DisplayName = "Passive"),
	Defensive	UMETA(DisplayName = "Defensive"),
	Balanced	UMETA(DisplayName = "Balanced"),
	Aggressive	UMETA(DisplayName = "Aggressive"),
	Berserker	UMETA(DisplayName = "Berserker")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIStateChanged, ERiftbornAIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRiftbornAITargetChanged, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThreatDetected, AActor*, Threat, float, ThreatLevel);

/**
 * Simple AI behavior component for arena combat.
 * Provides state machine, target tracking, and threat assessment.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftbornAIBehaviorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornAIBehaviorComponent();

	// Behavior settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
	ERiftbornAIAggression Aggression = ERiftbornAIAggression::Balanced;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
	float ReactionTime = 0.2f;  // Delay before responding to stimuli

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
	float AccuracyMultiplier = 1.0f;  // AI aim accuracy (1.0 = perfect)

	// Perception
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float SightRange = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float SightAngle = 90.0f;  // Field of view in degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float HearingRange = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float LoseTargetTime = 5.0f;  // Time without LOS before target is lost

	// Combat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float IdealCombatRange = 800.0f;  // Preferred distance to fight at

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float MinCombatRange = 200.0f;  // Too close, back up

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float MaxCombatRange = 2000.0f;  // Too far, close in

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float TakeCoverHealthPercent = 0.3f;  // Seek cover below this health

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float RetreatHealthPercent = 0.15f;  // Retreat below this health

	// Patrol
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
	TArray<FVector> PatrolPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
	float PatrolWaitTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
	bool bPatrolLoop = true;

	// Current state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ERiftbornAIState CurrentState = ERiftbornAIState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	AActor* CurrentTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float TimeSinceTargetSeen = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 CurrentPatrolIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentThreatLevel = 0.0f;

	// Team
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	int32 TeamId = 0;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIStateChanged OnAIStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnRiftbornAITargetChanged OnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnThreatDetected OnThreatDetected;

	// Core functions
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetState(ERiftbornAIState NewState);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ClearTarget();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ReportThreat(AActor* Threat, float ThreatLevel);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void OnDamageReceived(AActor* DamageSource, float DamageAmount);

	// Perception
	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	bool CanSeeActor(AActor* Target) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	bool HasLineOfSight(const FVector& TargetLocation) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	TArray<AActor*> GetVisibleEnemies() const;

	// Decision making
	UFUNCTION(BlueprintCallable, Category = "AI|Decision")
	AActor* SelectBestTarget(const TArray<AActor*>& PotentialTargets);

	UFUNCTION(BlueprintCallable, Category = "AI|Decision")
	float CalculateThreatLevel(AActor* Threat) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Decision")
	bool ShouldTakeCover() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Decision")
	bool ShouldRetreat() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Decision")
	bool ShouldChase() const;

	// Patrol
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	FVector GetNextPatrolPoint();

	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	void AdvancePatrol();

	// Combat positioning
	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	bool IsInIdealRange() const;

	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	bool IsTooClose() const;

	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	bool IsTooFar() const;

	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	float GetDistanceToTarget() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateTargetTracking(float DeltaTime);
	void UpdateStateLogic();
	float GetOwnerHealthPercent() const;
};
