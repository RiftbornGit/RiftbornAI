// Copyright RiftbornAI. All Rights Reserved.
// Template Actions - Pre-built game development patterns that execute instantly without AI

#pragma once

#include "CoreMinimal.h"
#include "TemplateActions.generated.h"

/**
 * Category for template actions
 */
UENUM(BlueprintType)
enum class ETemplateActionCategory : uint8
{
	Character,
	Movement,
	Combat,
	UI,
	Level,
	GameMode,
	AI,
	Audio,
	Effects
};

/**
 * Result of executing a template action
 */
USTRUCT(BlueprintType)
struct FTemplateResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> CreatedAssets;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> ModifiedAssets;
};

/**
 * Definition of a template action
 */
USTRUCT(BlueprintType)
struct FTemplateAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	ETemplateActionCategory Category = ETemplateActionCategory::Character;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> Keywords;  // For matching user queries

	// Function pointer for execution
	TFunction<FTemplateResult()> Execute;
};

/**
 * Template Actions Registry - Pre-built patterns for common game dev tasks
 * These execute INSTANTLY without calling Claude API
 */
class RIFTBORNAI_API FTemplateActionRegistry
{
public:
	static FTemplateActionRegistry& Get();

	// Register a template
	void RegisterTemplate(const FTemplateAction& Action);

	// Find matching templates for a query
	TArray<FTemplateAction> FindMatchingTemplates(const FString& Query, int32 MaxResults = 5) const;

	// Execute a template by ID
	FTemplateResult ExecuteTemplate(const FString& TemplateId);

	// Get all templates
	const TArray<FTemplateAction>& GetAllTemplates() const { return Templates; }

	// Get templates by category
	TArray<FTemplateAction> GetTemplatesByCategory(ETemplateActionCategory Category) const;

private:
	FTemplateActionRegistry();
	void RegisterBuiltinTemplates();

	TArray<FTemplateAction> Templates;
	TMap<FString, int32> TemplateIdToIndex;

	// Score how well a query matches a template
	float ScoreMatch(const FString& Query, const FTemplateAction& Template) const;
};

/**
 * Built-in template implementations
 */
class RIFTBORNAI_API FBuiltinTemplates
{
public:
	// Character templates
	static FTemplateResult SetupThirdPersonCharacter();
	static FTemplateResult SetupFirstPersonCharacter();
	static FTemplateResult AddDoubleJump();
	static FTemplateResult AddSprint();
	static FTemplateResult AddCrouch();

	// Combat templates
	static FTemplateResult AddBasicMeleeAttack();
	static FTemplateResult AddProjectileAttack();
	static FTemplateResult AddHealthSystem();
	static FTemplateResult AddDamageNumbers();

	// Movement templates
	static FTemplateResult AddDash();
	static FTemplateResult AddWallJump();
	static FTemplateResult AddGrappleHook();

	// GameMode templates
	static FTemplateResult SetupArenaGameMode();
	static FTemplateResult SetupWaveBasedGameMode();

	// UI templates
	static FTemplateResult AddHealthBar();
	static FTemplateResult AddMinimapWidget();
	static FTemplateResult AddInventoryUI();

	// Level templates
	static FTemplateResult SpawnPlayerStart();
	static FTemplateResult CreateBasicArena();
};
