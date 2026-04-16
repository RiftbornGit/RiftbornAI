// AIToolsModule.h
// AI and Behavior Tree tools for RiftbornAI
// Tools: create_behavior_tree, add_bt_task, add_bt_decorator, add_bt_service,
//        create_blackboard, set_blackboard_key, run_behavior_tree, get_ai_info

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * AI Tools Module
 * 
 * Provides tools for AI and Behavior Trees:
 * - create_behavior_tree: Create a new behavior tree asset
 * - add_bt_task: Add a task node to behavior tree
 * - add_bt_decorator: Add a decorator (condition) to behavior tree
 * - add_bt_service: Add a service node to behavior tree
 * - create_blackboard: Create a blackboard data asset
 * - set_blackboard_key: Add or set a key in a blackboard
 * - run_behavior_tree: Assign and run behavior tree on an AI
 * - get_ai_info: Get AI controller and behavior tree info for an actor
 */
class RIFTBORNAI_API FAIToolsModule : public TToolModuleBase<FAIToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("AITools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_CreateBehaviorTree(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddBTTask(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddBTDecorator(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddBTService(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateBlackboard(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetBlackboardKey(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RunBehaviorTree(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetAIInfo(const FClaudeToolCall& Call);
    
};
