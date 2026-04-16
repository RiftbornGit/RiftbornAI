// SRiftbornTrajectoryViewer.h
// Editor-only panel that shows the reliability scorecard and recent trajectories
// so a developer can answer "why did the agent stop picking this tool?" without
// grepping logs. Pulls data from FBrainMetrics::GenerateReport and
// FBrainTrajectoryLogger::LoadTrajectories — no duplicated state.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

struct FTrajectory;

/**
 * SRiftbornTrajectoryViewer — a developer observability panel.
 *
 * Left pane: reliability report (FBrainMetrics::GenerateReport).
 * Right pane: recent trajectories for a selected task family. Selecting a
 * trajectory reveals its step list.
 *
 * This is intentionally read-only. Mutating the reliability data is the
 * responsibility of the tool runtime, not the viewer.
 */
class RIFTBORNAI_API SRiftbornTrajectoryViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRiftbornTrajectoryViewer) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// --- reliability report (left pane) ---
	FReply OnRefreshReport();
	FText  GetReportText() const;

	// --- trajectory browser (right pane) ---
	void  RefreshTaskFamilies();
	void  OnTaskFamilySelected(TSharedPtr<FString> Family, ESelectInfo::Type SelectInfo);
	void  LoadTrajectoriesFor(const FString& TaskFamily);
	TSharedRef<ITableRow> MakeTrajectoryRow(TSharedPtr<FTrajectory> Traj, const TSharedRef<STableViewBase>& Table);
	void  OnTrajectorySelected(TSharedPtr<FTrajectory> Traj, ESelectInfo::Type SelectInfo);

	// --- step list ---
	struct FStepRow
	{
		FString Phase;
		FString ToolName;
		FString Summary;
		bool    bSuccess = false;
		float   TimeSeconds = 0.0f;
	};
	TSharedRef<ITableRow> MakeStepRow(TSharedPtr<FStepRow> Row, const TSharedRef<STableViewBase>& Table);

	// State
	FString                              CachedReport;
	TArray<TSharedPtr<FString>>          TaskFamilyOptions;
	TSharedPtr<FString>                  SelectedTaskFamily;
	TArray<TSharedPtr<FTrajectory>>      Trajectories;
	TArray<TSharedPtr<FStepRow>>         Steps;

	TSharedPtr<SListView<TSharedPtr<FTrajectory>>> TrajectoryListView;
	TSharedPtr<SListView<TSharedPtr<FStepRow>>>    StepListView;
};
