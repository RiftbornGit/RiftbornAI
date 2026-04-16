// Copyright RiftbornAI. All Rights Reserved.
// AutoSuggestionTrigger - Monitors graph events and triggers suggestions automatically
// Gap #1 complement: Bridges the gap from "press Alt+Space" to "suggestions appear as you work"

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"

class UEdGraph;
class UEdGraphNode;
class FBlueprintEditor;

/**
 * Trigger conditions for auto-suggestions.
 */
enum class EAutoSuggestTrigger : uint8
{
	/** Node was just placed in the graph */
	NodePlaced,
	
	/** Selection changed (user clicked a different node) */
	SelectionChanged,
	
	/** Wire was connected between two pins */
	WireConnected,
	
	/** Wire was disconnected */
	WireDisconnected,
	
	/** Graph was opened or switched */
	GraphOpened,
	
	/** User paused (stopped typing/moving) for debounce period */
	IdlePause
};

/**
 * Auto-Suggestion Trigger
 * 
 * This system monitors Blueprint graph events and automatically triggers
 * the copilot suggestion flow when the context changes.
 * 
 * Unlike Alt+Space (manual trigger), this fires automatically:
 *   - When user places a new node → suggest what to wire to
 *   - When user selects a node → suggest continuation
 *   - When user connects/disconnects a wire → suggest next step
 *   - After idle period → preemptive suggestion
 * 
 * Debounce ensures we don't spam the LLM. Only fires after the user
 * stops making changes for DebounceMs (default: 500ms).
 * 
 * This is the key UX feature that makes it feel like Copilot:
 * the user doesn't have to ask — suggestions just appear.
 */
class RIFTBORNAI_API FAutoSuggestionTrigger : public FTickableGameObject
{
public:
	static FAutoSuggestionTrigger& Get();
	
	/** Start monitoring for auto-suggest triggers */
	void Enable();
	
	/** Stop monitoring */
	void Disable();
	
	/** Is auto-suggest enabled? */
	bool IsEnabled() const { return bEnabled; }
	
	/** Set debounce delay (milliseconds) */
	void SetDebounceMs(int32 Ms) { DebounceMs = FMath::Max(100, Ms); }
	
	/** Get debounce delay */
	int32 GetDebounceMs() const { return DebounceMs; }
	
	/** Set whether to only suggest when a Blueprint editor is focused */
	void SetRequireBlueprintFocus(bool bRequire) { bRequireBPFocus = bRequire; }
	
	/** Manually notify of a graph event (for integration with existing systems) */
	void NotifyEvent(EAutoSuggestTrigger Trigger, UEdGraph* Graph = nullptr, UEdGraphNode* Node = nullptr);
	
	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	
private:
	FAutoSuggestionTrigger();
	~FAutoSuggestionTrigger();
	
	/** Register delegates for graph change events */
	void RegisterDelegates();
	
	/** Unregister delegates */
	void UnregisterDelegates();
	
	/** Called when a node is added to a graph */
	void OnNodeAdded(UEdGraphNode* Node);
	
	/** Called when selection changes in a Blueprint editor */
	void OnSelectionChanged();
	
	/** Fire the suggestion through BlueprintCopilotService */
	void FireSuggestion();
	
	/** Check if conditions are met to fire */
	bool ShouldFire() const;
	
	/** Track the last known selection to detect real changes */
	TArray<TWeakObjectPtr<UEdGraphNode>> LastSelectedNodes;
	
	/** State */
	bool bEnabled = false;
	bool bRequireBPFocus = true;
	bool bPendingFire = false;
	float TimeSinceLastEvent = 0.0f;
	int32 DebounceMs = 500;
	
	/** Last trigger type (for logging) */
	EAutoSuggestTrigger LastTrigger = EAutoSuggestTrigger::IdlePause;
	
	/** Cooldown: don't fire again within this window after a fire */
	float CooldownSeconds = 3.0f;
	float TimeSinceLastFire = 999.0f;
	
	/** Delegate handles for cleanup */
	FDelegateHandle OnNodeAddedHandle;
	FDelegateHandle OnSelectionChangedHandle;
};
