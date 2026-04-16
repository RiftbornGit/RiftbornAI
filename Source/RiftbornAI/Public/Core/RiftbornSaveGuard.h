// RiftbornSaveGuard.h — Centralized level save with D3D12 crash mitigation
//
// Problem: FEditorFileUtils::SaveCurrentLevel() triggers DataValidation which
// calls Slate render during save. On D3D12, this races the swapchain Present()
// causing E_ACCESSDENIED (0x80070005) crash.
//
// Solution: All tool code calls Riftborn_SaveCurrentLevel() instead.
// This defers the actual save and batches it, avoiding the crash.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "FileHelpers.h"
#include "Misc/FeedbackContext.h"
#include "Misc/SlowTask.h"
#include "ShaderCompiler.h"
#include "UObject/Package.h"
#include "RiftbornLog.h"

struct FRiftbornSaveGuardState
{
    bool bDeferSaves = false;  // Default OFF: request-scoped defer guards batch and flush saves safely
    bool bPendingSave = false;
    bool bSerializationBusy = false;
    bool bEngineSavingPackage = false;
    bool bEngineGarbageCollecting = false;
    bool bLastObservedSerializationBusy = false;
    int32 DeferDepth = 0;
    int32 TotalSaveRequests = 0;
    int32 TotalDeferredSaveRequests = 0;
    int32 TotalSaveAttempts = 0;
    int32 TotalSuccessfulSaves = 0;
    bool bLastSaveSucceeded = false;
    double LastSaveDurationSeconds = 0.0;
    double LastSerializationBusyDurationSeconds = 0.0;
    FString LastSaveStatus = TEXT("No save activity recorded");
    FString LastSaveReason = TEXT("none");
    FString SerializationBusyReason = TEXT("idle");
    FDateTime LastSaveRequestAtUtc;
    FDateTime LastSaveAttemptAtUtc;
    FDateTime LastSaveCompletedAtUtc;
    FDateTime SerializationBusySinceUtc;
    FDateTime SerializationBusyLastObservedAtUtc;
    FDateTime LastSerializationBusyCompletedAtUtc;

    // Generic "engine is doing something long" state — covers the progress-bar
    // notifications for texture compression, shader compilation, asset imports,
    // async loading, DDC builds. When any of these are true the bridge response
    // may spike latency and audit harnesses should wait instead of time out.
    bool bEngineBusy = false;
    bool bSlowTaskInProgress = false;
    bool bAsyncLoading = false;
    int32 ShaderCompileJobs = 0;
    FString SlowTaskMessage;       // Current slow-task frame message ("Compressing source art…")
    FString SlowTaskFrameTitle;    // Current slow-task frame title ("Saving file: Foo.uasset")
    double SlowTaskProgressFraction = 0.0; // 0..1, total across nested frames
    FDateTime EngineBusySinceUtc;
    double LastEngineBusyDurationSeconds = 0.0;
};

inline constexpr double Riftborn_SaveBusyWarningSeconds = 15.0;
inline constexpr double Riftborn_SaveBusyStallSeconds = 60.0;

inline FRiftbornSaveGuardState& Riftborn_GetSaveGuardState()
{
    static FRiftbornSaveGuardState State;
    return State;
}

inline FString Riftborn_DescribeSerializationBusyReason(bool bSavingPackage, bool bGarbageCollecting)
{
    if (bSavingPackage && bGarbageCollecting)
    {
        return TEXT("saving_packages_and_collecting_garbage");
    }
    if (bSavingPackage)
    {
        return TEXT("saving_packages");
    }
    if (bGarbageCollecting)
    {
        return TEXT("collecting_garbage");
    }
    return TEXT("idle");
}

inline void Riftborn_ObserveEngineBusyState()
{
    FRiftbornSaveGuardState& State = Riftborn_GetSaveGuardState();
    const bool bSavingPackage = GIsSavingPackage;
    const bool bGarbageCollecting = IsGarbageCollecting();
    const bool bSerializationBusy = bSavingPackage || bGarbageCollecting;
    const FDateTime Now = FDateTime::UtcNow();

    State.bEngineSavingPackage = bSavingPackage;
    State.bEngineGarbageCollecting = bGarbageCollecting;
    State.bSerializationBusy = bSerializationBusy;
    State.SerializationBusyReason = Riftborn_DescribeSerializationBusyReason(bSavingPackage, bGarbageCollecting);

    if (bSerializationBusy)
    {
        if (!State.bLastObservedSerializationBusy || State.SerializationBusySinceUtc.GetTicks() <= 0)
        {
            State.SerializationBusySinceUtc = Now;
        }
        State.SerializationBusyLastObservedAtUtc = Now;
    }
    else if (State.bLastObservedSerializationBusy)
    {
        State.LastSerializationBusyCompletedAtUtc = Now;
        if (State.SerializationBusySinceUtc.GetTicks() > 0)
        {
            State.LastSerializationBusyDurationSeconds =
                (Now - State.SerializationBusySinceUtc).GetTotalSeconds();
        }
        State.SerializationBusySinceUtc = FDateTime();
        State.SerializationBusyLastObservedAtUtc = FDateTime();
        State.SerializationBusyReason = TEXT("idle");
    }

    State.bLastObservedSerializationBusy = bSerializationBusy;

    // Slow-task / progress-bar detection. Catches "Compressing source art for texture: X",
    // "Saving file: Foo.uasset…", asset imports, DDC builds — anything that puts a
    // progress bar on screen and stalls the game thread / HTTP responses.
    bool bSlowTaskActive = false;
    FString FrameMsg;
    FString FrameTitle;
    double FrameFraction = 0.0;
    if (GWarn)
    {
        const TArray<FSlowTask*>& ScopeStack = GWarn->GetScopeStack();
        if (ScopeStack.Num() > 0)
        {
            bSlowTaskActive = true;
            if (FSlowTask* Top = ScopeStack.Last())
            {
                FrameMsg = Top->FrameMessage.ToString();
                FrameTitle = Top->DefaultMessage.ToString();
                if (Top->TotalAmountOfWork > 0.0f)
                {
                    FrameFraction = FMath::Clamp(Top->CompletedWork / Top->TotalAmountOfWork, 0.0f, 1.0f);
                }
            }
        }
    }
    State.bSlowTaskInProgress = bSlowTaskActive;
    State.SlowTaskMessage = FrameMsg;
    State.SlowTaskFrameTitle = FrameTitle;
    State.SlowTaskProgressFraction = FrameFraction;

    State.bAsyncLoading = IsAsyncLoading();
    State.ShaderCompileJobs = GShaderCompilingManager ? GShaderCompilingManager->GetNumRemainingJobs() : 0;

    const bool bEngineBusy = bSerializationBusy || bSlowTaskActive
        || State.bAsyncLoading || State.ShaderCompileJobs > 0;

    if (bEngineBusy && !State.bEngineBusy)
    {
        State.EngineBusySinceUtc = Now;
    }
    else if (!bEngineBusy && State.bEngineBusy && State.EngineBusySinceUtc.GetTicks() > 0)
    {
        State.LastEngineBusyDurationSeconds = (Now - State.EngineBusySinceUtc).GetTotalSeconds();
        State.EngineBusySinceUtc = FDateTime();
    }
    State.bEngineBusy = bEngineBusy;
}

inline double Riftborn_GetCurrentSerializationBusySeconds(const FRiftbornSaveGuardState& State)
{
    if (!State.bSerializationBusy || State.SerializationBusySinceUtc.GetTicks() <= 0)
    {
        return 0.0;
    }

    return (FDateTime::UtcNow() - State.SerializationBusySinceUtc).GetTotalSeconds();
}

inline bool Riftborn_ExecuteCurrentLevelSave(const FString& Reason)
{
    FRiftbornSaveGuardState& State = Riftborn_GetSaveGuardState();
    State.TotalSaveAttempts++;
    State.LastSaveAttemptAtUtc = FDateTime::UtcNow();
    State.LastSaveReason = Reason;
    const bool bWasPendingSave = State.bPendingSave;
    State.bPendingSave = false;

    const bool bSaved = FEditorFileUtils::SaveCurrentLevel();

    State.LastSaveCompletedAtUtc = FDateTime::UtcNow();
    State.LastSaveDurationSeconds = (State.LastSaveCompletedAtUtc - State.LastSaveAttemptAtUtc).GetTotalSeconds();
    State.bLastSaveSucceeded = bSaved;
    if (bSaved)
    {
        State.TotalSuccessfulSaves++;
        State.LastSaveStatus = FString::Printf(TEXT("Saved current level via %s"), *Reason);
    }
    else
    {
        if (bWasPendingSave)
        {
            State.bPendingSave = true;
        }
        State.LastSaveStatus = FString::Printf(TEXT("Failed to save current level via %s"), *Reason);
    }

    return bSaved;
}

/**
 * Call this instead of FEditorFileUtils::SaveCurrentLevel() in all tool code.
 *
 * POLICY: Individual tools should NOT save the level. They mark packages dirty
 * (via MarkPackageDirty) and this function records the request, but the actual
 * disk write only happens when:
 *   - The user explicitly calls save_level / save_dirty_assets
 *   - The bridge autosave interval elapses
 *   - The save_level parameter is wired to an explicit save path
 *
 * This avoids the constant save-after-every-tool-call that caused:
 *   1. Multi-second pauses on every operation
 *   2. D3D12 swapchain race conditions during DataValidation Slate render
 *   3. Unnecessary disk I/O during multi-step workflows
 */
inline bool Riftborn_SaveCurrentLevel()
{
    FRiftbornSaveGuardState& State = Riftborn_GetSaveGuardState();
    State.TotalSaveRequests++;
    State.TotalDeferredSaveRequests++;
    State.LastSaveRequestAtUtc = FDateTime::UtcNow();
    State.bPendingSave = true;
    State.LastSaveReason = TEXT("deferred_by_policy");
    State.LastSaveStatus = TEXT("Level marked dirty — save deferred until explicit save_level call");
    return true;
}

/**
 * Check if saves are currently deferred.
 */
inline bool Riftborn_IsSaveDeferred()
{
    return Riftborn_GetSaveGuardState().bDeferSaves;
}

inline TSharedPtr<FJsonObject> Riftborn_BuildSaveGuardStateJson()
{
    Riftborn_ObserveEngineBusyState();
    const FRiftbornSaveGuardState& State = Riftborn_GetSaveGuardState();
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    const double SerializationBusySeconds = Riftborn_GetCurrentSerializationBusySeconds(State);
    const bool bBusyWarning = State.bSerializationBusy && SerializationBusySeconds >= Riftborn_SaveBusyWarningSeconds;
    const bool bBusyStalled = State.bSerializationBusy && SerializationBusySeconds >= Riftborn_SaveBusyStallSeconds;

    Root->SetBoolField(TEXT("deferred"), State.bDeferSaves);
    Root->SetBoolField(TEXT("pending_save"), State.bPendingSave);
    Root->SetBoolField(TEXT("serialization_busy"), State.bSerializationBusy);
    Root->SetBoolField(TEXT("save_in_progress"), State.bEngineSavingPackage);
    Root->SetBoolField(TEXT("garbage_collecting"), State.bEngineGarbageCollecting);
    Root->SetBoolField(TEXT("busy_warning"), bBusyWarning);
    Root->SetBoolField(TEXT("busy_stalled"), bBusyStalled);
    Root->SetNumberField(TEXT("defer_depth"), State.DeferDepth);
    Root->SetNumberField(TEXT("total_save_requests"), State.TotalSaveRequests);
    Root->SetNumberField(TEXT("total_deferred_requests"), State.TotalDeferredSaveRequests);
    Root->SetNumberField(TEXT("total_save_attempts"), State.TotalSaveAttempts);
    Root->SetNumberField(TEXT("total_successful_saves"), State.TotalSuccessfulSaves);
    Root->SetBoolField(TEXT("has_attempt_history"), State.TotalSaveAttempts > 0);
    Root->SetBoolField(TEXT("last_save_succeeded"), State.bLastSaveSucceeded);
    Root->SetNumberField(TEXT("last_save_duration_seconds"), State.LastSaveDurationSeconds);
    Root->SetNumberField(TEXT("busy_seconds"), SerializationBusySeconds);
    Root->SetNumberField(TEXT("last_busy_duration_seconds"), State.LastSerializationBusyDurationSeconds);
    Root->SetStringField(TEXT("last_save_status"), State.LastSaveStatus);
    Root->SetStringField(TEXT("last_save_reason"), State.LastSaveReason);
    Root->SetStringField(TEXT("busy_reason"), State.SerializationBusyReason);

    // Generic engine-busy surface so bridge clients can wait on long tasks (texture
    // compression, shader compilation, async imports) instead of treating a slow
    // HTTP response as a failure.
    Root->SetBoolField(TEXT("engine_busy"), State.bEngineBusy);
    Root->SetBoolField(TEXT("slow_task_in_progress"), State.bSlowTaskInProgress);
    Root->SetStringField(TEXT("slow_task_message"), State.SlowTaskMessage);
    Root->SetStringField(TEXT("slow_task_title"), State.SlowTaskFrameTitle);
    Root->SetNumberField(TEXT("slow_task_progress"), State.SlowTaskProgressFraction);
    Root->SetBoolField(TEXT("async_loading"), State.bAsyncLoading);
    Root->SetNumberField(TEXT("shader_compile_jobs"), State.ShaderCompileJobs);
    Root->SetNumberField(TEXT("last_engine_busy_duration_seconds"), State.LastEngineBusyDurationSeconds);
    if (State.bEngineBusy && State.EngineBusySinceUtc.GetTicks() > 0)
    {
        Root->SetNumberField(TEXT("engine_busy_seconds"),
            (FDateTime::UtcNow() - State.EngineBusySinceUtc).GetTotalSeconds());
    }

    if (State.LastSaveRequestAtUtc.GetTicks() > 0)
    {
        Root->SetStringField(TEXT("last_save_request_at"), State.LastSaveRequestAtUtc.ToIso8601());
    }
    if (State.LastSaveAttemptAtUtc.GetTicks() > 0)
    {
        Root->SetStringField(TEXT("last_save_attempt_at"), State.LastSaveAttemptAtUtc.ToIso8601());
    }
    if (State.LastSaveCompletedAtUtc.GetTicks() > 0)
    {
        Root->SetStringField(TEXT("last_save_completed_at"), State.LastSaveCompletedAtUtc.ToIso8601());
    }
    if (State.SerializationBusySinceUtc.GetTicks() > 0)
    {
        Root->SetStringField(TEXT("busy_since"), State.SerializationBusySinceUtc.ToIso8601());
    }
    if (State.SerializationBusyLastObservedAtUtc.GetTicks() > 0)
    {
        Root->SetStringField(TEXT("busy_last_observed_at"), State.SerializationBusyLastObservedAtUtc.ToIso8601());
    }
    if (State.LastSerializationBusyCompletedAtUtc.GetTicks() > 0)
    {
        Root->SetStringField(TEXT("last_busy_completed_at"), State.LastSerializationBusyCompletedAtUtc.ToIso8601());
    }

    FString SummaryText;
    if (State.bSerializationBusy)
    {
        SummaryText = bBusyStalled
            ? FString::Printf(
                TEXT("Editor busy with %s for %.1fs; treat this as stalled until progress is confirmed."),
                *State.SerializationBusyReason,
                SerializationBusySeconds)
            : FString::Printf(
                TEXT("Editor busy with %s for %.1fs."),
                *State.SerializationBusyReason,
                SerializationBusySeconds);
    }
    else if (State.bPendingSave)
    {
        SummaryText = TEXT("Save request queued for deferred execution.");
    }
    else if (State.TotalSaveAttempts > 0)
    {
        SummaryText = State.bLastSaveSucceeded
            ? FString::Printf(TEXT("Last save succeeded via %s."), *State.LastSaveReason)
            : FString::Printf(TEXT("Last save failed via %s."), *State.LastSaveReason);
    }
    else if (State.bDeferSaves)
    {
        SummaryText = TEXT("Save deferral is active with no pending save.");
    }
    else
    {
        SummaryText = TEXT("No save activity recorded in this editor session.");
    }

    Root->SetStringField(TEXT("summary_text"), SummaryText);
    return Root;
}

/**
 * RAII guard that defers all saves while in scope.
 * On destruction, performs one batched save if any were requested.
 *
 * Usage:
 *   {
 *       FRiftbornSaveDeferGuard Guard;
 *       // ... multiple tool calls that may request persistence ...
 *   } // save requests remain queued for explicit/timed autosave
 */
class RIFTBORNAI_API FRiftbornSaveDeferGuard
{
public:
    FRiftbornSaveDeferGuard()
        : bPreviousState(Riftborn_GetSaveGuardState().bDeferSaves)
    {
        FRiftbornSaveGuardState& State = Riftborn_GetSaveGuardState();
        State.bDeferSaves = true;
        State.DeferDepth++;
    }

    ~FRiftbornSaveDeferGuard()
    {
        FRiftbornSaveGuardState& State = Riftborn_GetSaveGuardState();
        State.DeferDepth--;
        if (State.DeferDepth <= 0)
        {
            State.bDeferSaves = bPreviousState;
            State.DeferDepth = 0;
        }
    }

private:
    bool bPreviousState;
};
