// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/Blueprint.h"
#include "BlueprintGraphAPI.generated.h"

/**
 * Node information structure for Blueprint graph analysis
 */
USTRUCT(BlueprintType)
struct FBlueprintNodeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Node")
	FString NodeID;

	UPROPERTY(BlueprintReadOnly, Category = "Node")
	FString NodeTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Node")
	FString NodeClass;

	UPROPERTY(BlueprintReadOnly, Category = "Node")
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Node")
	TArray<FString> InputPins;

	UPROPERTY(BlueprintReadOnly, Category = "Node")
	TArray<FString> OutputPins;

	UPROPERTY(BlueprintReadOnly, Category = "Node")
	TMap<FString, FString> PinTypes;

	UPROPERTY(BlueprintReadOnly, Category = "Node")
	TArray<FString> ConnectedNodes;
};

/**
 * Pin connection information
 */
USTRUCT(BlueprintType)
struct FBlueprintPinConnection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Connection")
	FString SourceNodeID;

	UPROPERTY(BlueprintReadOnly, Category = "Connection")
	FString SourcePinName;

	UPROPERTY(BlueprintReadOnly, Category = "Connection")
	FString TargetNodeID;

	UPROPERTY(BlueprintReadOnly, Category = "Connection")
	FString TargetPinName;

	UPROPERTY(BlueprintReadOnly, Category = "Connection")
	FString PinType;
};

/**
 * Blueprint graph structure snapshot
 */
USTRUCT(BlueprintType)
struct FBlueprintGraphInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Graph")
	FString GraphName;

	UPROPERTY(BlueprintReadOnly, Category = "Graph")
	TArray<FBlueprintNodeInfo> Nodes;

	UPROPERTY(BlueprintReadOnly, Category = "Graph")
	TArray<FBlueprintPinConnection> Connections;

	UPROPERTY(BlueprintReadOnly, Category = "Graph")
	int32 TotalNodes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Graph")
	int32 TotalConnections = 0;
};

/**
 * Blueprint graph manipulation API
 * 
 * Capabilities:
 * - Read graph node structure with connections
 * - Extract semantic information (execution flow, data flow)
 * - Modify graphs safely with validation
 * - Detect circular dependencies
 * - Validate changes before committing
 * - Rollback on errors
 */
UCLASS()
class RIFTBORNAI_API UBlueprintGraphAPI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==================== GRAPH READING ====================

	/**
	 * Get graph information from a Blueprint
	 * @param Blueprint - Blueprint asset to analyze
	 * @param GraphName - Name of graph to read (empty = EventGraph)
	 * @return Graph structure with nodes and connections
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static FBlueprintGraphInfo GetBlueprintGraph(UBlueprint* Blueprint, FString GraphName = TEXT(""));

	/**
	 * Get all graphs in a Blueprint
	 * @param Blueprint - Blueprint to analyze
	 * @return Array of all graph names
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static TArray<FString> GetAllGraphNames(UBlueprint* Blueprint);

	/**
	 * Get detailed information about a specific node
	 * @param Blueprint - Blueprint containing the node
	 * @param NodeID - Unique identifier of the node
	 * @return Node information structure
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static FBlueprintNodeInfo GetNodeInfo(UBlueprint* Blueprint, FString NodeID);

	/**
	 * Trace execution path from a starting node
	 * @param Blueprint - Blueprint to analyze
	 * @param StartNodeID - Node to start from
	 * @return Ordered list of node IDs in execution order
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static TArray<FString> TraceExecutionPath(UBlueprint* Blueprint, FString StartNodeID);

	/**
	 * Detect circular dependencies in graph
	 * @param Blueprint - Blueprint to analyze
	 * @param OutCircularNodes - Nodes involved in cycles
	 * @return True if cycles detected
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool DetectCircularDependencies(UBlueprint* Blueprint, TArray<FString>& OutCircularNodes);

	// ==================== GRAPH MODIFICATION ====================

	/**
	 * Add a node to Blueprint graph with validation
	 * @param Blueprint - Target Blueprint
	 * @param GraphName - Graph to add node to
	 * @param NodeClass - UK2Node class name
	 * @param Position - Node position in graph
	 * @param OutNodeID - Unique ID of created node
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool AddNodeToGraph(UBlueprint* Blueprint, FString GraphName, FString NodeClass, FVector2D Position, FString& OutNodeID);

	/**
	 * Remove a node from graph safely (checks dependencies)
	 * @param Blueprint - Target Blueprint
	 * @param NodeID - Node to remove
	 * @param bForce - Force removal even if connected
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool RemoveNodeFromGraph(UBlueprint* Blueprint, FString NodeID, bool bForce = false);

	/**
	 * Connect two nodes via pins with type validation
	 * @param Blueprint - Target Blueprint
	 * @param SourceNodeID - Source node
	 * @param SourcePinName - Source pin
	 * @param TargetNodeID - Target node
	 * @param TargetPinName - Target pin
	 * @return True if connection valid and created
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool ConnectNodes(UBlueprint* Blueprint, FString SourceNodeID, FString SourcePinName, FString TargetNodeID, FString TargetPinName);

	/**
	 * Disconnect nodes safely
	 * @param Blueprint - Target Blueprint
	 * @param SourceNodeID - Source node
	 * @param SourcePinName - Source pin (empty = disconnect all)
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool DisconnectNodes(UBlueprint* Blueprint, FString SourceNodeID, FString SourcePinName = TEXT(""));

	/**
	 * Move node to new position (for organization)
	 * @param Blueprint - Target Blueprint
	 * @param NodeID - Node to move
	 * @param NewPosition - New position
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool MoveNode(UBlueprint* Blueprint, FString NodeID, FVector2D NewPosition);

	// ==================== VALIDATION & SAFETY ====================

	/**
	 * Validate Blueprint graph for errors
	 * @param Blueprint - Blueprint to validate
	 * @param OutErrors - List of error messages
	 * @param OutWarnings - List of warning messages
	 * @return True if no errors (warnings OK)
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool ValidateGraph(UBlueprint* Blueprint, TArray<FString>& OutErrors, TArray<FString>& OutWarnings);

	/**
	 * Check if connection between pins is type-safe
	 * @param SourcePinType - Source pin type
	 * @param TargetPinType - Target pin type
	 * @return True if connection is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool IsConnectionValid(FString SourcePinType, FString TargetPinType);

	/**
	 * Snapshot Blueprint state for rollback
	 * @param Blueprint - Blueprint to snapshot
	 * @return Snapshot ID for later restoration
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static FString CreateSnapshot(UBlueprint* Blueprint);

	/**
	 * Restore Blueprint from snapshot
	 * @param Blueprint - Blueprint to restore
	 * @param SnapshotID - Snapshot to restore from
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool RestoreSnapshot(UBlueprint* Blueprint, FString SnapshotID);

	/**
	 * Compare two snapshots and return differences
	 * @param SnapshotID1 - First snapshot (baseline)
	 * @param SnapshotID2 - Second snapshot (current)
	 * @param OutDiffJson - JSON string describing differences
	 * @return True if comparison successful
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool CompareSnapshots(FString SnapshotID1, FString SnapshotID2, FString& OutDiffJson);

	/**
	 * Export Blueprint to human-readable text format for VCS review
	 * @param Blueprint - Blueprint to export
	 * @param bIncludePositions - Include node positions (may cause noisy diffs)
	 * @return Text representation of Blueprint graph
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static FString ExportToReviewableText(UBlueprint* Blueprint, bool bIncludePositions = false);

	/**
	 * List all available snapshots for a Blueprint
	 * @param BlueprintName - Name of Blueprint (empty = all)
	 * @return Array of snapshot IDs with timestamps
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static TArray<FString> ListSnapshots(FString BlueprintName = TEXT(""));

	// ==================== SEMANTIC ANALYSIS ====================

	/**
	 * Analyze Blueprint complexity metrics
	 * @param Blueprint - Blueprint to analyze
	 * @param OutCyclomaticComplexity - Cyclomatic complexity score
	 * @param OutMaxDepth - Maximum execution depth
	 * @param OutNodeCount - Total node count
	 * @return Analysis successful
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static bool AnalyzeComplexity(UBlueprint* Blueprint, int32& OutCyclomaticComplexity, int32& OutMaxDepth, int32& OutNodeCount);

	/**
	 * Find all nodes of specific type
	 * @param Blueprint - Blueprint to search
	 * @param NodeClassName - Class name to find
	 * @return List of matching node IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static TArray<FString> FindNodesByType(UBlueprint* Blueprint, FString NodeClassName);

	/**
	 * Get all variables referenced by a node
	 * @param Blueprint - Blueprint containing node
	 * @param NodeID - Node to analyze
	 * @return List of variable names referenced
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Blueprint")
	static TArray<FString> GetNodeVariableReferences(UBlueprint* Blueprint, FString NodeID);

private:
	// Internal helpers
	static UEdGraphNode* FindNodeByID(UBlueprint* Blueprint, const FString& NodeID);
	static UEdGraphPin* FindPinByName(UEdGraphNode* Node, const FString& PinName);
	static FString GenerateNodeID(UEdGraphNode* Node);
	static FString GetPinTypeString(UEdGraphPin* Pin);
	static void CollectConnectedNodes(UEdGraphNode* StartNode, TSet<UEdGraphNode*>& VisitedNodes, bool bFollowExecution = true);
};
