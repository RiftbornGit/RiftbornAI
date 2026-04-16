// Copyright RiftbornAI. All Rights Reserved.
// Vision Capture Tools — frame sequence, depth buffer, resolution control

#pragma once

#include "CoreMinimal.h"
#include "Tools/ToolModuleBase.h"
#include "Core/ClaudeToolUse_Types.h"  // FOnAsyncToolComplete

/**
 * FVisionCaptureToolsModule
 * 
 * Vision Capture Tools — frame sequence, depth buffer, resolution control
 */
class RIFTBORNAI_API FVisionCaptureToolsModule : public TToolModuleBase<FVisionCaptureToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("VisionCapture"); }
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Feature 6: Resolution-controlled capture
	static FClaudeToolResult Tool_CaptureAtResolution(const FClaudeToolCall& Call);

	// Feature 4: Depth buffer spatial reasoning
	static FClaudeToolResult Tool_CaptureDepthBuffer(const FClaudeToolCall& Call);

	// Feature 2: Frame sequence capture (video/animation review)
	static FClaudeToolResult Tool_CaptureFrameSequence(const FClaudeToolCall& Call);

	// 360 orbit capture with multi-angle vision analysis
	static FClaudeToolResult Tool_VerifyScene360(const FClaudeToolCall& Call);
	/** Async sibling — captures still inline (SceneCapture2D requires GT) but
	 *  pumps the editor between frames and runs vision analysis off-thread. */
	static void Tool_VerifyScene360Async(const FClaudeToolCall& Call, FOnAsyncToolComplete OnComplete);

	// Vision-scored scene quality assessment
	static FClaudeToolResult Tool_ScoreSceneQuality(const FClaudeToolCall& Call);
	/** Async sibling — vision LLM call moved to a worker thread. */
	static void Tool_ScoreSceneQualityAsync(const FClaudeToolCall& Call, FOnAsyncToolComplete OnComplete);

	// Before/after scene comparison via vision
	static FClaudeToolResult Tool_CompareSceneStates(const FClaudeToolCall& Call);
	/** Async sibling — vision LLM call moved to a worker thread. */
	static void Tool_CompareSceneStatesAsync(const FClaudeToolCall& Call, FOnAsyncToolComplete OnComplete);

	// Reference-image decomposition: sends a pasted reference image to the
	// vision LLM, receives a structured scene graph (biome, hero structures,
	// foliage, characters, camera framing, time of day, palette), caches it
	// per editor session so downstream tools can consult it instead of
	// drifting from the original reference during long agent runs.
	static FClaudeToolResult Tool_AnalyzeReferenceImage(const FClaudeToolCall& Call);

	// Viewport capture with per-actor label overlay. Renders the scene, then
	// projects each visible actor's centroid to pixel space and draws its
	// label on top via UCanvas. Vision LLMs can read the labels, so the
	// agent stops guessing "which object is which" from silhouette alone.
	static FClaudeToolResult Tool_CaptureWithLabels(const FClaudeToolCall& Call);

	// Viewport capture plus a JSON sidecar describing what's actually in
	// frame (camera pose/FOV, visible actors with class/location/bounds,
	// visible lights). LLM has pixel truth AND structured truth.
	static FClaudeToolResult Tool_CaptureWithMetadata(const FClaudeToolCall& Call);

	// Three orthogonal views (front / side / top) of a target actor
	// stitched into a single wide PNG. Lets the LLM reason about 3D shape
	// without orbit-capture round trips.
	static FClaudeToolResult Tool_CaptureTriptych(const FClaudeToolCall& Call);
};
