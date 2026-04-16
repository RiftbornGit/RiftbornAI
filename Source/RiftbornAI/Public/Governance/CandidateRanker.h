// Copyright 2026 RiftbornAI. All Rights Reserved.
//
// CandidateRanker — Phase 3 of the judgment layer.
//
// Takes N proposed tool calls and selects the best before execution.
// Consumes existing signals without duplicating them:
//   - BudgetGate (Phase 1) votes pass/fail on each candidate.
//   - Heuristic cost score (cheaper = better when outcomes are equivalent).
//   - Optional caller-supplied affinity (e.g. "prefer candidates that
//     touch the fewest actors").
//
// This is a RANKER, not a generator. It's up to the caller (agent code,
// plan executor, Python arbiter) to propose N candidates. Ranker picks.
//
// Not a replacement for FPlanExecutor — an adapter above it that solves
// "I have 3 ways to do this, which one?". Integration points live in
// the existing agent loop (RiftbornAgentCore_Execution.cpp) and in the
// Python plan arbiter (Bridge/core/plan_arbiter.py, shim over risk_tiers).
//
// Header-only for live-coding compat. Split at next full UBT rebuild.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Governance/BudgetGate.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

/** Why a candidate was scored the way it was. */
struct FRankReason
{
    FString Source;  // "BudgetGate" / "CostHeuristic" / "Affinity"
    double Delta;    // positive = made candidate better, negative = worse
    FString Detail;  // human-readable explanation
};

/** Per-candidate scoring record. */
struct FRankedCandidate
{
    int32 OriginalIndex = -1;
    FClaudeToolCall Call;
    double Score = 0.0;
    bool bEliminated = false;       // true if a hard constraint rejected it
    FString EliminationReason;
    TArray<FRankReason> Reasons;
};

/** Outcome of a ranking pass. */
struct FRankingResult
{
    int32 SelectedIndex = -1;       // original index of the winner; -1 if all eliminated
    FString SelectedReason;
    TArray<FRankedCandidate> Ranked; // sorted by score desc, eliminated last

    FString ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetNumberField(TEXT("selected_index"), SelectedIndex);
        Obj->SetStringField(TEXT("selected_reason"), SelectedReason);

        TArray<TSharedPtr<FJsonValue>> CArr;
        for (const FRankedCandidate& C : Ranked)
        {
            TSharedPtr<FJsonObject> CObj = MakeShared<FJsonObject>();
            CObj->SetNumberField(TEXT("original_index"), C.OriginalIndex);
            CObj->SetStringField(TEXT("tool"), C.Call.ToolName);
            CObj->SetNumberField(TEXT("score"), C.Score);
            CObj->SetBoolField(TEXT("eliminated"), C.bEliminated);
            if (!C.EliminationReason.IsEmpty())
            {
                CObj->SetStringField(TEXT("elimination_reason"), C.EliminationReason);
            }
            TArray<TSharedPtr<FJsonValue>> RArr;
            for (const FRankReason& R : C.Reasons)
            {
                TSharedPtr<FJsonObject> RObj = MakeShared<FJsonObject>();
                RObj->SetStringField(TEXT("source"), R.Source);
                RObj->SetNumberField(TEXT("delta"), R.Delta);
                RObj->SetStringField(TEXT("detail"), R.Detail);
                RArr.Add(MakeShared<FJsonValueObject>(RObj));
            }
            CObj->SetArrayField(TEXT("reasons"), RArr);
            CArr.Add(MakeShared<FJsonValueObject>(CObj));
        }
        Obj->SetArrayField(TEXT("ranked"), CArr);

        FString Out;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
        FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
        return Out;
    }
};

/**
 * Optional caller-supplied knobs for how to rank.
 * Defaults produce a sane "prefer cheap, veto budget-fails" ranking.
 */
struct FRankingOptions
{
    /** BudgetGate veto power: eliminated candidates can't win, even if others are worse. */
    bool bEnforceBudgetGate = true;

    /** Weight for "cheaper is better" heuristic. Set to 0 to ignore. */
    double CostHeuristicWeight = 1.0;

    /** Weight for "touches fewer arguments is better" heuristic. */
    double ArgumentCountWeight = 0.2;

    /** Weight for caller's affinity score per candidate. See FRankingOptions::Affinity. */
    double AffinityWeight = 1.0;

    /**
     * Optional caller-supplied scores, indexed by candidate (matches input array).
     * Higher is better. Array size should match candidate count; shorter arrays
     * treat missing entries as 0.
     */
    TArray<double> Affinity;
};

/**
 * Thin, stateless ranker. No singleton — safe to call from any thread, no
 * lifecycle to manage. Takes candidates + options, returns the ranked
 * breakdown + the winner.
 */
class FCandidateRanker
{
public:
    static FRankingResult Rank(
        const TArray<FClaudeToolCall>& Candidates,
        const FRankingOptions& Options = FRankingOptions())
    {
        FRankingResult Result;
        Result.Ranked.Reserve(Candidates.Num());

        for (int32 Index = 0; Index < Candidates.Num(); ++Index)
        {
            FRankedCandidate RC;
            RC.OriginalIndex = Index;
            RC.Call = Candidates[Index];

            // 1. BudgetGate veto.
            if (Options.bEnforceBudgetGate)
            {
                const FBudgetEvaluation BE = FBudgetGate::Get().Evaluate(RC.Call);
                if (!BE.bPass)
                {
                    RC.bEliminated = true;
                    RC.EliminationReason = BE.Reason;
                    FRankReason R;
                    R.Source = TEXT("BudgetGate");
                    R.Delta = 0.0;
                    R.Detail = BE.Reason;
                    RC.Reasons.Add(R);
                    Result.Ranked.Add(MoveTemp(RC));
                    continue;
                }
                else
                {
                    FRankReason R;
                    R.Source = TEXT("BudgetGate");
                    R.Delta = 0.0;
                    R.Detail = TEXT("passed");
                    RC.Reasons.Add(R);
                }
            }

            // 2. Cost heuristic: cheaper tools get a higher score.
            if (Options.CostHeuristicWeight != 0.0)
            {
                const double Cost = EstimateCost(RC.Call);
                const double Delta = -Cost * Options.CostHeuristicWeight;
                RC.Score += Delta;
                FRankReason R;
                R.Source = TEXT("CostHeuristic");
                R.Delta = Delta;
                R.Detail = FString::Printf(TEXT("est_cost=%.2f"), Cost);
                RC.Reasons.Add(R);
            }

            // 3. Argument-count heuristic: fewer args = smaller blast radius.
            if (Options.ArgumentCountWeight != 0.0)
            {
                const double ArgCount = static_cast<double>(RC.Call.Arguments.Num());
                const double Delta = -ArgCount * Options.ArgumentCountWeight;
                RC.Score += Delta;
                FRankReason R;
                R.Source = TEXT("ArgumentCount");
                R.Delta = Delta;
                R.Detail = FString::Printf(TEXT("%d args"), static_cast<int32>(ArgCount));
                RC.Reasons.Add(R);
            }

            // 4. Caller affinity (e.g. LLM confidence or structural preference).
            if (Options.AffinityWeight != 0.0 && Options.Affinity.IsValidIndex(Index))
            {
                const double Delta = Options.Affinity[Index] * Options.AffinityWeight;
                RC.Score += Delta;
                FRankReason R;
                R.Source = TEXT("Affinity");
                R.Delta = Delta;
                R.Detail = FString::Printf(TEXT("caller=%.3f"), Options.Affinity[Index]);
                RC.Reasons.Add(R);
            }

            Result.Ranked.Add(MoveTemp(RC));
        }

        // Sort by score desc, eliminated last regardless of score.
        Result.Ranked.Sort([](const FRankedCandidate& A, const FRankedCandidate& B)
        {
            if (A.bEliminated != B.bEliminated)
            {
                return !A.bEliminated; // non-eliminated first
            }
            return A.Score > B.Score;
        });

        // Pick the winner: highest-scored non-eliminated candidate, or none.
        for (const FRankedCandidate& RC : Result.Ranked)
        {
            if (!RC.bEliminated)
            {
                Result.SelectedIndex = RC.OriginalIndex;
                Result.SelectedReason = FString::Printf(
                    TEXT("Highest-scored non-eliminated candidate (score=%.3f)"), RC.Score);
                break;
            }
        }

        if (Result.SelectedIndex < 0)
        {
            Result.SelectedReason = TEXT("All candidates eliminated (budget-gate failures or empty input)");
        }

        return Result;
    }

private:
    /** Per-tool cost heuristic. Higher = more expensive / more destructive. */
    static double EstimateCost(const FClaudeToolCall& Call)
    {
        const FString& T = Call.ToolName;

        // Cheap reads.
        if (T.StartsWith(TEXT("get_")) ||
            T.StartsWith(TEXT("list_")) ||
            T.StartsWith(TEXT("find_")) ||
            T.StartsWith(TEXT("inspect_")) ||
            T.StartsWith(TEXT("describe_")) ||
            T.StartsWith(TEXT("verify_")) ||
            T.StartsWith(TEXT("assert_")))
        {
            return 0.1;
        }

        // Property mutations — small blast radius.
        if (T.StartsWith(TEXT("set_")) ||
            T.StartsWith(TEXT("move_")) ||
            T.StartsWith(TEXT("scale_")) ||
            T.StartsWith(TEXT("rotate_")) ||
            T.StartsWith(TEXT("focus_")))
        {
            return 1.0;
        }

        // Spawns / creates — medium cost.
        if (T.StartsWith(TEXT("spawn_")) ||
            T.StartsWith(TEXT("add_")) ||
            T.StartsWith(TEXT("create_")) ||
            T.StartsWith(TEXT("duplicate_")) ||
            T.StartsWith(TEXT("import_")))
        {
            return 5.0;
        }

        // Paints / sculpts / batch ops — large blast radius.
        if (T.StartsWith(TEXT("paint_")) ||
            T.StartsWith(TEXT("sculpt_")) ||
            T.StartsWith(TEXT("scatter_")) ||
            T.StartsWith(TEXT("batch_")) ||
            T.StartsWith(TEXT("apply_")))
        {
            return 12.0;
        }

        // Builds / packages / cooks — heaviest.
        if (T.StartsWith(TEXT("build_")) ||
            T.StartsWith(TEXT("cook_")) ||
            T.StartsWith(TEXT("package_")) ||
            T.StartsWith(TEXT("run_ubt")))
        {
            return 50.0;
        }

        // Destructive unless-whitelisted.
        if (T.StartsWith(TEXT("delete_")) ||
            T.StartsWith(TEXT("remove_")))
        {
            return 8.0;
        }

        // Default midrange for unknown tools.
        return 3.0;
    }
};
