// Copyright RiftbornAI. All Rights Reserved.
// Step Row Widget - Single row in the Tasks ListView
//
// Displays one step with:
// - Status icon (pending/executing/success/failed)
// - Tool name and description
// - Risk indicator
// - Undo button (when available)
// - Confirm button (for high-risk steps needing confirmation)

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableRow.h"
#include "UI/RiftbornCopilotViewModel.h"

class FRiftbornCopilotController;

/**
 * SRiftbornStepRow - Single step display in ListView
 * 
 * ARCHITECTURE:
 * - Holds weak reference to FStepVM (stable pointer from ViewModel)
 * - Queries step state on every Tick/Paint (no caching)
 * - Calls Controller for confirm/undo actions
 * 
 * This ensures the row always shows current truth without manual refresh.
 */
class RIFTBORNAI_API SRiftbornStepRow : public SMultiColumnTableRow<TSharedPtr<FStepVM>>
{
public:
	SLATE_BEGIN_ARGS(SRiftbornStepRow) {}
		SLATE_ARGUMENT(TSharedPtr<FStepVM>, Step)
		SLATE_ARGUMENT(TSharedPtr<FRiftbornCopilotController>, Controller)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable);

	// SMultiColumnTableRow interface
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	// === WIDGET BUILDERS ===
	
	/** Build status icon (checkmark, X, spinner, etc.) */
	TSharedRef<SWidget> BuildStatusIcon();
	
	/** Build the main content (tool name, description) */
	TSharedRef<SWidget> BuildContent();
	
	/** Build action buttons (undo, confirm) */
	TSharedRef<SWidget> BuildActions();

	// === DYNAMIC GETTERS (called every frame) ===
	
	/** Get icon brush based on current status */
	const FSlateBrush* GetStatusBrush() const;
	
	/** Get status color */
	FSlateColor GetStatusColor() const;
	
	/** Get label text */
	FText GetLabelText() const;
	
	/** Get result/error text */
	FText GetResultText() const;
	
	/** Should show undo button? */
	EVisibility GetUndoVisibility() const;
	
	/** Should show confirm button? */
	EVisibility GetConfirmVisibility() const;
	
	/** Get risk indicator color */
	FSlateColor GetRiskColor() const;

	// === BUTTON HANDLERS ===
	
	FReply OnUndoClicked();
	FReply OnConfirmClicked();

private:
	TWeakPtr<FStepVM> WeakStep;
	TWeakPtr<FRiftbornCopilotController> WeakController;
};
