// Copyright RiftbornAI. All Rights Reserved.
// Agent Timeline Widget - Real-time event stream visualization
//
// Shows the agent's work as it happens:
// - Event stream updates in real-time
// - Colored by event type (read=blue, edit=yellow, build=orange, etc.)
// - Collapsible sections for long event sequences
// - Error events highlighted in red
//
// EVENT SOURCE (2026-02-03):
// FAgentEventStream - unified canonical stream (fed by ToolExecutionBroadcaster)

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Agent/AgentEvent.h"
#include "Agent/AgentEventStream.h"
#include "Agent/AgentTaskRunner.h"

/**
 * Display data for a single timeline event (UI layer)
 */
struct FTimelineEventVM
{
	FGuid EventId;
	EAgentEventType Type;
	FString Summary;
	FString Details;
	FDateTime Timestamp;
	bool bSuccess = true;
	bool bExpanded = false;
	
	// For grouping
	FString Category;  // "read", "edit", "build", "test", "verify"
	
	static FTimelineEventVM FromAgentEvent(const FAgentEvent& Event);
};

/**
 * SAgentTimeline - Real-time event visualization
 * 
 * ARCHITECTURE:
 * - Subscribes to FAgentEventStream for live updates
 * - Uses SListView with virtualization for performance
 * - Color-codes events by type
 * - Shows progress/state in header
 * 
 * USAGE:
 * - Embedded in main Copilot panel
 * - Shows "Observable Work" - what the agent is doing
 * - Enables "proof of work" auditing
 */
class RIFTBORNAI_API SAgentTimeline : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAgentTimeline) {}
		SLATE_ARGUMENT(TSharedPtr<FAgentTaskRunner>, TaskRunner)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SAgentTimeline();
	
	/** Clear all events and reset */
	void Clear();
	
	/** Bind to a new event stream */
	void BindToStream(TSharedPtr<FAgentEventStream> Stream);

private:
	// === Event Stream ===
	TSharedPtr<FAgentEventStream> EventStream;
	TWeakPtr<FAgentTaskRunner> WeakRunner;
	FDelegateHandle StreamSubscriptionHandle;
	
	// === Display Data ===
	TArray<TSharedPtr<FTimelineEventVM>> EventItems;
	TSharedPtr<SListView<TSharedPtr<FTimelineEventVM>>> EventListView;
	
	// === State ===
	ETaskRunnerState CurrentState = ETaskRunnerState::Idle;
	int32 TotalEvents = 0;
	int32 ErrorCount = 0;
	bool bAutoScroll = true;
	
	// === ListView ===
	
	TSharedRef<ITableRow> GenerateEventRow(
		TSharedPtr<FTimelineEventVM> Item,
		const TSharedRef<STableViewBase>& OwnerTable);
	
	void OnEventSelectionChanged(
		TSharedPtr<FTimelineEventVM> Selected,
		ESelectInfo::Type SelectInfo);

	// === Stream Handlers ===
	
	/** Called when new event arrives */
	void OnEventReceived(const FAgentEvent& Event);
	
	/** Called when runner state changes */
	void OnRunnerStateChanged(ETaskRunnerState NewState);
	

	// === UI Builders ===
	
	/** Header showing state and progress */
	TSharedRef<SWidget> BuildHeader();
	
	/** Footer with controls */
	TSharedRef<SWidget> BuildFooter();
	
	/** Update header to reflect current state */
	void RefreshHeader();

	// === Helpers ===
	
	/** Get icon for event type */
	const FSlateBrush* GetEventIcon(EAgentEventType Type) const;
	
	/** Get color for event type */
	FSlateColor GetEventColor(EAgentEventType Type) const;
	
	/** Get category label for event type */
	FString GetEventCategory(EAgentEventType Type) const;
	
	/** Format timestamp relative to task start */
	FText FormatRelativeTime(const FDateTime& Time) const;
	
	/** Scroll to bottom (if auto-scroll enabled) */
	void ScrollToLatest();

	// === Header Widgets (for updating) ===
	TSharedPtr<STextBlock> StateText;
	TSharedPtr<STextBlock> ProgressText;
	TSharedPtr<SProgressBar> ProgressBar;
	
	// Task start time for relative timestamps
	FDateTime TaskStartTime;
};

/**
 * Single row in the timeline
 */
class RIFTBORNAI_API STimelineEventRow : public SMultiColumnTableRow<TSharedPtr<FTimelineEventVM>>
{
public:
	SLATE_BEGIN_ARGS(STimelineEventRow) {}
		SLATE_ARGUMENT(TSharedPtr<FTimelineEventVM>, Event)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable);
	
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FTimelineEventVM> EventVM;
	
	FSlateColor GetRowColor() const;
	const FSlateBrush* GetStatusIcon() const;
};
