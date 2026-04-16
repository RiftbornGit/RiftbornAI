// Copyright RiftbornAI. All Rights Reserved.
// SApprovalDialog.h - Semantic approval dialog for plan execution

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class FRiftbornCopilotController;

/**
 * SApprovalDialog - Shows semantic approval confirmation
 * 
 * Displayed when user clicks "Approve & Execute".
 * Shows: "4 actions (2 mutating), Undo: Yes/No"
 * Requires explicit confirmation before execution.
 */
class RIFTBORNAI_API SApprovalDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SApprovalDialog) {}
		SLATE_ARGUMENT(TSharedPtr<FRiftbornCopilotController>, Controller)
		SLATE_EVENT(FSimpleDelegate, OnApproved)
		SLATE_EVENT(FSimpleDelegate, OnRejected)
		SLATE_EVENT(FSimpleDelegate, OnCancelled)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	/** Show the dialog (makes visible) */
	void ShowDialog();
	
	/** Hide the dialog */
	void HideDialog();
	
	/** Is the dialog currently visible? */
	bool IsDialogVisible() const { return bIsVisible; }

private:
	TSharedPtr<FRiftbornCopilotController> Controller;
	FSimpleDelegate OnApproved;
	FSimpleDelegate OnRejected;
	FSimpleDelegate OnCancelled;
	bool bIsVisible = false;
	
	/** Build the summary section */
	TSharedRef<SWidget> BuildSummary();
	
	/** Build the risk breakdown */
	TSharedRef<SWidget> BuildRiskBreakdown();
	
	/** Build the buttons */
	TSharedRef<SWidget> BuildButtons();
	
	/** Get dynamic text for approval */
	FText GetApprovalText() const;
	
	/** Get dynamic text for risk level */
	FText GetRiskLevelText() const;
	FSlateColor GetRiskLevelColor() const;
	
	/** Handlers */
	FReply OnApproveClicked();
	FReply OnRejectClicked();
	FReply OnCancelClicked();
};
