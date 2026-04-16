// Copyright Riftborn Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "RiftbornPlayableCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;

/**
 * ARiftbornPlayableCharacter
 *
 * Playable third-person character base class used by the
 * create_character_from_third_person tool when the project does NOT have a
 * UE5 Third Person template Blueprint to duplicate (the common case for
 * projects that only include the Mannequin assets at /Game/Characters/
 * Mannequins/ without the full template).
 *
 * Ships with:
 *  - Capsule + mesh transform that match the stock UE5 mannequin defaults,
 *    so the feet sit flush with the capsule bottom and the mesh faces +X.
 *  - SpringArm "CameraBoom" + "FollowCamera" third-person rig configured
 *    the same way the UE5 Third Person template BP does it.
 *  - Character movement tuned for the Third Person template feel
 *    (orient-to-movement, 500 u/s walk speed, 700 u/s jump Z velocity).
 *  - Enhanced Input wiring (Move / Look / Jump) that probes the common
 *    project paths /Game/Input/... and /Game/ThirdPerson/Input/... if those
 *    assets exist. Soft failure — if the assets are missing, the character
 *    still possesses cleanly, it just has no bound inputs.
 *
 * The scaffold path in FCharacterToolsModule::Tool_CreateCharacterFromThirdPerson
 * uses this class as the default parent when the caller asks for a
 * "PlayableCharacter" (or does not specify a parent class at all), so every
 * scaffolded Blueprint is playable out of the box without any additional
 * Blueprint event-graph wiring.
 */
UCLASS(Blueprintable, BlueprintType)
class RIFTBORNAI_API ARiftbornPlayableCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARiftbornPlayableCharacter();

	virtual void PawnClientRestart() override;

	/** True when the default Enhanced Input references are all configured. */
	UFUNCTION(BlueprintPure, Category="Input")
	bool HasConfiguredDefaultInputAssets() const;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Forward/strafe movement from a 2D axis input. X = right, Y = forward. */
	void Move(const FInputActionValue& Value);

	/** Mouse / right-stick camera look from a 2D axis input. X = yaw, Y = pitch. */
	void Look(const FInputActionValue& Value);

	/** SpringArm "CameraBoom" that holds the follow camera behind the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Third-person follow camera attached to the end of the boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Enhanced Input mapping context applied to the local player on possession. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Move action (Axis2D). Resolves to WASD + gamepad left stick in the default IMC. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look action (Axis2D). Resolves to Mouse + gamepad right stick in the default IMC. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	/** Jump action (Digital). Resolves to Spacebar + gamepad face button bottom in the default IMC. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> JumpAction;
};
