// Copyright RiftbornAI. All Rights Reserved.
// Material Introspection System - Phase 3 Material Observability

#pragma once

#include "CoreMinimal.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MaterialInspector.generated.h"

/**
 * Texture slot info
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMaterialTextureSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ParameterName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString TexturePath;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString TextureType;  // 2D, Cube, Volume, etc.

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	int32 Width = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	int32 Height = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString CompressionSettings;
};

/**
 * Scalar parameter info
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMaterialScalarParam
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ParameterName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	float Value = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	float MinValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	float MaxValue = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Group;
};

/**
 * Vector parameter info
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMaterialVectorParam
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ParameterName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FLinearColor Value = FLinearColor::Black;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsColor = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Group;
};

/**
 * Static switch parameter info
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMaterialSwitchParam
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ParameterName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bValue = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Group;
};

/**
 * Material feature flags
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMaterialFeatures
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bHasEmissive = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bHasOpacity = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bHasOpacityMask = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bHasNormal = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bHasWorldPositionOffset = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bHasSubsurfaceColor = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsTranslucent = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsMasked = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsUnlit = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsTwoSided = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString BlendMode;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ShadingModel;
};

/**
 * Complete material snapshot
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMaterialSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString MaterialPath;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString MaterialName;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ParentMaterialPath;  // For material instances

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsMaterialInstance = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString SnapshotVersion = TEXT("1.0");

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ContentHash;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FMaterialFeatures Features;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FMaterialScalarParam> ScalarParameters;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FMaterialVectorParam> VectorParameters;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FMaterialTextureSlot> TextureSlots;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FMaterialSwitchParam> SwitchParameters;
};

/**
 * Result of material operations
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMaterialOpResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString CreatedPath;

	static FMaterialOpResult Success(const FString& Msg = TEXT("OK"), const FString& Path = TEXT(""))
	{
		FMaterialOpResult R;
		R.bSuccess = true;
		R.Message = Msg;
		R.CreatedPath = Path;
		return R;
	}

	static FMaterialOpResult Fail(const FString& Msg)
	{
		FMaterialOpResult R;
		R.bSuccess = false;
		R.Message = Msg;
		return R;
	}
};

/**
 * Material Inspector - Deep introspection for material learning
 * 
 * Phase 3: Material Introspection + Parametric Authoring
 * 
 * This class provides material observability:
 * - InspectMaterial: Extract all parameters, textures, features
 * - InspectMaterialInstance: Extract instance overrides
 * - CreateMaterialInstance: Create parametric instance from parent
 * - SetScalarParam/SetVectorParam/SetTextureParam: Modify instances
 * - BindToNiagaraRenderer: Connect material to Niagara renderer
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UMaterialInspector : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Inspect a material and extract all parameters/features
	 * @param Material - Material to inspect
	 * @return Complete material snapshot
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static FMaterialSnapshot InspectMaterial(UMaterialInterface* Material);

	/**
	 * Save material snapshot to JSON file
	 * @param Material - Material to snapshot
	 * @param FilePath - Output file path
	 * @return True if saved successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static bool SaveMaterialSnapshot(UMaterialInterface* Material, const FString& FilePath);

	/**
	 * Create a dynamic material instance from a parent
	 * @param Parent - Parent material or material instance
	 * @param PackagePath - Where to save
	 * @param InstanceName - Name for new instance
	 * @return Result with created path
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static FMaterialOpResult CreateMaterialInstance(UMaterialInterface* Parent, const FString& PackagePath, const FString& InstanceName);

	/**
	 * Set a scalar parameter on a material instance
	 * @param Instance - Target material instance
	 * @param ParamName - Parameter name
	 * @param Value - New value
	 * @return Result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static FMaterialOpResult SetScalarParam(UMaterialInstanceDynamic* Instance, const FString& ParamName, float Value);

	/**
	 * Set a vector/color parameter on a material instance
	 * @param Instance - Target material instance
	 * @param ParamName - Parameter name
	 * @param Value - New value
	 * @return Result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static FMaterialOpResult SetVectorParam(UMaterialInstanceDynamic* Instance, const FString& ParamName, FLinearColor Value);

	/**
	 * Set a texture parameter on a material instance
	 * @param Instance - Target material instance
	 * @param ParamName - Parameter name
	 * @param TexturePath - Path to texture asset
	 * @return Result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static FMaterialOpResult SetTextureParam(UMaterialInstanceDynamic* Instance, const FString& ParamName, const FString& TexturePath);

	/**
	 * Clone a material instance with new parameters
	 * @param Source - Source material instance
	 * @param PackagePath - Where to save
	 * @param InstanceName - Name for new instance
	 * @return Result with created path
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static FMaterialOpResult CloneMaterialInstance(UMaterialInterface* Source, const FString& PackagePath, const FString& InstanceName);

	/**
	 * Get materials used by a Niagara system
	 * @param System - Niagara system to inspect
	 * @return Array of material paths
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static TArray<FString> GetNiagaraMaterials(class UNiagaraSystem* System);

	/**
	 * Check if a material is suitable for VFX (particles)
	 * @param Material - Material to check
	 * @return True if material has VFX-appropriate settings
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Material")
	static bool IsVFXCompatible(UMaterialInterface* Material);

private:
	/** Extract scalar parameters from material */
	static TArray<FMaterialScalarParam> ExtractScalarParams(UMaterialInterface* Material);

	/** Extract vector parameters from material */
	static TArray<FMaterialVectorParam> ExtractVectorParams(UMaterialInterface* Material);

	/** Extract texture slots from material */
	static TArray<FMaterialTextureSlot> ExtractTextureSlots(UMaterialInterface* Material);

	/** Extract switch parameters from material */
	static TArray<FMaterialSwitchParam> ExtractSwitchParams(UMaterialInterface* Material);

	/** Extract material features/flags */
	static FMaterialFeatures ExtractFeatures(UMaterialInterface* Material);

	/** Compute content hash for material */
	static FString ComputeMaterialHash(UMaterialInterface* Material);
};
