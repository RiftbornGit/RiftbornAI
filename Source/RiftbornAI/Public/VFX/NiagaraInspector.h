// Copyright RiftbornAI. All Rights Reserved.
// Niagara Deep Introspection System - Phase 1 Observability Layer

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "NiagaraInspector.generated.h"

// Forward declarations
class UNiagaraSystem;
class UNiagaraEmitter;

/**
 * Module stage enumeration matching Niagara's execution stages
 */
UENUM(BlueprintType)
enum class ENiagaraModuleStage : uint8
{
	SystemSpawn,
	SystemUpdate,
	EmitterSpawn,
	EmitterUpdate,
	ParticleSpawn,
	ParticleUpdate,
	Unknown
};

/**
 * Detailed parameter value extracted from Niagara
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraInspectedParam
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString TypeName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ValueString;  // Serialized value

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsUserExposed = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsReadOnly = false;
};

/**
 * Module info extracted from a Niagara script stage
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraInspectedModule
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ModuleName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ModuleClassName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	ENiagaraModuleStage Stage = ENiagaraModuleStage::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	int32 StackIndex = -1;  // Position in module stack

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bEnabled = true;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FNiagaraInspectedParam> Parameters;
};

/**
 * Renderer info extracted from an emitter
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraInspectedRenderer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString RendererType;  // Sprite, Mesh, Ribbon, Light

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString MaterialPath;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bEnabled = true;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TMap<FString, FString> Properties;  // Key-value of renderer properties
};

/**
 * Complete emitter snapshot
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraInspectedEmitter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString UniqueEmitterName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bEnabled = true;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString SimTarget;  // CPUSim or GPUCompute

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FNiagaraInspectedModule> Modules;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FNiagaraInspectedRenderer> Renderers;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FNiagaraInspectedParam> LocalParameters;
};

/**
 * Complete system snapshot - the core observability output
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraSystemSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString SystemPath;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString SystemName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString SnapshotVersion = TEXT("1.0");

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ContentHash;  // SHA256 of serialized content

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FNiagaraInspectedEmitter> Emitters;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FNiagaraInspectedParam> UserParameters;  // System-level user params

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FNiagaraInspectedParam> SystemParameters;  // Internal system params
};

/**
 * Diff result between two snapshots
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraSnapshotDiff
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString SystemA;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString SystemB;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIdentical = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> AddedEmitters;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> RemovedEmitters;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> ModifiedEmitters;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> ParameterChanges;  // "EmitterName.ModuleName.ParamName: oldVal -> newVal"

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> ModuleStackChanges;  // "EmitterName: [module order diff]"
};

/**
 * Niagara Inspector - Deep introspection for VFX learning
 * 
 * This class provides the observability layer required for Phase 1:
 * - Enumerate emitters and their modules
 * - Extract all parameter values
 * - Capture renderer bindings
 * - Produce JSON snapshots
 * - Diff two systems
 */
UCLASS()
class RIFTBORNAI_API UNiagaraInspector : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Take a complete snapshot of a Niagara system
	 * @param System - The system to inspect
	 * @return Complete snapshot with all emitters, modules, and parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Niagara")
	static FNiagaraSystemSnapshot InspectSystem(UNiagaraSystem* System);

	/**
	 * Take a snapshot and return as JSON string
	 * @param System - The system to inspect
	 * @return JSON string representation of the snapshot
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Niagara")
	static FString InspectSystemToJson(UNiagaraSystem* System);

	/**
	 * Load a system by path and inspect it
	 * @param SystemPath - Asset path (e.g., /Game/VFX/NS_Fire)
	 * @return JSON string representation of the snapshot
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Niagara")
	static FString InspectSystemByPath(const FString& SystemPath);

	/**
	 * Diff two Niagara systems
	 * @param SystemA - First system
	 * @param SystemB - Second system
	 * @return Diff result showing all differences
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Niagara")
	static FNiagaraSnapshotDiff DiffSystems(UNiagaraSystem* SystemA, UNiagaraSystem* SystemB);

	/**
	 * Diff two systems by path
	 * @param PathA - Path to first system
	 * @param PathB - Path to second system
	 * @return JSON string of diff result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Niagara")
	static FString DiffSystemsByPath(const FString& PathA, const FString& PathB);

	/**
	 * Save snapshot to disk as JSON
	 * @param Snapshot - The snapshot to save
	 * @param FilePath - Destination file path
	 * @return True if saved successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Niagara")
	static bool SaveSnapshotToFile(const FNiagaraSystemSnapshot& Snapshot, const FString& FilePath);

	/**
	 * Load snapshot from disk
	 * @param FilePath - Source file path
	 * @param OutSnapshot - Loaded snapshot
	 * @return True if loaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Niagara")
	static bool LoadSnapshotFromFile(const FString& FilePath, FNiagaraSystemSnapshot& OutSnapshot);

private:
	/** Inspect a single emitter */
	static FNiagaraInspectedEmitter InspectEmitter(struct FVersionedNiagaraEmitter& VersionedEmitter, const struct FNiagaraEmitterHandle& Handle);

	/** Extract modules from a script stage */
	static TArray<FNiagaraInspectedModule> ExtractModules(class UNiagaraScript* Script, ENiagaraModuleStage Stage);

	/** Extract renderers from an emitter */
	static TArray<FNiagaraInspectedRenderer> ExtractRenderers(class UNiagaraEmitter* Emitter);

	/** Convert parameter to string representation */
	static FString ParameterToString(const struct FNiagaraVariable& Variable, const struct FNiagaraParameterStore* Store);

	/** Compute content hash for snapshot */
	static FString ComputeContentHash(const FNiagaraSystemSnapshot& Snapshot);

	/** Convert snapshot to JSON object */
	static TSharedPtr<FJsonObject> SnapshotToJsonObject(const FNiagaraSystemSnapshot& Snapshot);

	/** Convert diff to JSON object */
	static TSharedPtr<FJsonObject> DiffToJsonObject(const FNiagaraSnapshotDiff& Diff);
};
