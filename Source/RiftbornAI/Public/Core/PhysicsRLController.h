// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsRLController.generated.h"

class USkeletalMeshComponent;

/**
 * Minimal physics-based controller for skeleton locomotion experiments.
 * Provides clamp enforcement and basic actuation hooks.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API UPhysicsRLController : public UActorComponent
{
	GENERATED_BODY()

public:
	UPhysicsRLController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL")
	bool bTrainingMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL")
	bool bEnableClamps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Clamps")
	float MaxLinearSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Clamps")
	float MaxAngularVelocityDeg = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Clamps")
	float LinearDamping = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Clamps")
	float AngularDamping = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Actuation")
	float MaxImpulse = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Actuation")
	float MaxTorque = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL")
	FString PolicyPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RL")
	int32 StepCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	bool bAssistUpright = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	float AssistStrength = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	float AssistDamping = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	bool bAssistPose = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	float PoseStrength = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	float PoseDamping = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	bool bAssistHeight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	float HeightStrength = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	float HeightDamping = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Assist")
	float HeightMaxForce = 15000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RL|Assist")
	float TargetPelvisHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Sensors")
	TArray<FName> FootBones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Sensors")
	float FootTraceLength = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Sensors")
	float FootContactDistance = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL|Control")
	TArray<FName> ControlBones;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RL|Control")
	TArray<FVector> CurrentTorques;

	UFUNCTION(BlueprintCallable, Category = "RL")
	void InitializeRL();

	UFUNCTION(BlueprintCallable, Category = "RL")
	void StartRL();

	UFUNCTION(BlueprintCallable, Category = "RL")
	void StopRL();

	UFUNCTION(BlueprintCallable, Category = "RL")
	void ApplyImpulseToBone(FName BoneName, const FVector& Impulse);

	UFUNCTION(BlueprintCallable, Category = "RL")
	void ApplyTorqueToBone(FName BoneName, const FVector& Torque);

	UFUNCTION(BlueprintCallable, Category = "RL")
	void ApplyClampSettings();

	UFUNCTION(BlueprintCallable, Category = "RL|Control")
	void SetControlBones(const TArray<FName>& Bones);

	UFUNCTION(BlueprintCallable, Category = "RL|Control")
	void SetTorques(const TArray<FVector>& Torques);

	UFUNCTION(BlueprintCallable, Category = "RL|Observation")
	TArray<float> GetObservation();

	UFUNCTION(BlueprintCallable, Category = "RL|Observation")
	TArray<float> GetDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "RL|Observation")
	FString GetFallReason();

	UFUNCTION(BlueprintCallable, Category = "RL|Debug")
	TArray<FName> GetPhysicalBones();

	UFUNCTION(BlueprintCallable, Category = "RL|Assist")
	void CaptureReferencePose();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> CachedMesh;

	TMap<FName, FQuat> ReferencePose;

	void CacheSkeletalMesh();
	void EnforceClamps();
};
