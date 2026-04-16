// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RiftbornAssetSnapshot.h"
#include "RiftbornVerificationPipeline.h"
#include "RiftbornActionManifest.generated.h"

/**
 * Record of an action execution
 * Provides provenance and audit trail
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornActionManifest
{
	GENERATED_BODY()

	/** Unique action ID */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ActionId;

	/** Request ID from client */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString RequestId;

	/** Action name/type */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ActionName;

	/** When action started */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime StartTime = FDateTime();

	/** When action completed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime EndTime = FDateTime();

	/** Execution duration in milliseconds */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	float DurationMs = 0.0f;

	/** Assets modified by this action */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> AssetsModified;

	/** Assets created by this action */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> AssetsCreated;

	/** Files created (Python/C++) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> FilesCreated;

	/** Snapshot file paths */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> SnapshotPaths;

	/** UE transaction count during action */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 EngineTransactionCount = 0;

	/** Whether action succeeded */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bSuccess = false;

	/** Whether verification passed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bVerificationPassed = false;

	/** Verification results */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FRiftbornPipelineResult VerificationResult;

	/** Error messages */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Errors;

	/** Warning messages */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Warnings;

	/** Whether rollback was needed */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bRollbackTriggered = false;

	/** Whether rollback succeeded */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bRollbackSucceeded = false;

	/** Summary message */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Summary;

	/** Python code executed (if Python action) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString PythonCode;

	/** Git commit hash at time of action (if available) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString GitCommitHash;

	/** User who triggered action */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString User;

	/** Machine/hostname */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Machine;

	FRiftbornActionManifest()
		: StartTime(FDateTime::Now())
		, EndTime(FDateTime::Now())
	{
	}

	/** Generate unique action ID */
	void GenerateActionId()
	{
		ActionId = FGuid::NewGuid().ToString();
	}

	/** Mark action as started */
	void MarkStarted()
	{
		StartTime = FDateTime::Now();
	}

	/** Mark action as completed */
	void MarkCompleted()
	{
		EndTime = FDateTime::Now();
		DurationMs = (EndTime - StartTime).GetTotalMilliseconds();
	}

	/** Export to JSON */
	FString ToJson() const;

	/** Import from JSON */
	bool FromJson(const FString& JsonString);

	/** Save to disk */
	bool SaveToDisk(const FString& FilePath) const;

	/** Load from disk */
	static bool LoadFromDisk(const FString& FilePath, FRiftbornActionManifest& OutManifest);
};

/**
 * Subsystem for managing action manifests
 * Provides an audit trail of recorded actions
 */
UCLASS()
class RIFTBORNAI_API URiftbornManifestSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Create new manifest for an action
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Manifest")
	FRiftbornActionManifest CreateManifest(const FString& ActionName, const FString& RequestId);

	/**
	 * Save manifest to disk
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Manifest")
	bool SaveManifest(const FRiftbornActionManifest& Manifest);

	/**
	 * Get all manifests for a date range
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Manifest")
	TArray<FRiftbornActionManifest> GetManifests(FDateTime StartDate, FDateTime EndDate);

	/**
	 * Get manifest by action ID
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Manifest")
	bool GetManifestByActionId(const FString& ActionId, FRiftbornActionManifest& OutManifest);

	/**
	 * Get all failed actions
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Manifest")
	TArray<FRiftbornActionManifest> GetFailedActions();

	/**
	 * Get all actions that required rollback
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Manifest")
	TArray<FRiftbornActionManifest> GetRolledBackActions();

	/**
	 * Clean up old manifests (older than days)
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Manifest")
	int32 CleanupOldManifests(int32 DaysToKeep);

	/**
	 * Get manifest directory
	 */
	FString GetManifestDirectory() const;

private:
	/** Generate manifest file path */
	FString GetManifestFilePath(const FString& ActionId) const;

	/** Ensure manifest directory exists */
	void EnsureManifestDirectoryExists();
};
