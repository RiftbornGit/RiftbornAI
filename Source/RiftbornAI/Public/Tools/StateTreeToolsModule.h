// Copyright RiftbornAI. All Rights Reserved.
// StateTree Tools Module - UE5 StateTree editing and management
//
// StateTrees are UE5's hierarchical state machine framework that can replace
// or complement Behavior Trees for AI logic, game flow, and entity logic.
// Available since UE 5.1.
//
// NOTE: Depends on StateTreeModule and StateTreeEditorModule.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * StateTree Tools Module
 *
 * Provides the currently supported StateTree surface:
 * - create_state_tree: Create a new StateTree asset
 * - list_state_trees: List all StateTree assets in the project
 * - get_state_tree_info: Inspect an existing StateTree
 * - add_state_tree_state: Add a state to a StateTree
 *
 * Advanced task, transition, condition, evaluator, and component-assignment
 * automation is intentionally not exposed until it is backed by working
 * UE 5.7 implementations.
 */
class RIFTBORNAI_API FStateTreeToolsModule : public TToolModuleBase<FStateTreeToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("StateTreeTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateStateTree(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListStateTrees(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetStateTreeInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddState(const FClaudeToolCall& Call);

};
