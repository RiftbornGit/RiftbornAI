// Copyright RiftbornAI. All Rights Reserved.
// Sublevel Management Tools — Create, list, and move actors between streaming sublevels

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Sublevel Tools Module
 *
 * Provides tools for streaming sublevel management:
 * - create_sublevel: Create a new streaming sublevel and add it to the world
 * - move_actors_to_sublevel: Move actors to a target sublevel by label
 * - list_sublevels: List all streaming sublevels with their state
 */
class RIFTBORNAI_API FSublevelToolsModule : public TToolModuleBase<FSublevelToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("SublevelTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateSublevel(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_MoveActorsToSublevel(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListSublevels(const FClaudeToolCall& Call);
};
