// Copyright 2026 Hivemind AI Bot. All Rights Reserved.
// PROOF-GRADE: Deterministic talent tree data asset for node definitions.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RiftbornTalentTreeComponent.h"
#include "RiftbornTalentTreeDataAsset.generated.h"

/**
 * URiftbornTalentTreeDataAsset - Canonical configuration source for talent nodes.
 * 
 * PURPOSE: Define talent tree structure in a data-driven, Blueprint-editable way.
 * The TalentTreeComponent references this for node definitions.
 * 
 * DESIGN RULES:
 * - This is CONFIG, not STATE.
 * - NodeIds must be unique within the asset.
 * - Graph validation happens at edit time (not runtime).
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API URiftbornTalentTreeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Display name for this talent tree. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree")
	FText TreeDisplayName;

	/** Description of this talent tree. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree")
	FText TreeDescription;

	/** All talent nodes in this tree. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree")
	TArray<FRiftbornTalentNode> Nodes;

	/** Starting points when this tree is first acquired. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree")
	int32 InitialPoints = 0;

	// ========================================================================
	// VALIDATION / QUERIES
	// ========================================================================

	/** Find node by ID. Returns nullptr if not found. (C++ only - use GetNodeById for Blueprint) */
	const FRiftbornTalentNode* FindNode(FName NodeId) const;

	/** Get all node IDs in this tree. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	TArray<FName> GetAllNodeIds() const;

	/** Check if a node ID exists in this tree. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	bool HasNode(FName NodeId) const;

	/** Validate tree integrity (unique IDs, valid prereqs). Returns error messages. */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Talent")
	TArray<FString> ValidateTree() const;

#if WITH_EDITOR
	/** Editor validation. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
