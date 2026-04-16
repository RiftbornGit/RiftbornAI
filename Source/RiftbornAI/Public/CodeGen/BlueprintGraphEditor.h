// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "BlueprintEditor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "BlueprintGraphEditor.generated.h"

/**
 * Wire routing style for Blueprint connections
 */
UENUM(BlueprintType)
enum class EBlueprintWireStyle : uint8
{
	Default     UMETA(DisplayName = "Default (Curved)"),
	Manhattan   UMETA(DisplayName = "Manhattan (90° angles)"),
	Subway      UMETA(DisplayName = "Subway (45° angles)")
};

/**
 * Information about a Blueprint node to be created
 */
USTRUCT(BlueprintType)
struct FBPNodeCreationInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Blueprint")
	FString NodeType;  // Function, Event, Variable, Branch, Sequence, etc.

	UPROPERTY(BlueprintReadWrite, Category = "Blueprint")
	FString NodeName;  // For functions/events

	UPROPERTY(BlueprintReadWrite, Category = "Blueprint")
	FVector2D Position;  // X, Y coordinates

	UPROPERTY(BlueprintReadWrite, Category = "Blueprint")
	TMap<FString, FString> Properties;  // Additional properties

	FBPNodeCreationInfo()
		: Position(FVector2D::ZeroVector)
	{}
};

/**
 * Information about a pin connection
 */
USTRUCT(BlueprintType)
struct FBPPinConnectionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Blueprint")
	int32 SourceNodeIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Blueprint")
	FString SourcePinName;

	UPROPERTY(BlueprintReadWrite, Category = "Blueprint")
	int32 TargetNodeIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Blueprint")
	FString TargetPinName;

	FBPPinConnectionRequest()
		: SourceNodeIndex(-1)
		, TargetNodeIndex(-1)
	{}
};

/**
 * Blueprint Graph Editor - Create and edit Blueprint graphs programmatically
 * 
 * Features:
 * - Create nodes (functions, events, variables, branches)
 * - Connect pins with exec/data wires
 * - Position nodes with automatic layout
 * - Manhattan/Subway/Default wire routing styles
 * - Compile and validate Blueprints
 * 
 * Example usage:
 *   Editor->CreateBlueprint("BP_MyActor", "Actor");
 *   Editor->AddNode("BeginPlay", "Event");
 *   Editor->AddNode("PrintString", "Function");
 *   Editor->ConnectNodes(0, "exec", 1, "execute");
 *   Editor->CompileBlueprint();
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UBlueprintGraphEditor : public UObject
{
	GENERATED_BODY()

public:
	UBlueprintGraphEditor();

	/**
	 * Create a new Blueprint asset
	 * 
	 * @param BlueprintName - Name of the Blueprint (e.g., "BP_MyActor")
	 * @param ParentClassName - Parent class (e.g., "Actor", "Character", "Pawn")
	 * @param PackagePath - Package path (e.g., "/Game/Blueprints")
	 * @return True if created successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool CreateBlueprint(const FString& BlueprintName, const FString& ParentClassName, const FString& PackagePath);

	/**
	 * Open existing Blueprint for editing
	 * 
	 * @param BlueprintPath - Asset path to Blueprint
	 * @return True if loaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool OpenBlueprint(const FString& BlueprintPath);

	/**
	 * Check if a Blueprint is currently open for editing
	 * 
	 * @return True if a Blueprint is open
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool HasBlueprintOpen() const;

	/**
	 * Get the current Blueprint being edited
	 * 
	 * @return The current Blueprint or nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	UBlueprint* GetCurrentBlueprint() const;

	/**
	 * Add a node to the Blueprint graph
	 * 
	 * @param NodeInfo - Information about the node to create
	 * @return Index of created node (-1 if failed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	int32 AddNode(const FBPNodeCreationInfo& NodeInfo);

	/**
	 * Connect two nodes via their pins
	 * 
	 * @param SourceNodeIndex - Index of source node
	 * @param SourcePinName - Name of source pin (e.g., "exec", "ReturnValue")
	 * @param TargetNodeIndex - Index of target node
	 * @param TargetPinName - Name of target pin (e.g., "execute", "Condition")
	 * @return True if connected successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool ConnectNodes(int32 SourceNodeIndex, const FString& SourcePinName, int32 TargetNodeIndex, const FString& TargetPinName);

	/**
	 * Set wire routing style
	 * 
	 * @param Style - Wire routing style (Default/Manhattan/Subway)
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	void SetWireStyle(EBlueprintWireStyle Style);

	/**
	 * Auto-arrange nodes using force-directed layout
	 * 
	 * @param Spacing - Spacing between nodes (default 400 units)
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	void AutoArrangeNodes(float Spacing = 400.0f);

	/**
	 * Compile the Blueprint
	 *
	 * @return True if compiled successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool CompileBlueprint();

	/**
	 * Compile the Blueprint with error reporting
	 *
	 * @param OutErrors - Array to receive compilation error messages
	 * @param OutWarnings - Array to receive compilation warning messages
	 * @return True if compiled successfully (no errors)
	 */
	bool CompileBlueprintWithErrors(TArray<FString>& OutErrors, TArray<FString>& OutWarnings);

	/**
	 * Save the Blueprint asset
	 * 
	 * @return True if saved successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool SaveBlueprint();

	/**
	 * Get list of all nodes in graph
	 * 
	 * @return Array of node information
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	TArray<FBPNodeCreationInfo> GetNodes() const;

	/**
	 * Get list of all connections in graph
	 * 
	 * @return Array of pin connections
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	TArray<FBPPinConnectionRequest> GetConnections() const;

	/**
	 * Clear all nodes from graph
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	void ClearGraph();

	/**
	 * Set a default value on a node's input pin
	 * 
	 * @param NodeIndex - Index of the node
	 * @param PinName - Name of the input pin
	 * @param Value - String value to set
	 * @return True if set successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool SetPinValue(int32 NodeIndex, const FString& PinName, const FString& Value);

	/**
	 * Add a variable to the Blueprint
	 * 
	 * @param VariableName - Name of the variable
	 * @param VariableType - Type (bool, int, float, FString, FVector, etc.)
	 * @param DefaultValue - Optional default value as string
	 * @param bInstanceEditable - Whether to expose to editor
	 * @return True if added successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool AddVariable(const FString& VariableName, const FString& VariableType, const FString& DefaultValue = TEXT(""), bool bInstanceEditable = true);

	/**
	 * Add a custom function to the Blueprint
	 * 
	 * @param FunctionName - Name of the function
	 * @param ReturnType - Return type (void, bool, int, float, etc.)
	 * @param bPureFunction - Whether this is a pure function (no side effects)
	 * @return True if added successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Blueprint Graph Editor")
	bool AddFunction(const FString& FunctionName, const FString& ReturnType = TEXT("void"), bool bPureFunction = false);

private:
	// Current Blueprint being edited
	UPROPERTY()
	UBlueprint* CurrentBlueprint;

	// Current event graph
	UPROPERTY()
	UEdGraph* CurrentGraph;

	// Created nodes (indexed for connection)
	UPROPERTY()
	TArray<UEdGraphNode*> CreatedNodes;

	// Wire routing style
	EBlueprintWireStyle WireStyle;

	// Helper: Create function call node
	UK2Node_CallFunction* CreateFunctionNode(const FString& FunctionName, const FVector2D& Position);

	// Helper: Create event node
	UK2Node_Event* CreateEventNode(const FString& EventName, const FVector2D& Position);

	// Helper: Create variable get/set node
	UEdGraphNode* CreateVariableNode(const FString& VariableName, bool bIsGetter, const FVector2D& Position);

	// Helper: Create branch node
	UK2Node_IfThenElse* CreateBranchNode(const FVector2D& Position);

	// Helper: Create sequence node
	UK2Node_ExecutionSequence* CreateSequenceNode(int32 OutputCount, const FVector2D& Position);

	// Helper: Find pin by name
	UEdGraphPin* FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction);

	// Helper: Apply wire routing style to connection
	void ApplyWireRouting(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin);

	// Helper: Calculate Manhattan route waypoints
	TArray<FVector2D> CalculateManhattanRoute(const FVector2D& Start, const FVector2D& End);

	// Helper: Calculate Subway route waypoints
	TArray<FVector2D> CalculateSubwayRoute(const FVector2D& Start, const FVector2D& End);
};
