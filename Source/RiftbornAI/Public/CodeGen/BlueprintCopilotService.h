// Copyright RiftbornAI. All Rights Reserved.
// BlueprintCopilotService - Manages inline suggestions in Blueprint editor

#pragma once

#include "CoreMinimal.h"
#include "BlueprintPatch.h"
#include "EditorContextService.h"

class UBlueprint;
class UEdGraph;
class FBlueprintEditor;
class IInputProcessor;

/**
 * Singleton service that manages Blueprint copilot suggestions.
 * 
 * Responsibilities:
 * - Handle Ctrl+Space trigger
 * - Request suggestions from LLM
 * - Manage overlay display
 * - Apply/reject patches
 */
class RIFTBORNAI_API FBlueprintCopilotService
{
public:
	static FBlueprintCopilotService& Get();
	
	// === Initialization ===
	
	/** Register keyboard shortcut handlers */
	void RegisterCommands();
	
	/** Unregister handlers */
	void UnregisterCommands();
	
	// === Trigger ===
	
	/** Called when Ctrl+Space is pressed - initiates suggestion flow */
	void OnTriggerSuggestion();
	
	/** Check if we're in a valid context for suggestions */
	bool CanTriggerSuggestion() const;
	
	// === Suggestion Flow ===
	
	/** Request a suggestion from the LLM based on current context */
	void RequestSuggestion(const FBlueprintGraphContext& Context);
	
	/** Called when LLM returns suggestions */
	void OnSuggestionsReceived(const TArray<FBlueprintPatch>& Patches);
	
	/** Called on LLM error */
	void OnSuggestionError(const FString& Error);
	
	// === Overlay Control ===
	
	/** Show the suggestion overlay with current patches */
	void ShowOverlay();
	
	/** Hide the overlay */
	void HideOverlay();
	
	/** Accept current suggestion (Tab) */
	void AcceptSuggestion();
	
	/** Reject/dismiss suggestion (Esc) */
	void RejectSuggestion();
	
	/** Navigate to next suggestion (if multiple) */
	void NextSuggestion();
	
	/** Navigate to previous suggestion */
	void PreviousSuggestion();
	
	// === State ===
	
	/** Are we currently showing a suggestion? */
	bool IsShowingSuggestion() const { return bShowingSuggestion; }
	
	/** Is a suggestion request in flight? */
	bool IsRequestInFlight() const { return bRequestInFlight; }
	
	/** Get current patch set */
	const FBlueprintPatchSet& GetCurrentPatches() const { return CurrentPatches; }
	
	/** Cancel any in-flight LLM request */
	void CancelRequest();
	
	/** Set streaming mode on/off */
	void SetStreamingEnabled(bool bEnabled) { bStreamingEnabled = bEnabled; }
	
	/** Set debounce delay for selection changes (ms) */
	void SetDebounceDelay(int32 DelayMs) { DebounceDelayMs = DelayMs; }

private:
	FBlueprintCopilotService();
	~FBlueprintCopilotService();
	
	// Get the focused Blueprint editor (if any)
	FBlueprintEditor* GetFocusedBlueprintEditor() const;
	
	// Build LLM prompt from context
	FString BuildPromptFromContext(const FBlueprintGraphContext& Context);
	
	// Inject/remove overlay from graph editor window
	void InjectOverlayIntoGraphEditor(TSharedPtr<class SGraphEditor> GraphEditor);
	
	// Calculate anchor position for ghost nodes based on cursor context
	FVector2D CalculateAnchorPosition(TSharedPtr<class SGraphEditor> GraphEditor);
	
	// Process complete LLM response into patches
	void ProcessLLMResponse(uint32 RequestId, const FString& Response);
	
	// Debounced trigger (called after debounce timer)
	void DebouncedTrigger();
	
	// State
	bool bShowingSuggestion = false;
	bool bRequestInFlight = false;
	bool bOverlayInjected = false;
	bool bStreamingEnabled = true;
	FBlueprintPatchSet CurrentPatches;
	
	// Streaming state
	FString StreamingBuffer;
	bool bStreamingInProgress = false;
	
	// Debounce state
	int32 DebounceDelayMs = 300;
	FTimerHandle DebounceTimerHandle;
	bool bDebouncePending = false;
	
	// The Blueprint/Graph we're suggesting for
	TWeakObjectPtr<UBlueprint> TargetBlueprintWeak;
	TWeakObjectPtr<UEdGraph> TargetGraphWeak;
	
	// Request tracking for cancellation
	uint32 CurrentRequestId = 0;
	
	// Overlay widget (created on demand)
	TSharedPtr<class SBlueprintCopilotOverlay> OverlayWidget;
	
	// Window where overlay was injected
	TWeakPtr<class SWindow> InjectedWindowWeak;
	
	// AI provider for suggestions (uses user's configured provider/model)
	TSharedPtr<class IAIProvider> AIProvider;
	
	// Input processor (stored for cleanup)
	TSharedPtr<IInputProcessor> CopilotInputProcessor;
	
	// Command bindings
	TSharedPtr<FUICommandList> CommandList;
};
