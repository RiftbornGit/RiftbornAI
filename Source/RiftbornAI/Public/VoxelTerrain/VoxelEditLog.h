// Copyright Riftborn. All Rights Reserved.
#pragma once

#if RIFTBORN_WITH_GEOMETRY_SCRIPTING

#include "CoreMinimal.h"

/** Type of voxel edit operation. */
enum class EVoxelEditType : uint8
{
	SubtractSphere = 0,
	AddSphere,
	AddSphereWithMaterial,
	SmoothSphere,
	SubtractBox,
	AddBox
};

/** A single deterministic voxel edit that can be replicated across clients. */
struct FVoxelEditOp
{
	EVoxelEditType Type = EVoxelEditType::SubtractSphere;
	FVector Center = FVector::ZeroVector;
	float Radius = 0.0f;
	FVector Extent = FVector::ZeroVector;
	float Strength = 0.0f;      // For SmoothSphere; 0 otherwise
	uint8 MaterialID = 0;       // For AddSphereWithMaterial; 0 otherwise
	double Timestamp = 0.0;
	uint32 SequenceNumber = 0;
	uint32 PlayerID = 0;

	void Serialize(FArchive& Ar)
	{
		uint8 TypeByte = static_cast<uint8>(Type);
		Ar << TypeByte;
		if (Ar.IsLoading())
		{
			Type = static_cast<EVoxelEditType>(TypeByte);
		}
		Ar << Center;
		Ar << Radius;
		Ar << Extent;
		Ar << Strength;
		Ar << MaterialID;
		Ar << Timestamp;
		Ar << SequenceNumber;
		Ar << PlayerID;
	}

	bool IsValid() const
	{
		if (Type == EVoxelEditType::SubtractBox || Type == EVoxelEditType::AddBox)
		{
			return Extent.X > 0.0f && Extent.Y > 0.0f && Extent.Z > 0.0f && Extent.GetMax() < 50000.0f;
		}
		return Radius > 0.0f && Radius < 50000.0f;
	}
};

/** Filter for relevancy-based edit replication. */
struct FVoxelEditRelevancyFilter
{
	FVector PlayerPosition = FVector::ZeroVector;
	float RelevancyRadius = 0.0f;

	bool IsRelevant(const FVoxelEditOp& Op) const
	{
		if (RelevancyRadius <= 0.0f)
		{
			return true;
		}
		const float OpRadius = (Op.Type == EVoxelEditType::SubtractBox || Op.Type == EVoxelEditType::AddBox)
			? Op.Extent.Size()
			: Op.Radius;
		return FVector::Dist(PlayerPosition, Op.Center) < (RelevancyRadius + OpRadius);
	}
};

/**
 * Thread-safe ordered log of voxel edit operations.
 * Supports deterministic replay, relevancy-filtered replication, and compaction.
 */
class FVoxelEditLog
{
public:
	/**
	 * Record a new edit. Assigns a sequence number and appends to the log.
	 * @return Assigned sequence number, or 0 if the edit was invalid.
	 */
	uint32 RecordEdit(FVoxelEditOp& Op);

	/** Insert a received edit in sequence order (client-side replication). */
	void ReceiveEdit(const FVoxelEditOp& Op);

	/** Get all edits after a given sequence number, filtered by relevancy. */
	TArray<FVoxelEditOp> GetEditsSince(
		uint32 SequenceNumber,
		const FVoxelEditRelevancyFilter& Filter) const;

	/** Get all edits whose sphere overlaps a world-space region. */
	TArray<FVoxelEditOp> GetEditsInRegion(const FVector& Center, float Radius) const;

	int32 GetEditCount() const;
	uint32 GetLatestSequence() const;

	void Clear();

	/**
	 * Remove edits that are fully superseded by later edits.
	 * An edit is superseded if a later edit of the same type fully contains it spatially.
	 */
	void Compact(uint32 BeforeSequence);

	void Serialize(FArchive& Ar);

private:
	TArray<FVoxelEditOp> Edits;
	uint32 NextSequence = 1;
	mutable FCriticalSection EditLock;

	bool ValidateEdit(const FVoxelEditOp& Op) const;
};

#endif // RIFTBORN_WITH_GEOMETRY_SCRIPTING
