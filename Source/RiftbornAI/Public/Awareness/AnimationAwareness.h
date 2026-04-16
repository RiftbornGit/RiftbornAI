// AnimationAwareness.h - Animation state understanding for AI Agents

#pragma once

#include "CoreMinimal.h"
#include "AnimationAwareness.generated.h"

UENUM(BlueprintType)
enum class EAnimationPlayState : uint8
{
    Stopped,
    Playing,
    Paused,
    BlendingIn,
    BlendingOut
};

UENUM(BlueprintType)
enum class EAnimationType : uint8
{
    Sequence,
    Montage,
    BlendSpace,
    StateMachine,
    PoseAsset
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAnimationSlotInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString SlotName;
    UPROPERTY() FString ActiveMontage;
    UPROPERTY() float Weight = 0.0f;
    UPROPERTY() bool bIsPlaying = false;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPlayingAnimationInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString AnimationName;
    UPROPERTY() FString AssetPath;
    UPROPERTY() EAnimationType Type = EAnimationType::Sequence;
    UPROPERTY() EAnimationPlayState PlayState = EAnimationPlayState::Stopped;
    UPROPERTY() float CurrentTime = 0.0f;
    UPROPERTY() float Duration = 0.0f;
    UPROPERTY() float PlayRate = 1.0f;
    UPROPERTY() float BlendWeight = 1.0f;
    UPROPERTY() bool bIsLooping = false;
    UPROPERTY() FString SlotName;
    UPROPERTY() FString CurrentSection;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FStateMachineInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString MachineName;
    UPROPERTY() FString CurrentState;
    UPROPERTY() float TimeInState = 0.0f;
    UPROPERTY() TArray<FString> AvailableTransitions;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FSkeletalMeshAnimState
{
    GENERATED_BODY()
    
    UPROPERTY() FString ActorName;
    UPROPERTY() FString MeshName;
    UPROPERTY() FString AnimBlueprintClass;
    UPROPERTY() TArray<FPlayingAnimationInfo> PlayingAnimations;
    UPROPERTY() TArray<FAnimationSlotInfo> Slots;
    UPROPERTY() TArray<FStateMachineInfo> StateMachines;
    UPROPERTY() bool bIsRagdoll = false;
    UPROPERTY() float RootMotionScale = 1.0f;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAnimNotifyInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString NotifyName;
    UPROPERTY() FString AnimationName;
    UPROPERTY() float TriggerTime = 0.0f;
    UPROPERTY() float Duration = 0.0f;
    UPROPERTY() bool bIsState = false;
};

class RIFTBORNAI_API FAnimationAwareness
{
public:
    static FAnimationAwareness& Get();
    
    // Actor animation state
    FSkeletalMeshAnimState GetAnimationState(AActor* Actor) const;
    TArray<FSkeletalMeshAnimState> GetAllAnimatingActors() const;
    
    // Playing animations
    TArray<FPlayingAnimationInfo> GetPlayingAnimations(AActor* Actor) const;
    FPlayingAnimationInfo GetActiveMontage(AActor* Actor) const;
    bool IsPlayingMontage(AActor* Actor, const FString& MontageName) const;
    FString GetCurrentMontageSection(AActor* Actor) const;
    
    // State machines
    TArray<FStateMachineInfo> GetStateMachines(AActor* Actor) const;
    FString GetCurrentAnimState(AActor* Actor, const FString& MachineName = TEXT("")) const;
    
    // Slots
    TArray<FAnimationSlotInfo> GetAnimSlots(AActor* Actor) const;
    bool IsSlotActive(AActor* Actor, const FString& SlotName) const;
    
    // Ragdoll
    bool IsRagdoll(AActor* Actor) const;
    
    // Notifies
    TArray<FAnimNotifyInfo> GetUpcomingNotifies(AActor* Actor, float LookAheadTime = 1.0f) const;
    
    // Utility
    static FString PlayStateToString(EAnimationPlayState State);
    static FString AnimTypeToString(EAnimationType Type);
    
private:
    FAnimationAwareness();
};
