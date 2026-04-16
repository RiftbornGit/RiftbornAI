// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class URiftbornBridgeMonitor;

/**
 * Bridge Status Widget
 * 
 * Displays real-time health status of the Python bridge:
 * - Connection state (Connected/Degraded/Offline)
 * - Success rate percentage
 * - Recent errors
 * - Last ping time
 * 
 * Updates every 2 seconds to reflect current state.
 */
class SBridgeStatusWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBridgeStatusWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SBridgeStatusWidget();

private:
	/** Periodic UI update ticker */
	FTSTicker::FDelegateHandle TickerHandle;
	
	/** Bridge monitor subsystem */
	URiftbornBridgeMonitor* Monitor;
	
	/** Cached state for UI updates */
	FText StatusText;
	FText StatusIcon;
	FSlateColor StatusColor;
	FText SuccessRateText;
	FText LastPingText;
	FText RecentErrorsText;
	
	/** Update UI with latest bridge state (called every 2 seconds) */
	bool UpdateBridgeStatus(float DeltaTime);
	
	/** Refresh button handler */
	FReply OnRefreshClicked();
	
	/** Get color for current bridge state */
	FSlateColor GetStatusColor() const;
	
	/** Get status icon text */
	FText GetStatusIcon() const;
	
	/** Get formatted success rate text */
	FText GetSuccessRateText() const;
	
	/** Get recent errors formatted for display */
	FText GetRecentErrorsText() const;
};
