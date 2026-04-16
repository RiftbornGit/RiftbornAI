// Copyright Riftborn Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiftbornSessionProbe.generated.h"

/**
 * ARiftbornSessionProbe
 * 
 * Minimal actor placed in arena maps to prove PIE session started successfully.
 * Logs a sentinel line on BeginPlay that automation tests can detect.
 * 
 * Usage:
 *   1. Place this actor in your arena map
 *   2. Run PIE (manually or via automation test)
 *   3. Check log for: RIFTBORN_SESSION_BEGINPLAY
 * 
 * The automation test looks for this line to confirm the session is alive.
 */
UCLASS(Blueprintable, Placeable, meta=(DisplayName="Riftborn Session Probe"))
class RIFTBORNAI_API ARiftbornSessionProbe : public AActor
{
	GENERATED_BODY()
	
public:
	ARiftbornSessionProbe();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Has BeginPlay been called? Automation tests poll this. */
	UPROPERTY(BlueprintReadOnly, Category="Riftborn")
	bool bSessionReady = false;

	/** Tick count since BeginPlay. Used to prove the world is actually ticking. */
	UPROPERTY(BlueprintReadOnly, Category="Riftborn")
	int32 TickCount = 0;

	/** Map name captured at BeginPlay for logging. */
	UPROPERTY(BlueprintReadOnly, Category="Riftborn")
	FString MapName;

private:
	/** Log the sentinel line that Python/automation will parse. */
	void LogSessionReady();
};
