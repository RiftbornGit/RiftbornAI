// Copyright RiftbornAI. All Rights Reserved.
// EmbodiedActor.h - Level 5: Embodied World Actor with Continuous Presence

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentSteward.h"
#include "AgentPlan.h"
#include "WorldStateDigest.h"

/**
 * Agent state for embodied presence
 */
enum class EAgentState : uint8
{
    Observing,      // Passively observing, waiting for opportunities
    Planning,       // Generating a plan for a goal
    Executing,      // Actively executing a plan
    Waiting,        // Waiting for action effects to manifest
    Recovering,     // Recovering from failure
    Idle            // No current objective, minimal sensing
};

/**
 * Resource type for budget tracking
 */
enum class EResourceType : uint8
{
    Time,           // Wall-clock time budget
    Tokens,         // LLM tokens (expensive)
    ToolCalls,      // Tool call budget
    Attention,      // Cognitive focus (finite resource)
    Energy          // Abstract energy for actions
};

/**
 * FSensorReading - Single observation from the environment
 */
struct RIFTBORNAI_API FSensorReading
{
    /** Sensor that produced this reading */
    FName SensorId;
    
    /** What was observed */
    FString Observation;
    
    /** Confidence in the observation (0-1) */
    float Confidence = 1.0f;
    
    /** When this was observed */
    FDateTime Timestamp;
    
    /** Staleness - how old is this observation? */
    float GetAge() const
    {
        return (FDateTime::UtcNow() - Timestamp).GetTotalSeconds();
    }
    
    /** Is this observation still fresh? */
    bool IsFresh(float MaxAgeSeconds = 5.0f) const
    {
        return GetAge() < MaxAgeSeconds;
    }
};

/**
 * FSensor - Virtual sensor that observes the environment
 * 
 * Sensors poll the environment at configurable rates and
 * produce readings that feed into the agent's world model.
 */
struct RIFTBORNAI_API FSensor
{
    /** Unique sensor ID */
    FName SensorId;
    
    /** Human-readable name */
    FString DisplayName;
    
    /** How often to poll (seconds) */
    float PollInterval = 1.0f;
    
    /** Last poll time */
    FDateTime LastPollTime;
    
    /** Cost per poll (attention/energy) */
    float PollCost = 0.1f;
    
    /** Is this sensor currently enabled? */
    bool bEnabled = true;
    
    /** Latest reading */
    FSensorReading LatestReading;
    
    /** Historical readings (ring buffer) */
    TArray<FSensorReading> History;
    static constexpr int32 MaxHistory = 50;
    
    FSensor() = default;
    
    FSensor(FName InId, const FString& InName, float InInterval = 1.0f)
        : SensorId(InId)
        , DisplayName(InName)
        , PollInterval(InInterval)
    {
        LastPollTime = FDateTime::MinValue();
    }
    
    /** Should we poll now? */
    bool ShouldPoll() const
    {
        if (!bEnabled) return false;
        return (FDateTime::UtcNow() - LastPollTime).GetTotalSeconds() >= PollInterval;
    }
    
    /** Record a new reading */
    void RecordReading(const FSensorReading& Reading)
    {
        LatestReading = Reading;
        LatestReading.Timestamp = FDateTime::UtcNow();
        
        History.Add(LatestReading);
        while (History.Num() > MaxHistory)
        {
            History.RemoveAt(0);
        }
        
        LastPollTime = FDateTime::UtcNow();
    }
    
    /** Get change between current and previous reading */
    float GetDelta() const
    {
        // Subclasses implement specific delta calculations
        return 0.0f;
    }
};

/**
 * FResourceBudget - Tracks and limits resource consumption
 */
struct RIFTBORNAI_API FResourceBudget
{
    EResourceType Type;
    
    /** Current available amount */
    float Available = 100.0f;
    
    /** Maximum capacity */
    float Maximum = 100.0f;
    
    /** Regeneration rate per second */
    float RegenRate = 1.0f;
    
    /** Last update time for regeneration */
    FDateTime LastUpdate;
    
    FResourceBudget() = default;
    
    FResourceBudget(EResourceType InType, float InMax, float InRegen)
        : Type(InType)
        , Available(InMax)
        , Maximum(InMax)
        , RegenRate(InRegen)
    {
        LastUpdate = FDateTime::UtcNow();
    }
    
    /** Update regeneration */
    void Tick()
    {
        FDateTime Now = FDateTime::UtcNow();
        float DeltaSeconds = (Now - LastUpdate).GetTotalSeconds();
        LastUpdate = Now;
        
        Available = FMath::Min(Maximum, Available + RegenRate * DeltaSeconds);
    }
    
    /** Can we afford this cost? */
    bool CanAfford(float Cost) const
    {
        return Available >= Cost;
    }
    
    /** Spend resource, returns false if insufficient */
    bool Spend(float Cost)
    {
        if (!CanAfford(Cost)) return false;
        Available -= Cost;
        return true;
    }
    
    /** Get fraction of resource remaining (0-1) */
    float GetFraction() const
    {
        return Maximum > 0.0f ? Available / Maximum : 0.0f;
    }
};

/**
 * FActionEffect - Describes delayed effect of an action
 */
struct RIFTBORNAI_API FActionEffect
{
    /** What action caused this */
    FString ActionName;
    
    /** Expected effect */
    FString ExpectedEffect;
    
    /** When the action was initiated */
    FDateTime InitiatedAt;
    
    /** Expected time for effect to manifest */
    float ExpectedDelaySeconds = 0.0f;
    
    /** Has the effect been observed? */
    bool bEffectObserved = false;
    
    /** When the effect was observed (if it was) */
    FDateTime ObservedAt;
    
    /** Is the effect still pending? */
    bool IsPending() const
    {
        return !bEffectObserved;
    }
    
    /** Time remaining until expected effect */
    float GetTimeRemaining() const
    {
        float Elapsed = (FDateTime::UtcNow() - InitiatedAt).GetTotalSeconds();
        return FMath::Max(0.0f, ExpectedDelaySeconds - Elapsed);
    }
    
    /** Is the effect overdue? */
    bool IsOverdue() const
    {
        return IsPending() && GetTimeRemaining() <= 0.0f;
    }
};

/**
 * FPolicy - Learned decision rule for when to act
 */
struct RIFTBORNAI_API FPolicy
{
    /** Policy name */
    FName PolicyId;
    
    /** Condition that triggers this policy (simplified) */
    FString TriggerCondition;
    
    /** Action to take when triggered */
    FString ActionName;
    
    /** Priority (higher = more important) */
    float Priority = 0.5f;
    
    /** Success rate of this policy */
    float SuccessRate = 0.5f;
    
    /** Number of times this policy was applied */
    int32 TimesApplied = 0;
    
    /** Number of successful outcomes */
    int32 SuccessfulOutcomes = 0;
    
    /** Should this policy be applied given current state? */
    bool ShouldApply(const FWorldStateDigest& State) const
    {
        // Simple keyword matching for now
        // In full implementation, this would be learned
        return false;  // Subclasses implement
    }
    
    /** Update success rate after outcome */
    void RecordOutcome(bool bSuccess)
    {
        TimesApplied++;
        if (bSuccess)
        {
            SuccessfulOutcomes++;
        }
        SuccessRate = static_cast<float>(SuccessfulOutcomes) / TimesApplied;
    }
};

/**
 * FEmbodiedActor - Level 5: Continuous Embodied Presence in the World
 * 
 * This is the final level of the agent capability ladder.
 * The agent:
 * 1. Exists continuously in the environment (not just responding to requests)
 * 2. Has sensors that observe the world proactively
 * 3. Manages resource budgets (time, tokens, attention)
 * 4. Understands that actions have delays (effects aren't instant)
 * 5. Learns policies for when to act, wait, or interrupt
 * 
 * This transforms the agent from a reactive tool into an embodied presence
 * that maintains situational awareness and acts with purpose.
 */
class RIFTBORNAI_API FEmbodiedActor
{
public:
    static FEmbodiedActor& Get()
    {
        static FEmbodiedActor Instance;
        return Instance;
    }
    
    /** Initialize the embodied actor */
    void Initialize();
    
    /** Main tick - called every frame or at fixed interval */
    void Tick(float DeltaSeconds);
    
    /** Get current agent state */
    EAgentState GetState() const { return CurrentState; }
    
    /** Set a new goal for the actor to pursue */
    void SetGoal(const FAgentGoal& Goal);
    
    /** Interrupt current activity (e.g., for urgent goal) */
    void Interrupt(const FAgentGoal& UrgentGoal);
    
    /** Get current world model (accumulated observations) */
    const FWorldStateDigest& GetWorldModel() const { return WorldModel; }
    
    // =========================================================================
    // Sensor Management
    // =========================================================================
    
    /** Register a new sensor */
    void RegisterSensor(const FSensor& Sensor);
    
    /** Enable/disable a sensor */
    void SetSensorEnabled(FName SensorId, bool bEnabled);
    
    /** Get latest reading from a sensor */
    FSensorReading GetSensorReading(FName SensorId) const;
    
    /** Poll all sensors that are due */
    void PollSensors();
    
    // =========================================================================
    // Resource Management
    // =========================================================================
    
    /** Get current resource level */
    float GetResource(EResourceType Type) const;
    
    /** Can we afford an action with given costs? */
    bool CanAffordAction(const TMap<EResourceType, float>& Costs) const;
    
    /** Spend resources for an action */
    bool SpendResources(const TMap<EResourceType, float>& Costs);
    
    /** Get resource efficiency (success per resource spent) */
    float GetResourceEfficiency() const;
    
    // =========================================================================
    // Action & Effect Management
    // =========================================================================
    
    /** Initiate an action with expected delay */
    void InitiateAction(const FString& ActionName, float ExpectedDelay, const FString& ExpectedEffect);
    
    /** Check if an action's effect has manifested */
    bool HasEffectManifested(const FString& ActionName) const;
    
    /** Get all pending action effects */
    TArray<FActionEffect> GetPendingEffects() const;
    
    /** Record that an effect was observed */
    void RecordEffectObserved(const FString& ActionName);
    
    // =========================================================================
    // Policy Learning
    // =========================================================================
    
    /** Register a policy */
    void RegisterPolicy(const FPolicy& Policy);
    
    /** Get best policy for current situation */
    FPolicy* GetBestPolicy();
    
    /** Record outcome of applying a policy */
    void RecordPolicyOutcome(FName PolicyId, bool bSuccess);
    
    /** Get most successful policies */
    TArray<FPolicy> GetTopPolicies(int32 Count = 5) const;
    
    // =========================================================================
    // Persistence
    // =========================================================================
    
    bool SaveState(const FString& FilePath);
    bool LoadState(const FString& FilePath);
    
    // =========================================================================
    // Delegation to Subsystems
    // =========================================================================
    
    /** Access to the environment steward (Level 4) */
    FEnvironmentSteward& GetSteward() { return FEnvironmentSteward::Get(); }
    
    /** Access to plan executor (Level 3) */
    FPlanExecutor& GetPlanExecutor() { return PlanExecutor; }
    
private:
    FEmbodiedActor() = default;
    
    // Current state
    EAgentState CurrentState = EAgentState::Idle;
    
    // Time tracking
    FDateTime StartTime;
    float TotalUptime = 0.0f;
    
    // World model (accumulated from sensors)
    FWorldStateDigest WorldModel;
    
    // Sensors
    TMap<FName, FSensor> Sensors;
    
    // Resource budgets
    TMap<EResourceType, FResourceBudget> Resources;
    
    // Pending action effects
    TArray<FActionEffect> PendingEffects;
    
    // Learned policies
    TMap<FName, FPolicy> Policies;
    
    // Current goal and plan
    FAgentGoal CurrentGoal;
    FAgentPlan CurrentPlan;
    FPlanExecutor PlanExecutor;
    
    // Statistics
    int32 TotalActions = 0;
    int32 SuccessfulActions = 0;
    float TotalResourcesSpent = 0.0f;
    
    // Internal methods
    void TransitionTo(EAgentState NewState);
    void ProcessObservations();
    void UpdateWorldModel();
    void CheckPendingEffects();
    void SelectNextAction();
    void InitializeDefaultSensors();
    void InitializeDefaultResources();
    void InitializeDefaultPolicies();
};

/**
 * Standard sensors for the embodied actor
 */
namespace ActorSensors
{
    // World state sensor (captures full digest)
    static const FName WorldState(TEXT("sensor_world_state"));
    
    // Actor change sensor (detects new/removed actors)
    static const FName ActorChanges(TEXT("sensor_actor_changes"));
    
    // Health sensor (monitors system health via steward)
    static const FName SystemHealth(TEXT("sensor_system_health"));
    
    // Resource sensor (monitors resource levels)
    static const FName Resources(TEXT("sensor_resources"));
    
    // Effect sensor (checks if pending effects manifested)
    static const FName Effects(TEXT("sensor_effects"));
    
    // Goal sensor (checks goal satisfaction)
    static const FName GoalProgress(TEXT("sensor_goal_progress"));
}

/**
 * Standard policies for embodied behavior
 */
namespace ActorPolicies
{
    // Wait when an action is pending
    static const FName WaitForEffect(TEXT("policy_wait_for_effect"));
    
    // Act when goal is not progressing
    static const FName ActOnStall(TEXT("policy_act_on_stall"));
    
    // Interrupt when critical alert
    static const FName InterruptOnCritical(TEXT("policy_interrupt_on_critical"));
    
    // Conserve when resources low
    static const FName ConserveResources(TEXT("policy_conserve_resources"));
    
    // Explore when idle
    static const FName ExploreWhenIdle(TEXT("policy_explore_when_idle"));
}
