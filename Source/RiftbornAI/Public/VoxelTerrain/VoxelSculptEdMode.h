// Copyright Riftborn. All Rights Reserved.
// Voxel Sculpt Editor Mode — viewport brush for painting/sculpting voxel terrain.
// Provides click-to-dig, click-to-raise, material painting in the editor viewport.
#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

#if WITH_EDITOR

class UWorld;

/**
 * FVoxelSculptEdMode — editor mode for voxel terrain sculpting.
 * Activated via the editor mode toolbar. Provides:
 * - Click to dig (subtract sphere at cursor)
 * - Shift+click to raise (add sphere at cursor)
 * - Ctrl+click to smooth
 * - Adjustable brush radius via mouse wheel
 * - Material painting mode
 */
class RIFTBORNAI_API FVoxelSculptEdMode : public FEdMode
{
public:
	static const FEditorModeID EM_VoxelSculpt;

	FVoxelSculptEdMode();
	virtual ~FVoxelSculptEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual bool HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	virtual bool InputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override;
	virtual bool UsesToolkits() const override { return false; }

	// Brush settings
	float BrushRadius = 200.0f;
	float BrushStrength = 0.5f;
	uint8 PaintMaterialID = 0;

	enum class ESculptTool : uint8
	{
		Dig,
		Raise,
		Smooth,
		Paint,
	};
	ESculptTool ActiveTool = ESculptTool::Dig;

private:
	FVector LastBrushLocation = FVector::ZeroVector;
	bool bBrushValid = false;

	bool TraceTerrainUnderCursor(FEditorViewportClient* ViewportClient, UWorld* World, FVector& OutHitLocation) const;
	void ApplySculptAtLocation(UWorld* World, const FVector& Location);
};

#endif // WITH_EDITOR
