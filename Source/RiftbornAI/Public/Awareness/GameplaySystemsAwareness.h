// GameplaySystemsAwareness.h - GAS, AI, and gameplay state for Agents

#pragma once

#include "CoreMinimal.h"
#include "GameplaySystemsAwareness.generated.h"

UENUM(BlueprintType)
enum class EAbilityState : uint8
{
    Ready,
    OnCooldown,
    Active,
    Blocked,
    NotLearned
};

UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle,
    Patrol,
    Chase,
    Attack,
    Flee,
    Search,
    Dead,
    Custom
};

UENUM(BlueprintType, meta=(ScriptName="RiftbornGameStateEnum"))
enum class EGameState : uint8
{
    MainMenu,
    Loading,
    Playing,
    Paused,
    Cutscene,
    GameOver,
    Victory
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAbilityInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString AbilityName;
    UPROPERTY() FString AbilityClass;
    UPROPERTY() EAbilityState State = EAbilityState::Ready;
    UPROPERTY() float CooldownRemaining = 0.0f;
    UPROPERTY() float CooldownTotal = 0.0f;
    UPROPERTY() int32 Level = 1;
    UPROPERTY() float Cost = 0.0f;
    UPROPERTY() FString CostAttribute;
    UPROPERTY() TArray<FString> Tags;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FGameplayEffectInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString EffectName;
    UPROPERTY() FString EffectClass;
    UPROPERTY() float Duration = 0.0f;
    UPROPERTY() float TimeRemaining = 0.0f;
    UPROPERTY() int32 StackCount = 1;
    UPROPERTY() FString SourceActor;
    UPROPERTY() TArray<FString> ModifiedAttributes;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAttributeSetInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString SetName;
    UPROPERTY() TMap<FString, float> BaseValues;
    UPROPERTY() TMap<FString, float> CurrentValues;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAIControllerInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString ActorName;
    UPROPERTY() FString ControllerClass;
    UPROPERTY() EAIState CurrentState = EAIState::Idle;
    UPROPERTY() FString BehaviorTreeName;
    UPROPERTY() FString CurrentTaskName;
    UPROPERTY() FString BlackboardValues;
    UPROPERTY() FVector FocusLocation = FVector::ZeroVector;
    UPROPERTY() FString FocusActor;
    UPROPERTY() float Perception_SightRadius = 0.0f;
    UPROPERTY() float Perception_HearingRadius = 0.0f;
    UPROPERTY() TArray<FString> PerceivedActors;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FQuestInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString QuestName;
    UPROPERTY() FString QuestID;
    UPROPERTY() FString Description;
    UPROPERTY() FString CurrentObjective;
    UPROPERTY() int32 CurrentStep = 0;
    UPROPERTY() int32 TotalSteps = 1;
    UPROPERTY() bool bIsActive = false;
    UPROPERTY() bool bIsComplete = false;
    UPROPERTY() TArray<FString> CompletedObjectives;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FGameplayState
{
    GENERATED_BODY()
    
    UPROPERTY() EGameState State = EGameState::Playing;
    UPROPERTY() FString CurrentLevelName;
    UPROPERTY() float GameTime = 0.0f;
    UPROPERTY() int32 Score = 0;
    UPROPERTY() TArray<FQuestInfo> ActiveQuests;
    UPROPERTY() TArray<FAIControllerInfo> AIControllers;
};

class RIFTBORNAI_API FGameplaySystemsAwareness
{
public:
    static FGameplaySystemsAwareness& Get();
    
    // Game State
    FGameplayState GetGameState() const;
    EGameState GetCurrentGameState() const;
    bool IsPaused() const;
    float GetGameTime() const;
    
    // GAS - Abilities
    TArray<FAbilityInfo> GetAbilities(AActor* Actor) const;
    FAbilityInfo GetAbilityInfo(AActor* Actor, const FString& AbilityName) const;
    bool CanActivateAbility(AActor* Actor, const FString& AbilityName) const;
    TArray<FAbilityInfo> GetAbilitiesOnCooldown(AActor* Actor) const;
    TArray<FAbilityInfo> GetReadyAbilities(AActor* Actor) const;
    
    // GAS - Effects
    TArray<FGameplayEffectInfo> GetActiveEffects(AActor* Actor) const;
    bool HasEffect(AActor* Actor, const FString& EffectName) const;
    int32 GetEffectStackCount(AActor* Actor, const FString& EffectName) const;
    
    // GAS - Attributes
    TArray<FAttributeSetInfo> GetAttributeSets(AActor* Actor) const;
    float GetAttributeValue(AActor* Actor, const FString& AttributeName) const;
    float GetAttributeBase(AActor* Actor, const FString& AttributeName) const;
    
    // GAS - Tags
    TArray<FString> GetGameplayTags(AActor* Actor) const;
    bool HasGameplayTag(AActor* Actor, const FString& Tag) const;
    
    // AI
    TArray<FAIControllerInfo> GetAIControllers() const;
    FAIControllerInfo GetAIInfo(AActor* Actor) const;
    EAIState GetAIState(AActor* Actor) const;
    FString GetCurrentBehaviorTask(AActor* Actor) const;
    TArray<FString> GetPerceivedActors(AActor* Actor) const;
    
    // Quests
    TArray<FQuestInfo> GetActiveQuests() const;
    FQuestInfo GetQuestInfo(const FString& QuestID) const;
    bool IsQuestActive(const FString& QuestID) const;
    bool IsQuestComplete(const FString& QuestID) const;
    
    // Utility
    static FString AbilityStateToString(EAbilityState State);
    static FString AIStateToString(EAIState State);
    static FString GameStateToString(EGameState State);
    
private:
    FGameplaySystemsAwareness();
};
