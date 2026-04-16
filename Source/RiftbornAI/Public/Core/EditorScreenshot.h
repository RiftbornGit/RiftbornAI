// Copyright RiftbornAI. All Rights Reserved.
//
// Centralised editor-viewport screenshot helper. Replaces the half-dozen
// near-identical `static bool *VCapCaptureSync(...)` functions that were
// scattered across Tools/ and ToolImpl/ — each duplicating the same
// FScreenshotRequest + 50ms-sleep polling anti-pattern that froze the
// editor visibly during AI-driven tool runs.
//
// The implementation uses UGameViewportClient::OnScreenshotCaptured to
// know exactly when the readback lands (rather than polling for the file
// to appear on disk), and pumps Slate while waiting so the editor stays
// visibly responsive. The GT is still blocked for the readback duration —
// that's a UE constraint we can't move around — but the editor no longer
// presents a frozen frame.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

namespace RiftbornEditorScreenshot
{
	/** Capture the active editor viewport to a PNG at OutPath.
	 *
	 *  Subscribes to the engine screenshot delegate, queues the request,
	 *  and pumps Slate (PumpMessages + Tick + FlushRenderingCommands) on
	 *  every wait iteration so the editor remains responsive while the
	 *  GPU readback is in flight.
	 *
	 *  @return true on success (file written), false on timeout / encode
	 *          failure / no active editor. Errors are logged.
	 *
	 *  Must be called on the game thread.
	 */
	RIFTBORNAI_API bool CaptureEditorViewport(const FString& OutPath, float TimeoutSeconds = 3.0f);

	/** True async variant — does NOT block the game thread. Subscribes to
	 *  UGameViewportClient::OnScreenshotCaptured, queues the request, and
	 *  returns immediately. OnComplete fires on the game thread when the
	 *  GPU readback lands and the PNG has been written (or the timeout has
	 *  fired). The continuation receives bSuccess so callers can branch
	 *  on capture failure without re-checking file existence.
	 *
	 *  Use this from contexts that can yield control back to the editor
	 *  while the screenshot is in flight — primarily the agentic loop's
	 *  async-registered screenshot tools.
	 */
	DECLARE_DELEGATE_OneParam(FOnEditorScreenshotComplete, bool /*bSuccess*/);
	RIFTBORNAI_API void CaptureEditorViewportAsync(
		const FString& OutPath,
		FOnEditorScreenshotComplete OnComplete,
		float TimeoutSeconds = 5.0f);
}
