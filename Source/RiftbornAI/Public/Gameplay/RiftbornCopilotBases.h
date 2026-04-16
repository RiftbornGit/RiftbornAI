// Copyright RiftbornAI. All Rights Reserved.
// Base classes for UE Copilot - Pre-configured with common components

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "RiftbornCopilotBases.generated.h"

/**
 * Base class for pickup items.
 * Has: StaticMesh, SphereCollision (overlap trigger), optional PointLight
 * Use: Health pickups, ammo, powerups, collectibles
 */
UCLASS(Blueprintable, BlueprintType)
class RIFTBORNAI_API ARiftbornPickupBase : public AActor
{
    GENERATED_BODY()

public:
    ARiftbornPickupBase();

    /** The visible mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* Mesh;

    /** Overlap trigger for pickup detection */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* PickupTrigger;

    /** Optional glow effect */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* GlowLight;

    /** Pickup sound */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* PickupSound;

    // --- Pickup Properties (set in blueprint children) ---
    
    /** Can this pickup be collected? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    bool bIsActive = true;

    /** Time until respawn after pickup (0 = no respawn) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    float RespawnTime = 10.0f;

    /** Does the pickup rotate? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    bool bRotates = true;

    /** Rotation speed in degrees/second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    float RotationSpeed = 90.0f;

    /** Does the pickup bob up and down? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    bool bBobs = true;

    /** Bob height in units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    float BobHeight = 20.0f;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Called when something overlaps the trigger */
    UFUNCTION()
    void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /** Override in child blueprints to define pickup behavior */
    UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
    void OnPickedUp(AActor* PickedUpBy);

    /** Respawn the pickup */
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void Respawn();

    /** Deactivate after pickup */
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void Deactivate();

private:
    FVector InitialLocation;
    float BobTime = 0.0f;
    FTimerHandle RespawnTimerHandle;
};


/**
 * Base class for projectiles.
 * Has: StaticMesh, SphereCollision (hit detection), ProjectileMovement
 * Use: Bullets, rockets, grenades, magic spells
 */
UCLASS(Blueprintable, BlueprintType)
class RIFTBORNAI_API ARiftbornProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    ARiftbornProjectileBase();

    /** Collision sphere */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    /** Visual mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* Mesh;

    // --- Projectile Properties ---

    /** Damage dealt on hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Damage = 25.0f;

    /** Movement speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Speed = 3000.0f;

    /** Lifetime before auto-destroy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Lifetime = 5.0f;

    /** Does it explode on impact? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    bool bExplodes = false;

    /** Explosion radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float ExplosionRadius = 200.0f;

    /** Who fired this projectile - use GetInstigator() from AActor instead */
    // Instigator is already defined in AActor, no need to redeclare

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    /** Override in child blueprints */
    UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
    void OnHit(AActor* HitActor, const FHitResult& Hit);

    UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
    void OnExplode(FVector Location);
};


/**
 * Base class for trigger volumes.
 * Has: BoxCollision trigger
 * Use: Checkpoints, kill zones, spawn triggers, area effects
 */
UCLASS(Blueprintable, BlueprintType)
class RIFTBORNAI_API ARiftbornTriggerBase : public AActor
{
    GENERATED_BODY()

public:
    ARiftbornTriggerBase();

    /** Trigger volume */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerVolume;

    // --- Trigger Properties ---

    /** Is the trigger active? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bIsActive = true;

    /** Only trigger once? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bTriggerOnce = false;

    /** Delay before trigger activates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float TriggerDelay = 0.0f;

    /** Only trigger for player? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bPlayerOnly = true;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /** Override in child blueprints */
    UFUNCTION(BlueprintNativeEvent, Category = "Trigger")
    void OnTriggered(AActor* TriggeringActor);

    UFUNCTION(BlueprintNativeEvent, Category = "Trigger")
    void OnTriggerExit(AActor* ExitingActor);

private:
    bool bHasTriggered = false;
};


/**
 * Base class for spawners.
 * Has: SpawnPoint marker, configurable spawn logic
 * Use: Enemy spawners, item spawners, wave spawners
 */
UCLASS(Blueprintable, BlueprintType)
class RIFTBORNAI_API ARiftbornSpawnerBase : public AActor
{
    GENERATED_BODY()

public:
    ARiftbornSpawnerBase();

    /** Visual marker (editor only) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SpawnMarker;

    // --- Spawner Properties ---

    /** Class to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TSubclassOf<AActor> SpawnClass;

    /** Auto-spawn on begin play? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    bool bAutoSpawn = false;

    /** Interval between spawns (0 = spawn once) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    float SpawnInterval = 0.0f;

    /** Maximum spawned at once */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    int32 MaxSpawned = 1;

    /** Spawn radius (random offset) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    float SpawnRadius = 100.0f;

    /** Current spawn count */
    UPROPERTY(BlueprintReadOnly, Category = "Spawner")
    int32 CurrentSpawnCount = 0;

    /** Spawn an actor */
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    AActor* SpawnActor();

    /** Spawn multiple actors */
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    TArray<AActor*> SpawnActors(int32 Count);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnSpawnedActorDestroyed(AActor* DestroyedActor);

    /** Override to customize spawn behavior */
    UFUNCTION(BlueprintNativeEvent, Category = "Spawner")
    void OnActorSpawned(AActor* SpawnedActor);

private:
    TArray<AActor*> SpawnedActors;
    FTimerHandle SpawnTimerHandle;
    void DoSpawn();
};
