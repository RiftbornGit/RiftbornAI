// Copyright RiftbornAI. All Rights Reserved.
// BlueprintFocusTracker - Track focused Blueprint editor, graph, node, and pin
//
// This replaces scanning open assets with explicit focus tracking.
// Key insight: We don't "discover" focus - we TRACK it via events.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphPin.h"

class SGraphEditor;
class UBlueprint;
class UEdGraph;
class UEdGraphNode;
class FBlueprintEditor;

/**
 * Captured pin context from last click.
 * This is the "cursor" for Blueprint copilot.
 */
struct RIFTBORNAI_API FCapturedPinContext
{
	/** The node owning the pin */
	TWeakObjectPtr<UEdGraphNode> Node;
	
	/** Pin name (stable identifier) */
	FName PinName;
	
	/** Pin direction */
	EEdGraphPinDirection Direction = EGPD_Output;
	
	/** Pin category (exec, bool, float, object, etc) */
	FName PinCategory;
	
	/** Pin subcategory (for object types) */
	FName PinSubCategory;
	
	/** Timestamp of capture (for staleness detection) */
	double CaptureTime = 0.0;
	
	/** Position in graph space where click occurred */
	FVector2D GraphPosition = FVector2D::ZeroVector;
	
	/** Is this context still valid? */
	bool IsValid() const;
	
	/** Clear the context */
	void Clear();
	
	/** Get a debug string */
	FString ToString() const;
};

/**
 * FBlueprintFocusTracker - Singleton that tracks the currently focused Blueprint editor.
 * 
 * DESIGN PRINCIPLES:
 * 1. Never scan assets to find focus - track it via events
 * 2. Use weak pointers - editors/graphs can close at any time
 * 3. O(1) access to current focus state
 * 4. "Cursor" = last clicked pin (not hover, not "selected")
 * 
 * HOW IT WORKS:
 * 1. On Blueprint editor open: register weak pointer
 * 2. On graph editor focus: update ActiveGraphEditor
 * 3. On mouse down in graph: capture clicked pin via IInputProcessor
 * 4. On selection change: update SelectedNodeGuids
 * 5. On editor close: clear weak pointers
 * 
 * USAGE:
 * - FBlueprintFocusTracker::Get().GetActiveGraphEditor() -> O(1)
 * - FBlueprintFocusTracker::Get().GetCursorPin() -> last clicked pin
 * - FBlueprintFocusTracker::Get().GetSelectedNodes() -> current selection
 */
class RIFTBORNAI_API FBlueprintFocusTracker
{
public:
	static FBlueprintFocusTracker& Get();
	
	// === Lifecycle ===
	
	/** Initialize tracking - call from module startup */
	void Initialize();
	
	/** Shutdown tracking - call from module shutdown */
	void Shutdown();
	
	// === Focus State (O(1) access) ===
	
	/** Get the currently focused graph editor widget (may be null) */
	TSharedPtr<SGraphEditor> GetActiveGraphEditor() const;
	
	/** Get the currently focused Blueprint (may be null) */
	UBlueprint* GetActiveBlueprint() const;
	
	/** Get the currently focused graph (may be null) */
	UEdGraph* GetActiveGraph() const;
	
	/** Get the focused Blueprint editor instance (may be null) */
	FBlueprintEditor* GetActiveBlueprintEditor() const;
	
	/** Is a Blueprint editor currently focused? */
	bool HasActiveBlueprintEditor() const;
	
	// === Cursor State ===
	
	/** Get the last clicked pin context (the "cursor") */
	const FCapturedPinContext& GetCursorPin() const { return CursorPin; }
	
	/** Does cursor have a valid pin? */
	bool HasCursorPin() const { return CursorPin.IsValid(); }
	
	/** Clear the cursor pin */
	void ClearCursorPin();
	
	// === Selection State ===
	
	/** Get currently selected node GUIDs */
	const TArray<FGuid>& GetSelectedNodeGuids() const { return SelectedNodeGuids; }
	
	/** Get number of selected nodes */
	int32 GetSelectedNodeCount() const { return SelectedNodeGuids.Num(); }
	
	/** Get the primary selected node (first in selection) */
	UEdGraphNode* GetPrimarySelectedNode() const;
	
	// === Events (for cache invalidation) ===
	
	DECLARE_MULTICAST_DELEGATE(FOnFocusChanged);
	DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged);
	DECLARE_MULTICAST_DELEGATE(FOnCursorPinChanged);
	
	/** Fires when active graph editor changes */
	FOnFocusChanged OnFocusChanged;
	
	/** Fires when node selection changes */
	FOnSelectionChanged OnSelectionChanged;
	
	/** Fires when cursor pin changes (click on pin) */
	FOnCursorPinChanged OnCursorPinChanged;
	
	// === Internal (called by hooks) ===
	
	/** Called when a Blueprint editor is opened */
	void NotifyBlueprintEditorOpened(FBlueprintEditor* Editor);

	/** Called when a Blueprint asset editor is closed */
	void NotifyBlueprintAssetClosed(UBlueprint* Blueprint);
	
	/** Called when a graph editor gains focus */
	void NotifyGraphEditorFocused(TSharedPtr<SGraphEditor> GraphEditor, UEdGraph* Graph, UBlueprint* Blueprint);
	
	/** Called when selection changes in the active graph */
	void NotifySelectionChanged(const TSet<UObject*>& SelectedObjects);
	
	/** Called when a pin is clicked (from input processor) */
	void NotifyPinClicked(UEdGraphNode* Node, FName PinName, EEdGraphPinDirection Direction, 
						  FName PinCategory, FVector2D GraphPosition);

private:
	FBlueprintFocusTracker() = default;
	~FBlueprintFocusTracker() = default;
	
	// Prevent copying
	FBlueprintFocusTracker(const FBlueprintFocusTracker&) = delete;
	FBlueprintFocusTracker& operator=(const FBlueprintFocusTracker&) = delete;
	
	/** Register hooks with Blueprint editor module */
	void RegisterEditorHooks();
	
	/** Unregister hooks */
	void UnregisterEditorHooks();
	
	/** Register the input processor for pin click capture */
	void RegisterInputProcessor();
	
	/** Unregister input processor */
	void UnregisterInputProcessor();
	
	// === Tracked State ===
	
	/** Currently focused graph editor widget */
	TWeakPtr<SGraphEditor> ActiveGraphEditorWeak;
	
	/** Currently focused Blueprint */
	TWeakObjectPtr<UBlueprint> ActiveBlueprintWeak;
	
	/** Currently focused graph */
	TWeakObjectPtr<UEdGraph> ActiveGraphWeak;
	
	/** Currently focused Blueprint editor instance */
	TWeakPtr<FBlueprintEditor> ActiveBlueprintEditorWeak;
	
	/** Currently selected node GUIDs */
	TArray<FGuid> SelectedNodeGuids;
	
	/** Last clicked pin (the cursor) */
	FCapturedPinContext CursorPin;
	
	/** Input processor for pin click capture */
	TSharedPtr<class FBlueprintPinInputProcessor> InputProcessor;
	
	/** Delegate handles for cleanup */
	FDelegateHandle OnAssetOpenedInEditorHandle;
	FDelegateHandle OnAssetClosedInEditorHandle;
	
	/** Is initialized? */
	bool bIsInitialized = false;
};
