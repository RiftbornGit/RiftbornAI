// PhysicsAwareness.h - Physics state understanding for AI Agents

#pragma once

#include "CoreMinimal.h"
#include "PhysicsAwareness.generated.h"

UENUM(BlueprintType)
enum class EPhysicsBodyState : uint8
{
    Static,
    Kinematic,
    Simulating,
    Sleeping,
    Frozen
};

UENUM(BlueprintType)
enum class ECollisionType : uint8
{
    NoCollision,
    QueryOnly,
    PhysicsOnly,
    QueryAndPhysics,
    Custom
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPhysicsBodyInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString ActorName;
    UPROPERTY() FString ComponentName;
    UPROPERTY() EPhysicsBodyState State = EPhysicsBodyState::Static;
    UPROPERTY() ECollisionType CollisionType = ECollisionType::NoCollision;
    UPROPERTY() FVector Location = FVector::ZeroVector;
    UPROPERTY() FVector Velocity = FVector::ZeroVector;
    UPROPERTY() FVector AngularVelocity = FVector::ZeroVector;
    UPROPERTY() float Mass = 0.0f;
    UPROPERTY() float LinearDamping = 0.0f;
    UPROPERTY() float AngularDamping = 0.0f;
    UPROPERTY() bool bGravityEnabled = true;
    UPROPERTY() FString CollisionProfile;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FConstraintInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString ConstraintName;
    UPROPERTY() FString ParentActor;
    UPROPERTY() FString ChildActor;
    UPROPERTY() FString ConstraintType;
    UPROPERTY() bool bIsBroken = false;
    UPROPERTY() FVector LinearLimits = FVector::ZeroVector;
    UPROPERTY() FVector AngularLimits = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPhysicsWorldState
{
    GENERATED_BODY()
    
    UPROPERTY() FVector Gravity = FVector(0, 0, -980.0f);
    UPROPERTY() int32 SimulatingBodies = 0;
    UPROPERTY() int32 SleepingBodies = 0;
    UPROPERTY() int32 StaticBodies = 0;
    UPROPERTY() int32 Constraints = 0;
    UPROPERTY() int32 BrokenConstraints = 0;
    UPROPERTY() float PhysicsTimeMs = 0.0f;
    UPROPERTY() int32 SubstepCount = 1;
    UPROPERTY() TArray<FPhysicsBodyInfo> ActiveBodies;
};

class RIFTBORNAI_API FPhysicsAwareness
{
public:
    static FPhysicsAwareness& Get();
    
    FPhysicsWorldState GetWorldState() const;
    FVector GetGravity() const;
    
    TArray<FPhysicsBodyInfo> GetSimulatingBodies() const;
    TArray<FPhysicsBodyInfo> GetBodiesInRadius(const FVector& Center, float Radius) const;
    FPhysicsBodyInfo GetBodyInfo(AActor* Actor) const;
    
    TArray<FConstraintInfo> GetConstraints() const;
    TArray<FConstraintInfo> GetBrokenConstraints() const;
    
    bool IsSimulating(AActor* Actor) const;
    bool IsSleeping(AActor* Actor) const;
    FVector GetVelocity(AActor* Actor) const;
    float GetMass(AActor* Actor) const;
    
    // Raycasts
    bool RaycastSingle(const FVector& Start, const FVector& End, FHitResult& OutHit) const;
    TArray<FHitResult> RaycastMulti(const FVector& Start, const FVector& End) const;
    bool SweepSingle(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit) const;
    
    static FString BodyStateToString(EPhysicsBodyState State);
    static FString CollisionTypeToString(ECollisionType Type);
    
private:
    FPhysicsAwareness();
};
