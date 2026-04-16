// AnimationAuthoring.h - Animation State Machine Generation from Natural Language
// Creates animation blueprints, state machines, and blend spaces from descriptions

#pragma once

#include "CoreMinimal.h"
#include "AnimationAuthoring.generated.h"

// ============================================================================
// ANIMATION TYPES
// ============================================================================

UENUM(BlueprintType)
enum class EAAuthAnimStateType : uint8
{
    Idle,
    Locomotion,
    Jump,
    Fall,
    Land,
    Attack,
    Hit,
    Death,
    Dodge,
    Block,
    Interact,
    Custom
};

UENUM(BlueprintType)
enum class ETransitionCondition : uint8
{
    Speed,              // Movement speed threshold
    IsInAir,            // Airborne state
    IsFalling,          // Falling
    IsJumping,          // Jump initiated
    IsAttacking,        // In attack
    IsDead,             // Death state
    IsBlocking,         // Blocking
    InputPressed,       // Input received
    AnimationComplete,  // Current anim finished
    TimeElapsed,        // Time in state
    Custom              // Custom condition
};

UENUM(BlueprintType)
enum class EAAuthBlendSpaceType : uint8
{
    BlendSpace1D,       // Single axis (e.g., speed)
    BlendSpace2D,       // Two axes (e.g., speed + direction)
    AimOffset           // Aim offset blend
};

// ============================================================================
// ANIMATION STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAnimationSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SlotName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AnimationPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bLooping = false;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAnimTransitionRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ETransitionCondition Condition = ETransitionCondition::AnimationComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString VariableName;  // For custom conditions

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Threshold = 0.0f;  // For speed, time thresholds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bGreaterThan = true;  // > or < threshold

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float BlendTime = 0.2f;  // Transition blend duration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bBidirectional = false;  // Can transition both ways
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAnimStateSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString StateName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAAuthAnimStateType StateType = EAAuthAnimStateType::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FAnimationSlot> Animations;  // Animations in this state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsDefault = false;  // Entry state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString BlendSpacePath;  // Optional blend space

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, FAnimTransitionRule> Transitions;  // State -> Rule
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlendSpacePoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AnimationPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float XValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float YValue = 0.0f;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAAuthBlendSpaceSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAAuthBlendSpaceType BlendType = EAAuthBlendSpaceType::BlendSpace1D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString XAxisName = TEXT("Speed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector2D XAxisRange = FVector2D(0.0f, 600.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString YAxisName = TEXT("Direction");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector2D YAxisRange = FVector2D(-180.0f, 180.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FBlendSpacePoint> SamplePoints;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAnimBlueprintSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SkeletonPath;  // Target skeleton

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FAnimStateSpec> States;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FAAuthBlendSpaceSpec> BlendSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> RequiredVariables;  // Speed, Direction, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bUseRootMotion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString DefaultStateName;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAAuthAnimMontageSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SkeletonPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> AnimationPaths;  // Sections

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> SectionNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, float> NotifyTimes;  // Notify name -> time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float BlendIn = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float BlendOut = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SlotName = TEXT("DefaultSlot");
};

// ============================================================================
// ANIMATION AUTHORING SYSTEM
// ============================================================================

class RIFTBORNAI_API FAnimationAuthoring
{
public:
    static FAnimationAuthoring& Get()
    {
        static FAnimationAuthoring Instance;
        return Instance;
    }

    // ========================================================================
    // STATE MACHINE GENERATION
    // ========================================================================

    // Create state machine from description:
    // "Player character with idle, walk, run, jump, fall, land, and attack states"
    FAnimBlueprintSpec GenerateStateMachine(const FString& Description, const FString& SkeletonPath);

    // Create state from description: "attack state with 3 combo animations"
    FAnimStateSpec GenerateState(const FString& Description);

    // Generate transitions between states
    TArray<FAnimTransitionRule> GenerateTransitions(const FString& FromState, const FString& ToState);

    // ========================================================================
    // BLEND SPACE GENERATION
    // ========================================================================

    // Create locomotion blend space: "2D blend space for walk/run with direction"
    FAAuthBlendSpaceSpec GenerateBlendSpace(const FString& Description);

    // Create aim offset: "aim offset for looking up/down and left/right"
    FAAuthBlendSpaceSpec GenerateAimOffset(const FString& Description);

    // ========================================================================
    // MONTAGE GENERATION
    // ========================================================================

    // Create montage from description: "attack combo with 3 sections and hit notifies"
    FAAuthAnimMontageSpec GenerateMontage(const FString& Description, const FString& SkeletonPath);

    // ========================================================================
    // PRESET GENERATORS
    // ========================================================================

    // Generate standard humanoid locomotion setup
    FAnimBlueprintSpec GenerateHumanoidLocomotion(const FString& SkeletonPath);

    // Generate combat state machine
    FAnimBlueprintSpec GenerateCombatStateMachine(const FString& SkeletonPath, int32 ComboCount = 3);

    // Generate creature locomotion (4-legged)
    FAnimBlueprintSpec GenerateQuadrupedLocomotion(const FString& SkeletonPath);

    // ========================================================================
    // ASSET CREATION
    // ========================================================================

    // Create actual Animation Blueprint from spec
    bool CreateAnimBlueprint(const FAnimBlueprintSpec& Spec, const FString& DestPath);

    // Create Blend Space asset
    bool CreateBlendSpace(const FAAuthBlendSpaceSpec& Spec, const FString& DestPath);

    // Create Animation Montage
    bool CreateMontage(const FAAuthAnimMontageSpec& Spec, const FString& DestPath);

    // ========================================================================
    // UTILITY
    // ========================================================================

    // Parse animation type from text
    EAAuthAnimStateType ParseStateType(const FString& Text);

    // Get required variables for state type
    TArray<FString> GetRequiredVariables(EAAuthAnimStateType StateType);

    // Validate skeleton has required bones
    bool ValidateSkeleton(const FString& SkeletonPath, const TArray<FString>& RequiredBones);

private:
    FAnimationAuthoring() = default;

    // Internal helpers
    void AddLocomotionStates(FAnimBlueprintSpec& Spec);
    void AddCombatStates(FAnimBlueprintSpec& Spec, int32 ComboCount);
    void AddTransitionRules(FAnimStateSpec& FromState, const FString& ToStateName, ETransitionCondition Condition, float Threshold = 0.0f);
};
