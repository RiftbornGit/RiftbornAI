// Copyright 2026 Hivemind AI Bot. All Rights Reserved.
// PROOF-GRADE: Deterministic stat surface for verifiable gameplay effects.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornStatsComponent.generated.h"

/**
 * URiftbornStatsComponent - Minimal, deterministic stat surface.
 * 
 * PURPOSE: Provide a stable, proof-grade substrate for gameplay effects.
 * Talents, abilities, and items modify stats here. Predicates verify here.
 * 
 * DESIGN RULES:
 * - All stat access is deterministic (sorted iteration)
 * - All mutations increment StateRevision
 * - Hash is stable across sessions (sorted keys, fixed precision)
 * - No random state, no time-dependent behavior
 */
UCLASS(ClassGroup=(Riftborn), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftbornStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornStatsComponent();

	// ========================================================================
	// STAT ACCESS (PROOF-GRADE)
	// ========================================================================

	/** Get stat value. Returns DefaultValue if stat doesn't exist. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats")
	float GetStat(FName StatName, float DefaultValue = 0.0f) const;

	/** Check if stat exists. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats")
	bool HasStat(FName StatName) const;

	/** Get all stat names (sorted). */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats")
	TArray<FName> GetAllStatNames() const;

	// ========================================================================
	// STAT MUTATION (tracked, revision-incrementing)
	// ========================================================================

	/** Set stat to absolute value. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats")
	void SetStat(FName StatName, float Value);

	/** Add delta to stat (creates with delta if doesn't exist). */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats")
	void AddStat(FName StatName, float Delta);

	/** Multiply stat by factor. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats")
	void MultiplyStat(FName StatName, float Factor);

	/** Remove stat entirely. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats")
	void RemoveStat(FName StatName);

	/** Reset all stats to empty. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats")
	void ResetStats();

	// ========================================================================
	// DETERMINISTIC FINGERPRINTING (PROOF-GRADE)
	// ========================================================================

	/** 
	 * Get deterministic hash of all stats.
	 * Format: Sort keys lexicographically, join as "key=value\n", SHA256 hex.
	 * Values formatted to 6 decimal places for stability.
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats|Proof")
	FString GetStatsHashHex() const;

	/** Get current state revision (increments on any mutation). */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats|Proof")
	int32 GetStateRevision() const { return StateRevision; }

	/** Get stat count. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats|Proof")
	int32 GetStatCount() const { return Stats.Num(); }

	// ========================================================================
	// SERIALIZATION (for save/load roundtrip)
	// ========================================================================

	/** Serialize stats to JSON string. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats|Persistence")
	FString SerializeToJson() const;

	/** Deserialize stats from JSON string. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Stats|Persistence")
	bool DeserializeFromJson(const FString& JsonString);

protected:
	/** The stat storage. Keys are stat names, values are current values. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Stats")
	TMap<FName, float> Stats;

	/** Revision counter - increments on any state change. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Riftborn|Stats|Proof")
	int32 StateRevision = 0;

private:
	/** Increment revision (called after any mutation). */
	void IncrementRevision();
};
