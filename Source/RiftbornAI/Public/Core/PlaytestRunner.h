// PlaytestRunner.h — Lightweight automated PIE verification
//
// After an agentic session that creates gameplay-relevant systems
// (GameMode, Character, Input, GAS, etc.), automatically start PIE,
// verify the game is minimally functional, and report results.
//
// This is NOT the full FAutomatedPlaytester (which does persona-driven
// exploratory testing). This is a quick sanity check: "does the game
// even run?"

#pragma once

#include "CoreMinimal.h"

/**
 * Result of a quick playtest verification.
 * Captures the minimum signals needed to determine if a gameplay
 * session is fundamentally working.
 */
struct RIFTBORNAI_API FQuickPlaytestResult
{
    // ======== Core health checks ========
    bool bPIEStarted = false;         // Did PIE launch at all?
    bool bPIESurvived = false;        // Did PIE stay alive for the test duration?
    bool bPawnSpawned = false;        // Is there a player pawn in the world?
    bool bPawnHasController = false;  // Does the pawn have a PlayerController?
    bool bPawnCanMove = false;        // Did the pawn expose a movement-ready signal (movement component or observed displacement)?
    bool bNoCrashes = true;           // No crashes/ensures during PIE?

    // ======== Performance ========
    float AvgFPS = 0.0f;             // Average FPS during test window
    float MinFPS = 0.0f;             // Minimum FPS observed
    bool bPerformanceAcceptable = false; // Above minimum threshold (30 FPS)

    // ======== Diagnostics ========
    FString PawnClassName;            // What class is the player pawn?
    FString GameModeClassName;        // What GameMode is active?
    FVector PawnStartLocation = FVector::ZeroVector;
    FVector PawnEndLocation = FVector::ZeroVector;
    int32 ActorCount = 0;            // Total actors in PIE world
    float TestDurationSeconds = 0.0f; // How long the test ran

    // ======== Error collection ========
    TArray<FString> Errors;           // Error messages collected
    TArray<FString> Warnings;         // Warning messages collected

    /** Is the playtest a pass? All critical checks must pass. */
    bool Passed() const
    {
        return bPIEStarted && bPIESurvived && bPawnSpawned && bNoCrashes && bPerformanceAcceptable;
    }

    /** Human-readable summary for injection into the LLM. */
    FString ToSummaryString() const
    {
        FString Summary;
        Summary += FString::Printf(TEXT("=== PLAYTEST RESULT: %s ===\n"), Passed() ? TEXT("PASS") : TEXT("FAIL"));
        Summary += FString::Printf(TEXT("PIE Started: %s\n"), bPIEStarted ? TEXT("YES") : TEXT("NO"));
        Summary += FString::Printf(TEXT("PIE Survived: %s (%.1fs)\n"), bPIESurvived ? TEXT("YES") : TEXT("CRASHED"), TestDurationSeconds);
        Summary += FString::Printf(TEXT("Pawn Spawned: %s"), bPawnSpawned ? TEXT("YES") : TEXT("NO"));
        if (bPawnSpawned)
        {
            Summary += FString::Printf(TEXT(" (%s)"), *PawnClassName);
        }
        Summary += TEXT("\n");
        Summary += FString::Printf(TEXT("Has Controller: %s\n"), bPawnHasController ? TEXT("YES") : TEXT("NO"));
        Summary += FString::Printf(TEXT("Movement Ready: %s"), bPawnCanMove ? TEXT("YES") : TEXT("NO"));
        if (bPawnCanMove)
        {
            float Dist = FVector::Dist(PawnStartLocation, PawnEndLocation);
            Summary += FString::Printf(TEXT(" (moved %.1f units)"), Dist);
        }
        Summary += TEXT("\n");
        Summary += FString::Printf(TEXT("Performance: %.0f avg FPS, %.0f min FPS — %s\n"),
            AvgFPS, MinFPS, bPerformanceAcceptable ? TEXT("OK") : TEXT("BELOW 30 FPS"));
        Summary += FString::Printf(TEXT("GameMode: %s\n"), *GameModeClassName);
        Summary += FString::Printf(TEXT("Actors in world: %d\n"), ActorCount);

        if (Errors.Num() > 0)
        {
            Summary += FString::Printf(TEXT("\nErrors (%d):\n"), Errors.Num());
            for (int32 i = 0; i < FMath::Min(Errors.Num(), 5); i++)
            {
                Summary += FString::Printf(TEXT("- %s\n"), *Errors[i].Left(200));
            }
        }
        if (Warnings.Num() > 0)
        {
            Summary += FString::Printf(TEXT("\nWarnings (%d):\n"), Warnings.Num());
            for (int32 i = 0; i < FMath::Min(Warnings.Num(), 3); i++)
            {
                Summary += FString::Printf(TEXT("- %s\n"), *Warnings[i].Left(200));
            }
        }

        // Actionable guidance for the LLM
        if (!Passed())
        {
            Summary += TEXT("\n--- WHAT TO FIX ---\n");
            if (!bPIEStarted)
            {
                Summary += TEXT("- PIE failed to start. Check for compile errors or missing GameMode.\n");
            }
            if (!bPIESurvived)
            {
                Summary += TEXT("- PIE crashed. Check Output Log for fatal errors. Likely a null pointer in BeginPlay.\n");
            }
            if (bPIEStarted && !bPawnSpawned)
            {
                Summary += TEXT("- No player pawn. Check: GameMode has DefaultPawnClass set, PlayerStart actor exists in level.\n");
            }
            if (bPawnSpawned && !bPawnHasController)
            {
                Summary += TEXT("- Pawn has no controller. Check: GameMode PlayerControllerClass is set.\n");
            }
            if (bPawnSpawned && !bPawnCanMove)
            {
                Summary += TEXT("- Pawn lacks a movement-ready signal. Check: locomotion component initializes correctly and verify live input bindings separately.\n");
            }
            if (!bPerformanceAcceptable)
            {
                Summary += FString::Printf(TEXT("- FPS too low (%.0f avg). Check for heavy tick logic, too many actors, or unoptimized rendering.\n"), AvgFPS);
            }
        }

        return Summary;
    }
};

/**
 * FPlaytestRunner — Quick automated PIE verification
 *
 * Lifecycle:
 *   1. RunQuickTestAsync() → starts PIE on game thread
 *   2. Polls for ~3 seconds, sampling FPS and checking pawn/controller plus a bounded movement-ready signal
 *   3. Stops PIE
 *   4. Calls completion callback with FQuickPlaytestResult
 *
 * This runs entirely on the game thread via ticker callbacks
 * (PIE requires GT access for world queries).
 */
class RIFTBORNAI_API FPlaytestRunner
{
public:
    static FPlaytestRunner& Get();

    /**
     * Run a quick playtest asynchronously.
     * Starts PIE, runs checks for a few seconds, stops PIE, reports results.
     * @param OnComplete - Called on game thread when test finishes
     */
    void RunQuickTestAsync(TFunction<void(const FQuickPlaytestResult&)> OnComplete);

    /** Start a quick playtest without registering a completion callback. */
    void RunQuickTest();

    /** Is a test currently running? */
    bool IsRunning() const { return bIsRunning; }
    const FQuickPlaytestResult& GetLastResult() const { return CurrentResult; }

    /** Configuration */
    void SetTestDuration(float Seconds) { TestDurationSeconds = FMath::Clamp(Seconds, 1.0f, 10.0f); }
    void SetMinAcceptableFPS(float FPS) { MinAcceptableFPS = FMath::Max(FPS, 10.0f); }

private:
    FPlaytestRunner() = default;

    // Test execution phases (called via ticker)
    void StartPIE();
    void PollPIE(float DeltaTime);
    void FinishTest();

    // Helpers
    void CollectPawnInfo(UWorld* PIEWorld);
    void SampleFPS();
    void StopPIE();

    // State
    bool bIsRunning = false;
    float TestDurationSeconds = 3.0f;
    float MinAcceptableFPS = 30.0f;
    float ElapsedTime = 0.0f;
    TArray<float> FPSSamples;

    // Result being built
    FQuickPlaytestResult CurrentResult;
    TFunction<void(const FQuickPlaytestResult&)> CompletionCallback;

    // Ticker handle for polling
    FTSTicker::FDelegateHandle TickerHandle;

    // Pawn tracking
    bool bInitialPawnCheckDone = false;
    FVector InitialPawnLocation = FVector::ZeroVector;

    // Was PIE already running before we started? (don't stop it if so)
    bool bPIEWasAlreadyRunning = false;
};
