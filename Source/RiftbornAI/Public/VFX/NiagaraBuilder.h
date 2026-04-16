// Copyright RiftbornAI. All Rights Reserved.
// Niagara Authoring Primitives - Phase 2 Atomic Operations

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraBuilder.generated.h"

// Forward declarations
class UNiagaraScript;
class UNiagaraNodeFunctionCall;

/**
 * Result of an atomic operation
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraOpResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString CreatedId;  // ID of created element (emitter, module, etc.)

	static FNiagaraOpResult Success(const FString& Msg = TEXT("OK"), const FString& Id = TEXT(""))
	{
		FNiagaraOpResult R;
		R.bSuccess = true;
		R.Message = Msg;
		R.CreatedId = Id;
		return R;
	}

	static FNiagaraOpResult Fail(const FString& Msg)
	{
		FNiagaraOpResult R;
		R.bSuccess = false;
		R.Message = Msg;
		return R;
	}
};

/**
 * Specification for creating an emitter
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FEmitterSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString Name = TEXT("NewEmitter");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	bool bGPUSim = false;  // false = CPU, true = GPU

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	bool bLocalSpace = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	int32 MaxParticles = 1000;
};

/**
 * Specification for adding a module
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FModuleSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString ModuleScriptPath;  // Path to NiagaraScript asset

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString TargetEmitterName;  // Which emitter to add to

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString Stage;  // SystemSpawn, SystemUpdate, EmitterSpawn, EmitterUpdate, ParticleSpawn, ParticleUpdate

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	int32 StackIndex = -1;  // -1 = append at end
};

/**
 * Specification for setting a parameter
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FParamSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString EmitterName;  // Empty = system-level param

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString ModuleName;  // Empty = emitter-level param

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString ParamName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString ParamType;  // Float, Int, Bool, Vector, Color, etc.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString Value;  // String representation to parse
};

/**
 * Specification for binding a renderer
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRendererSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString EmitterName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString RendererType;  // Sprite, Mesh, Ribbon, Light

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	FString MaterialPath;  // Optional material to bind

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RiftbornAI")
	TMap<FString, FString> Properties;  // Key-value properties
};

/**
 * Niagara Builder - Atomic operations for VFX construction
 * 
 * Phase 2: Authoring Primitives
 * 
 * This class provides the "assembly language" for Niagara:
 * - CreateSystem: Create a new empty Niagara system
 * - CreateEmitter: Add an emitter to a system
 * - AddModule: Add a module to an emitter stage
 * - RemoveModule: Remove a module from an emitter
 * - SetParam: Set a parameter value
 * - BindRenderer: Add and configure a renderer
 * 
 * All operations are deterministic and reversible.
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UNiagaraBuilder : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Create a new empty Niagara system asset
	 * @param PackagePath - Where to save (e.g., "/Game/RiftbornAI/Generated/VFX")
	 * @param AssetName - Name of the system (e.g., "NS_Fire_Generated")
	 * @return Result with created system path
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult CreateSystem(const FString& PackagePath, const FString& AssetName);

	/**
	 * Create an emitter and add it to a system
	 * @param System - Target system
	 * @param Spec - Emitter specification
	 * @return Result with created emitter ID
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult CreateEmitter(UNiagaraSystem* System, const FEmitterSpec& Spec);

	/**
	 * Add a module to an emitter's script stage
	 * @param System - Target system
	 * @param Spec - Module specification
	 * @return Result with added module info
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult AddModule(UNiagaraSystem* System, const FModuleSpec& Spec);

	/**
	 * Remove a module from an emitter
	 * @param System - Target system
	 * @param EmitterName - Emitter containing the module
	 * @param ModuleName - Module to remove
	 * @param Stage - Stage the module is in
	 * @return Result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult RemoveModule(UNiagaraSystem* System, const FString& EmitterName, const FString& ModuleName, const FString& Stage);

	/**
	 * Set a parameter value
	 * @param System - Target system
	 * @param Spec - Parameter specification
	 * @return Result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult SetParam(UNiagaraSystem* System, const FParamSpec& Spec);

	/**
	 * Get a parameter value
	 * @param System - Target system
	 * @param EmitterName - Emitter name (empty for system param)
	 * @param ParamPath - Full parameter path
	 * @param OutValue - Retrieved value as string
	 * @return True if found
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static bool GetParam(UNiagaraSystem* System, const FString& EmitterName, const FString& ParamPath, FString& OutValue);

	/**
	 * Add a renderer to an emitter
	 * @param System - Target system
	 * @param Spec - Renderer specification
	 * @return Result with renderer info
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult AddRenderer(UNiagaraSystem* System, const FRendererSpec& Spec);

	/**
	 * Remove a renderer from an emitter
	 * @param System - Target system
	 * @param EmitterName - Emitter containing the renderer
	 * @param RendererType - Type of renderer to remove (first match)
	 * @return Result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult RemoveRenderer(UNiagaraSystem* System, const FString& EmitterName, const FString& RendererType);

	/**
	 * Compile a system after modifications
	 * @param System - System to compile
	 * @return Result with compile status
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult CompileSystem(UNiagaraSystem* System);

	/**
	 * Save a system to disk
	 * @param System - System to save
	 * @return Result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult SaveSystem(UNiagaraSystem* System);

	/**
	 * Clone an existing system as a starting point
	 * @param SourcePath - Path to source system
	 * @param DestPackagePath - Destination package path
	 * @param DestAssetName - New asset name
	 * @return Result with new system path
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static FNiagaraOpResult CloneSystem(const FString& SourcePath, const FString& DestPackagePath, const FString& DestAssetName);

	/**
	 * Get or create a minimal RL-friendly Niagara template.
	 * This template is designed for RL training:
	 * - Opaque sprite particles (not additive)
	 * - User.SpawnRate parameter exposed and responsive
	 * - Bright visible color
	 * - No complex post-processing
	 * 
	 * @return Loaded system, or nullptr if creation failed
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static UNiagaraSystem* GetOrCreateRLTemplate();

	/**
	 * Find the best available Niagara template for RL training.
	 * Searches in priority order:
	 * 1. /Game/RiftbornAI/Templates/NS_RLTemplate (custom)
	 * 2. /Niagara/DefaultAssets/Templates/NS_SimpleSprite (engine)
	 * 3. /Niagara/DefaultAssets/NS_SimpleSprite (engine alt)
	 * 4. JumpPad (fallback with warning)
	 * 
	 * @param OutWarning - Set if using a suboptimal template
	 * @return Best available system, or nullptr if none found
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|NiagaraBuilder")
	static UNiagaraSystem* FindBestRLTemplate(FString& OutWarning);

private:
	/** Find emitter handle by name */
	static FNiagaraEmitterHandle* FindEmitterHandle(UNiagaraSystem* System, const FString& EmitterName);

	/** Get script for a stage */
	static UNiagaraScript* GetScriptForStage(UNiagaraEmitter* Emitter, const FString& StageName);

	/** Parse string value to Niagara variable */
	static bool ParseValueToVariable(const FString& TypeName, const FString& Value, FNiagaraVariable& OutVar);
};
