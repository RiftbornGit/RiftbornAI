// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// ExplorationContract.h - Exploration contracts and failure pattern tracking
//
// Exploration Contracts: Explicitly declared experimental actions with bounded risk.
// Failure Patterns: Cross-goal regret generalization for pre-emptive warnings.
//
// Extracted from RegretScope.h for modularity.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

#include "RegretEnums.h"  // For EExplorationTier, ExplorationTierToString, GetExplorationBudget

// =============================================================================
// EXPLORATION CONTRACTS - Sanctioned risk with bounded consequences
// =============================================================================
//
// Problem: As regret logic grows richer, the agent risks becoming too conservative.
// If penalties accumulate faster than recovery pathways expand, the agent will:
// - Avoid exploration
// - Over-optimize for "not being punished"
// - Stagnate
//
// Solution: Exploration Contracts - explicitly declared experimental actions.
// Key Properties:
// 1. Opt-in: Agent must explicitly request exploration mode
// 2. Bounded: Each contract has a max penalty budget
// 3. Time-limited: Contracts expire
// 4. Auditable: All exploration actions are logged separately
// 5. Non-transferable: Exploration regret doesn't become production regret
// =============================================================================

/**
 * An exploration contract - explicitly sanctioned risk.
 * Agent declares intent to explore BEFORE taking action.
 */
struct FExplorationContract
{
    // Identity
    FGuid ContractId;
    FDateTime CreatedAt;
    FDateTime ExpiresAt;

    // Scope
    EExplorationTier Tier = EExplorationTier::Tactical;
    FString GoalContext;            // What is the agent trying to explore?
    FString Hypothesis;             // What does the agent expect to learn?
    TArray<FString> AllowedTools;   // Which tools are covered by this contract?

    // Budget
    float MaxPenaltyBudget = 0.0f;  // Max penalty before contract is exhausted
    float ConsumedBudget = 0.0f;    // How much penalty has been consumed
    int32 MaxActions = 10;          // Max actions under this contract
    int32 ConsumedActions = 0;      // How many actions have been taken

    // State
    bool bActive = true;            // Is this contract still valid?
    bool bExhausted = false;        // Has budget been fully consumed?
    bool bExpired = false;          // Has time limit passed?

    // Audit trail
    TArray<FGuid> ExplorationEvents; // Regret events under this contract

    /** Create a new exploration contract */
    static FExplorationContract Create(
        EExplorationTier Tier,
        const FString& Goal,
        const FString& Hypothesis,
        int32 DurationSeconds = 300
    )
    {
        FExplorationContract Contract;
        Contract.ContractId = FGuid::NewGuid();
        Contract.CreatedAt = FDateTime::Now();
        Contract.ExpiresAt = Contract.CreatedAt + FTimespan::FromSeconds(DurationSeconds);
        Contract.Tier = Tier;
        Contract.GoalContext = Goal;
        Contract.Hypothesis = Hypothesis;
        Contract.MaxPenaltyBudget = GetExplorationBudget(Tier);

        return Contract;
    }

    /** Check if contract is still valid */
    bool IsValid() const
    {
        if (!bActive) return false;
        if (bExhausted) return false;
        if (bExpired) return false;
        if (FDateTime::Now() > ExpiresAt) return false;
        if (ConsumedActions >= MaxActions) return false;

        return true;
    }

    /** Consume penalty budget. Returns true if successful, false if exhausted. */
    bool ConsumeBudget(float Penalty)
    {
        if (!IsValid()) return false;

        ConsumedBudget += Penalty;
        ConsumedActions++;

        if (ConsumedBudget >= MaxPenaltyBudget)
        {
            bExhausted = true;
            return false;
        }

        return true;
    }

    /** Get remaining budget */
    float GetRemainingBudget() const
    {
        return FMath::Max(0.0f, MaxPenaltyBudget - ConsumedBudget);
    }

    /** Finalize contract with outcome */
    void Finalize(bool bSuccess, const FString& Outcome)
    {
        bActive = false;
        FinalOutcome = Outcome;
        bSuccessful = bSuccess;
        FinalizedAt = FDateTime::Now();
    }

    // Outcome
    FString FinalOutcome;
    bool bSuccessful = false;
    FDateTime FinalizedAt;

    /** Serialize to JSON */
    TSharedPtr<FJsonObject> ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject);

        Obj->SetStringField(TEXT("contract_id"), ContractId.ToString());
        Obj->SetStringField(TEXT("created_at"), CreatedAt.ToIso8601());
        Obj->SetStringField(TEXT("expires_at"), ExpiresAt.ToIso8601());
        Obj->SetStringField(TEXT("tier"), ExplorationTierToString(Tier));
        Obj->SetStringField(TEXT("goal_context"), GoalContext);
        Obj->SetStringField(TEXT("hypothesis"), Hypothesis);
        Obj->SetNumberField(TEXT("max_penalty_budget"), MaxPenaltyBudget);
        Obj->SetNumberField(TEXT("consumed_budget"), ConsumedBudget);
        Obj->SetNumberField(TEXT("remaining_budget"), GetRemainingBudget());
        Obj->SetNumberField(TEXT("max_actions"), MaxActions);
        Obj->SetNumberField(TEXT("consumed_actions"), ConsumedActions);
        Obj->SetBoolField(TEXT("active"), bActive);
        Obj->SetBoolField(TEXT("exhausted"), bExhausted);
        Obj->SetBoolField(TEXT("expired"), bExpired);
        Obj->SetBoolField(TEXT("successful"), bSuccessful);
        Obj->SetStringField(TEXT("final_outcome"), FinalOutcome);

        TArray<TSharedPtr<FJsonValue>> ToolArray;
        for (const FString& Tool : AllowedTools)
        {
            ToolArray.Add(MakeShareable(new FJsonValueString(Tool)));
        }
        Obj->SetArrayField(TEXT("allowed_tools"), ToolArray);

        TArray<TSharedPtr<FJsonValue>> EventArray;
        for (const FGuid& Id : ExplorationEvents)
        {
            EventArray.Add(MakeShareable(new FJsonValueString(Id.ToString())));
        }
        Obj->SetArrayField(TEXT("exploration_events"), EventArray);

        return Obj;
    }

    /** Deserialize from JSON */
    static FExplorationContract FromJson(const TSharedPtr<FJsonObject>& Obj)
    {
        FExplorationContract Contract;

        FGuid::Parse(Obj->GetStringField(TEXT("contract_id")), Contract.ContractId);
        FDateTime::ParseIso8601(*Obj->GetStringField(TEXT("created_at")), Contract.CreatedAt);
        FDateTime::ParseIso8601(*Obj->GetStringField(TEXT("expires_at")), Contract.ExpiresAt);

        FString TierStr = Obj->GetStringField(TEXT("tier"));
        if (TierStr.Equals(TEXT("tactical"), ESearchCase::IgnoreCase))
            Contract.Tier = EExplorationTier::Tactical;
        else if (TierStr.Equals(TEXT("capability"), ESearchCase::IgnoreCase))
            Contract.Tier = EExplorationTier::Capability;
        else if (TierStr.Equals(TEXT("novel"), ESearchCase::IgnoreCase))
            Contract.Tier = EExplorationTier::Novel;
        else if (TierStr.Equals(TEXT("experimental"), ESearchCase::IgnoreCase))
            Contract.Tier = EExplorationTier::Experimental;

        Contract.GoalContext = Obj->GetStringField(TEXT("goal_context"));
        Contract.Hypothesis = Obj->GetStringField(TEXT("hypothesis"));
        Contract.MaxPenaltyBudget = static_cast<float>(Obj->GetNumberField(TEXT("max_penalty_budget")));
        Contract.ConsumedBudget = static_cast<float>(Obj->GetNumberField(TEXT("consumed_budget")));

        double MaxAct = 10.0;
        Obj->TryGetNumberField(TEXT("max_actions"), MaxAct);
        Contract.MaxActions = static_cast<int32>(MaxAct);

        double ConsumedAct = 0.0;
        Obj->TryGetNumberField(TEXT("consumed_actions"), ConsumedAct);
        Contract.ConsumedActions = static_cast<int32>(ConsumedAct);

        Contract.bActive = Obj->GetBoolField(TEXT("active"));
        Obj->TryGetBoolField(TEXT("exhausted"), Contract.bExhausted);
        Obj->TryGetBoolField(TEXT("expired"), Contract.bExpired);
        Obj->TryGetBoolField(TEXT("successful"), Contract.bSuccessful);
        Obj->TryGetStringField(TEXT("final_outcome"), Contract.FinalOutcome);

        const TArray<TSharedPtr<FJsonValue>>* ToolArray;
        if (Obj->TryGetArrayField(TEXT("allowed_tools"), ToolArray))
        {
            for (const auto& Val : *ToolArray)
            {
                Contract.AllowedTools.Add(Val->AsString());
            }
        }

        const TArray<TSharedPtr<FJsonValue>>* EventArray;
        if (Obj->TryGetArrayField(TEXT("exploration_events"), EventArray))
        {
            for (const auto& Val : *EventArray)
            {
                FGuid Id;
                FGuid::Parse(Val->AsString(), Id);
                Contract.ExplorationEvents.Add(Id);
            }
        }

        return Contract;
    }
};

/**
 * Exploration registry - manages active exploration contracts.
 */
class FExplorationRegistry
{
public:
    /** Request a new exploration contract */
    FGuid RequestContract(
        EExplorationTier Tier,
        const FString& Goal,
        const FString& Hypothesis,
        const TArray<FString>& Tools,
        int32 DurationSeconds = 300
    )
    {
        FExplorationContract Contract = FExplorationContract::Create(
            Tier, Goal, Hypothesis, DurationSeconds
        );
        Contract.AllowedTools = Tools;

        ActiveContracts.Add(Contract.ContractId, Contract);

        UE_LOG(LogTemp, Log,
            TEXT("EXPLORATION CONTRACT: Tier=%s Goal=%s Budget=%.1f Duration=%ds"),
            *ExplorationTierToString(Tier), *Goal, Contract.MaxPenaltyBudget, DurationSeconds);

        return Contract.ContractId;
    }

    /** Check if a tool action is covered by an active contract */
    bool IsActionCovered(const FString& ToolName, FGuid& OutContractId)
    {
        CleanupExpired();

        for (auto& Pair : ActiveContracts)
        {
            if (Pair.Value.IsValid())
            {
                // Check if tool is in allowed list, or if list is empty (all tools allowed)
                if (Pair.Value.AllowedTools.Num() == 0 ||
                    Pair.Value.AllowedTools.Contains(ToolName))
                {
                    OutContractId = Pair.Key;
                    return true;
                }
            }
        }

        return false;
    }

    /** Record regret under an exploration contract (consumes budget) */
    bool RecordExplorationRegret(const FGuid& ContractId, const FGuid& EventId, float Penalty)
    {
        if (!ActiveContracts.Contains(ContractId))
        {
            return false;
        }

        FExplorationContract& Contract = ActiveContracts[ContractId];

        if (!Contract.ConsumeBudget(Penalty))
        {
            // Contract exhausted
            Contract.Finalize(false, TEXT("Budget exhausted"));
            return false;
        }

        Contract.ExplorationEvents.Add(EventId);
        return true;
    }

    /** Finalize a contract with success/failure */
    void FinalizeContract(const FGuid& ContractId, bool bSuccess, const FString& Outcome)
    {
        if (ActiveContracts.Contains(ContractId))
        {
            ActiveContracts[ContractId].Finalize(bSuccess, Outcome);

            // Move to history
            ContractHistory.Add(ActiveContracts[ContractId]);
            ActiveContracts.Remove(ContractId);
        }
    }

    /** Get active contract count */
    int32 GetActiveContractCount() const { return ActiveContracts.Num(); }

    /** Get exploration history count */
    int32 GetHistoryCount() const { return ContractHistory.Num(); }

private:
    void CleanupExpired()
    {
        TArray<FGuid> ToRemove;

        for (auto& Pair : ActiveContracts)
        {
            if (FDateTime::Now() > Pair.Value.ExpiresAt)
            {
                Pair.Value.bExpired = true;
                Pair.Value.Finalize(false, TEXT("Contract expired"));
                ContractHistory.Add(Pair.Value);
                ToRemove.Add(Pair.Key);
            }
        }

        for (const FGuid& Id : ToRemove)
        {
            ActiveContracts.Remove(Id);
        }
    }

    TMap<FGuid, FExplorationContract> ActiveContracts;
    TArray<FExplorationContract> ContractHistory;
};

// =============================================================================
// CROSS-GOAL REGRET GENERALIZATION
// =============================================================================
//
// Problem: If tool X repeatedly fails in different goals for the same causal reason,
// the agent should pre-emptively avoid it BEFORE taking the hit again.
//
// Solution: Track failure patterns across goals, not just within goals.
// When a pattern emerges (same tool, same failure mode, multiple goals),
// elevate the warning before the agent commits.

/**
 * A failure pattern - captures recurring failure modes.
 */
struct FFailurePattern
{
    // Identity
    FGuid PatternId;
    FDateTime FirstSeen;
    FDateTime LastSeen;

    // Pattern definition
    FString ToolName;               // Which tool
    FString FailureMode;            // Classified failure type (e.g., "actor_not_found", "timeout")
    FString ClaimFamily;            // Which claim family was violated

    // Evidence
    TArray<FGuid> ContributingEvents;  // Regret events that match this pattern
    TArray<FString> GoalContexts;      // Which goals saw this pattern
    int32 OccurrenceCount = 0;

    // Analysis
    float TotalPenalty = 0.0f;         // Sum of penalties from this pattern
    float AveragePenalty = 0.0f;

    // State
    bool bGeneralized = false;         // Has this pattern crossed the generalization threshold?
    float GeneralizationConfidence = 0.0f;  // 0.0 - 1.0

    /** Add an occurrence of this pattern */
    void AddOccurrence(const FGuid& EventId, const FString& GoalContext, float Penalty)
    {
        LastSeen = FDateTime::Now();
        ContributingEvents.Add(EventId);
        OccurrenceCount++;
        TotalPenalty += Penalty;
        AveragePenalty = TotalPenalty / OccurrenceCount;

        // Track unique goals
        if (!GoalContexts.Contains(GoalContext))
        {
            GoalContexts.Add(GoalContext);
        }

        // Update generalization confidence
        UpdateConfidence();
    }

    /** Calculate generalization confidence */
    void UpdateConfidence()
    {
        float OccurrenceScore = FMath::Clamp(OccurrenceCount / 5.0f, 0.0f, 1.0f);
        float GoalDiversityScore = FMath::Clamp(GoalContexts.Num() / 3.0f, 0.0f, 1.0f);

        // Time decay: full weight if within 1 hour, decays over 24 hours
        double HoursSinceLast = (FDateTime::Now() - LastSeen).GetTotalHours();
        float RecencyScore = FMath::Clamp(1.0f - static_cast<float>(HoursSinceLast / 24.0), 0.0f, 1.0f);

        // Combined confidence (cross-goal diversity is weighted highest)
        GeneralizationConfidence =
            0.3f * OccurrenceScore +
            0.5f * GoalDiversityScore +
            0.2f * RecencyScore;

        // Mark as generalized if confidence exceeds threshold
        bGeneralized = (GeneralizationConfidence >= 0.6f && GoalContexts.Num() >= 2);
    }

    /** Get a human-readable explanation of this pattern */
    FString GetExplanation() const
    {
        return FString::Printf(
            TEXT("Tool '%s' has failed with '%s' (claim: %s) %d times across %d goals. Confidence: %.1f%%"),
            *ToolName, *FailureMode, *ClaimFamily, OccurrenceCount, GoalContexts.Num(),
            GeneralizationConfidence * 100.0f
        );
    }

    /** Serialize to JSON */
    TSharedPtr<FJsonObject> ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject);

        Obj->SetStringField(TEXT("pattern_id"), PatternId.ToString());
        Obj->SetStringField(TEXT("first_seen"), FirstSeen.ToIso8601());
        Obj->SetStringField(TEXT("last_seen"), LastSeen.ToIso8601());
        Obj->SetStringField(TEXT("tool_name"), ToolName);
        Obj->SetStringField(TEXT("failure_mode"), FailureMode);
        Obj->SetStringField(TEXT("claim_family"), ClaimFamily);
        Obj->SetNumberField(TEXT("occurrence_count"), OccurrenceCount);
        Obj->SetNumberField(TEXT("total_penalty"), TotalPenalty);
        Obj->SetNumberField(TEXT("average_penalty"), AveragePenalty);
        Obj->SetBoolField(TEXT("generalized"), bGeneralized);
        Obj->SetNumberField(TEXT("confidence"), GeneralizationConfidence);
        Obj->SetStringField(TEXT("explanation"), GetExplanation());

        TArray<TSharedPtr<FJsonValue>> GoalArray;
        for (const FString& Goal : GoalContexts)
        {
            GoalArray.Add(MakeShareable(new FJsonValueString(Goal)));
        }
        Obj->SetArrayField(TEXT("goal_contexts"), GoalArray);

        return Obj;
    }
};

/**
 * Failure pattern registry - tracks cross-goal failure patterns.
 */
class FFailurePatternRegistry
{
public:
    /** Record a failure and check for pattern emergence */
    void RecordFailure(
        const FString& ToolName,
        const FString& FailureMode,
        const FString& ClaimFamily,
        const FString& GoalContext,
        const FGuid& EventId,
        float Penalty
    )
    {
        // Generate pattern key
        FString PatternKey = FString::Printf(TEXT("%s::%s::%s"), *ToolName, *FailureMode, *ClaimFamily);

        if (!Patterns.Contains(PatternKey))
        {
            // New pattern
            FFailurePattern NewPattern;
            NewPattern.PatternId = FGuid::NewGuid();
            NewPattern.FirstSeen = FDateTime::Now();
            NewPattern.LastSeen = FDateTime::Now();
            NewPattern.ToolName = ToolName;
            NewPattern.FailureMode = FailureMode;
            NewPattern.ClaimFamily = ClaimFamily;

            Patterns.Add(PatternKey, NewPattern);
        }

        FFailurePattern& Pattern = Patterns[PatternKey];
        Pattern.AddOccurrence(EventId, GoalContext, Penalty);

        // Log if pattern became generalized
        if (Pattern.bGeneralized && !GeneralizedPatterns.Contains(PatternKey))
        {
            GeneralizedPatterns.Add(PatternKey);

            UE_LOG(LogTemp, Warning,
                TEXT("PATTERN GENERALIZED: %s"),
                *Pattern.GetExplanation());
        }
    }

    /** Check if a tool should be warned about before use */
    bool ShouldWarnBeforeUse(const FString& ToolName, TArray<FString>& OutWarnings)
    {
        bool bShouldWarn = false;

        for (const auto& Pair : Patterns)
        {
            if (Pair.Value.ToolName == ToolName && Pair.Value.bGeneralized)
            {
                OutWarnings.Add(Pair.Value.GetExplanation());
                bShouldWarn = true;
            }
        }

        return bShouldWarn;
    }

    /** Get pre-emptive penalty suggestion based on patterns */
    float GetPreEmptivePenaltyHint(const FString& ToolName) const
    {
        float MaxConfidence = 0.0f;
        float SuggestedPenalty = 0.0f;

        for (const auto& Pair : Patterns)
        {
            if (Pair.Value.ToolName == ToolName && Pair.Value.bGeneralized)
            {
                if (Pair.Value.GeneralizationConfidence > MaxConfidence)
                {
                    MaxConfidence = Pair.Value.GeneralizationConfidence;
                    SuggestedPenalty = Pair.Value.AveragePenalty;
                }
            }
        }

        // Scale by confidence
        return SuggestedPenalty * MaxConfidence;
    }

    /** Get all generalized patterns for a tool */
    TArray<FFailurePattern> GetGeneralizedPatternsForTool(const FString& ToolName) const
    {
        TArray<FFailurePattern> Result;

        for (const auto& Pair : Patterns)
        {
            if (Pair.Value.ToolName == ToolName && Pair.Value.bGeneralized)
            {
                Result.Add(Pair.Value);
            }
        }

        return Result;
    }

    /** Get total pattern count */
    int32 GetPatternCount() const { return Patterns.Num(); }

    /** Get generalized pattern count */
    int32 GetGeneralizedCount() const { return GeneralizedPatterns.Num(); }

    /** Export all patterns to JSON */
    TSharedPtr<FJsonObject> ToJson() const
    {
        TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject);

        TArray<TSharedPtr<FJsonValue>> PatternArray;
        for (const auto& Pair : Patterns)
        {
            PatternArray.Add(MakeShareable(new FJsonValueObject(Pair.Value.ToJson())));
        }
        Obj->SetArrayField(TEXT("patterns"), PatternArray);
        Obj->SetNumberField(TEXT("total_count"), Patterns.Num());
        Obj->SetNumberField(TEXT("generalized_count"), GeneralizedPatterns.Num());

        return Obj;
    }

private:
    TMap<FString, FFailurePattern> Patterns;  // Key = "tool::mode::claim"
    TSet<FString> GeneralizedPatterns;         // Keys that have been generalized
};
