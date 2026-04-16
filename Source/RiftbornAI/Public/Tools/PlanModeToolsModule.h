// Copyright RiftbornAI. All Rights Reserved.
// Plan Mode Tools - MCP wrappers for FExecEngine plan execution

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"
#include "Core/PlanExecutor.h"

/**
 * FPlanModeToolsModule
 *
 * Thin MCP wrappers around FExecEngine for plan creation,
 * step-by-step execution, and status queries.
 *
 * Tools:
 * - create_plan        Parse and store a frozen plan
 * - execute_plan       Run N steps of a stored plan
 * - get_plan_status    Query current plan state
 */
class RIFTBORNAI_API FPlanModeToolsModule : public TToolModuleBase<FPlanModeToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("PlanMode"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreatePlan(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExecutePlan(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetPlanStatus(const FClaudeToolCall& Call);

private:
    /** In-memory plan storage keyed by PlanId string */
    static TMap<FString, FExecutablePlan>& GetPlanStore();

    /** Shared helper: look up a plan or fill error result */
    static FExecutablePlan* FindPlanOrError(const FString& PlanId, FClaudeToolResult& OutResult);

    /** Map plan state to a status string */
    static FString PlanStatusString(const FExecutablePlan& Plan);
};
