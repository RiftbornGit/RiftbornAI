// Copyright RiftbornAI. All Rights Reserved.
// RiftbornAgentCore - The Central Intelligence
//
// This is NOT a chat UI wrapper. This is the BRAIN that:
// 1. Maintains a structured project graph (not chat history)
// 2. Plans multi-step actions (not single tool calls)
// 3. Verifies every change against invariants
// 4. Learns from success/failure patterns (skill acquisition)
// 5. Performs root cause analysis on errors

#pragma once

#include "CoreMinimal.h"
#include "AgentPlan.h"
#include "RiftbornVerificationPipeline.h"
#include "ClaudeToolUse.h"
#include "Containers/Ticker.h"

// Local intent enum used by the agent core.
enum class EUserIntent : uint8
{
	Unknown, Create, Modify, Delete, Query, Debug, Configure, Play, Build
};

// Local planning types used by the agent core.
struct FActionStep
{
	enum class EStatus : uint8 { Pending, Running, Success, Failed, Skipped };

	int32 Step = 0;
	FString ToolName;
	FString Description;
	TMap<FString, FString> Arguments;
	FString Rationale;
	TArray<FString> Dependencies;
	EStatus Status = EStatus::Pending;
	FString ErrorMessage;
	FString Result;
};

struct FActionPlan
{
	TArray<FActionStep> Actions;
	FString Goal;
	FString Reasoning;
	float Confidence = 0.0f;
	bool bComplete = false;
	bool bFailed = false;
	FString FailureReason;

	FActionStep* GetNextAction()
	{
		for (FActionStep& A : Actions)
		{
			if (A.Status == FActionStep::EStatus::Pending) return &A;
		}
		return nullptr;
	}

	bool IsComplete() const
	{
		for (const FActionStep& A : Actions)
		{
			if (A.Status == FActionStep::EStatus::Pending || A.Status == FActionStep::EStatus::Running)
				return false;
		}
		return true;
	}

};

// Local planner shim for compatibility with existing call sites.
class FAutonomousPlanner
{
public:
	static FAutonomousPlanner& Get() { static FAutonomousPlanner Instance; return Instance; }
	FActionPlan CreatePlan(const FString& Request, const TArray<FString>& AvailableTools = {}) { return FActionPlan(); }
	bool AttemptRecovery(FActionPlan& Plan) { return false; }
	bool ValidatePlan(const FActionPlan& Plan) { return Plan.Actions.Num() > 0; }
};

using FPlannedAction = FActionStep;

// Forward declarations
class FProjectAssetIndex;
class FDeepCodeIndexer;

/**
 * Simple project graph types for caching (used in serialization)
 */
struct RIFTBORNAI_API FProjectGraphAsset
{
    FString Path;
    FString Name;
    FString Class;
};

struct RIFTBORNAI_API FProjectGraphCode
{
    FString Path;
    FString ModuleName;
};

/**
 * Project understanding - not just chat context
 */
struct RIFTBORNAI_API FProjectGraph
{
    // Asset Graph
    struct FAssetNode
    {
        FString Path;
        FString Type;                   // Blueprint, Widget, GameMode, etc.
        FString ParentClass;
        TArray<FString> Components;
        TArray<FString> Dependencies;   // What this asset references
        TArray<FString> Dependents;     // What references this asset
        TMap<FString, FString> Properties; // Key properties we care about
        FDateTime LastModified;
        bool bCompiles = true;
    };
    TMap<FString, FAssetNode> Assets;
    
    // Code Graph  
    struct FCodeNode
    {
        FString FilePath;
        FString ClassName;
        FString ParentClass;
        TArray<FString> Functions;
        TArray<FString> Properties;
        TArray<FString> Includes;       // Dependencies
        TArray<FString> IncludedBy;     // Dependents
        bool bCompiles = true;
        FString LastCompileError;
    };
    TMap<FString, FCodeNode> Code;
    
    // Level Graph
    struct FLevelNode
    {
        FString LevelPath;
        TArray<FString> ActorClasses;
        TArray<FString> ActorInstances;
        FString GameMode;
        FString PlayerController;
        bool bLoads = true;
    };
    TMap<FString, FLevelNode> Levels;
    
    // System Configuration
    struct FSystemConfig
    {
        FString DefaultGameMode;
        FString DefaultPawn;
        FString DefaultPlayerController;
        TArray<FString> InputContexts;
        TArray<FString> RequiredPlugins;
    };
    FSystemConfig Config;
    
    // Health Status
    int32 TotalAssets = 0;
    int32 CompilingAssets = 0;
    int32 BrokenAssets = 0;
    TArray<FString> CriticalIssues;
};

/**
 * Intent Pattern - Abstract representation that generalizes across similar requests
 * "spawn a red cube" → Verb=SPAWN, ObjectType=PRIMITIVE, Shape=CUBE, Color=RED
 */
struct RIFTBORNAI_API FSkillIntentPattern
{
    // Core intent classification
    FString Verb;                       // SPAWN, CREATE, DELETE, MODIFY, QUERY
    FString ObjectType;                 // PRIMITIVE, BLUEPRINT, LEVEL, MATERIAL, ACTOR
    
    // Extracted parameters (placeholders)
    TMap<FString, FString> Parameters;  // shape→cube, color→red, name→MyCube
    
    // Pattern matching
    float Match(const FSkillIntentPattern& Other) const
    {
        float Score = 0.0f;
        if (Verb == Other.Verb) Score += 0.4f;
        if (ObjectType == Other.ObjectType) Score += 0.3f;
        
        // Check parameter overlap
        int32 CommonParams = 0;
        for (const auto& Pair : Parameters)
        {
            if (Other.Parameters.Contains(Pair.Key))
            {
                CommonParams++;
            }
        }
        if (Parameters.Num() > 0)
        {
            Score += 0.3f * (float)CommonParams / (float)Parameters.Num();
        }
        return Score;
    }
};

/**
 * Parameter Vocabulary - Maps natural language to concrete engine values
 */
struct RIFTBORNAI_API FParameterVocabulary
{
    // Shape vocabulary
    static FString ResolveShape(const FString& NaturalWord)
    {
        static TMap<FString, FString> ShapeMap = {
            {TEXT("cube"), TEXT("/Engine/BasicShapes/Cube")},
            {TEXT("box"), TEXT("/Engine/BasicShapes/Cube")},
            {TEXT("sphere"), TEXT("/Engine/BasicShapes/Sphere")},
            {TEXT("ball"), TEXT("/Engine/BasicShapes/Sphere")},
            {TEXT("cylinder"), TEXT("/Engine/BasicShapes/Cylinder")},
            {TEXT("cone"), TEXT("/Engine/BasicShapes/Cone")},
            {TEXT("plane"), TEXT("/Engine/BasicShapes/Plane")},
            {TEXT("floor"), TEXT("/Engine/BasicShapes/Plane")},
        };
        FString Lower = NaturalWord.ToLower();
        if (const FString* Found = ShapeMap.Find(Lower))
        {
            return *Found;
        }
        return FString();
    }
    
    // Actor class vocabulary
    static FString ResolveActorClass(const FString& NaturalWord)
    {
        static TMap<FString, FString> ClassMap = {
            {TEXT("cube"), TEXT("StaticMeshActor")},
            {TEXT("box"), TEXT("StaticMeshActor")},
            {TEXT("sphere"), TEXT("StaticMeshActor")},
            {TEXT("ball"), TEXT("StaticMeshActor")},
            {TEXT("light"), TEXT("PointLight")},
            {TEXT("point light"), TEXT("PointLight")},
            {TEXT("spotlight"), TEXT("SpotLight")},
            {TEXT("spot light"), TEXT("SpotLight")},
            {TEXT("directional light"), TEXT("DirectionalLight")},
            {TEXT("sun"), TEXT("DirectionalLight")},
            {TEXT("camera"), TEXT("CameraActor")},
            {TEXT("player start"), TEXT("PlayerStart")},
            {TEXT("spawn point"), TEXT("PlayerStart")},
            {TEXT("trigger"), TEXT("TriggerBox")},
            {TEXT("trigger box"), TEXT("TriggerBox")},
            {TEXT("trigger sphere"), TEXT("TriggerSphere")},
        };
        FString Lower = NaturalWord.ToLower();
        if (const FString* Found = ClassMap.Find(Lower))
        {
            return *Found;
        }
        return FString();
    }
    
    // Color vocabulary
    static FLinearColor ResolveColor(const FString& NaturalWord)
    {
        static TMap<FString, FLinearColor> ColorMap = {
            {TEXT("red"), FLinearColor::Red},
            {TEXT("blue"), FLinearColor::Blue},
            {TEXT("green"), FLinearColor::Green},
            {TEXT("yellow"), FLinearColor::Yellow},
            {TEXT("white"), FLinearColor::White},
            {TEXT("black"), FLinearColor::Black},
            {TEXT("orange"), FLinearColor(1.0f, 0.5f, 0.0f)},
            {TEXT("purple"), FLinearColor(0.5f, 0.0f, 0.5f)},
            {TEXT("pink"), FLinearColor(1.0f, 0.4f, 0.7f)},
            {TEXT("cyan"), FLinearColor(0.0f, 1.0f, 1.0f)},
        };
        FString Lower = NaturalWord.ToLower();
        if (const FLinearColor* Found = ColorMap.Find(Lower))
        {
            return *Found;
        }
        return FLinearColor::White;
    }
};

/**
 * Skill - A learned pattern of successful actions
 * NOW WITH REAL GENERALIZATION
 */
struct RIFTBORNAI_API FLearnedSkill
{
    FString Name;                       // "Create ability with cooldown"
    FString Intent;                     // Original request (for exact match fallback)
    
    // === NEW: Pattern-based matching ===
    FSkillIntentPattern Pattern;             // Abstract pattern that generalizes
    TArray<FString> RequiredArgs;       // Args that MUST be provided (e.g., "class_name" for spawn)
    TMap<FString, FString> ArgTemplates;// Templates for args: "mesh" → "{shape}" placeholder
    
    // === NEW: Applicability predicates ===
    TArray<FString> ApplicabilityRules; // "ObjectType==PRIMITIVE", "HasParameter:shape"
    bool bRequiresAssetResolution = false;  // Needs resolve_asset step first
    bool bRequiresBlueprintOpen = false;    // Needs open_blueprint step first
    
    TArray<FPlannedAction> Actions;     // The sequence that worked
    float SuccessRate = 1.0f;           // How often this works
    int32 TimesUsed = 0;
    int32 FailureCount = 0;             // NEW: Track failures for refinement
    TArray<FString> FailureReasons;     // NEW: Why did it fail?
    TArray<FString> Prerequisites;      // What must be true before this works
    TArray<FString> Postconditions;     // What should be true after
    FDateTime LastUsed;
    
    // Check if this skill applies to a given intent pattern
    bool IsApplicable(const FSkillIntentPattern& QueryPattern) const
    {
        // Must match verb and object type
        if (Pattern.Verb != QueryPattern.Verb) return false;
        if (Pattern.ObjectType != QueryPattern.ObjectType) return false;
        
        // Check applicability rules
        for (const FString& Rule : ApplicabilityRules)
        {
            if (Rule.StartsWith(TEXT("HasParameter:")))
            {
                FString RequiredParam = Rule.Mid(13);
                if (!QueryPattern.Parameters.Contains(RequiredParam))
                {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // Bind template arguments from query pattern
    TMap<FString, FString> BindArguments(const FSkillIntentPattern& QueryPattern) const
    {
        TMap<FString, FString> BoundArgs;
        
        for (const auto& Template : ArgTemplates)
        {
            FString Value = Template.Value;
            
            // Replace placeholders like {shape} with actual values
            for (const auto& Param : QueryPattern.Parameters)
            {
                FString Placeholder = FString::Printf(TEXT("{%s}"), *Param.Key);
                Value = Value.Replace(*Placeholder, *Param.Value);
            }
            
            // Resolve vocabulary if needed
            if (Template.Key == TEXT("mesh"))
            {
                FString Resolved = FParameterVocabulary::ResolveShape(Value);
                if (!Resolved.IsEmpty())
                {
                    Value = Resolved;
                }
            }
            else if (Template.Key == TEXT("class_name"))
            {
                FString Resolved = FParameterVocabulary::ResolveActorClass(Value);
                if (!Resolved.IsEmpty())
                {
                    Value = Resolved;
                }
            }
            
            BoundArgs.Add(Template.Key, Value);
        }
        
        return BoundArgs;
    }
};

/**
 * Reflection result from verifying an action
 */
struct RIFTBORNAI_API FReflectionResult
{
    bool bSuccess = false;
    TArray<FString> InvariantsChecked;
    TArray<FString> InvariantsPassed;
    TArray<FString> InvariantsFailed;
    FString RootCause;                  // If failed, what's the actual problem
    TArray<FString> SuggestedFixes;
    bool bCanAutoFix = false;
};

/**
 * The Agent's current state machine
 */
UENUM()
enum class EAgentState : uint8
{
    Idle,
    Understanding,      // Analyzing request, building context
    Planning,           // Creating action plan
    Executing,          // Running actions
    Verifying,          // Checking results
    Reflecting,         // Analyzing what went wrong
    Learning,           // Updating skills/patterns
    AwaitingConfirmation, // Waiting for user to approve destructive action
    Failed,
    Complete
};

/**
 * A single agent task with full lifecycle
 */
struct RIFTBORNAI_API FAgentTask
{
    FGuid TaskId;
    FString OriginalRequest;
    FString UnderstandingContext;   // What we learned about the request
    FActionPlan Plan;
    EAgentState State = EAgentState::Idle;
    
    // Execution tracking
    int32 CurrentStep = 0;
    int32 TotalSteps = 0;
    TArray<FString> ExecutionLog;
    TArray<UObject*> ModifiedAssets;
    TArray<FClaudeToolResult> ToolResults;  // Results from tool execution (for claims verification)
    
    // Level 3 plan representation (agentic plan executor)
    FAgentPlan AgentPlan;
    bool bUseAgentPlan = false;

    // Task metadata
    FString TaskFamily;
    FString TaskSource = TEXT("user");
    bool bProactive = false;
    FGuid ProactiveGoalId;
    FAgentGoal GoalDefinition;
    bool bHasGoalDefinition = false;

    // Experience logging
    bool bTrajectoryStarted = false;
    FString SelectedStrategyId;
    FString SelectedStrategyFamily;
    FString UsedSkillName;  // Name of learned skill used (empty if LLM-planned)
    
    // Timing for learning metrics
    FDateTime StartTime;
    float ExecutionTimeSeconds = 0.0f;
    
    // Token usage tracking (for accurate metrics)
    int32 TokensUsed = 0;  // Total tokens consumed during this task
    int32 LLMCallCount = 0;  // Number of LLM API calls made
    
    // Error tracking for invariant checking
    int32 ErrorCountBefore = -1;
    int32 ErrorCountAfter = -1;
    
    // Verification
    FRiftbornPipelineResult VerificationResult;
    FReflectionResult ReflectionResult;
    
    // Recovery
    int32 RetryCount = 0;
    int32 MaxRetries = 3;
    TArray<FString> AttemptedApproaches;
    
    // Result
    bool bSuccess = false;
    FString Result;
    FString FailureReason;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAgentStateChanged, const FAgentTask&, EAgentState);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAgentProgress, const FAgentTask&, const FString& /* Message */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAgentTaskComplete, const FAgentTask&, bool /* bSuccess */);

/**
 * RiftbornAgentCore - The Central Intelligence
 * 
 * This is NOT:
 * - A chat history manager
 * - A UI widget
 * - A simple LLM wrapper
 * 
 * This IS:
 * - The decision-making engine
 * - The planner that breaks down complex tasks
 * - The verifier that ensures changes work
 * - The learner that improves from experience
 * - The debugger that finds root causes
 */
class RIFTBORNAI_API FRiftbornAgentCore
{
public:
    static FRiftbornAgentCore& Get();
    
    // =========================================================================
    // INITIALIZATION
    // =========================================================================
    
    /** Initialize the agent with project understanding */
    void Initialize(const FString& ProjectPath);
    
    /** Rebuild project graph (call after major changes) */
    void RefreshProjectGraph();
    
    /** Get current project understanding */
    const FProjectGraph& GetProjectGraph() const { return ProjectGraph; }
    
    // =========================================================================
    // TASK MANAGEMENT
    // =========================================================================
    
    /** Start a new agent task (returns task ID) */
    FGuid StartTask(const FString& Request, const FString& Source = TEXT("user"), const FAgentGoal* GoalDefinition = nullptr);
    
    /** Get current task state */
    const FAgentTask* GetTask(const FGuid& TaskId) const;
    
    /** Cancel a running task */
    void CancelTask(const FGuid& TaskId);

    /** Update the goal/request for an active task and restart planning from the new request */
    bool UpdateTaskGoal(const FGuid& TaskId, const FString& NewGoal, bool bAppend, FString* OutErrorMessage = nullptr);

    /** Iterate over all active tasks (thread-safe snapshot) */
    void ForEachTask(TFunction<void(const FAgentTask&)> Callback) const
    {
        FScopeLock Lock(&const_cast<FCriticalSection&>(TaskLock));
        for (const auto& Pair : ActiveTasks)
        {
            Callback(Pair.Value);
        }
    }
    
    /** User confirms a pending action */
    void ConfirmAction(const FGuid& TaskId, bool bApproved);

    // =========================================================================
    // ASYNC TASK EXECUTION (Non-blocking)
    // =========================================================================

    /**
     * Start a task asynchronously on a background thread
     * This does NOT block the game thread - UI remains responsive
     * @param Request - The user's request
     * @param OnComplete - Called on game thread when task finishes
     * @param OnProgress - Called on game thread for progress updates
     * @return Task ID for tracking/cancellation
     */
    FGuid StartTaskAsync(
        const FString& Request,
        TFunction<void(bool bSuccess, const FString& Result)> OnComplete = nullptr,
        TFunction<void(EAgentState State, const FString& Message)> OnProgress = nullptr
    );

    /** Check if an async task is still running */
    bool IsTaskRunning(const FGuid& TaskId) const;

    /** Cancel an async task (thread-safe) */
    void CancelTaskAsync(const FGuid& TaskId);

    // =========================================================================
    // AGENT LOOP (Call this every tick for active tasks)
    // =========================================================================

    /** Main agent tick - advances active tasks */
    void Tick(float DeltaTime);

    /** Returns true if the agent has any active tasks or async tasks in flight.
     *  Used by the Slate panel to decide whether to call Tick(). */
    bool IsRunning() const
    {
        FScopeLock Lock(&const_cast<FCriticalSection&>(TaskLock));
        return ActiveTasks.Num() > 0 || RunningAsyncTasks.Num() > 0;
    }

    /** Set maximum plan steps (0 = unlimited) */
    void SetPlanStepLimit(int32 MaxSteps);

    /** Get current plan step limit */
    int32 GetPlanStepLimit() const { return PlanStepLimit; }

    /** Get last plan step count */
    int32 GetLastPlanStepCount() const { return LastPlanStepCount; }
    
    // =========================================================================
    // SKILL SYSTEM
    // =========================================================================
    
    /** Find relevant skills for a request */
    TArray<FLearnedSkill> FindRelevantSkills(const FString& Request, int32 MaxResults = 5);
    
    /** Find skills by pattern matching (REAL GENERALIZATION) */
    TArray<FLearnedSkill> FindSkillsByPattern(const FSkillIntentPattern& Pattern, int32 MaxResults = 5);
    
    /** Extract intent pattern from natural language request */
    static FSkillIntentPattern ExtractIntentPattern(const FString& Request);
    
    /** Get all learned skills */
    const TArray<FLearnedSkill>& GetAllSkills() const { return LearnedSkills; }
    
    /** Manually teach a skill */
    void TeachSkill(const FLearnedSkill& Skill);
    
    /** Record skill failure for refinement */
    void RecordSkillFailure(const FString& SkillName, const FString& Reason);
    
    // =========================================================================
    // DELEGATES
    // =========================================================================
    
    FOnAgentStateChanged OnStateChanged;
    FOnAgentProgress OnProgress;
    FOnAgentTaskComplete OnTaskComplete;
    
private:
    FRiftbornAgentCore() = default;
    
    // =========================================================================
    // AGENT LOOP PHASES
    // =========================================================================
    
    /** Phase 1: Understand the request in project context */
    void ProcessUnderstanding(FAgentTask& Task);
    
    /** Phase 2: Create multi-step plan */
    void ProcessPlanning(FAgentTask& Task);
    
    /** Phase 3: Execute plan step by step */
    void ProcessExecution(FAgentTask& Task);
    
    /** Phase 4: Verify changes work */
    void ProcessVerification(FAgentTask& Task);
    
    /** Phase 5: If failed, analyze why */
    void ProcessReflection(FAgentTask& Task);
    
    /** Phase 6: Learn from success or failure */
    void ProcessLearning(FAgentTask& Task);
    
    // =========================================================================
    // INTERNAL HELPERS
    // =========================================================================
    
    /** Execute a single tool call */
    bool ExecuteToolCall(const FPlannedAction& Action, FString& OutResult, FString& OutError, FClaudeToolResult* OutToolResult = nullptr);
    
    /** Check invariants after action */
    FReflectionResult CheckInvariants(const FAgentTask& Task);
    
    /** Perform root cause analysis */
    FString AnalyzeRootCause(const FAgentTask& Task, const TArray<FString>& Errors);
    
    /** Try to automatically fix an issue */
    bool TryAutoFix(FAgentTask& Task, const FReflectionResult& Reflection);

    /** Record successful skill */
    void RecordSkill(const FAgentTask& Task);
    
    /** Register default invariants */
    void RegisterDefaultInvariants();

    /** Register the self-driven ticker while work is active */
    void EnsureTickerRegistered();

    /** Remove the self-driven ticker when no work remains */
    void ReleaseTickerIfIdle();
    
    /** Update skill success/failure stats */
    void UpdateSkillStats(const FString& SkillName, bool bSuccess);
    
    /** Build understanding context from project graph */
    FString BuildUnderstandingContext(const FString& Request);
    
    /** Get relevant project context for request */
    TArray<FString> GetRelevantContext(const FString& Request);
    
    /** Build action plan (handles compound requests) */
    FActionPlan BuildActionPlan(const FString& Request);

    /** Apply plan complexity limit (if configured) */
    void ApplyPlanComplexityLimit(FActionPlan& Plan);
    
    /** Build agent plan from action plan */
    FAgentPlan BuildAgentPlanFromActionPlan(const FActionPlan& ActionPlan, const FString& Request);
    
    /** Apply trust-based tool selection bias */
    void ApplyToolTrustBias(FActionPlan& Plan);
    
    /** Check if tool has required args */
    bool IsToolCompatibleWithArgs(const FString& ToolName, const TMap<FString, FString>& Args) const;
    
    /** Sync action plan statuses from agent plan */
    void SyncActionPlanStatusesFromAgentPlan(FAgentTask& Task, const FAgentPlan& Plan);
    
    // =========================================================================
    // STATE
    // =========================================================================
    
    FString ProjectPath;
    FProjectGraph ProjectGraph;
    
    // Active tasks
    TMap<FGuid, FAgentTask> ActiveTasks;

    // Plan executors per task
    TMap<FGuid, TUniquePtr<FPlanExecutor>> PlanExecutors;

    // Async execution state
    TSet<FGuid> CancelledTasks;            // Tasks marked for cancellation
    TSet<FGuid> RunningAsyncTasks;         // Tasks running on background threads
    mutable FCriticalSection TaskLock;     // Thread-safe access to task state

    // Async callbacks (stored per task)
    struct FAsyncCallbacks
    {
        TFunction<void(bool bSuccess, const FString& Result)> OnComplete;
        TFunction<void(EAgentState State, const FString& Message)> OnProgress;
    };
    TMap<FGuid, FAsyncCallbacks> AsyncCallbacks;
    
    // Learned patterns
    TArray<FLearnedSkill> LearnedSkills;
    
    // Invariants to always check
    TArray<TFunction<bool(const FAgentTask&)>> Invariants;

    // Plan tuning
    int32 PlanStepLimit = 0;
    int32 LastPlanStepCount = 0;

    // Proactive scheduling
    float ProactiveOverridePriority = 0.75f;
    
    // Persistence paths
    FString SkillsFilePath;
    FString GraphCachePath;
    
    void LoadSkills();
    void SaveSkills();
    void LoadGraphCache();
    void SaveGraphCache();
    
    /** Determine task family for metrics grouping */
    FString DetermineTaskFamily(const FString& Request) const;

    /** Determine strategy family for bandit */
    FString DetermineStrategyFamily(const FString& TaskFamily) const;

    /** Maybe schedule proactive goals when idle */
    void MaybeScheduleProactiveGoal();

    /** Check if any user tasks are active */
    bool HasActiveUserTask() const;

    /** Check if any proactive tasks are active */
    bool HasActiveProactiveTask() const;

    /** Compute reward for strategy bandit */
    float ComputeStrategyReward(const FAgentTask& Task, bool bSuccess) const;

    /** Begin trajectory logging for a task */
    void MaybeBeginTrajectory(FAgentTask& Task);

    /** End trajectory logging for a task */
    void EndTrajectory(FAgentTask& Task, bool bSuccess);

    /** Build executed tool history for trajectory logging */
    TArray<FString> BuildExecutedToolHistory(const FAgentTask& Task) const;

    /** Log trajectory step for action plan */
    void LogTrajectoryStepForAction(FAgentTask& Task, const FPlannedAction& Action, const FClaudeToolResult& Result, bool bSuccess);

    /** Log trajectory step for agent plan step */
    void LogTrajectoryStepForPlanStep(FAgentTask& Task, const FPlanStep& Step, bool bSuccess);

    /** Ticker delegate handle for self-driven tick */
    FTSTicker::FDelegateHandle TickerHandle;
};
