// BrainMetrics.h
// Real metrics tracking - actual numbers, no faking
// If these numbers don't improve over time, the brain is NOT learning.

#pragma once

#include "CoreMinimal.h"
#include "RegretScope.h"

/**
 * Brain Performance Metrics
 * 
 * These are the ONLY numbers that matter:
 * 1. Success rate - does the brain complete tasks?
 * 2. LLM dependence - how much does it rely on the LLM?
 * 3. Efficiency - tokens/tools per task
 * 4. Generalization - can it handle new variations?
 * 
 * If these don't improve, the system is just an LLM wrapper.
 */
class RIFTBORNAI_API FBrainMetrics
{
public:
    static FBrainMetrics& Get();
    
    // ==========================================================================
    // Core Metrics (the only ones that matter)
    // ==========================================================================
    
    /** Current success rate across all task families */
    float GetOverallSuccessRate() const;
    
    /** Success rate for specific task family */
    float GetSuccessRate(const FString& TaskFamily) const;
    
    /** LLM dependency ratio (0 = brain only, 1 = LLM only) */
    float GetLLMDependency(const FString& TaskFamily) const;
    
    /** Average tokens per successful task */
    float GetTokenEfficiency(const FString& TaskFamily) const;
    
    /** Average tool calls per successful task */
    float GetToolEfficiency(const FString& TaskFamily) const;
    
    /** Is the brain actually getting better? Compare recent vs historical */
    bool IsImproving(const FString& TaskFamily, int32 RecentWindow = 10) const;
    
    // ==========================================================================
    // Trend Analysis
    // ==========================================================================
    
    struct FTrend
    {
        float SuccessRateChange;    // Positive = improving
        float TokenEfficiencyChange; // Negative = improving (using fewer)
        float ToolEfficiencyChange;  // Negative = improving
        bool bIsImproving;           // Overall judgment
    };
    
    /** Compare recent performance to historical baseline */
    FTrend ComputeTrend(const FString& TaskFamily, int32 RecentWindow = 10, int32 BaselineWindow = 50) const;
    
    // ==========================================================================
    // P3 INTEGRATION: Unified Action Score (Policy)
    // ==========================================================================
    
    /**
     * Score weights for computing unified action scores.
     * These control how different factors influence tool selection.
     */
    struct FScoreWeights
    {
        float SuccessWeight = 1.0f;       // Weight of predicted success probability
        float TrustPenaltyWeight = 0.5f;  // Weight of accumulated trust penalty
        float SurpriseWeight = 0.3f;      // Weight of expected surprise (unpredictability)
        float TimeCostWeight = 0.1f;      // Weight of expected execution time
    };
    
    /**
     * Compute unified action score for policy decisions.
     * Higher score = better action choice.
     * 
     * Score = w1*P(success) - w2*TrustPenalty - w3*ExpectedSurprise - w4*Cost(time)
     * 
     * This is the POLICY - a single scalar that determines tool preference.
     * 
     * @param ToolName - The tool to score
     * @param Context - Optional context string (e.g., "PIE:TestMap")
     * @return Score value (higher is better, can be negative for blocked tools)
     */
    float ComputeActionScore(const FString& ToolName, const FString& Context = TEXT("")) const;
    
    /**
     * Get the best tool from candidates based on action scores.
     * Unlike GetBestToolFromList which only uses trust, this uses the full policy.
     * 
     * @param Candidates - List of tool names to choose from
     * @param Context - Optional context string
     * @return Best tool name, or empty if all blocked
     */
    FString GetBestToolByScore(const TArray<FString>& Candidates, const FString& Context = TEXT("")) const;
    
    /**
     * Get ranked tools from candidates in score order (best first).
     * 
     * @param Candidates - List of tool names to rank
     * @param Context - Optional context string
     * @return Ranked tools with their scores
     */
    TArray<TPair<FString, float>> GetRankedToolsByScore(const TArray<FString>& Candidates, const FString& Context = TEXT("")) const;
    
    /**
     * Set custom score weights for policy tuning.
     */
    void SetScoreWeights(const FScoreWeights& Weights);
    
    /**
     * Get current score weights.
     */
    FScoreWeights GetScoreWeights() const;
    
    // ==========================================================================
    // Recording
    // ==========================================================================
    
    /** Record a completed task attempt */
    void RecordTaskAttempt(
        const FString& TaskFamily,
        bool bSuccess,
        int32 TokensUsed,
        int32 ToolCallsUsed,
        int32 LLMCallsUsed,       // How many times did we query LLM?
        int32 BrainOnlyDecisions, // How many decisions made without LLM?
        float TimeSeconds
    );
    
    /** Record individual tool execution for learning */
    void RecordToolExecution(
        const FString& ToolName,
        bool bSuccess,
        float ExecutionTimeMs = 0.0f
    );
    
    /** 
     * Record a suboptimal outcome - tool succeeded technically but failed expectations
     * This is the key policy consequence: suboptimal = conceptual failure
     * Affects future trust/predictions for this tool
     */
    void RecordSuboptimalOutcome(
        const FString& ToolName,
        const FString& Reason,
        int32 ReasonCode,
        float ViolationMagnitude = 1.0f
    );
    
    /**
     * Reset trust penalty for a tool (for testing/CI only)
     * Clears TrustPenalty and SuboptimalOutcomes
     */
    void ResetToolPenalty(const FString& ToolName);
    
    /**
     * Record a SCOPED regret event with full context
     * This is the primary regret recording mechanism.
     * 
     * @param ToolName - The tool that caused regret
     * @param Reason - Human-readable reason
     * @param ReasonCode - Numeric code for categorization
     * @param BaseMagnitude - Raw regret magnitude before scope weighting
     * @param Scope - Regret scope (Tactical, Strategic, Optionality, Deferred)
     * @param ExpectedOutcome - What was expected to happen
     * @param ActualOutcome - What actually happened
     * @param ExpectationDelta - Numeric distance from expectation
     */
    void RecordScopedRegret(
        const FString& ToolName,
        const FString& Reason,
        int32 ReasonCode,
        float BaseMagnitude,
        ERegretScope Scope,
        const FString& ExpectedOutcome,
        const FString& ActualOutcome,
        float ExpectationDelta
    );
    
    /**
     * Get current world context string for regret events
     */
    FString GetCurrentWorldContext() const;
    
    /**
     * Record a suboptimal outcome with CONTEXT
     * Context = ToolName + Mode (PIE/Editor) + MapName
     * This prevents global punishment for context-specific failures
     */
    void RecordSuboptimalOutcomeWithContext(
        const FString& ToolName,
        const FString& Context,  // e.g., "PIE:TestMap" or "Editor:MainLevel"
        const FString& Reason,
        int32 ReasonCode,
        float ViolationMagnitude = 1.0f
    );
    
    /**
     * Record suboptimal outcome with WEIGHTED penalty based on claim calibration
     * Claims that have historically produced false positives get lower weight
     */
    void RecordSuboptimalOutcomeWeighted(
        const FString& ToolName,
        const FString& Reason,
        int32 ReasonCode,
        float ViolationMagnitude,
        FName ClaimKey
    );
    
    /**
     * Check if a tool is blocked in a specific context
     */
    bool IsToolBlockedByTrustInContext(const FString& ToolName, const FString& Context) const;
    
    /** Record an LLM query */
    void RecordLLMQuery(int32 TokensUsed, float LatencyMs = 0.0f);

    /** Force-flush both the task-family history and the per-tool metrics to disk.
     *  Normally called automatically by RecordTaskAttempt; use directly before
     *  crash-prone operations or in tests that need guaranteed persistence. */
    void FlushToDisk();

    /** Get tool success rate */
    float GetToolSuccessRate(const FString& ToolName) const;
    
    /** Get most reliable tools */
    TArray<FString> GetMostReliableTools(int32 TopN = 10) const;
    
    /**
     * Check if a tool is blocked due to trust penalty
     * This is the ENFORCEMENT gate - returns true if TrustPenalty >= 3.0
     */
    bool IsToolBlockedByTrust(const FString& ToolName) const;
    
    /**
     * Get current trust penalty for a tool
     * Returns the accumulated penalty value
     */
    float GetToolTrustPenalty(const FString& ToolName) const;
    
    /**
     * Get alternative tools that are NOT blocked
     * Returns tools in the same category as BlockedTool that can be used instead
     * This enables Level 2 closure: blocked → alternative → success
     */
    TArray<FString> GetAlternativeTools(const FString& BlockedTool) const;
    
    /**
     * Get the best tool from a list based on trust scores
     * Returns the tool with lowest TrustPenalty (most trusted)
     * Excludes any tools that are currently blocked
     */
    FString GetBestToolFromList(const TArray<FString>& Candidates) const;

    // ==========================================================================
    // Per-Tool Statistics
    // ==========================================================================
    
    struct FToolStats
    {
        int32 TotalCalls = 0;
        int32 SuccessfulCalls = 0;
        int32 SuboptimalOutcomes = 0;  // Technically succeeded but failed expectations
        float TrustPenalty = 0.0f;     // Accumulated penalty from suboptimal outcomes
        float TotalExecutionTimeMs = 0.0f;
        
        /** Is this tool blocked? (TrustPenalty >= 3.0) */
        bool IsBlocked() const { return TrustPenalty >= 3.0f; }
    };
    
    /** Get stats for a specific tool */
    FToolStats GetToolStats(const FString& ToolName) const;
    
    /** Get all tool stats as array of pairs */
    TArray<TPair<FString, FToolStats>> GetAllToolStats() const;

    // ==========================================================================
    // Level 4: Environment Steward Access
    // ==========================================================================

    /** Per-tool metrics entry (for steward access) */
    struct FToolMetricsEntry
    {
        int32 TotalCalls = 0;
        int32 SuccessfulCalls = 0;
        int32 SuboptimalOutcomes = 0;
        int32 VerifiedSuccesses = 0;
        float TrustPenalty = 0.0f;
        float TotalTimeMs = 0.0f;
        FDateTime LastPenaltyTime;
        FDateTime LastSuccessTime;
    };
    
    /** Claim calibration data (for steward access) */
    struct FClaimCalibrationData
    {
        FName ClaimKey;
        int32 TruePositives = 0;
        int32 FalsePositives = 0;
        int32 TrueNegatives = 0;
        int32 FalseNegatives = 0;
        
        float GetPrecision() const
        {
            int32 Flagged = TruePositives + FalsePositives;
            return Flagged > 0 ? static_cast<float>(TruePositives) / Flagged : 1.0f;
        }
    };
    
    /** Get all tool metrics (for Environment Steward health monitoring) */
    TMap<FString, FToolMetricsEntry> GetAllToolMetrics() const;
    
    /** Get all claim calibrations (for precision tracking) */
    TMap<FName, FClaimCalibrationData> GetAllClaimCalibrations() const;

    /** Adjust trust penalty for a specific tool (negative reduces penalty) */
    void AdjustToolTrustPenalty(const FString& ToolName, float Delta);

    /** Scale all tool penalties by factor (0-1 reduces, >1 increases) */
    void ScaleToolPenalties(float Factor);

    /** Apply immediate decay to all penalties */
    void ApplyImmediateDecay(float Amount);

    /** Decay claim calibration counts */
    void DecayClaimCalibrations(float Factor);

    /** Reset all claim calibration data */
    void ResetClaimCalibrations();

    /** Set minimum claim weight */
    void SetClaimWeightFloor(float Floor);

    /** Set maximum claim weight */
    void SetClaimWeightCeiling(float Ceiling);

    /** Get minimum claim weight */
    float GetClaimWeightFloor() const { return ClaimWeightFloor; }

    /** Get maximum claim weight */
    float GetClaimWeightCeiling() const { return ClaimWeightCeiling; }

    // ==========================================================================
    // Aggregate Counters (for brain report)
    // ==========================================================================
    
    /** Total tool calls across all tools */
    int32 TotalToolCalls = 0;
    
    /** Successful tool calls */
    int32 SuccessfulToolCalls = 0;
    
    /** Total LLM queries made */
    int32 TotalLLMQueries = 0;
    
    /** Total tokens used across all queries */
    int32 TotalTokensUsed = 0;
    
    // ==========================================================================
    // Reporting
    // ==========================================================================
    
    /** Get human-readable report */
    FString GenerateReport() const;
    
    /** Log current metrics to output */
    void LogMetrics() const;
    
    /** Export metrics to JSON file */
    void ExportToJson(const FString& FilePath) const;
    
    /** Record a verified success for a tool (claims satisfied) - public interface */
    void RecordVerifiedSuccess(const FString& ToolName);
    
private:
    FBrainMetrics();
    
    /** P3 Integration: Score weights for action policy */
    FScoreWeights ScoreWeights;
    
    /** Persistent storage path for family metrics */
    FString GetMetricsPath() const;
    
    /** Persistent storage path for tool metrics */
    FString GetToolMetricsPath() const;
    
    /** Load historical data */
    void LoadHistory();
    
    /** Save current data */
    void SaveHistory();
    
    /** Load tool-level metrics */
    void LoadToolMetrics();
    
    /** Save tool-level metrics */
    void SaveToolMetrics();
    
    // Per-task-family tracking
    struct FTaskFamilyMetrics
    {
        int32 TotalAttempts = 0;
        int32 Successes = 0;
        int64 TotalTokens = 0;
        int32 TotalToolCalls = 0;
        int32 TotalLLMCalls = 0;
        int32 TotalBrainDecisions = 0;
        float TotalTimeSeconds = 0.0f;
        
        // Recent history for trend analysis (circular buffer)
        struct FAttempt
        {
            bool bSuccess;
            int32 Tokens;
            int32 Tools;
            int32 LLMCalls;
            int32 BrainDecisions;
            float Time;
            FDateTime Timestamp;
        };
        TArray<FAttempt> RecentAttempts; // Keep last 100
    };
    
    TMap<FString, FTaskFamilyMetrics> MetricsByFamily;
    
    // Per-tool tracking for learning which tools work best
    struct FToolMetrics
    {
        int32 TotalCalls = 0;
        int32 Successes = 0;
        int32 SuboptimalOutcomes = 0;  // Succeeded technically but failed expectations
        int32 VerifiedSuccesses = 0;   // Successes that also satisfied all claims
        float TotalTimeMs = 0.0f;
        float TrustPenalty = 0.0f;     // Accumulated penalty from suboptimal outcomes
        FDateTime LastPenaltyTime;     // When penalty was last applied (for decay)
        FDateTime LastSuccessTime;     // When tool last succeeded with claims satisfied
        TArray<float> RecentTimes; // Last 100 execution times
        
        /** Get effective success rate (penalizes suboptimal outcomes) */
        float GetEffectiveSuccessRate() const
        {
            if (TotalCalls == 0) return 1.0f;
            // Suboptimal = 0.5 failure (not as bad as real failure, but penalized)
            float EffectiveSuccesses = static_cast<float>(Successes) - (SuboptimalOutcomes * 0.5f);
            return FMath::Clamp(EffectiveSuccesses / TotalCalls, 0.0f, 1.0f);
        }
        
        /** Apply time-based decay to trust penalty */
        void ApplyDecay()
        {
            if (TrustPenalty <= 0.0f) return;
            
            const FDateTime Now = FDateTime::Now();
            const FTimespan TimeSincePenalty = Now - LastPenaltyTime;
            
            // Decay rate: 0.1 penalty per hour
            // After 10 hours of no suboptimal outcomes, fully recovered
            const double HoursSincePenalty = TimeSincePenalty.GetTotalHours();
            const float DecayAmount = static_cast<float>(HoursSincePenalty) * 0.1f;
            
            TrustPenalty = FMath::Max(0.0f, TrustPenalty - DecayAmount);
        }
        
        /** Should we block this tool due to poor trust? */
        bool ShouldBlockDueToTrust() const
        {
            // Block if trust penalty exceeds threshold
            return TrustPenalty >= 3.0f;  // 3 suboptimal outcomes = block
        }
        
        /** Record a verified success (claims satisfied) - accelerates trust recovery */
        void RecordVerifiedSuccess()
        {
            VerifiedSuccesses++;
            LastSuccessTime = FDateTime::Now();
            
            // Verified success reduces penalty by 0.5 (faster than time decay)
            TrustPenalty = FMath::Max(0.0f, TrustPenalty - 0.5f);
        }
    };
    TMap<FString, FToolMetrics> MetricsByTool;
    
    // Context-specific trust penalties
    // Key: "ToolName|Context" (e.g., "spawn_actor|PIE:TestMap")
    // Allows tool to be blocked in one context but not others
    struct FContextPenalty
    {
        int32 SuboptimalCount = 0;
        float TrustPenalty = 0.0f;
        FString LastReason;
        FDateTime LastPenaltyTime;  // FIX: Added for time-based decay (was missing, causing immortal penalties)
        
        bool ShouldBlock() const { return TrustPenalty >= 3.0f; }
        
        /** Apply time-based decay (same rate as tool penalties: 0.1 per hour) */
        void ApplyDecay()
        {
            if (TrustPenalty <= 0.0f || LastPenaltyTime.GetTicks() == 0) return;
            const FTimespan TimeSince = FDateTime::Now() - LastPenaltyTime;
            const float DecayAmount = static_cast<float>(TimeSince.GetTotalHours()) * 0.1f;
            TrustPenalty = FMath::Max(0.0f, TrustPenalty - DecayAmount);
        }
    };
    TMap<FString, FContextPenalty> ContextPenalties;
    
    /** Build context key for penalty lookup */
    static FString BuildContextKey(const FString& ToolName, const FString& Context)
    {
        return FString::Printf(TEXT("%s|%s"), *ToolName, *Context);
    }
    
public:
    // =========================================================================
    // CLAIM CALIBRATION - Track false positives and weight claims by reliability
    // These are public as they're used by FExpectationTracker
    // =========================================================================
    
    /** Source of a claim (affects trust weight) */
    enum class EClaimSource : uint8
    {
        LLMGenerated,      // LLM predicted this claim - lower initial trust
        HeuristicRule,     // Built-in heuristic - medium trust
        LearnedPrior,      // Learned from past outcomes - highest trust
        UserSpecified      // User explicitly set - absolute trust
    };
    
    /** Calibration data for a claim type */
    struct FClaimCalibration
    {
        FName ClaimKey;                // e.g., "ActorCount", "ErrorCount"
        EClaimSource Source = EClaimSource::HeuristicRule;
        
        int32 TimesApplied = 0;        // How many times this claim was checked
        int32 TruePositives = 0;       // Correctly flagged suboptimal
        int32 FalsePositives = 0;      // Incorrectly flagged (tool was actually fine)
        int32 TrueNegatives = 0;       // Correctly passed
        int32 FalseNegatives = 0;      // Missed a real problem
        
        /** Precision: Of times we flagged suboptimal, how often were we right? */
        float GetPrecision() const
        {
            const int32 Flagged = TruePositives + FalsePositives;
            return Flagged > 0 ? static_cast<float>(TruePositives) / Flagged : 1.0f;
        }
        
        /** Recall: Of actual suboptimal cases, how often did we catch them? */
        float GetRecall() const
        {
            const int32 Actual = TruePositives + FalseNegatives;
            return Actual > 0 ? static_cast<float>(TruePositives) / Actual : 1.0f;
        }
        
        /** Weight to apply to this claim's violation (based on precision) */
        float GetWeight() const
        {
            // Source-based base weight
            float BaseWeight = 1.0f;
            switch (Source)
            {
                case EClaimSource::LLMGenerated:  BaseWeight = 0.5f; break;
                case EClaimSource::HeuristicRule: BaseWeight = 0.8f; break;
                case EClaimSource::LearnedPrior:  BaseWeight = 1.0f; break;
                case EClaimSource::UserSpecified: BaseWeight = 1.0f; break;
            }
            
            // Adjust by precision (high false-positive rate = lower weight)
            // Only adjust after sufficient samples
            if (TimesApplied >= 10)
            {
                BaseWeight *= GetPrecision();
            }
            
            return FMath::Clamp(BaseWeight, 0.1f, 1.0f);
        }
    };
    
    /** Record claim outcome for calibration */
    void RecordClaimOutcome(
        FName ClaimKey,
        EClaimSource Source,
        bool bClaimFlagged,     // Did this claim flag suboptimal?
        bool bActuallySuboptimal // Was the outcome actually suboptimal (verified)?
    );
    
    /** Get weight for a claim based on calibration */
    float GetClaimWeight(FName ClaimKey) const;
    
    /** Apply time-based decay to all tool penalties */
    void ApplyGlobalDecay();

private:
    TMap<FName, FClaimCalibration> ClaimCalibrations;

    float ClaimWeightFloor = 0.1f;
    float ClaimWeightCeiling = 1.0f;
    
    /** Counter for periodic tool metrics persistence (replaces static local) */
    int32 ToolMetricsSaveCounter = 0;
    
    mutable FCriticalSection MetricsLock;
};

// ==========================================================================
// Macros for easy metric recording
// ==========================================================================

#define BRAIN_RECORD_SUCCESS(Family, Tokens, Tools, LLMCalls, BrainCalls, Time) \
    FBrainMetrics::Get().RecordTaskAttempt(Family, true, Tokens, Tools, LLMCalls, BrainCalls, Time)

#define BRAIN_RECORD_FAILURE(Family, Tokens, Tools, LLMCalls, BrainCalls, Time) \
    FBrainMetrics::Get().RecordTaskAttempt(Family, false, Tokens, Tools, LLMCalls, BrainCalls, Time)

#define BRAIN_LOG_METRICS() \
    FBrainMetrics::Get().LogMetrics()
