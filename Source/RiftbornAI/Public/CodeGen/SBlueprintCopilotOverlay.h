// Copyright RiftbornAI. All Rights Reserved.
// SBlueprintCopilotOverlay - Visual overlay for Blueprint suggestions

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "BlueprintPatch.h"

class SGraphEditor;
class SGraphPanel;
class UEdGraph;

/**
 * Ghost node representation for overlay rendering.
 * Not a real UEdGraphNode, just visual data.
 */
struct FGhostNodeVisual
{
	/** Display title */
	FString Title;
	
	/** Node class hint for icon/color */
	FString NodeClass;
	
	/** Position in graph space */
	FVector2D Position;
	
	/** Approximate size for bounding box */
	FVector2D Size = FVector2D(150, 60);
	
	/** Color tint */
	FLinearColor Color = FLinearColor(0.2f, 0.8f, 0.2f, 0.6f);
	
	/** Is this an exec flow node? */
	bool bIsExecNode = false;
	
	FGhostNodeVisual() = default;
	FGhostNodeVisual(const FString& InTitle, const FVector2D& InPosition)
		: Title(InTitle), Position(InPosition) {}
};

/**
 * Ghost wire representation for overlay rendering.
 */
struct FGhostWireVisual
{
	/** Source position in graph space */
	FVector2D SourcePos;
	
	/** Target position in graph space */
	FVector2D TargetPos;
	
	/** Is this an exec wire? */
	bool bIsExecWire = false;
	
	/** Color */
	FLinearColor Color = FLinearColor(0.2f, 0.8f, 0.2f, 0.4f);
	
	FGhostWireVisual() = default;
	FGhostWireVisual(const FVector2D& InSource, const FVector2D& InTarget)
		: SourcePos(InSource), TargetPos(InTarget) {}
};

/**
 * Overlay widget that renders ghost nodes on top of a Blueprint graph.
 * 
 * This is a transparent overlay that sits on top of the SGraphEditor
 * and renders the suggestion preview (ghost nodes, wires, hint text).
 * 
 * It does NOT modify the actual graph - just visualizes what would happen.
 */
class RIFTBORNAI_API SBlueprintCopilotOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBlueprintCopilotOverlay)
		: _Patch(nullptr)
		, _GraphEditor(nullptr)
	{}
		/** The patch being previewed */
		SLATE_ARGUMENT(const FBlueprintPatch*, Patch)
		
		/** The graph editor we're overlaying */
		SLATE_ARGUMENT(TWeakPtr<SGraphEditor>, GraphEditor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// SWidget interface
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, 
						  const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, 
						  int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	// === Public API ===
	
	/** Update the patch being previewed */
	void SetPatch(const FBlueprintPatch* InPatch);
	
	/** Clear the overlay */
	void Clear();
	
	/** Update suggestion index display (1/3, 2/3, etc) */
	void SetSuggestionIndex(int32 CurrentIndex, int32 TotalCount);
	
	/** Set anchor position (where the suggestion connects from) */
	void SetAnchorPosition(const FVector2D& GraphPosition);
	
	/** Set the graph editor reference (for coordinate transforms) */
	void SetGraphEditor(TSharedPtr<SGraphEditor> InGraphEditor) { GraphEditorWeak = InGraphEditor; }

	/** Set the target graph (for resolving existing node positions in wires) */
	void SetTargetGraph(UEdGraph* InGraph) { TargetGraph = InGraph; }

	/** Rebuild ghost visuals from current patch */
	void RebuildGhostVisuals();

private:
	/** Resolve an existing node's position from the target graph by GUID */
	FVector2D ResolveExistingNodePosition(const FString& NodeId, bool bOutputSide) const;

	/** Convert graph coordinates to widget local coordinates */
	FVector2D GraphToLocal(const FVector2D& GraphPos, const FGeometry& Geometry) const;
	
	/** Get the graph panel from the graph editor */
	TSharedPtr<SGraphPanel> GetGraphPanel() const;
	
	/** Paint ghost nodes */
	void PaintGhostNodes(const FGeometry& AllottedGeometry, 
						 FSlateWindowElementList& OutDrawElements, 
						 int32& LayerId) const;
	
	/** Paint ghost wires */
	void PaintGhostWires(const FGeometry& AllottedGeometry, 
						 FSlateWindowElementList& OutDrawElements, 
						 int32& LayerId) const;
	
	/** Paint hint text (Tab to accept, Esc to dismiss) */
	void PaintHintText(const FGeometry& AllottedGeometry, 
					   FSlateWindowElementList& OutDrawElements, 
					   int32& LayerId) const;
	
	/** The patch being previewed */
	const FBlueprintPatch* CurrentPatch = nullptr;
	
	/** The graph editor we're overlaying */
	TWeakPtr<SGraphEditor> GraphEditorWeak;

	/** The target graph (for resolving existing node positions) */
	TWeakObjectPtr<UEdGraph> TargetGraph;

	/** Ghost node visuals (computed from patch) */
	TArray<FGhostNodeVisual> GhostNodes;
	
	/** Ghost wire visuals (computed from patch) */
	TArray<FGhostWireVisual> GhostWires;
	
	/** Anchor position in graph space (where suggestion connects from) */
	FVector2D AnchorPosition = FVector2D::ZeroVector;
	
	/** Suggestion navigation display */
	int32 CurrentSuggestionIndex = 0;
	int32 TotalSuggestionCount = 1;
	
	/** Cached fonts */
	FSlateFontInfo HintFont;
	FSlateFontInfo TitleFont;
	
	/** Colors */
	FLinearColor GhostNodeColor = FLinearColor(0.3f, 0.8f, 0.3f, 0.5f);
	FLinearColor GhostNodeBorderColor = FLinearColor(0.2f, 0.7f, 0.2f, 0.8f);
	FLinearColor GhostWireColor = FLinearColor(0.3f, 0.8f, 0.3f, 0.4f);
	FLinearColor HintTextColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.9f);
	FLinearColor HintBackgroundColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.8f);
};
