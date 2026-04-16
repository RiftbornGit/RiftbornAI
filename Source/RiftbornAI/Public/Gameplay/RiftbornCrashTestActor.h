// Copyright Riftborn Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiftbornCrashTestActor.generated.h"

/**
 * ARiftbornCrashTestActor
 * 
 * Test actor that deliberately triggers Accessed None errors for certification testing.
 * 
 * This actor is used to verify that the RuntimeCert v1 no_log_errors invariant
 * correctly detects Blueprint Runtime Errors and Accessed None crashes.
 * 
 * WARNING: This actor will crash PIE on BeginPlay when bCrashOnBeginPlay is true!
 * Only use this for certification testing purposes.
 * 
 * Usage:
 *   1. Spawn this actor in a test map
 *   2. Set bCrashOnBeginPlay = true (or bLogErrorOnBeginPlay for soft test)
 *   3. Run PIE
 *   4. RuntimeCert should detect the error and fail no_log_errors check
 */
UCLASS(Blueprintable, Placeable, meta=(DisplayName="Riftborn Crash Test Actor"))
class RIFTBORNAI_API ARiftbornCrashTestActor : public AActor
{
	GENERATED_BODY()
	
public:
	ARiftbornCrashTestActor();

	virtual void BeginPlay() override;

	/** If true, will trigger an Accessed None crash on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crash Test")
	bool bCrashOnBeginPlay = true;

	/** If true, will log a Blueprint Runtime Error instead of actually crashing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crash Test")
	bool bLogErrorOnBeginPlay = false;

	/** Custom error message to log when bLogErrorOnBeginPlay is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crash Test")
	FString CustomErrorMessage = TEXT("Blueprint Runtime Error: Accessed None attempting to read property 'TestComponent' from null object");

private:
	/** Deliberately null pointer for crash testing. */
	UPROPERTY()
	USceneComponent* NullComponent = nullptr;

	/** Trigger the Accessed None crash. */
	void TriggerAccessedNone();

	/** Log a fake Blueprint Runtime Error. */
	void LogFakeError();
};
