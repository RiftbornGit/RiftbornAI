// Copyright RiftbornAI. All Rights Reserved.
// Tasks Panel - Real SListView bound to ViewModel
//
// This replaces the "snapshot dump" SVerticalBox approach.
// Key properties:
// - Binds to ViewModel delegates
// - Uses SListView for incremental updates
// - Does NOT own state - only reads and displays
// - Calls Controller for user actions

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "UI/RiftbornCopilotViewModel.h"

// Forward declarations
class FRiftbornCopilotController;
class SRiftbornStepRow;

/**
 * SRiftbornTasksPanel - ListView-based tasks display
 * 
 * ARCHITECTURE:
 * - Reads from ViewModel (never writes)
 * - Calls Controller for user actions (authorize, cancel, undo)
 * - Uses SListView for O(1) row updates instead of full rebuild
 * 
 * LIFECYCLE:
 * - OnPlanChanged: Rebuild header, swap StepItems array, refresh list
 * - OnStepChanged: Just request list refresh (rows re-query their step)
 * - OnEscalationChanged: Show/hide escalation card
 */
class RIFTBORNAI_API SRiftbornTasksPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRiftbornTasksPanel) {}
		SLATE_ARGUMENT(TSharedPtr<FRiftbornCopilotViewModel>, ViewModel)
		SLATE_ARGUMENT(TSharedPtr<FRiftbornCopilotController>, Controller)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SRiftbornTasksPanel();

private:
	// === LIST VIEW ===
	
	/** Generate a row widget for a step */
	TSharedRef<ITableRow> GenerateStepRow(
		TSharedPtr<FStepVM> Step,
		const TSharedRef<STableViewBase>& OwnerTable);
	
	/** Called when row selection changes */
	void OnStepSelectionChanged(
		TSharedPtr<FStepVM> SelectedStep,
		ESelectInfo::Type SelectInfo);

	// === VIEWMODEL HANDLERS ===
	
	/** Plan changed - rebuild header and step list */
	void OnPlanChanged();
	
	/** Individual step changed - refresh just that row */
	void OnStepChanged(const FGuid& StepId);
	
	/** Escalation state changed */
	void OnEscalationChanged();

	// === UI BUILDERS ===
	
	/** Build the header showing plan state */
	TSharedRef<SWidget> BuildPlanHeader();
	
	/** Build the footer with action buttons */
	TSharedRef<SWidget> BuildActionFooter();
	
	/** Build escalation card (shown inline when needed) */
	TSharedRef<SWidget> BuildEscalationCard();
	
	/** Update header to reflect current plan state */
	void RefreshHeader();

	// === BUTTON HANDLERS ===
	
	FReply OnAuthorizeClicked();
	FReply OnRejectClicked();
	FReply OnCancelClicked();
	FReply OnUndoClicked();
	
	// Escalation response
	FReply OnEscalationAction(FString Action);

	// === HELPERS ===
	
	/** Get color for plan state */
	FSlateColor GetPlanStateColor() const;
	
	/** Get text for plan state */
	FText GetPlanStateText() const;
	
	/** Check if we can show authorize button */
	bool CanAuthorize() const;
	
	/** Check if we can show cancel button */
	bool CanCancel() const;
	
	/** Check if we can show undo button */
	bool CanUndo() const;

private:
	// === BOUND OBJECTS ===
	TSharedPtr<FRiftbornCopilotViewModel> VM;
	TSharedPtr<FRiftbornCopilotController> Controller;
	
	// === WIDGETS ===
	TSharedPtr<SListView<TSharedPtr<FStepVM>>> StepListView;
	TSharedPtr<SVerticalBox> HeaderContainer;
	TSharedPtr<SVerticalBox> FooterContainer;
	TSharedPtr<SBox> EscalationContainer;
	
	// === LIST DATA ===
	// This array is what ListView binds to.
	// On plan change, we swap it with Plan->Steps.
	// The TSharedPtr<FStepVM> are stable - same objects as in VM.
	TArray<TSharedPtr<FStepVM>> StepItems;
	
	// === DELEGATE HANDLES ===
	FDelegateHandle PlanChangedHandle;
	FDelegateHandle StepChangedHandle;
	FDelegateHandle EscalationChangedHandle;
};
