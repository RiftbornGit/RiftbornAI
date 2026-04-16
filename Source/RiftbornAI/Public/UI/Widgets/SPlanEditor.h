// Copyright RiftbornAI. All Rights Reserved.
// SPlanEditor.h - Editable plan widget for the Planning Copilot
//
// This widget displays a PlanDraft and allows the user to:
// - Edit step arguments (position, scale, color, etc.)
// - Disable/enable individual steps
// - Reorder steps via drag or buttons
// - See computed risk and reversibility per step
// - View expected outputs
//
// Priority 7 (2026-01-31): PREFLIGHT INTEGRATION
// - Shows eligibility badge per step (Ready/NeedsContract/NeedsWitness/etc.)
// - Preflight panel shows overall PROOF approvability
// - Approve button disabled when preflight fails in PROOF mode
// - Fix suggestions shown for failing steps

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "UI/CopilotStateMachine.h"
#include "PlanPreflight.h"

class FRiftbornCopilotController;

/**
 * SPlanStepRow - Single row in the plan editor showing one step
 */
class RIFTBORNAI_API SPlanStepRow : public SMultiColumnTableRow<TSharedPtr<FDraftToolCall>>
{
public:
	SLATE_BEGIN_ARGS(SPlanStepRow) {}
		SLATE_ARGUMENT(TSharedPtr<FDraftToolCall>, Step)
		SLATE_ARGUMENT(TWeakPtr<FRiftbornCopilotController>, Controller)
		SLATE_ARGUMENT(TOptional<FStepPreflightResult>, PreflightResult)  // Priority 7: step preflight
		SLATE_EVENT(FSimpleDelegate, OnStepModified)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);
	
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FDraftToolCall> Step;
	TWeakPtr<FRiftbornCopilotController> WeakController;
	TOptional<FStepPreflightResult> PreflightResult;  // Priority 7
	FSimpleDelegate OnStepModified;
	
	// UI helpers
	FSlateColor GetRiskColor() const;
	FText GetRiskText() const;
	FText GetStepLabel() const;
	ECheckBoxState GetEnabledState() const;
	void OnEnabledChanged(ECheckBoxState NewState);
	
	// Priority 7: Eligibility helpers
	FSlateColor GetEligibilityColor() const;
	FText GetEligibilityText() const;
	FText GetEligibilityTooltip() const;
};

/**
 * SPlanEditor - Full plan editor widget
 */
class RIFTBORNAI_API SPlanEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPlanEditor) {}
		SLATE_ARGUMENT(TSharedPtr<FRiftbornCopilotController>, Controller)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	/** Refresh the display from the current draft */
	void RefreshFromDraft();
	
	/** Update preflight results and re-render eligibility badges */
	void UpdatePreflightResults(const FPlanPreflightResult& Results);

private:
	TSharedPtr<FRiftbornCopilotController> Controller;
	TSharedPtr<SListView<TSharedPtr<FDraftToolCall>>> StepListView;
	TArray<TSharedPtr<FDraftToolCall>> DisplaySteps;
	
	// Priority 7: Preflight state
	FPlanPreflightResult CachedPreflightResult;
	TSharedPtr<SVerticalBox> PreflightPanelContainer;
	TSharedPtr<SButton> ApproveButton;
	
	// Column generation
	TSharedRef<ITableRow> GenerateStepRow(TSharedPtr<FDraftToolCall> Step, const TSharedRef<STableViewBase>& OwnerTable);
	
	// Header widgets
	TSharedRef<SWidget> BuildHeaderSection();
	TSharedRef<SWidget> BuildRiskSummary();
	TSharedRef<SWidget> BuildExpectedOutputs();
	TSharedRef<SWidget> BuildPredicates();
	
	// Priority 7: Preflight widgets
	TSharedRef<SWidget> BuildPreflightPanel();
	TSharedRef<SWidget> BuildApprovalButtons();
	void RefreshPreflightPanel();
	
	// Event handlers
	void OnStepModified();
	void OnDraftChanged();
	void OnPreflightComplete(const FPlanPreflightResult& Results);
	FReply OnApproveClicked();
	FReply OnSaveDraftClicked();
	FReply OnRequestChangesClicked();
};

/**
 * SStepArgumentEditor - Popup/inline editor for step arguments
 */
class RIFTBORNAI_API SStepArgumentEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStepArgumentEditor) {}
		SLATE_ARGUMENT(TSharedPtr<FDraftToolCall>, Step)
		SLATE_ARGUMENT(TWeakPtr<FRiftbornCopilotController>, Controller)
		SLATE_EVENT(FSimpleDelegate, OnArgumentsChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FDraftToolCall> Step;
	TWeakPtr<FRiftbornCopilotController> WeakController;
	FSimpleDelegate OnArgumentsChanged;
	
	TSharedRef<SWidget> BuildArgumentRow(const FString& Key, const FString& Value);
	void OnArgumentValueChanged(const FString& Key, const FText& NewValue);
	void OnArgumentValueCommitted(const FString& Key, const FText& NewValue, ETextCommit::Type CommitType);
};
