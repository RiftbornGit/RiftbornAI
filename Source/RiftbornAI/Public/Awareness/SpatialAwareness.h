// SpatialAwareness.h - Spatial understanding of the 3D world for AI Agents
// Provides agents with awareness of positions, distances, bounds, and spatial relationships

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialAwareness.generated.h"

// ============================================================================
// SPATIAL RELATIONSHIP TYPES
// ============================================================================

UENUM(BlueprintType)
enum class ESpatialRelation : uint8
{
    Above,
    Below,
    InFrontOf,
    Behind,
    LeftOf,
    RightOf,
    Inside,
    Outside,
    Touching,
    Near,           // Within interaction distance
    Far,            // Beyond interaction distance
    Overlapping
};

UENUM(BlueprintType)
enum class EVisibilityState : uint8
{
    FullyVisible,
    PartiallyVisible,
    Occluded,
    OutOfRange,
    BehindCamera
};

UENUM(BlueprintType)
enum class ENavigationState : uint8
{
    Reachable,
    Unreachable,
    RequiresJump,
    RequiresFall,
    RequiresSwim,
    RequiresFlight,
    Blocked
};

// ============================================================================
// SPATIAL DATA STRUCTURES
// ============================================================================

/** Represents an actor's spatial footprint */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FActorSpatialInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ActorName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ActorClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FRotator Rotation = FRotator::ZeroRotator;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector BoundsExtent = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector BoundsOrigin = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float BoundingSphereRadius = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector ForwardVector = FVector::ForwardVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsMovable = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bHasCollision = false;
    
    /** Get human-readable description */
    FString GetDescription() const;
};

/** Relationship between two actors */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FSpatialRelationship
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ActorA;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ActorB;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Distance = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector DirectionAtoB = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ESpatialRelation RelationAtoB = ESpatialRelation::Near;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bLineOfSight = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ENavigationState NavigationState = ENavigationState::Reachable;
};

/** Level/area bounds */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornLevelBounds
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Min = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Max = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Center = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Size = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float DiagonalLength = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float PlayableArea = 0.0f;  // Square meters
};

/** Query result for spatial searches */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FSpatialQueryResult
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FActorSpatialInfo> Actors;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 TotalFound = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector QueryOrigin = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float QueryRadius = 0.0f;
};

// ============================================================================
// SPATIAL AWARENESS SYSTEM
// ============================================================================

/**
 * Spatial Awareness System
 * 
 * Provides agents with understanding of:
 * - Actor positions, bounds, and orientations
 * - Distances and directions between objects
 * - Line of sight and visibility
 * - Navigation and reachability
 * - Level layout and playable areas
 * 
 * Usage:
 *   FSpatialAwareness& SA = FSpatialAwareness::Get();
 *   FActorSpatialInfo Info = SA.GetActorSpatialInfo(MyActor);
 *   TArray<FActorSpatialInfo> Nearby = SA.GetActorsInRadius(Location, 1000.f);
 */
class RIFTBORNAI_API FSpatialAwareness
{
public:
    static FSpatialAwareness& Get();
    
    // =========================================================================
    // ACTOR QUERIES
    // =========================================================================
    
    /** Get spatial info for a specific actor */
    FActorSpatialInfo GetActorSpatialInfo(AActor* Actor) const;
    
    /** Get spatial info by actor name */
    FActorSpatialInfo GetActorSpatialInfoByName(const FString& ActorName) const;
    
    /** Get all actors in level with spatial info */
    TArray<FActorSpatialInfo> GetAllActorsSpatialInfo() const;
    
    /** Get actors within radius of a point */
    FSpatialQueryResult GetActorsInRadius(const FVector& Origin, float Radius) const;
    
    /** Get actors within a box */
    FSpatialQueryResult GetActorsInBox(const FVector& Origin, const FVector& Extent) const;
    
    /** Get actors of a specific class */
    TArray<FActorSpatialInfo> GetActorsByClass(const FString& ClassName) const;
    
    /** Find actor by name (fuzzy match) */
    AActor* FindActorByName(const FString& Name) const;
    
    // =========================================================================
    // DISTANCE & DIRECTION
    // =========================================================================
    
    /** Get distance between two actors */
    float GetDistanceBetweenActors(AActor* A, AActor* B) const;
    
    /** Get distance from point to actor */
    float GetDistanceToActor(const FVector& Point, AActor* Actor) const;
    
    /** Get direction from A to B (normalized) */
    FVector GetDirectionBetweenActors(AActor* From, AActor* To) const;
    
    /** Get closest actor to a point */
    AActor* GetClosestActorToPoint(const FVector& Point, const TArray<AActor*>& Candidates) const;
    
    /** Get N closest actors to a point */
    TArray<FActorSpatialInfo> GetClosestActors(const FVector& Point, int32 Count) const;
    
    /** Get farthest actor from a point */
    AActor* GetFarthestActorFromPoint(const FVector& Point, const TArray<AActor*>& Candidates) const;
    
    // =========================================================================
    // SPATIAL RELATIONSHIPS
    // =========================================================================
    
    /** Get spatial relationship between two actors */
    FSpatialRelationship GetRelationship(AActor* A, AActor* B) const;
    
    /** Determine relative position (above, below, left, right, etc.) */
    ESpatialRelation GetRelativePosition(AActor* Reference, AActor* Target) const;
    
    /** Check if actor is inside bounds of another */
    bool IsInsideBounds(AActor* Inner, AActor* Outer) const;
    
    /** Check if two actors overlap */
    bool AreOverlapping(AActor* A, AActor* B) const;
    
    // =========================================================================
    // LINE OF SIGHT & VISIBILITY
    // =========================================================================
    
    /** Check line of sight between two points */
    bool HasLineOfSight(const FVector& From, const FVector& To) const;
    
    /** Check if actor A can see actor B */
    bool CanSee(AActor* Observer, AActor* Target) const;
    
    /** Get visibility state of actor from a viewpoint */
    EVisibilityState GetVisibilityState(AActor* Target, const FVector& ViewPoint, const FVector& ViewDir) const;
    
    /** Trace for blocking geometry */
    AActor* GetBlockingActor(const FVector& From, const FVector& To) const;
    
    /** Get all actors visible from a point */
    TArray<FActorSpatialInfo> GetVisibleActors(const FVector& ViewPoint, const FVector& ViewDir, float FOV = 90.f) const;
    
    // =========================================================================
    // NAVIGATION
    // =========================================================================
    
    /** Check if point is on nav mesh */
    bool IsPointNavigable(const FVector& Point) const;
    
    /** Get navigation state between two points */
    ENavigationState GetNavigationState(const FVector& From, const FVector& To) const;
    
    /** Get path length between two points */
    float GetNavigationPathLength(const FVector& From, const FVector& To) const;
    
    /** Project point to nav mesh */
    FVector ProjectToNavMesh(const FVector& Point) const;
    
    /** Check if actor is reachable from another */
    bool IsReachable(AActor* From, AActor* To) const;
    
    // =========================================================================
    // LEVEL BOUNDS
    // =========================================================================
    
    /** Get level bounds */
    FRiftbornLevelBounds GetLevelBounds() const;
    
    /** Get playable area bounds (from nav mesh or kill volumes) */
    FRiftbornLevelBounds GetPlayableAreaBounds() const;
    
    /** Check if point is within level bounds */
    bool IsWithinLevelBounds(const FVector& Point) const;
    
    /** Get level center */
    FVector GetLevelCenter() const;
    
    // =========================================================================
    // SPATIAL SEARCH
    // =========================================================================
    
    /** Find empty space near a point */
    FVector FindEmptySpace(const FVector& NearPoint, float MinRadius, float SearchRadius) const;
    
    /** Find ground position below a point */
    FVector FindGroundPosition(const FVector& Point) const;
    
    /** Get spawn points in level */
    TArray<FVector> GetSpawnPoints() const;
    
    /** Get cover positions near a point */
    TArray<FVector> GetCoverPositions(const FVector& NearPoint, float Radius, const FVector& ThreatDirection) const;
    
    // =========================================================================
    // CAMERA & VIEWPORT
    // =========================================================================
    
    /** Get current editor camera position */
    FVector GetEditorCameraPosition() const;
    
    /** Get current editor camera direction */
    FVector GetEditorCameraDirection() const;
    
    /** Get actors in editor viewport */
    TArray<FActorSpatialInfo> GetActorsInViewport() const;
    
    /** Project world point to screen */
    FVector2D WorldToScreen(const FVector& WorldPoint) const;
    
    /** Check if point is on screen */
    bool IsOnScreen(const FVector& WorldPoint) const;
    
    // =========================================================================
    // UTILITY
    // =========================================================================
    
    /** Convert distance to human-readable string */
    static FString DistanceToString(float Distance);
    
    /** Convert direction to compass string (N, NE, E, etc.) */
    static FString DirectionToCompass(const FVector& Direction);
    
    /** Convert relation enum to string */
    static FString RelationToString(ESpatialRelation Relation);
    
    /** Convert navigation state to string */
    static FString NavigationStateToString(ENavigationState State);
    
    /** Convert visibility state to string */
    static FString VisibilityStateToString(EVisibilityState State);
    
private:
    FSpatialAwareness();
    
    // Cache
    mutable TMap<FString, TWeakObjectPtr<AActor>> ActorNameCache;
    mutable double LastCacheTime = 0.0;
    void RefreshCacheIfNeeded() const;
};
