// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArenaTelemetrySubsystem.generated.h"

/**
 * End state of an arena match.
 */
UENUM(BlueprintType)
enum class EArenaEndState : uint8
{
	None UMETA(DisplayName = "None"),
	Win UMETA(DisplayName = "Win"),
	Lose UMETA(DisplayName = "Lose"),
};

/**
 * Telemetry data for arena validation.
 * 
 * This is the SINGLE SOURCE OF TRUTH for gameplay validation.
 * Without this, your "gameplay validation" is flimsy guesswork.
 */
USTRUCT(BlueprintType)
struct FArenaTelemData
{
	GENERATED_BODY()

	/** Has the player pawn spawned? */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	bool bPlayerSpawned = false;

	/** Number of enemies that have spawned */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 EnemySpawnedCount = 0;

	/** Total damage events (player took damage OR enemy took damage) */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 DamageEventsCount = 0;

	/** Total kills (enemies killed by player) */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 KillsCount = 0;

	/** Is the player currently alive? */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	bool bPlayerAlive = true;

	/** Current end state */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	EArenaEndState EndState = EArenaEndState::None;

	/** Last error message if any */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	FString LastError;

	/** Timestamp when match started (PIE) */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	float MatchStartTime = 0.0f;

	/** Timestamp when end state was reached */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	float MatchEndTime = 0.0f;

	/** Convert to JSON for proof bundle */
	FString ToJson() const
	{
		return FString::Printf(
			TEXT("{\"player_spawned\":%s,\"enemy_spawned\":%d,\"damage_events\":%d,\"kills\":%d,\"player_alive\":%s,\"end_state\":\"%s\",\"last_error\":\"%s\"}"),
			bPlayerSpawned ? TEXT("true") : TEXT("false"),
			EnemySpawnedCount,
			DamageEventsCount,
			KillsCount,
			bPlayerAlive ? TEXT("true") : TEXT("false"),
			*UEnum::GetValueAsString(EndState),
			*LastError.Replace(TEXT("\""), TEXT("\\\""))
		);
	}
};

/**
 * ArenaTelemetrySubsystem - Tracks gameplay events for validation.
 * 
 * This subsystem provides DETERMINISTIC and PROVABLE validation of gameplay.
 * All arena components (spawners, health, etc.) should call into this to record events.
 * 
 * Usage from Python:
 *   result = execute_python('import unreal; telem = unreal.ArenaTelemetrySubsystem.get(); print(telem.get_telemetry().to_json())')
 * 
 * Usage from Blueprint:
 *   Get Arena Telemetry Subsystem -> Get Telemetry -> Read values
 */
UCLASS()
class RIFTBORNAI_API UArenaTelemetrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	/** Get the telemetry subsystem from a world context */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry", meta = (WorldContext = "WorldContextObject"))
	static UArenaTelemetrySubsystem* Get(const UObject* WorldContextObject);

	/** Reset all telemetry for a new match */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	void ResetTelemetry();

	/** Get current telemetry data */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	FArenaTelemData GetTelemetry() const { return TelemetryData; }

	/** Get telemetry as JSON string for Python probes */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	FString GetTelemetryJson() const { return TelemetryData.ToJson(); }

	//
	// Event recording - call these from gameplay components
	//

	/** Record that the player has spawned */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	void RecordPlayerSpawned();

	/** Record that an enemy has spawned */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	void RecordEnemySpawned();

	/** Record a damage event (any damage to player or enemy) */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	void RecordDamageEvent(float Damage, bool bIsPlayerDamage);

	/** Record a kill (enemy killed by player) */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	void RecordKill();

	/** Record player death */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	void RecordPlayerDeath();

	/** Record match end with given state */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	void RecordMatchEnd(EArenaEndState State);

	/** Record an error */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	void RecordError(const FString& Error);

	/** Check if all validation requirements are met */
	UFUNCTION(BlueprintCallable, Category = "Arena|Telemetry")
	bool ValidateRequirements(bool bRequireDamage, bool bRequireEndState) const;

	//
	// Delegates for external listeners
	//
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndStateReached, EArenaEndState, EndState);
	
	/** Fired when end state is reached */
	UPROPERTY(BlueprintAssignable, Category = "Arena|Telemetry")
	FOnEndStateReached OnEndStateReached;

private:
	/** The telemetry data */
	FArenaTelemData TelemetryData;
};
