// Copyright RiftbornAI. All Rights Reserved.
// AgentPlan.h - Plan representation for autonomous project execution (Level 3)

#pragma once

#include "CoreMinimal.h"
#include "Expectation.h"
#include "ClaudeToolUse.h"
#include "Dom/JsonObject.h"

/**
 * Plan step state
 */
enum class EPlanStepState : uint8
{
    NotStarted,     // Step has not begun
    InProgress,     // Step is currently executing
    Succeeded,      // Step completed successfully and met expectations
    Failed,         // Step failed technically (tool error)
    Suboptimal,     // Step succeeded technically but violated expectations
    Blocked,        // Step's tool is blocked, need alternative
    Skipped         // Step was skipped (dependency failed, or replanned)
};

/**
 * Plan state
 */
enum class EPlanState : uint8
{
    NotStarted,     // Plan has not begun execution
    InProgress,     // At least one step is executing
    Succeeded,      // All steps succeeded, goal achieved
    Failed,         // One or more critical steps failed, goal not achievable
    Replanning,     // Encountered failure, generating new plan
    Cancelled       // User or system cancelled the plan
};

/**
 * FPlanStep - Single action in a plan
 * 
 * A step represents one tool call with expected outcome.
 * Steps can have dependencies on other steps.
 */
struct RIFTBORNAI_API FPlanStep
{
    /** Unique ID within the plan */
    FGuid StepId;
    
    /** Human-readable label */
    FString Label;
    
    /** Tool to execute */
    FString ToolName;
    
    /** Arguments for the tool (JSON string) */
    FString ToolArguments;
    
    /** Expected outcome - claims that must be satisfied after execution */
    TArray<FStateClaim> ExpectedOutcome;
    
    /** Dependencies - step IDs that must complete before this step */
    TArray<FGuid> Dependencies;
    
    /** Current state */
    EPlanStepState State = EPlanStepState::NotStarted;
    
    /** Result of execution (if executed) */
    FClaudeToolResult ExecutionResult;
    
    /** Observation after execution (for expectation checking) */
    FOutcomeObservation Observation;
    
    /** Alternative steps if this one is blocked */
    TArray<FGuid> AlternativeSteps;
    
    /** Error message if failed/blocked */
    FString ErrorMessage;
    
    /** Execution time */
    float ExecutionTimeMs = 0.0f;
    
    /** Retry count */
    int32 RetryCount = 0;
    
    /** Maximum retries before giving up */
    int32 MaxRetries = 2;
    
    FPlanStep()
    {
        StepId = FGuid::NewGuid();
    }
    
    /** Check if step can execute (all dependencies met) */
    bool CanExecute(const TMap<FGuid, FPlanStep>& AllSteps) const
    {
        for (const FGuid& DepId : Dependencies)
        {
            if (const FPlanStep* Dep = AllSteps.Find(DepId))
            {
                if (Dep->State != EPlanStepState::Succeeded)
                {
                    return false;
                }
            }
        }
        return true;
    }
    
    /** Check if step is terminal (completed, failed, or blocked) */
    bool IsTerminal() const
    {
        return State == EPlanStepState::Succeeded ||
               State == EPlanStepState::Failed ||
               State == EPlanStepState::Blocked ||
               State == EPlanStepState::Skipped;
    }

    /** Serialize to JSON */
    TSharedPtr<FJsonObject> ToJson() const;

    /** Deserialize from JSON */
    static FPlanStep FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * FAgentGoal - What the agent is trying to achieve
 * 
 * A goal is defined by success criteria (claims on world state).
 * The goal is achieved when all success criteria are satisfied.
 */
struct RIFTBORNAI_API FAgentGoal
{
    /** Unique goal ID */
    FGuid GoalId;
    
    /** Human-readable description */
    FString Description;
    
    /** Success criteria - all must be satisfied for goal to be achieved */
    TArray<FStateClaim> SuccessCriteria;
    
    /** Optional deadline */
    TOptional<FDateTime> Deadline;
    
    /** Priority (higher = more important) */
    int32 Priority = 0;
    
    /** Who requested this goal (user, system, or self-generated) */
    FString RequestedBy;
    
    /** Timestamp when goal was created */
    FDateTime CreatedAt;
    
    FAgentGoal()
    {
        GoalId = FGuid::NewGuid();
        CreatedAt = FDateTime::UtcNow();
    }
    
    /** Check if goal is satisfied given current world state */
    bool IsSatisfied(const FWorldStateDigest& WorldState) const
    {
        for (const FStateClaim& Criterion : SuccessCriteria)
        {
            FClaimResult Result = WorldState.CheckClaim(Criterion);
            if (!Result.bSatisfied)
            {
                return false;
            }
        }
        return true;
    }
};

/**
 * FAgentPlan - Ordered sequence of steps to achieve a goal
 * 
 * A plan is generated by decomposing a goal into executable steps.
 * Plans can be re-generated (replanned) if steps fail or get blocked.
 */
struct RIFTBORNAI_API FAgentPlan
{
    /** Unique plan ID */
    FGuid PlanId;
    
    /** The goal this plan is trying to achieve */
    FAgentGoal Goal;
    
    /** All steps in the plan, keyed by step ID */
    TMap<FGuid, FPlanStep> Steps;
    
    /** Execution order (step IDs in order of execution) */
    TArray<FGuid> ExecutionOrder;
    
    /** Current plan state */
    EPlanState State = EPlanState::NotStarted;
    
    /** Index of current step being executed */
    int32 CurrentStepIndex = -1;
    
    /** Replan count (how many times we've had to regenerate) */
    int32 ReplanCount = 0;
    
    /** Maximum replans before giving up */
    int32 MaxReplans = 3;
    
    /** Timestamp when plan was created */
    FDateTime CreatedAt;
    
    /** Timestamp when execution started */
    FDateTime StartedAt;
    
    /** Timestamp when execution completed */
    FDateTime CompletedAt;
    
    /** Total tokens used for this plan */
    int32 TotalTokensUsed = 0;
    
    /** Total tools executed */
    int32 TotalToolsExecuted = 0;
    
    /** Reasoning for why this plan was generated */
    FString PlanReasoning;
    
    FAgentPlan()
    {
        PlanId = FGuid::NewGuid();
        CreatedAt = FDateTime::UtcNow();
    }
    
    /** Add a step to the plan */
    FGuid AddStep(const FPlanStep& Step)
    {
        FPlanStep NewStep = Step;
        Steps.Add(NewStep.StepId, NewStep);
        ExecutionOrder.Add(NewStep.StepId);
        return NewStep.StepId;
    }
    
    /** Get current step (if any) */
    FPlanStep* GetCurrentStep()
    {
        if (CurrentStepIndex >= 0 && CurrentStepIndex < ExecutionOrder.Num())
        {
            return Steps.Find(ExecutionOrder[CurrentStepIndex]);
        }
        return nullptr;
    }
    
    /** Get next executable step (respecting dependencies) */
    FPlanStep* GetNextExecutableStep()
    {
        for (const FGuid& StepId : ExecutionOrder)
        {
            FPlanStep* Step = Steps.Find(StepId);
            if (Step && Step->State == EPlanStepState::NotStarted && Step->CanExecute(Steps))
            {
                return Step;
            }
        }
        return nullptr;
    }
    
    /** Calculate progress (0.0 - 1.0) */
    float GetProgress() const
    {
        if (Steps.Num() == 0) return 0.0f;
        
        int32 CompletedSteps = 0;
        for (const auto& Pair : Steps)
        {
            if (Pair.Value.State == EPlanStepState::Succeeded)
            {
                CompletedSteps++;
            }
        }
        
        return static_cast<float>(CompletedSteps) / static_cast<float>(Steps.Num());
    }
    
    /** Check if plan is complete (all steps terminal) */
    bool IsComplete() const
    {
        for (const auto& Pair : Steps)
        {
            if (!Pair.Value.IsTerminal())
            {
                return false;
            }
        }
        return true;
    }
    
    /** Serialize entire plan to JSON */
    TSharedPtr<FJsonObject> ToJson() const;

    /** Deserialize from JSON */
    static FAgentPlan FromJson(const TSharedPtr<FJsonObject>& Json);

    /** Save plan to disk */
    bool SaveToDisk(const FString& FilePath) const;

    /** Load plan from disk */
    static FAgentPlan LoadFromDisk(const FString& FilePath);

    /** Check if plan succeeded (all steps succeeded and goal satisfied) */
    bool IsSuccessful(const FWorldStateDigest& WorldState) const
    {
        if (!IsComplete()) return false;
        
        // Check all steps succeeded
        for (const auto& Pair : Steps)
        {
            if (Pair.Value.State != EPlanStepState::Succeeded &&
                Pair.Value.State != EPlanStepState::Skipped)
            {
                return false;
            }
        }
        
        // Check goal is satisfied
        return Goal.IsSatisfied(WorldState);
    }
};

/**
 * FPlanExecutor - Executes plans step by step
 * 
 * The executor:
 * 1. Takes a plan
 * 2. Executes steps in order (respecting dependencies)
 * 3. Checks expectations after each step
 * 4. Handles failures, blocks, and replanning
 * 5. Reports progress and completion
 */
class RIFTBORNAI_API FPlanExecutor
{
public:
    FPlanExecutor();

    /** Set how many steps to execute per tick */
    static void SetExecutionBatchSize(int32 BatchSize);

    /** Get current batch size */
    static int32 GetExecutionBatchSize();
    
    /** Start executing a plan */
    void StartPlan(const FAgentPlan& Plan);
    
    /** Execute the next step (call repeatedly until complete) */
    bool ExecuteNextStep();
    
    /** Get current plan state */
    EPlanState GetPlanState() const { return CurrentPlan.State; }
    
    /** Get current plan progress (0.0 - 1.0) */
    float GetProgress() const { return CurrentPlan.GetProgress(); }
    
    /** Get the current plan (for inspection) */
    const FAgentPlan& GetCurrentPlan() const { return CurrentPlan; }
    
    /** Cancel current plan */
    void CancelPlan();
    
    /** Check if currently executing a plan */
    bool IsExecuting() const { return CurrentPlan.State == EPlanState::InProgress; }

    /** Persist current plan state to disk (call after each step) */
    bool PersistPlanState() const;

    /** Recover a plan from disk (call on startup) */
    bool RecoverPlanFromDisk();

    /** Clear persisted plan (call on completion/cancel) */
    void ClearPersistedPlan() const;

    /** Get the persistence file path for active plans */
    static FString GetActivePlanFilePath();
    
    /** Delegate for step completion */
    DECLARE_DELEGATE_TwoParams(FOnStepComplete, const FPlanStep& /*Step*/, bool /*bSuccess*/);
    FOnStepComplete OnStepComplete;
    
    /** Delegate for plan completion */
    DECLARE_DELEGATE_TwoParams(FOnPlanComplete, const FAgentPlan& /*Plan*/, bool /*bSuccess*/);
    FOnPlanComplete OnPlanComplete;
    
    /** Delegate for replan request */
    DECLARE_DELEGATE_TwoParams(FOnReplanRequested, const FAgentPlan& /*FailedPlan*/, const FPlanStep& /*FailedStep*/);
    FOnReplanRequested OnReplanRequested;
    
private:
    /** Execute a single step */
    bool ExecuteStep(FPlanStep& Step);
    
    /** Check step outcome against expectations */
    bool CheckStepExpectations(FPlanStep& Step);
    
    /** Handle step failure */
    void HandleStepFailure(FPlanStep& Step);
    
    /** Handle step block (tool blocked) */
    void HandleStepBlocked(FPlanStep& Step);
    
    /** Try to find and execute alternative for blocked step */
    bool TryAlternative(FPlanStep& BlockedStep);
    
    /** Request replan from LLM */
    void RequestReplan(FPlanStep& FailedStep);
    
    mutable FCriticalSection PlanLock;
    FAgentPlan CurrentPlan;

    static int32 ExecutionBatchSize;
};
