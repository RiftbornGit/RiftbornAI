// Copyright Riftborn. All Rights Reserved.
// Voxel Terrain Replication — multiplayer edit synchronization.
// Server-authoritative: server applies edits and multicasts to all clients.
// Clients can request edits via Server RPC.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelTerrainReplication.generated.h"

class FLifetimeProperty;

// Replicated voxel edit operation
USTRUCT()
struct FReplicatedVoxelEdit
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 Type = 0; // 0=SubtractSphere, 1=AddSphere, 2=Smooth, 3=SubtractBox, 4=AddBox

	UPROPERTY()
	FVector Center = FVector::ZeroVector;

	UPROPERTY()
	float Radius = 0.0f;

	UPROPERTY()
	FVector Extent = FVector::ZeroVector; // For box operations

	UPROPERTY()
	float Strength = 0.5f;

	UPROPERTY()
	uint8 MaterialID = 0;

	UPROPERTY()
	uint32 SequenceNumber = 0;
};

/**
 * UVoxelTerrainReplicationComponent — handles multiplayer edit sync.
 * Add to the VoxelTerrainHost actor alongside UVoxelTerrainComponent.
 *
 * Flow:
 * 1. Client calls ServerRequestEdit (Server RPC)
 * 2. Server validates, applies edit to VoxelTerrainComponent
 * 3. Server calls MulticastApplyEdit (Multicast RPC)
 * 4. All clients apply the edit locally
 */
UCLASS(ClassGroup=(RiftbornAI), meta=(BlueprintSpawnableComponent))
class RIFTBORNAI_API UVoxelTerrainReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelTerrainReplicationComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Request the server to apply an edit (called by clients)
	UFUNCTION(Server, Reliable, WithValidation, Category = "Voxel Replication")
	void ServerRequestEdit(FReplicatedVoxelEdit Edit);

	// Broadcast an edit to all clients (called by server after validation)
	UFUNCTION(NetMulticast, Reliable, Category = "Voxel Replication")
	void MulticastApplyEdit(FReplicatedVoxelEdit Edit);

	// Blueprint-callable: request a dig at world position
	UFUNCTION(BlueprintCallable, Category = "Voxel Replication")
	void RequestDig(FVector Center, float Radius);

	UFUNCTION(BlueprintCallable, Category = "Voxel Replication")
	void RequestRaise(FVector Center, float Radius);

	UFUNCTION(BlueprintCallable, Category = "Voxel Replication")
	void RequestDigBox(FVector Center, FVector Extent);

private:
	UPROPERTY()
	uint32 NextSequenceNumber = 0;

	void ApplyEditLocally(const FReplicatedVoxelEdit& Edit);
};
