// Copyright Riftborn. All Rights Reserved.
// PCG Voxel Sampler — samples voxel terrain data at PCG point positions.
// Adds density, material ID, and layer name attributes to point data.
#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "PCGVoxelSampler.generated.h"

/**
 * UPCGVoxelSamplerSettings — PCG node that queries voxel terrain at each point.
 * Adds attributes: VoxelDensity (float), VoxelMaterial (int32), VoxelLayer (string).
 * Use to filter PCG points based on terrain depth, material, or layer.
 */
UCLASS(BlueprintType, ClassGroup=(Procedural))
class RIFTBORNAI_API UPCGVoxelSamplerSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPCGVoxelSamplerSettings();

	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("VoxelSampler")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGVoxelSampler", "NodeTitle", "Voxel Terrain Sampler"); }
	virtual FText GetNodeTooltipText() const override;
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif
	virtual bool UseSeed() const override { return false; }
	virtual FString GetAdditionalTitleInformation() const override { return TEXT("VoxelSampler"); }

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface

public:
	// Remove points that are inside solid voxel terrain
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel Sampler")
	bool bRemovePointsInsideTerrain = false;

	// Remove points that are in air (not on/near terrain surface)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel Sampler")
	bool bRemovePointsInAir = false;

	// Maximum distance from surface to keep points (when bRemovePointsInAir)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel Sampler",
		meta = (EditCondition = "bRemovePointsInAir", ClampMin = "1.0"))
	float MaxSurfaceDistance = 200.0f;
};

class FPCGVoxelSamplerElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};
