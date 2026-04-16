// SRiftbornOnboardingPanel.h
// First-run registration screen.
//
// One job: get the user past the license gate.
//   - Full-bleed splash image as the background.
//   - Email-based beta signup (calls FRiftbornLicense::RequestBetaLicense).
//   - OR paste an existing license JSON and click Confirm.
// Old "environment detection + component checklist" panel was removed —
// it belongs in Settings, not first-run.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;
class SMultiLineEditableTextBox;
class STextBlock;

class RIFTBORNAI_API SRiftbornOnboardingPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRiftbornOnboardingPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnRequestBetaClicked();
	FReply OnInstallPastedKeyClicked();

	void SetStatus(const FString& Message, bool bError);

	TSharedPtr<SEditableTextBox>          EmailBox;
	TSharedPtr<SMultiLineEditableTextBox> LicenseKeyBox;
	TSharedPtr<STextBlock>                StatusLine;

	bool bBetaInFlight = false;
};
