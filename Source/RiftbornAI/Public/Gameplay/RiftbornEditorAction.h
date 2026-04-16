// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RiftbornAssetSnapshot.h"
#include "RiftbornEditorAction.generated.h"

/**
 * Context passed to editor actions
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornActionContext
{
	GENERATED_BODY()

	/** Project root path */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ProjectRoot;

	/** Current selection in editor */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<UObject*> SelectedObjects;

	/** Additional parameters (JSON string) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ExtraParams;

	/** Request ID for tracking */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString RequestId;
};

// Forward declare
struct FRiftbornActionableError;

/**
 * Result of an editor action
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornActionResult
{
	GENERATED_BODY()

	/** Whether action succeeded */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bSuccess = false;

	/** Error messages (legacy - use ActionableErrors for new code) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Errors;

	/** Warning messages */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Warnings;

	/** Assets touched by this action */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<UObject*> TouchedAssets;

	/** Human-readable summary */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Summary;

	void AddError(const FString& Error)
	{
		bSuccess = false;
		Errors.Add(Error);
	}

	void AddWarning(const FString& Warning)
	{
		Warnings.Add(Warning);
	}
	
	/** Add error and translate to actionable error */
	void AddActionableError(const FString& RawError);
	
	/** Get formatted error message for user display */
	FString GetFormattedErrors() const;
};

/**
 * Base interface for editor actions
 */
UCLASS(Abstract, Blueprintable)
class RIFTBORNAI_API URiftbornEditorAction : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Execute the action
	 * @param Context - Action context with parameters
	 * @return Result with success/failure and touched assets
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "RiftbornAI|Action")
	FRiftbornActionResult Execute(const FRiftbornActionContext& Context);

	/**
	 * Get list of assets that will be touched by this action
	 * Called before execution to enable pre-snapshotting
	 * @param Context - Action context
	 * @return Array of assets that will be modified
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "RiftbornAI|Action")
	TArray<UObject*> GetTouchedAssets(const FRiftbornActionContext& Context);

	/**
	 * Get human-readable name of this action
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "RiftbornAI|Action")
	FString GetActionName() const;

protected:
	// Default implementations
	virtual FRiftbornActionResult Execute_Implementation(const FRiftbornActionContext& Context);
	virtual TArray<UObject*> GetTouchedAssets_Implementation(const FRiftbornActionContext& Context);
	virtual FString GetActionName_Implementation() const;
};
