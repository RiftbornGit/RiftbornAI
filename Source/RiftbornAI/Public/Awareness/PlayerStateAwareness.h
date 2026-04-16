// PlayerStateAwareness.h - Player state and input understanding for AI Agents
// Provides agents with awareness of player state, input bindings, camera, and controls

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "PlayerStateAwareness.generated.h"

// ============================================================================
// INPUT ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EInputDeviceType : uint8
{
    KeyboardMouse,
    Gamepad,
    Touch,
    Motion,
    Unknown
};

UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Sprinting,
    Crouching,
    Jumping,
    Falling,
    Swimming,
    Flying,
    Climbing,
    Custom
};

UENUM(BlueprintType)
enum class EPlayerCombatState : uint8
{
    Neutral,
    InCombat,
    Attacking,
    Blocking,
    Dodging,
    Aiming,
    Reloading,
    Dead
};

// ============================================================================
// INPUT BINDING STRUCTURES
// ============================================================================

/** Single input binding info */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FInputBindingInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ActionName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString PrimaryKey;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SecondaryKey;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString GamepadButton;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsAxis = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Category;  // Movement, Combat, UI, etc.
};

/** Input mapping context summary */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FInputContextInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ContextName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Priority = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsActive = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FInputBindingInfo> Bindings;
};

// ============================================================================
// PLAYER STATE STRUCTURES
// ============================================================================

/** Camera state info */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FCameraStateInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Position = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FRotator Rotation = FRotator::ZeroRotator;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float FOV = 90.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector ForwardVector = FVector::ForwardVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString CameraMode;  // First person, third person, top-down, etc.
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsFirstPerson = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float ArmLength = 0.0f;  // For third person
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString LookAtTarget;  // Actor being looked at, if any
};

/** Player attributes/stats */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPlayerAttributeInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AttributeName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float CurrentValue = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MaxValue = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Percentage = 0.0f;
};

/** Inventory item info */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FInventoryItemInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ItemName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ItemClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Quantity = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsEquipped = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 SlotIndex = -1;
};

/** Complete player state */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornPlayerState
{
    GENERATED_BODY()
    
    // Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString PlayerName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString PawnClassName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 PlayerIndex = 0;
    
    // Position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Position = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FRotator Rotation = FRotator::ZeroRotator;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Velocity = FVector::ZeroVector;
    
    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EPlayerMovementState MovementState = EPlayerMovementState::Idle;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MovementSpeed = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsGrounded = true;
    
    // Combat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EPlayerCombatState CombatState = EPlayerCombatState::Neutral;
    
    // Camera
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FCameraStateInfo Camera;
    
    // Input
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EInputDeviceType ActiveInputDevice = EInputDeviceType::KeyboardMouse;
    
    // Attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FPlayerAttributeInfo> Attributes;
    
    // Inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FInventoryItemInfo> Inventory;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString EquippedWeapon;
    
    FString GetDescription() const;
};

// ============================================================================
// PLAYER STATE AWARENESS SYSTEM
// ============================================================================

/**
 * Player State Awareness System
 * 
 * Provides agents with understanding of:
 * - Player position, rotation, velocity
 * - Movement and combat states
 * - Input bindings and active controls
 * - Camera position and mode
 * - Player attributes (health, stamina, etc.)
 * - Inventory and equipment
 * 
 * Usage:
 *   FPlayerStateAwareness& PSA = FPlayerStateAwareness::Get();
 *   FRiftbornPlayerState State = PSA.GetPlayerState();
 *   TArray<FInputBindingInfo> Bindings = PSA.GetInputBindings();
 */
class RIFTBORNAI_API FPlayerStateAwareness
{
public:
    static FPlayerStateAwareness& Get();
    
    // =========================================================================
    // PLAYER STATE
    // =========================================================================
    
    /** Get complete player state (player 0) */
    FRiftbornPlayerState GetPlayerState(int32 PlayerIndex = 0) const;
    
    /** Get all players' states */
    TArray<FRiftbornPlayerState> GetAllPlayerStates() const;
    
    /** Get player count */
    int32 GetPlayerCount() const;
    
    // =========================================================================
    // POSITION & MOVEMENT
    // =========================================================================
    
    /** Get player position */
    FVector GetPlayerPosition(int32 PlayerIndex = 0) const;
    
    /** Get player rotation */
    FRotator GetPlayerRotation(int32 PlayerIndex = 0) const;
    
    /** Get player velocity */
    FVector GetPlayerVelocity(int32 PlayerIndex = 0) const;
    
    /** Get movement state */
    EPlayerMovementState GetMovementState(int32 PlayerIndex = 0) const;
    
    /** Get movement speed */
    float GetMovementSpeed(int32 PlayerIndex = 0) const;
    
    /** Check if grounded */
    bool IsGrounded(int32 PlayerIndex = 0) const;
    
    // =========================================================================
    // COMBAT STATE
    // =========================================================================
    
    /** Get combat state */
    EPlayerCombatState GetCombatState(int32 PlayerIndex = 0) const;
    
    /** Check if in combat */
    bool IsInCombat(int32 PlayerIndex = 0) const;
    
    /** Get equipped weapon name */
    FString GetEquippedWeapon(int32 PlayerIndex = 0) const;
    
    // =========================================================================
    // CAMERA
    // =========================================================================
    
    /** Get camera state */
    FCameraStateInfo GetCameraState(int32 PlayerIndex = 0) const;
    
    /** Get camera position */
    FVector GetCameraPosition(int32 PlayerIndex = 0) const;
    
    /** Get camera forward vector */
    FVector GetCameraForward(int32 PlayerIndex = 0) const;
    
    /** Get what player is looking at */
    FString GetLookAtTarget(int32 PlayerIndex = 0) const;
    
    /** Check if camera is first person */
    bool IsFirstPerson(int32 PlayerIndex = 0) const;
    
    // =========================================================================
    // INPUT
    // =========================================================================
    
    /** Get active input device type */
    EInputDeviceType GetActiveInputDevice(int32 PlayerIndex = 0) const;
    
    /** Get all input bindings */
    TArray<FInputBindingInfo> GetInputBindings() const;
    
    /** Get input binding for action */
    FInputBindingInfo GetBindingForAction(const FString& ActionName) const;
    
    /** Get all input mapping contexts */
    TArray<FInputContextInfo> GetInputMappingContexts() const;
    
    /** Get active mapping contexts */
    TArray<FInputContextInfo> GetActiveMappingContexts(int32 PlayerIndex = 0) const;
    
    /** Check if action is bound */
    bool IsActionBound(const FString& ActionName) const;
    
    /** Get key for action */
    FString GetKeyForAction(const FString& ActionName) const;
    
    // =========================================================================
    // ATTRIBUTES
    // =========================================================================
    
    /** Get player attribute value */
    float GetAttribute(const FString& AttributeName, int32 PlayerIndex = 0) const;
    
    /** Get attribute as percentage */
    float GetAttributePercentage(const FString& AttributeName, int32 PlayerIndex = 0) const;
    
    /** Get all attributes */
    TArray<FPlayerAttributeInfo> GetAllAttributes(int32 PlayerIndex = 0) const;
    
    /** Common attribute shortcuts */
    float GetHealth(int32 PlayerIndex = 0) const;
    float GetHealthPercentage(int32 PlayerIndex = 0) const;
    float GetStamina(int32 PlayerIndex = 0) const;
    float GetMana(int32 PlayerIndex = 0) const;
    
    // =========================================================================
    // INVENTORY
    // =========================================================================
    
    /** Get inventory items */
    TArray<FInventoryItemInfo> GetInventory(int32 PlayerIndex = 0) const;
    
    /** Check if player has item */
    bool HasItem(const FString& ItemName, int32 PlayerIndex = 0) const;
    
    /** Get item count */
    int32 GetItemCount(const FString& ItemName, int32 PlayerIndex = 0) const;
    
    /** Get equipped items */
    TArray<FInventoryItemInfo> GetEquippedItems(int32 PlayerIndex = 0) const;
    
    // =========================================================================
    // UTILITY
    // =========================================================================
    
    /** Convert movement state to string */
    static FString MovementStateToString(EPlayerMovementState State);
    
    /** Convert combat state to string */
    static FString CombatStateToString(EPlayerCombatState State);
    
    /** Convert input device to string */
    static FString InputDeviceToString(EInputDeviceType Device);
    
private:
    FPlayerStateAwareness();
};
