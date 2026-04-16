// Copyright 2026 RiftbornAI. All Rights Reserved.
//
// PerfGate — Phase 2 of the judgment layer.
//
// Post-execution performance regression detection. Collects per-frame
// samples via FCoreDelegates::OnEndFrame, maintains a rolling ring buffer,
// and exposes structured metrics (p50/p95/p99 frame time, thread splits,
// GPU time) plus budget comparison.
//
// Orthogonal to BudgetGate:
//   BudgetGate = pre-execution VETO on constraint violations.
//   PerfGate   = post-execution MEASUREMENT + regression detection.
//
// Sampling source: engine globals (GAverageFPS, GAverageMS, GGameThreadTime,
// GRenderThreadTime) — the same values that stat unit reads.
//
// Config: Config/tiers/perf_budgets.json defines named budgets per target
// (e.g. "editor", "pie_session", "packaged_pc_60"). Tools can call
// CheckAgainstBudget(Name) to get pass/fail plus per-metric violations.
//
// Header-only like BudgetGate for live-coding compatibility. Split into
// PerfGate{,_Sampler,_Loader}.cpp at next full UBT rebuild.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/IDelegateInstance.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/CriticalSection.h"
#include "HAL/PlatformTime.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include <atomic>

// Engine globals — declared ENGINE_API in Engine/Private/UnrealEngine.cpp.
// Same pattern existing RiftbornAI perf tools already use.
extern ENGINE_API float GAverageFPS;
extern ENGINE_API float GAverageMS;

/** One per-frame sample. 48 bytes. */
struct FPerfSample
{
    double TimestampSeconds = 0.0;
    double FrameMs = 0.0;
    double GameThreadMs = 0.0;
    double RenderThreadMs = 0.0;
    double GPUMs = 0.0;
};

/** Aggregated metrics over the current ring window. */
struct FPerfMetrics
{
    double FpsAvg = 0.0;
    double FrameMsAvg = 0.0;
    double FrameMsP50 = 0.0;
    double FrameMsP95 = 0.0;
    double FrameMsP99 = 0.0;
    double GameThreadMsAvg = 0.0;
    double RenderThreadMsAvg = 0.0;
    double GPUMsAvg = 0.0;
    int32 SampleCount = 0;
    double WindowSeconds = 0.0;

    /** Structured JSON for tool output. */
    FString ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetNumberField(TEXT("fps_avg"), FpsAvg);
        Obj->SetNumberField(TEXT("frame_ms_avg"), FrameMsAvg);
        Obj->SetNumberField(TEXT("frame_ms_p50"), FrameMsP50);
        Obj->SetNumberField(TEXT("frame_ms_p95"), FrameMsP95);
        Obj->SetNumberField(TEXT("frame_ms_p99"), FrameMsP99);
        Obj->SetNumberField(TEXT("game_thread_ms_avg"), GameThreadMsAvg);
        Obj->SetNumberField(TEXT("render_thread_ms_avg"), RenderThreadMsAvg);
        Obj->SetNumberField(TEXT("gpu_ms_avg"), GPUMsAvg);
        Obj->SetNumberField(TEXT("sample_count"), SampleCount);
        Obj->SetNumberField(TEXT("window_seconds"), WindowSeconds);
        FString Out;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
        FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
        return Out;
    }
};

/** Budget thresholds for a named target. Any field <=0 means "no limit". */
struct FPerfBudget
{
    double MinFpsAvg = 0.0;
    double MaxFrameMsP95 = 0.0;
    double MaxFrameMsP99 = 0.0;
    double MaxGameThreadMsAvg = 0.0;
    double MaxRenderThreadMsAvg = 0.0;
    double MaxGPUMsAvg = 0.0;
};

/** Result of comparing current metrics to a named budget. */
struct FPerfBudgetResult
{
    bool bPass = true;
    FString BudgetName;
    FPerfMetrics Metrics;
    TArray<FString> Violations;

    FString ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetBoolField(TEXT("pass"), bPass);
        Obj->SetStringField(TEXT("budget_name"), BudgetName);

        TSharedPtr<FJsonObject> MetricsObj = MakeShared<FJsonObject>();
        MetricsObj->SetNumberField(TEXT("fps_avg"), Metrics.FpsAvg);
        MetricsObj->SetNumberField(TEXT("frame_ms_p50"), Metrics.FrameMsP50);
        MetricsObj->SetNumberField(TEXT("frame_ms_p95"), Metrics.FrameMsP95);
        MetricsObj->SetNumberField(TEXT("frame_ms_p99"), Metrics.FrameMsP99);
        MetricsObj->SetNumberField(TEXT("game_thread_ms_avg"), Metrics.GameThreadMsAvg);
        MetricsObj->SetNumberField(TEXT("render_thread_ms_avg"), Metrics.RenderThreadMsAvg);
        MetricsObj->SetNumberField(TEXT("gpu_ms_avg"), Metrics.GPUMsAvg);
        MetricsObj->SetNumberField(TEXT("sample_count"), Metrics.SampleCount);
        Obj->SetObjectField(TEXT("metrics"), MetricsObj);

        TArray<TSharedPtr<FJsonValue>> VArr;
        for (const FString& V : Violations)
        {
            VArr.Add(MakeShared<FJsonValueString>(V));
        }
        Obj->SetArrayField(TEXT("violations"), VArr);

        FString Out;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
        FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
        return Out;
    }
};

/**
 * PerfGate singleton. Thread-safe. Samples on the game thread via
 * FCoreDelegates::OnEndFrame. Metrics/budget queries are safe from any thread.
 */
class FPerfGate
{
public:
    static FPerfGate& Get()
    {
        static FPerfGate Instance;
        return Instance;
    }

    /** Hook end-of-frame delegate and load budgets. Idempotent. */
    void Initialize()
    {
        FScopeLock ScopeLock(&Lock);
        if (bInitialized) return;

        // Bind to OnEndFrame so we sample on game thread after render submission.
        EndFrameHandle = FCoreDelegates::OnEndFrame.AddRaw(this, &FPerfGate::OnEndFrame);

        const FString PluginCfg = FPaths::ProjectPluginsDir()
            / TEXT("RiftbornAI/Config/tiers/perf_budgets.json");
        const FString ProjectCfg = FPaths::ProjectConfigDir()
            / TEXT("RiftbornAI/perf_budgets.json");

        FString PathUsed;
        if (FPaths::FileExists(PluginCfg)) PathUsed = PluginCfg;
        else if (FPaths::FileExists(ProjectCfg)) PathUsed = ProjectCfg;

        if (!PathUsed.IsEmpty() && LoadBudgetsFile(PathUsed))
        {
            UE_LOG(LogTemp, Log,
                TEXT("[PerfGate] loaded %d budgets from %s"),
                Budgets.Num(), *PathUsed);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[PerfGate] no budgets file (checked %s, %s) — CheckAgainstBudget will fail until one exists"),
                *PluginCfg, *ProjectCfg);
        }

        bInitialized = true;
    }

    /** Unhook end-of-frame delegate. Called from module Shutdown. */
    void Shutdown()
    {
        FScopeLock ScopeLock(&Lock);
        if (!bInitialized) return;
        if (EndFrameHandle.IsValid())
        {
            FCoreDelegates::OnEndFrame.Remove(EndFrameHandle);
            EndFrameHandle.Reset();
        }
        Samples.Empty();
        NumValidSamples = 0;
        NextIndex = 0;
        bInitialized = false;
    }

    /** Clear ring buffer and budget map. Testing. */
    void Reset()
    {
        FScopeLock ScopeLock(&Lock);
        Samples.Empty();
        NumValidSamples = 0;
        NextIndex = 0;
        Budgets.Empty();
        TotalSamplesCollected.store(0, std::memory_order_relaxed);
    }

    /** Current ring-buffer capacity (compile-time constant). */
    static constexpr int32 MaxSamples = 300;

    /** Total samples collected since Initialize (lock-free read). */
    int64 GetTotalSamplesCollected() const
    {
        return TotalSamplesCollected.load(std::memory_order_relaxed);
    }

    /** Register a named budget programmatically (used by tests). */
    void RegisterBudget(const FString& Name, const FPerfBudget& Budget)
    {
        FScopeLock ScopeLock(&Lock);
        Budgets.Add(Name, Budget);
    }

    /** Is the named budget configured? */
    bool HasBudget(const FString& Name) const
    {
        FScopeLock ScopeLock(&Lock);
        return Budgets.Contains(Name);
    }

    /** Compute percentile metrics across the current ring window. */
    FPerfMetrics GetCurrentMetrics() const
    {
        TArray<FPerfSample> SnapshotSamples;
        {
            FScopeLock ScopeLock(&Lock);
            SnapshotSamples = Samples;
        }
        return ComputeMetrics(SnapshotSamples);
    }

    /** Compare current metrics against a named budget. */
    FPerfBudgetResult CheckAgainstBudget(const FString& BudgetName) const
    {
        FPerfBudgetResult R;
        R.BudgetName = BudgetName;
        R.Metrics = GetCurrentMetrics();

        FPerfBudget Budget;
        {
            FScopeLock ScopeLock(&Lock);
            const FPerfBudget* Found = Budgets.Find(BudgetName);
            if (!Found)
            {
                R.bPass = false;
                R.Violations.Add(FString::Printf(
                    TEXT("Budget '%s' not configured in perf_budgets.json"),
                    *BudgetName));
                return R;
            }
            Budget = *Found;
        }

        if (R.Metrics.SampleCount == 0)
        {
            R.bPass = false;
            R.Violations.Add(TEXT("No samples yet — has Initialize run and has at least one frame elapsed?"));
            return R;
        }

        auto Check = [&](double Limit, double Actual, const TCHAR* Metric, bool bLower)
        {
            if (Limit <= 0.0) return; // no limit set
            const bool bViolated = bLower ? (Actual < Limit) : (Actual > Limit);
            if (bViolated)
            {
                R.bPass = false;
                R.Violations.Add(FString::Printf(
                    TEXT("%s: %.2f %s budget %.2f"),
                    Metric,
                    Actual,
                    bLower ? TEXT("below") : TEXT("above"),
                    Limit));
            }
        };

        Check(Budget.MinFpsAvg,          R.Metrics.FpsAvg,            TEXT("fps_avg"),             true);
        Check(Budget.MaxFrameMsP95,      R.Metrics.FrameMsP95,        TEXT("frame_ms_p95"),        false);
        Check(Budget.MaxFrameMsP99,      R.Metrics.FrameMsP99,        TEXT("frame_ms_p99"),        false);
        Check(Budget.MaxGameThreadMsAvg, R.Metrics.GameThreadMsAvg,   TEXT("game_thread_ms_avg"),  false);
        Check(Budget.MaxRenderThreadMsAvg,R.Metrics.RenderThreadMsAvg,TEXT("render_thread_ms_avg"),false);
        Check(Budget.MaxGPUMsAvg,        R.Metrics.GPUMsAvg,          TEXT("gpu_ms_avg"),          false);

        return R;
    }

    /** Is the gate initialized and sampling? */
    bool IsInitialized() const { return bInitialized; }

private:
    FPerfGate() = default;

    // Ring-buffer push. Must run on game thread (OnEndFrame contract).
    void OnEndFrame()
    {
        FPerfSample S;
        S.TimestampSeconds = FPlatformTime::Seconds();
        // GAverageMS is a 1-second sliding average — treat as the frame ms for this tick.
        // That smooths out single-frame spikes; the p95/p99 comes from repeated GAverageMS
        // reads across many ticks which still captures sustained regressions.
        S.FrameMs = static_cast<double>(GAverageMS);
        S.GameThreadMs = static_cast<double>(FPlatformTime::ToMilliseconds(GGameThreadTime));
        S.RenderThreadMs = static_cast<double>(FPlatformTime::ToMilliseconds(GRenderThreadTime));
#if WITH_EDITOR || !UE_BUILD_SHIPPING
        // GGPUFrameTime is declared in RHI; available in editor/dev builds.
        extern uint32 GGPUFrameTime;
        S.GPUMs = static_cast<double>(FPlatformTime::ToMilliseconds(GGPUFrameTime));
#endif

        FScopeLock ScopeLock(&Lock);
        if (Samples.Num() < MaxSamples)
        {
            Samples.Add(S);
        }
        else
        {
            Samples[NextIndex] = S;
        }
        NextIndex = (NextIndex + 1) % MaxSamples;
        if (NumValidSamples < MaxSamples) ++NumValidSamples;
        TotalSamplesCollected.fetch_add(1, std::memory_order_relaxed);
    }

    static FPerfMetrics ComputeMetrics(const TArray<FPerfSample>& In)
    {
        FPerfMetrics M;
        M.SampleCount = In.Num();
        if (In.Num() == 0) return M;

        double SumFrame = 0.0, SumGT = 0.0, SumRT = 0.0, SumGPU = 0.0;
        TArray<double> FrameMsList;
        FrameMsList.Reserve(In.Num());

        double MinT = In[0].TimestampSeconds, MaxT = In[0].TimestampSeconds;
        for (const FPerfSample& S : In)
        {
            SumFrame += S.FrameMs;
            SumGT    += S.GameThreadMs;
            SumRT    += S.RenderThreadMs;
            SumGPU   += S.GPUMs;
            FrameMsList.Add(S.FrameMs);
            if (S.TimestampSeconds < MinT) MinT = S.TimestampSeconds;
            if (S.TimestampSeconds > MaxT) MaxT = S.TimestampSeconds;
        }

        const double N = static_cast<double>(In.Num());
        M.FrameMsAvg = SumFrame / N;
        M.GameThreadMsAvg = SumGT / N;
        M.RenderThreadMsAvg = SumRT / N;
        M.GPUMsAvg = SumGPU / N;
        M.FpsAvg = (M.FrameMsAvg > 0.0) ? (1000.0 / M.FrameMsAvg) : 0.0;
        M.WindowSeconds = MaxT - MinT;

        FrameMsList.Sort();
        auto Percentile = [&FrameMsList](double P) -> double
        {
            if (FrameMsList.Num() == 0) return 0.0;
            const int32 Idx = FMath::Clamp(
                static_cast<int32>(P * (FrameMsList.Num() - 1)),
                0, FrameMsList.Num() - 1);
            return FrameMsList[Idx];
        };
        M.FrameMsP50 = Percentile(0.50);
        M.FrameMsP95 = Percentile(0.95);
        M.FrameMsP99 = Percentile(0.99);

        return M;
    }

    bool LoadBudgetsFile(const FString& FilePath)
    {
        FString Content;
        if (!FFileHelper::LoadFileToString(Content, *FilePath)) return false;

        TSharedPtr<FJsonObject> Root;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("[PerfGate] could not parse %s"), *FilePath);
            return false;
        }

        const TSharedPtr<FJsonObject>* BudgetsObj = nullptr;
        if (!Root->TryGetObjectField(TEXT("budgets"), BudgetsObj) || !BudgetsObj)
        {
            UE_LOG(LogTemp, Error, TEXT("[PerfGate] %s missing 'budgets' field"), *FilePath);
            return false;
        }

        int32 Loaded = 0;
        for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*BudgetsObj)->Values)
        {
            const TSharedPtr<FJsonObject>* BudgetObj = nullptr;
            if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(BudgetObj) || !BudgetObj) continue;

            FPerfBudget B;
            double DVal = 0.0;
            if ((*BudgetObj)->TryGetNumberField(TEXT("min_fps_avg"), DVal)) B.MinFpsAvg = DVal;
            if ((*BudgetObj)->TryGetNumberField(TEXT("max_frame_ms_p95"), DVal)) B.MaxFrameMsP95 = DVal;
            if ((*BudgetObj)->TryGetNumberField(TEXT("max_frame_ms_p99"), DVal)) B.MaxFrameMsP99 = DVal;
            if ((*BudgetObj)->TryGetNumberField(TEXT("max_game_thread_ms_avg"), DVal)) B.MaxGameThreadMsAvg = DVal;
            if ((*BudgetObj)->TryGetNumberField(TEXT("max_render_thread_ms_avg"), DVal)) B.MaxRenderThreadMsAvg = DVal;
            if ((*BudgetObj)->TryGetNumberField(TEXT("max_gpu_ms_avg"), DVal)) B.MaxGPUMsAvg = DVal;

            Budgets.Add(Pair.Key, B);
            ++Loaded;
        }

        return Loaded > 0;
    }

    mutable FCriticalSection Lock;
    TArray<FPerfSample> Samples;                    // ring buffer, grows to MaxSamples then overwrites
    int32 NextIndex = 0;
    int32 NumValidSamples = 0;
    TMap<FString, FPerfBudget> Budgets;
    FDelegateHandle EndFrameHandle;
    bool bInitialized = false;

    mutable std::atomic<int64> TotalSamplesCollected{0};
};
