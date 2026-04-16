// Copyright RiftbornAI. All Rights Reserved.
// BatchOperations.h - Bulk operations on assets, actors, and Blueprints

#pragma once

#include "CoreMinimal.h"

/**
 * Result of a single operation within a batch
 */
struct RIFTBORNAI_API FBatchOperationItem
{
    FString ItemId;           // Identifier (actor label, asset path, etc.)
    bool bSuccess = false;
    FString ErrorMessage;
    double DurationMs = 0.0;
    
    // Additional result data
    TMap<FString, FString> ResultData;
};

/**
 * Result of a batch operation
 */
struct RIFTBORNAI_API FBatchOperationResult
{
    bool bOverallSuccess = false;
    int32 TotalItems = 0;
    int32 SuccessCount = 0;
    int32 FailedCount = 0;
    int32 SkippedCount = 0;
    double TotalDurationMs = 0.0;
    
    TArray<FBatchOperationItem> ItemResults;
    TArray<FString> Warnings;
    
    // Summary
    FString GetSummary() const
    {
        return FString::Printf(TEXT("Batch: %d/%d succeeded (%.1f%%), %d failed, %d skipped in %.1fms"),
            SuccessCount, TotalItems,
            TotalItems > 0 ? (float)SuccessCount / TotalItems * 100.0f : 0.0f,
            FailedCount, SkippedCount, TotalDurationMs);
    }
    
    // Get failed items
    TArray<FString> GetFailedItems() const
    {
        TArray<FString> Failed;
        for (const auto& Item : ItemResults)
        {
            if (!Item.bSuccess)
            {
                Failed.Add(FString::Printf(TEXT("%s: %s"), *Item.ItemId, *Item.ErrorMessage));
            }
        }
        return Failed;
    }
};

/**
 * Filter criteria for batch selection
 */
struct RIFTBORNAI_API FBatchFilter
{
    // Actor filters
    TArray<FString> ActorClassNames;        // e.g., "StaticMeshActor", "PointLight"
    TArray<FString> ActorLabelPatterns;     // Wildcards supported: "Light_*", "*Enemy*"
    FBox BoundsFilter = FBox(EForceInit::ForceInit); // Only actors in this box
    bool bBoundsFilterEnabled = false;
    
    // Asset filters
    TArray<FString> AssetPaths;             // Specific paths
    TArray<FString> AssetPathPatterns;      // "/Game/Meshes/*"
    TArray<FString> AssetClasses;           // "StaticMesh", "Material", "Blueprint"
    
    // Common filters
    TArray<FString> Tags;                   // Actors/assets with these tags
    FString NameContains;                   // Name contains this string
    
    // Limits
    int32 MaxItems = 1000;                  // Safety limit
};

/**
 * Transform operation parameters
 */
struct RIFTBORNAI_API FBatchTransformParams
{
    // Absolute transforms (applied if not default)
    TOptional<FVector> NewLocation;
    TOptional<FRotator> NewRotation;
    TOptional<FVector> NewScale;
    
    // Relative transforms (applied after absolute)
    FVector LocationOffset = FVector::ZeroVector;
    FRotator RotationOffset = FRotator::ZeroRotator;
    FVector ScaleMultiplier = FVector::OneVector;
    
    // Snap settings
    float LocationSnapGrid = 0.0f;          // 0 = no snap
    float RotationSnapDegrees = 0.0f;       // 0 = no snap
};

/**
 * Property modification parameters
 */
struct RIFTBORNAI_API FBatchPropertyParams
{
    FString PropertyPath;       // e.g., "Mobility", "bHidden", "Tags"
    FString NewValue;           // String representation of new value
    bool bAddToArray = false;   // For array properties: add vs replace
};

/**
 * Singleton class for batch operations on actors and assets
 * 
 * Usage:
 *     FBatchFilter Filter;
 *     Filter.ActorClassNames.Add(TEXT("PointLight"));
 *     
 *     FBatchTransformParams Transform;
 *     Transform.ScaleMultiplier = FVector(2.0f);
 *     
 *     FBatchOperationResult Result = FBatchOperations::Get().TransformActors(Filter, Transform);
 */
class RIFTBORNAI_API FBatchOperations
{
public:
    static FBatchOperations& Get();
    
    // ========================================
    // Actor Operations
    // ========================================
    
    /**
     * Delete multiple actors matching filter
     */
    FBatchOperationResult DeleteActors(const FBatchFilter& Filter);
    
    /**
     * Transform multiple actors
     */
    FBatchOperationResult TransformActors(const FBatchFilter& Filter, const FBatchTransformParams& Params);
    
    /**
     * Set property on multiple actors
     */
    FBatchOperationResult SetActorProperty(const FBatchFilter& Filter, const FBatchPropertyParams& Params);
    
    /**
     * Add tag to multiple actors
     */
    FBatchOperationResult AddActorTag(const FBatchFilter& Filter, const FName& Tag);
    
    /**
     * Remove tag from multiple actors
     */
    FBatchOperationResult RemoveActorTag(const FBatchFilter& Filter, const FName& Tag);
    
    /**
     * Set visibility on multiple actors
     */
    FBatchOperationResult SetActorVisibility(const FBatchFilter& Filter, bool bVisible);
    
    /**
     * Set mobility on multiple actors
     */
    FBatchOperationResult SetActorMobility(const FBatchFilter& Filter, EComponentMobility::Type Mobility);
    
    /**
     * Duplicate actors with optional transform offset
     */
    FBatchOperationResult DuplicateActors(const FBatchFilter& Filter, const FVector& Offset = FVector::ZeroVector);
    
    /**
     * Replace mesh on multiple StaticMeshActors
     */
    FBatchOperationResult ReplaceStaticMesh(const FBatchFilter& Filter, const FString& NewMeshPath);
    
    /**
     * Apply material to multiple actors
     */
    FBatchOperationResult ApplyMaterial(const FBatchFilter& Filter, const FString& MaterialPath, int32 SlotIndex = 0);
    
    // ========================================
    // Asset Operations
    // ========================================
    
    /**
     * Delete multiple assets
     */
    FBatchOperationResult DeleteAssets(const FBatchFilter& Filter);
    
    /**
     * Move/rename multiple assets
     */
    FBatchOperationResult MoveAssets(const FBatchFilter& Filter, const FString& NewBasePath);
    
    /**
     * Duplicate multiple assets
     */
    FBatchOperationResult DuplicateAssets(const FBatchFilter& Filter, const FString& DestPath, const FString& Suffix = TEXT("_Copy"));
    
    /**
     * Fix up redirectors for assets
     */
    FBatchOperationResult FixupRedirectors(const FBatchFilter& Filter);
    
    // ========================================
    // Blueprint Operations
    // ========================================
    
    /**
     * Compile multiple Blueprints
     */
    FBatchOperationResult CompileBlueprints(const FBatchFilter& Filter);
    
    /**
     * Reparent multiple Blueprints to new parent class
     */
    FBatchOperationResult ReparentBlueprints(const FBatchFilter& Filter, const FString& NewParentClass);
    
    // ========================================
    // Query Operations
    // ========================================
    
    /**
     * Find actors matching filter (no modification)
     */
    TArray<AActor*> FindActors(const FBatchFilter& Filter);
    
    /**
     * Count actors matching filter
     */
    int32 CountActors(const FBatchFilter& Filter);
    
    /**
     * Find assets matching filter
     */
    TArray<FAssetData> FindAssets(const FBatchFilter& Filter);
    
    /**
     * Count assets matching filter
     */
    int32 CountAssets(const FBatchFilter& Filter);
    
    // ========================================
    // Undo Support
    // ========================================
    
    /**
     * Begin a batch transaction (for undo)
     */
    void BeginBatchTransaction(const FString& Description);
    
    /**
     * End the current batch transaction
     */
    void EndBatchTransaction();
    
    /**
     * Cancel the current batch transaction (rollback)
     */
    void CancelBatchTransaction();
    
    // Constructor needs to be public for MakeUnique, but should only be called by Get()
    FBatchOperations();
    
private:
    // Allow MakeUnique to access constructor
    template<typename T, typename... TArgs>
    friend TUniquePtr<T> MakeUnique(TArgs&&... Args);
    
    bool MatchesFilter(AActor* Actor, const FBatchFilter& Filter) const;
    bool MatchesFilter(const FAssetData& Asset, const FBatchFilter& Filter) const;
    bool MatchesPattern(const FString& String, const FString& Pattern) const;
    
    UWorld* GetEditorWorld() const;
    
    bool bInTransaction = false;
    
    static TUniquePtr<FBatchOperations> Instance;
};

/**
 * Convenience macros for common batch operations
 */
#define BATCH_DELETE_BY_CLASS(ClassName) \
    { \
        FBatchFilter F; \
        F.ActorClassNames.Add(TEXT(ClassName)); \
        FBatchOperations::Get().DeleteActors(F); \
    }

#define BATCH_SCALE_BY_CLASS(ClassName, Scale) \
    { \
        FBatchFilter F; \
        F.ActorClassNames.Add(TEXT(ClassName)); \
        FBatchTransformParams T; \
        T.ScaleMultiplier = Scale; \
        FBatchOperations::Get().TransformActors(F, T); \
    }
