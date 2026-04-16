// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RiftbornEditorAction.h"
#include "RiftbornBlueprintModifierAction.generated.h"

/**
 * Test action that modifies a Blueprint's display name
 * Used to verify transaction workflow and rollback
 */
UCLASS()
class RIFTBORNAI_API URiftbornBlueprintModifierAction : public URiftbornEditorAction
{
	GENERATED_BODY()

public:
	/** Blueprint asset path to modify */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FString BlueprintPath;

	/** New display name to set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FString NewDisplayName;

	/** Whether to simulate a failure (for rollback testing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	bool bSimulateFailure = false;

	virtual FRiftbornActionResult Execute_Implementation(const FRiftbornActionContext& Context) override;
	virtual TArray<UObject*> GetTouchedAssets_Implementation(const FRiftbornActionContext& Context) override;
	virtual FString GetActionName_Implementation() const override;
};
