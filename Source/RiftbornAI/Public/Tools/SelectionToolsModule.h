// Copyright RiftbornAI. All Rights Reserved.
// Selection Tools Module - Editor selection and undo/redo operations

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Selection Tools Module
 * Provides tools for working with editor selection and undo/redo transactions.
 * 
 * Tools included:
 * - get_selected_actors: Get currently selected actors in the viewport
 * - resolve_scene_graph_node: Resolve a scene-graph actor id/path into a live editor actor
 * - select_scene_graph_node: Select/focus a live actor using a scene-graph actor id/path
 * - select_actors: Select multiple actors by label
 * - deselect_all: Clear the selection
 * - select_by_class: Select all actors of a specific class
 * - select_by_tag: Select all actors with a specific tag
 * - begin_transaction: Start an undo transaction
 * - end_transaction: End the current undo transaction
 * - undo: Undo the last action
 * - redo: Redo the last undone action
 */
class RIFTBORNAI_API FSelectionToolsModule : public TToolModuleBase<FSelectionToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("SelectionTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Selection tool implementations
    static FClaudeToolResult Tool_GetSelectedActors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ResolveSceneGraphNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SelectSceneGraphNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SelectActors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DeselectAll(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SelectByClass(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SelectByTag(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddToSelection(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RemoveFromSelection(const FClaudeToolCall& Call);
    
    // Transaction tool implementations
    static FClaudeToolResult Tool_BeginTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EndTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_Undo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_Redo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetUndoHistory(const FClaudeToolCall& Call);
    
private:
    // Active transaction tracking
    static TSharedPtr<class FScopedTransaction> CurrentTransaction;
    static FString CurrentTransactionName;
    
    // Helper functions
    static AActor* FindActorByPath(const FString& ActorPath);
    static AActor* ResolveActorReference(const FString& ActorId, const FString& Label, FString& OutResolutionSource);
    static TArray<AActor*> GetAllLevelActors();
};
