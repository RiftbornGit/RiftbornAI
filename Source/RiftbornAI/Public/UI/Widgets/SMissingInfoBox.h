// Copyright RiftbornAI. All Rights Reserved.
// SMissingInfoBox.h - Widget showing missing required information before plan can execute

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

struct FMissingInfo;
class FRiftbornCopilotController;

/**
 * SMissingInfoBox - Displays missing required information
 * 
 * Shows above the plan editor when FConversationState has missing info.
 * Each item shows what's missing and prompts the user for clarification.
 */
class RIFTBORNAI_API SMissingInfoBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMissingInfoBox) {}
		SLATE_ARGUMENT(TSharedPtr<FRiftbornCopilotController>, Controller)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	/** Refresh from current conversation state */
	void RefreshFromState();
	
	/** Is there any missing info to display? */
	bool HasMissingInfo() const;

private:
	TSharedPtr<FRiftbornCopilotController> Controller;
	TSharedPtr<SVerticalBox> MissingItemsBox;
	
	/** Build a single missing info row */
	TSharedRef<SWidget> BuildMissingInfoRow(const FMissingInfo& Info, int32 Index);
	
	/** Build the header section */
	TSharedRef<SWidget> BuildHeader();
	
	/** Called when user provides info for a missing item */
	void OnInfoProvided(int32 Index, const FString& Value);
};
