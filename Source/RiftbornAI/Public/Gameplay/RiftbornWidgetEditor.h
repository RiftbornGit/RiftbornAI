// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WidgetBlueprint.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "RiftbornWidgetEditor.generated.h"

/**
 * Widget editor functionality for RiftbornAI
 * Allows scripted creation and editing of UMG widgets
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API URiftbornWidgetEditor : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Create a new widget component and add it to a widget blueprint
	 * @param WidgetBlueprint The widget blueprint to edit
	 * @param ParentWidget The parent widget to add to (or null for root)
	 * @param WidgetClass The class of widget to create (TextBlock, Image, etc)
	 * @param WidgetName Name for the new widget
	 * @return The created widget, or null if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static UWidget* CreateWidgetComponent(
		UWidgetBlueprint* WidgetBlueprint,
		UWidget* ParentWidget,
		TSubclassOf<UWidget> WidgetClass,
		FName WidgetName
	);

	/**
	 * Set widget position (for Canvas Panel slots)
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetWidgetPosition(UWidget* Widget, FVector2D Position);

	/**
	 * Set widget size (for Canvas Panel slots)
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetWidgetSize(UWidget* Widget, FVector2D Size);

	/**
	 * Set widget anchors (for Canvas Panel slots)
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetWidgetAnchors(UWidget* Widget, FAnchors Anchors);

	/**
	 * Set text on a TextBlock widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetTextBlockText(UTextBlock* TextBlock, FText Text);

	/**
	 * Set font size on a TextBlock widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetTextBlockFontSize(UTextBlock* TextBlock, int32 FontSize);

	/**
	 * Set color on a TextBlock widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetTextBlockColor(UTextBlock* TextBlock, FLinearColor Color);

	/**
	 * Set percent on a ProgressBar widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetProgressBarPercent(UProgressBar* ProgressBar, float Percent);

	/**
	 * Set fill color on a ProgressBar widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetProgressBarColor(UProgressBar* ProgressBar, FLinearColor Color);

	/**
	 * Set brush on an Image widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetImageBrush(UImage* Image, const FSlateBrush& Brush);

	/**
	 * Set image color
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SetImageColor(UImage* Image, FLinearColor Color);

	/**
	 * Save widget blueprint changes
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool SaveWidgetBlueprint(UWidgetBlueprint* WidgetBlueprint);

	/**
	 * Get widget tree root from a widget blueprint
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static UWidget* GetWidgetTreeRoot(UWidgetBlueprint* WidgetBlueprint);

	/**
	 * Create a widget tree from JSON specification
	 * Format: {"type": "CanvasPanel", "children": [...]}
	 */
	UFUNCTION(BlueprintCallable, Category = "Riftborn|Widget Editor")
	static bool CreateWidgetFromJSON(UWidgetBlueprint* WidgetBlueprint, const FString& JSONSpec);

private:
	static UWidget* CreateWidgetInternal(
		UWidgetBlueprint* WidgetBlueprint,
		UWidget* ParentWidget,
		TSubclassOf<UWidget> WidgetClass,
		FName WidgetName
	);
};
