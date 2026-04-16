// Copyright RiftbornAI. All Rights Reserved.
// BlueprintPatch - Atomic unit of Blueprint graph modification for copilot suggestions

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "BlueprintPatch.generated.h"

/**
 * Represents a node to add to a Blueprint graph.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlueprintNodeAddition
{
	GENERATED_BODY()
	
	/** Node class name (e.g., "UK2Node_CallFunction", "UK2Node_IfThenElse") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FString NodeClass;
	
	/** Human-readable node title (e.g., "Print String", "Branch") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FString NodeTitle;
	
	/** For CallFunction nodes: fully qualified function reference (e.g., "KismetSystemLibrary.PrintString") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
	FString FunctionReference;
	
	/** For event nodes: event name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString EventName;
	
	/** For variable get/set: variable name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FString VariableName;
	
	/** Desired graph position (will be auto-adjusted if invalid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	FVector2D Position = FVector2D::ZeroVector;
	
	/** Pin name -> default value literal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	TMap<FString, FString> PinDefaults;
	
	/** Local ID for referencing in wire additions (e.g., "NEW:0", "NEW:1") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString LocalId;
	
	/** Optional comment to display on node */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FString Comment;
	
	FBlueprintNodeAddition() = default;
};

/**
 * Represents a wire (connection) to add between pins.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlueprintWireAddition
{
	GENERATED_BODY()
	
	/** Source node - either existing node GUID or "NEW:0" for newly added nodes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	FString SourceNodeId;
	
	/** Source pin name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	FString SourcePinName;
	
	/** Target node - either existing node GUID or "NEW:1" for newly added nodes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	FString TargetNodeId;
	
	/** Target pin name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	FString TargetPinName;
	
	FBlueprintWireAddition() = default;
	
	FBlueprintWireAddition(const FString& InSourceNode, const FString& InSourcePin,
						   const FString& InTargetNode, const FString& InTargetPin)
		: SourceNodeId(InSourceNode)
		, SourcePinName(InSourcePin)
		, TargetNodeId(InTargetNode)
		, TargetPinName(InTargetPin)
	{
	}
};

/**
 * Represents a variable to add to the Blueprint.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlueprintVariableAddition
{
	GENERATED_BODY()
	
	/** Variable name (must be unique in Blueprint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FString VariableName;
	
	/** Type: "bool", "float", "int32", "FVector", "FString", "UObject*", or class path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FString VariableType;
	
	/** Default value as string literal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FString DefaultValue;
	
	/** Category in Blueprint's variable list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	FString Category;
	
	/** Whether this variable is editable per instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	bool bInstanceEditable = true;
	
	/** Whether this variable is exposed on spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	bool bExposeOnSpawn = false;
	
	/** Whether this variable is private */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
	bool bPrivate = false;
	
	/** Tooltip/description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FString Description;
	
	FBlueprintVariableAddition() = default;
};

/**
 * Represents a property change on an existing node.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlueprintPropertyChange
{
	GENERATED_BODY()
	
	/** Node GUID to modify */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	FString NodeId;
	
	/** Pin name to modify (if pin default value change) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	FString PinName;
	
	/** Property name (for node properties) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FString PropertyName;
	
	/** Old value (for diff display) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FString OldValue;
	
	/** New value to set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FString NewValue;
	
	FBlueprintPropertyChange() = default;
};

/**
 * Cursor semantics for Blueprint copilot.
 * Defines where the suggestion anchor is.
 */
UENUM(BlueprintType)
enum class EBlueprintCursorType : uint8
{
	/** No specific cursor - suggest entry points or events */
	None,
	
	/** A specific pin is selected/hovered - suggest wiring */
	Pin,
	
	/** A node is selected - suggest next exec flow */
	Node,
	
	/** Multiple nodes selected - suggest refactoring */
	MultipleNodes,
	
	/** Graph has compile error - suggest fix */
	CompileError
};

/**
 * Cursor context for suggestion generation.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlueprintCursor
{
	GENERATED_BODY()
	
	/** Type of cursor anchor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	EBlueprintCursorType CursorType = EBlueprintCursorType::None;
	
	/** If Pin: the pin's owning node GUID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FString AnchorNodeId;
	
	/** If Pin: the pin name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FString AnchorPinName;
	
	/** If Pin: the pin type (exec, bool, float, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FString AnchorPinType;
	
	/** If Pin: is this an input or output pin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	bool bAnchorPinIsInput = true;
	
	/** If Node or MultipleNodes: selected node GUIDs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	TArray<FString> SelectedNodeIds;
	
	/** If CompileError: the error message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FString CompileError;
	
	/** Graph position for placement suggestion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector2D SuggestedPosition = FVector2D::ZeroVector;
	
	/** Helper: Is cursor valid for suggestion */
	bool IsValid() const { return CursorType != EBlueprintCursorType::None || !CompileError.IsEmpty(); }
	
	FBlueprintCursor() = default;
};

/**
 * A complete Blueprint patch - the atomic unit of copilot suggestion.
 * 
 * Design principles:
 * - Patches are self-contained and can be applied or reverted atomically
 * - Patches can be previewed as "ghost" nodes before applying
 * - Patches serialize to JSON for LLM generation
 * - Patches support diff preview (show what will change)
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlueprintPatch
{
	GENERATED_BODY()
	
	// ========================================================================
	// Identity & Metadata
	// ========================================================================
	
	/** Unique ID for this patch (for tracking/undo) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FGuid PatchId = FGuid();
	
	/** Human-readable description (shown in UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Description;
	
	/** Cursor context this patch responds to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	FBlueprintCursor SourceCursor;
	
	/** Confidence score from LLM (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	float Confidence = 0.0f;
	
	/** Generation timestamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FDateTime GeneratedAt = FDateTime();
	
	// ========================================================================
	// Structural Changes
	// ========================================================================
	
	/** Nodes to add */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Changes")
	TArray<FBlueprintNodeAddition> NodesToAdd;
	
	/** Wires to add (connections between pins) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Changes")
	TArray<FBlueprintWireAddition> WiresToAdd;
	
	/** Variables to add to the Blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Changes")
	TArray<FBlueprintVariableAddition> VariablesToAdd;
	
	/** Properties to change on existing nodes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Changes")
	TArray<FBlueprintPropertyChange> PropertiesToChange;
	
	/** Node GUIDs to remove (for refactoring patches) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Changes")
	TArray<FString> NodeIdsToRemove;

	/** Node GUIDs whose wire topology changed (detected by FromDiff) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diff")
	TArray<FString> ModifiedNodeIds;

	/** Node GUIDs that moved position (detected by FromDiff) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diff")
	TArray<FString> MovedNodeIds;
	
	// ========================================================================
	// Methods
	// ========================================================================
	
	FBlueprintPatch() = default;

	/** Ensure identity fields are populated for newly created patches */
	void InitializeIdentity();
	
	/** Is this patch valid (has at least one change) */
	bool IsValid() const
	{
		return NodesToAdd.Num() > 0 ||
			   WiresToAdd.Num() > 0 ||
			   VariablesToAdd.Num() > 0 ||
			   PropertiesToChange.Num() > 0 ||
			   NodeIdsToRemove.Num() > 0 ||
			   ModifiedNodeIds.Num() > 0 ||
			   MovedNodeIds.Num() > 0;
	}
	
	/** Is this patch empty (no changes) */
	bool IsEmpty() const { return !IsValid(); }
	
	/** Total number of changes in this patch */
	int32 ChangeCount() const
	{
		return NodesToAdd.Num() + WiresToAdd.Num() + VariablesToAdd.Num() +
			   PropertiesToChange.Num() + NodeIdsToRemove.Num() +
			   ModifiedNodeIds.Num() + MovedNodeIds.Num();
	}
	
	/** Serialize to JSON for LLM output parsing */
	FString ToJSON() const;
	
	/** Serialize to compact JSON (for prompts) */
	FString ToCompactJSON() const;
	
	/** Parse from JSON (LLM output) */
	static bool FromJSON(const FString& JSON, FBlueprintPatch& OutPatch);
	
	/** Apply this patch to a Blueprint graph
	 * @param Blueprint The Blueprint to modify
	 * @param Graph The specific graph to modify (nullptr = event graph)
	 * @param OutErrors Any errors encountered
	 * @param OutCreatedNodes Nodes that were created (for undo)
	 * @return true if applied successfully
	 */
	bool Apply(class UBlueprint* Blueprint, class UEdGraph* Graph, 
			   TArray<FString>& OutErrors, TArray<UEdGraphNode*>& OutCreatedNodes) const;
	
	/** Validate this patch BEFORE applying (non-destructive check)
	 * Checks:
	 * - Node classes exist and can be instantiated
	 * - Function references resolve
	 * - Pin defaults parse correctly
	 * - All wire connections are schema-valid
	 * - Variables don't collide with existing ones
	 * - Graph is editable
	 * @param Blueprint The target Blueprint
	 * @param Graph The target graph (nullptr = event graph)
	 * @param OutErrors Validation errors (if any)
	 * @return true if patch can be safely applied
	 */
	bool Validate(class UBlueprint* Blueprint, class UEdGraph* Graph, TArray<FString>& OutErrors) const;
	
	/** Create ghost (preview) nodes without modifying the graph
	 * @param Graph The graph to create ghosts for
	 * @param OutGhostNodes Temporary nodes for preview (caller must clean up)
	 */
	void CreateGhostNodes(class UEdGraph* Graph, TArray<UEdGraphNode*>& OutGhostNodes) const;
	
	/** Generate patch from a pair of graph snapshots (before/after) */
	static FBlueprintPatch FromDiff(const TArray<UEdGraphNode*>& Before, const TArray<UEdGraphNode*>& After);
};

/**
 * Collection of patches (for multi-suggestion UX)
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBlueprintPatchSet
{
	GENERATED_BODY()
	
	/** Ordered list of suggestions (first = most confident) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patches")
	TArray<FBlueprintPatch> Patches;
	
	/** Currently selected index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	int32 SelectedIndex = 0;
	
	/** Get current patch */
	const FBlueprintPatch* GetCurrent() const
	{
		if (Patches.IsValidIndex(SelectedIndex))
		{
			return &Patches[SelectedIndex];
		}
		return nullptr;
	}
	
	/** Navigate to next suggestion */
	bool Next()
	{
		if (SelectedIndex < Patches.Num() - 1)
		{
			SelectedIndex++;
			return true;
		}
		return false;
	}
	
	/** Navigate to previous suggestion */
	bool Previous()
	{
		if (SelectedIndex > 0)
		{
			SelectedIndex--;
			return true;
		}
		return false;
	}
	
	bool IsEmpty() const { return Patches.Num() == 0; }
	int32 Num() const { return Patches.Num(); }
};
