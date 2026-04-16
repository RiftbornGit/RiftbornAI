// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// Proof Binding Component - Logs sensor/actuator calls with cryptographic hashes
//
// PURPOSE:
// In PROOF mode, every sensor read and actuator write must be auditable.
// This component maintains a tamper-evident log of all creature I/O
// that can be verified against Python-side proof artifacts.
//
// USAGE:
// 1. Add to any creature actor that needs PROOF binding
// 2. Call LogSensorRead/LogActuatorWrite from your code
// 3. At episode end, get the proof hash via GetProofChainHash()
// 4. Compare against Python-side proof to detect tampering

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftbornProofBindingComponent.generated.h"

/**
 * Single entry in the proof chain
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FProofChainEntry
{
	GENERATED_BODY()
	
	/** Sequence number (monotonic) */
	UPROPERTY(BlueprintReadOnly, Category = "Proof")
	int64 SequenceNumber = 0;
	
	/** World time when entry was recorded */
	UPROPERTY(BlueprintReadOnly, Category = "Proof")
	double WorldTimeSeconds = 0.0;
	
	/** Type of operation: "sensor" or "actuator" */
	UPROPERTY(BlueprintReadOnly, Category = "Proof")
	FString OperationType;
	
	/** Name of the operation (e.g., "creature_get_state", "creature_move") */
	UPROPERTY(BlueprintReadOnly, Category = "Proof")
	FString OperationName;
	
	/** SHA256 hash of the operation arguments */
	UPROPERTY(BlueprintReadOnly, Category = "Proof")
	FString ArgsHash;
	
	/** SHA256 hash of the operation result */
	UPROPERTY(BlueprintReadOnly, Category = "Proof")
	FString ResultHash;
	
	/** Previous chain hash (for tamper evidence) */
	UPROPERTY(BlueprintReadOnly, Category = "Proof")
	FString PreviousChainHash;
	
	/** This entry's hash (includes all above fields) */
	UPROPERTY(BlueprintReadOnly, Category = "Proof")
	FString EntryHash;
};

/**
 * Component that maintains a cryptographic proof chain of sensor/actuator operations.
 * 
 * Each operation is logged with:
 * - Sequence number (monotonic, gap-free)
 * - World timestamp
 * - Operation type and name
 * - Hash of arguments
 * - Hash of results
 * - Chain link to previous entry (tamper-evident)
 * 
 * The final chain hash can be compared against Python-side proofs to verify
 * that no operations were missed, modified, or reordered.
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API URiftbornProofBindingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftbornProofBindingComponent();

	// === Proof Chain API ===
	
	/**
	 * Log a sensor read operation to the proof chain.
	 * 
	 * @param SensorName Name of the sensor (e.g., "creature_get_state")
	 * @param ArgsJson JSON string of the sensor arguments
	 * @param ResultJson JSON string of the sensor result
	 * @return The sequence number assigned to this entry
	 */
	UFUNCTION(BlueprintCallable, Category = "Proof")
	int64 LogSensorRead(const FString& SensorName, const FString& ArgsJson, const FString& ResultJson);
	
	/**
	 * Log an actuator write operation to the proof chain.
	 * 
	 * @param ActuatorName Name of the actuator (e.g., "creature_move")
	 * @param ArgsJson JSON string of the actuator arguments
	 * @param ResultJson JSON string of the actuator result
	 * @return The sequence number assigned to this entry
	 */
	UFUNCTION(BlueprintCallable, Category = "Proof")
	int64 LogActuatorWrite(const FString& ActuatorName, const FString& ArgsJson, const FString& ResultJson);
	
	/**
	 * Get the current proof chain hash.
	 * This hash represents the entire chain and will change if any entry is modified.
	 */
	UFUNCTION(BlueprintPure, Category = "Proof")
	FString GetProofChainHash() const;
	
	/**
	 * Get the number of entries in the proof chain.
	 */
	UFUNCTION(BlueprintPure, Category = "Proof")
	int32 GetProofChainLength() const;
	
	/**
	 * Get a specific entry from the proof chain.
	 * Returns false if index is out of bounds.
	 */
	UFUNCTION(BlueprintPure, Category = "Proof")
	bool GetProofChainEntry(int32 Index, FProofChainEntry& OutEntry) const;
	
	/**
	 * Clear the proof chain (for new episode).
	 * @param SessionId Optional session identifier for the new chain
	 */
	UFUNCTION(BlueprintCallable, Category = "Proof")
	void ResetProofChain(const FString& SessionId = TEXT(""));
	
	/**
	 * Export the entire proof chain as a JSON string.
	 */
	UFUNCTION(BlueprintPure, Category = "Proof")
	FString ExportProofChainJson() const;
	
	/**
	 * Get the session ID for this proof chain.
	 */
	UFUNCTION(BlueprintPure, Category = "Proof")
	FString GetSessionId() const { return SessionId; }
	
	/**
	 * Get the genesis hash for this proof chain.
	 * This is computed from the session ID and used as the first prev_hash.
	 */
	UFUNCTION(BlueprintPure, Category = "Proof")
	FString GetGenesisHash() const { return GenesisHash; }

protected:
	virtual void BeginPlay() override;

private:
	/** Compute SHA256 hash of a string */
	static FString ComputeSHA256(const FString& Input);
	
	/** Compute genesis hash from session ID */
	static FString ComputeGenesisHash(const FString& InSessionId);
	
	/** Log an operation to the chain (internal) */
	int64 LogOperation(const FString& OpType, const FString& OpName, const FString& ArgsJson, const FString& ResultJson);
	
	/** The proof chain entries */
	UPROPERTY()
	TArray<FProofChainEntry> ProofChain;
	
	/** Current sequence number */
	int64 NextSequenceNumber = 0;
	
	/** Current chain hash (updated after each entry) */
	FString CurrentChainHash;
	
	/** Genesis hash (computed from session ID) */
	FString GenesisHash;
	
	/** Session identifier */
	FString SessionId;
};
