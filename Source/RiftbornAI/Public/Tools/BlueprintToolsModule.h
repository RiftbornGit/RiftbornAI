// Copyright RiftbornAI. All Rights Reserved.
// Blueprint Tools Module - Tools for Blueprint creation and editing

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

// Forward declarations
class UBlueprintGraphEditor;

/**
 * Blueprint Tools Module
 * Provides tools for creating, editing, and compiling Blueprints.
 *
 * Tools:
 * - create_blueprint: Create a new Blueprint asset
 * - open_blueprint: Open existing Blueprint for editing
 * - add_blueprint_node: Add a node to the Blueprint graph
 * - connect_blueprint_nodes: Connect nodes via pins
 * - compile_blueprint: Compile and save Blueprint
 * - get_blueprint_graph: Read graph structure
 */
class RIFTBORNAI_API FBlueprintToolsModule : public TToolModuleBase<FBlueprintToolsModule>
{
public:
    /** Module name for registration */
    static FString StaticModuleName() { return TEXT("BlueprintTools"); }

    /** Register all Blueprint tools with the registry */
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // =========================================================================
    // Tool Implementations
    // =========================================================================

    /** Create a new Blueprint asset */
    static FClaudeToolResult Tool_CreateBlueprint(const FClaudeToolCall& Call);

    /** Open an existing Blueprint for editing */
    static FClaudeToolResult Tool_OpenBlueprint(const FClaudeToolCall& Call);

    /** Add a node to the current Blueprint graph */
    static FClaudeToolResult Tool_AddBlueprintNode(const FClaudeToolCall& Call);

    /** Connect two nodes in the Blueprint graph */
    static FClaudeToolResult Tool_ConnectBlueprintNodes(const FClaudeToolCall& Call);

    /** Compile and save the current Blueprint */
    static FClaudeToolResult Tool_CompileBlueprint(const FClaudeToolCall& Call);

    /** Get the node graph structure of a Blueprint */
    static FClaudeToolResult Tool_GetBlueprintGraph(const FClaudeToolCall& Call);

    /** Set a pin value on a node */
    static FClaudeToolResult Tool_SetBlueprintPinValue(const FClaudeToolCall& Call);

    /** Add a variable to a Blueprint */
    static FClaudeToolResult Tool_AddBlueprintVariable(const FClaudeToolCall& Call);

    /** Add a function to a Blueprint */
    static FClaudeToolResult Tool_AddBlueprintFunction(const FClaudeToolCall& Call);

    /** Setup common Blueprint event handlers (BeginPlay, Tick, etc.) */
    static FClaudeToolResult Tool_SetupBlueprintEvents(const FClaudeToolCall& Call);

    /** Convert Blueprint to C++ code - THE KILLER FEATURE */
    static FClaudeToolResult Tool_ConvertBlueprintToCpp(const FClaudeToolCall& Call);

    /** Check if Blueprint can be converted to C++ */
    static FClaudeToolResult Tool_CheckBlueprintConvertibility(const FClaudeToolCall& Call);

    /** Add a component to a Blueprint's component hierarchy */
    static FClaudeToolResult Tool_AddBlueprintComponent(const FClaudeToolCall& Call);

    // =========================================================================
    // Blueprint Diff Tools (ENH-007)
    // =========================================================================

    /** Create a snapshot of Blueprint state for version control */
    static FClaudeToolResult Tool_CreateBlueprintSnapshot(const FClaudeToolCall& Call);

    /** Compare two Blueprint snapshots to identify changes */
    static FClaudeToolResult Tool_CompareBlueprintSnapshots(const FClaudeToolCall& Call);

    /** Export Blueprint to human-readable text for code review */
    static FClaudeToolResult Tool_ExportBlueprintText(const FClaudeToolCall& Call);

    /** List available Blueprint snapshots */
    static FClaudeToolResult Tool_ListBlueprintSnapshots(const FClaudeToolCall& Call);

    // =========================================================================
    // EVENT WIRING TOOLS - Gameplay Causality (ENH-008)
    // =========================================================================

    /**
     * Add a restricted event handler to a Blueprint.
     * ONLY allows specific events (overlap, beginplay) and specific actions
     * (increment var, set var, destroy self, print string).
     * This is the core tool for expressing gameplay causality.
     *
     * @param blueprint - Blueprint path to modify
     * @param event - Event configuration (type, component, filter)
     * @param actions - Array of restricted actions to execute
     * @param idempotency_key - Key for undo/update operations
     */
    static FClaudeToolResult Tool_AddBlueprintEvent(const FClaudeToolCall& Call);

    /**
     * Remove an event handler previously added by add_blueprint_event.
     * Uses the idempotency_key to find and remove nodes.
     */
    static FClaudeToolResult Tool_RemoveBlueprintEvent(const FClaudeToolCall& Call);

    /** Auto-validate, repair, and recompile a Blueprint in a single call */
    static FClaudeToolResult Tool_ValidateAndRepairBlueprintLoop(const FClaudeToolCall& Call);

    // =========================================================================
    // GAMEPLAY LOGIC WIRING TOOLS (expanded, unrestricted)
    // =========================================================================

    /**
     * Wire arbitrary gameplay logic in a Blueprint EventGraph from a JSON description.
     * Supports Event, Branch, Sequence, Delay, CallFunction, Get/SetVariable,
     * ForEachLoop, SpawnActor, DestroyActor, PrintString.
     */
    static FClaudeToolResult Tool_WireBlueprintGameplay(const FClaudeToolCall& Call);

    /** Insert a Delay node between an existing node and its downstream connections. */
    static FClaudeToolResult Tool_AddBlueprintDelay(const FClaudeToolCall& Call);

    /** Insert a Branch node after an existing node, optionally wired to a bool variable. */
    static FClaudeToolResult Tool_AddBlueprintBranch(const FClaudeToolCall& Call);

private:
    /** Get or create the Blueprint editor instance */
    static UBlueprintGraphEditor* GetOrCreateBlueprintEditor();

    /** Current Blueprint editor for multi-step operations */
    static UBlueprintGraphEditor* CurrentBlueprintEditor;

    // =========================================================================
    // EVENT WIRING HELPERS
    // =========================================================================

    /** Allowed event types for add_blueprint_event (v1) */
    static bool IsAllowedEventType(const FString& EventType);

    /** Allowed action types for add_blueprint_event (v1) */
    static bool IsAllowedActionType(const FString& ActionType);

    /** Create event node for specified type */
    static UK2Node* CreateEventNodeForType(
        UEdGraph* Graph,
        const FString& EventType,
        const FString& ComponentName,
        const FVector2D& Position);

    /** Create action node for specified action */
    static UK2Node* CreateActionNode(
        UEdGraph* Graph,
        UBlueprint* Blueprint,
        const TSharedPtr<FJsonObject>& ActionDef,
        const FVector2D& Position);

    /** Tag nodes for undo/ownership tracking */
    static void TagNodeWithOwnership(
        UEdGraphNode* Node,
        const FString& ToolName,
        const FString& IdempotencyKey);

    /** Find nodes tagged with specific key */
    static TArray<UEdGraphNode*> FindNodesByIdempotencyKey(
        UEdGraph* Graph,
        const FString& IdempotencyKey);
};

