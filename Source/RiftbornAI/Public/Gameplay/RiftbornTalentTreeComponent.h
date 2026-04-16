// Copyright 2026 Hivemind AI Bot. All Rights Reserved.
// PROOF-GRADE: Deterministic talent tree state for verifiable progression.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornTalentTreeComponent.generated.h"

class URiftbornStatsComponent;
class URiftbornTalentTreeDataAsset;

/**
 * FRiftbornTalentEffect - Single effect applied by a talent node.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornTalentEffect
{
	GENERATED_BODY()

	/** Stat name to modify. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	FName StatName;

	/** Operation: "add", "multiply", "set" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	FString Operation = TEXT("add");

	/** Value for the operation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	float Value = 0.0f;
};

/**
 * FRiftbornTalentNode - Configuration for a single talent node.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornTalentNode
{
	GENERATED_BODY()

	/** Unique identifier for this node. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	FName NodeId;

	/** Display name for UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	FText DisplayName;

	/** Cost in talent points to unlock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	int32 Cost = 1;

	/** Prerequisite node IDs (must be unlocked first). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	TArray<FName> Prerequisites;

	/** Effects applied when this node is unlocked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
	TArray<FRiftbornTalentEffect> Effects;
};

/**
 * URiftbornTalentTreeComponent - Proof-grade talent tree state.
 * 
 * PURPOSE: Authoritative state for talent progression with deterministic hashing.
 * Blueprint can render and configure, but truth lives here.
 * 
 * DESIGN RULES:
 * - UnlockedNodes is authoritative (not derived from elsewhere)
 * - All hashes are deterministic (sorted iteration, fixed formats)
 * - StateRevision increments on any mutation
 * - TryUnlock is atomic: either fully succeeds or fails with no state change
 * - Effects apply through URiftbornStatsComponent (not arbitrary properties)
 */
UCLASS(ClassGroup=(Riftborn), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftbornTalentTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornTalentTreeComponent();

	// ========================================================================
	// CONFIGURATION
	// ========================================================================

	/** Optional data asset containing node definitions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Talent|Config")
	URiftbornTalentTreeDataAsset* TreeData;

	/** Inline node definitions (used if TreeData is null). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Talent|Config")
	TArray<FRiftbornTalentNode> InlineNodes;

	// ========================================================================
	// STATE QUERIES (PROOF-GRADE, read-only)
	// ========================================================================

	/** Check if a node is unlocked. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	bool IsNodeUnlocked(FName NodeId) const;

	/** Get available points. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	int32 GetAvailablePoints() const { return AvailablePoints; }

	/** Get all unlocked node IDs (sorted lexicographically). */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	TArray<FName> GetUnlockedNodes() const;

	/** Get unlocked node count. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	int32 GetUnlockedNodeCount() const { return UnlockedNodes.Num(); }

	/** Get node definition by ID. Returns nullptr if not found. */
	const FRiftbornTalentNode* GetNodeDefinition(FName NodeId) const;

	/** Check if prerequisites are met for a node. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	bool ArePrerequisitesMet(FName NodeId) const;

	/** Check if node can be unlocked (prereqs met, enough points, not already unlocked). */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	bool CanUnlockNode(FName NodeId) const;

	// ========================================================================
	// STATE MUTATION (tracked, revision-incrementing)
	// ========================================================================

	/**
	 * Attempt to unlock a node.
	 * @param NodeId The node to unlock.
	 * @param OutReason If failed, explains why.
	 * @return True if unlock succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	bool TryUnlock(FName NodeId, FString& OutReason);

	/** Add points (for testing/scaffolding). */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	void AddPoints(int32 Delta);

	/** Set points to exact value (for testing/loading). */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	void SetPoints(int32 NewPoints);

	/** Reset all progression state. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	void ResetState();

	// ========================================================================
	// DETERMINISTIC FINGERPRINTING (PROOF-GRADE)
	// ========================================================================

	/**
	 * Get deterministic hash of unlocked nodes.
	 * Format: Sort node IDs lexicographically, join with "\n", SHA256 hex.
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent|Proof")
	FString GetUnlockedNodesHashHex() const;

	/**
	 * Get deterministic hash of full state.
	 * Includes: points, unlocked hash, revision.
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent|Proof")
	FString GetStateHashHex() const;

	/** Get current state revision (increments on any mutation). */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent|Proof")
	int32 GetStateRevision() const { return StateRevision; }

	// ========================================================================
	// SERIALIZATION (for save/load roundtrip)
	// ========================================================================

	/** Serialize state to JSON string. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent|Persistence")
	FString SerializeToJson() const;

	/** Deserialize state from JSON string. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent|Persistence")
	bool DeserializeFromJson(const FString& JsonString);

	// ========================================================================
	// EFFECT APPLICATION
	// ========================================================================

	/**
	 * Apply a node's effects to a stats component.
	 * Called automatically on unlock if StatsComponent is set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	void ApplyNodeEffects(FName NodeId, URiftbornStatsComponent* StatsComponent);

	/** Optional: auto-target stats component for effect application. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riftborn|Talent|Config")
	URiftbornStatsComponent* TargetStatsComponent;

protected:
	/** Set of unlocked node IDs. AUTHORITATIVE STATE. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Talent|State")
	TSet<FName> UnlockedNodes;

	/** Available talent points. AUTHORITATIVE STATE. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Talent|State")
	int32 AvailablePoints = 0;

	/** Revision counter - increments on any state change. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Talent|Proof")
	int32 StateRevision = 0;

	virtual void BeginPlay() override;

private:
	/** Get all node definitions (from TreeData or InlineNodes). */
	const TArray<FRiftbornTalentNode>& GetAllNodes() const;

	/** Increment revision (called after any mutation). */
	void IncrementRevision();
};
