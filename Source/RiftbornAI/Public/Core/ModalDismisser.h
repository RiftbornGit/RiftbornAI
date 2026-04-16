#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"

class SButton;

/**
 * Safe modal dialog dismissal via Slate widget tree traversal.
 *
 * Instead of RequestDestroyWindow() (which causes HWND invalidation / D3D12
 * swapchain crashes), we find the dialog's dismiss button and execute its
 * Slate click delegate directly. The window destroys itself via its own
 * handler without relying on brittle synthetic pointer routing.
 */
class RIFTBORNAI_API FModalDismisser
{
public:
	struct FDismissResult
	{
		bool bDismissed = false;
		FString Classification;
		FString WindowTitle;
		FString ButtonClicked;
		FString FailReason;
	};

	/** Attempt to dismiss a specific modal by classification */
	static FDismissResult TryDismissModal(const FString& Classification);

	/** Attempt to dismiss a specific modal using caller-provided button text patterns. */
	static FDismissResult TryDismissModalWithButtonPatterns(
		const FString& Classification,
		const TArray<FString>& ButtonTextPatterns);

	/** Attempt to dismiss all known safe-to-dismiss modals. Returns one result per modal found. */
	static TArray<FDismissResult> DismissAllKnownModals();

	/** Check if a classification is safe to auto-dismiss (known dialog with known button pattern) */
	static bool IsSafeToAutoDismiss(const FString& Classification);

	/** Get the button text patterns for a given classification */
	static TArray<FString> GetButtonPatternsForClassification(const FString& Classification);

	/** Walk a window's widget tree for an SButton whose text matches a pattern */
	static TSharedPtr<SButton> FindButtonByText(
		TSharedRef<SWindow> Window,
		const TArray<FString>& ButtonTextPatterns);

	/** Activate a button through its Slate click delegate */
	static bool SimulateButtonClick(TSharedPtr<SButton> Button);

	/** Extract text from a widget (checks STextBlock children) */
	static FString GetWidgetText(TSharedRef<SWidget> Widget);

private:
	/** Recursively search widget tree for SButton with matching text */
	static TSharedPtr<SButton> FindButtonRecursive(
		TSharedRef<SWidget> Widget,
		const TArray<FString>& ButtonTextPatterns);
};
