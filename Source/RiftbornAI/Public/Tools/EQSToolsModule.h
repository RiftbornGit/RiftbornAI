// Copyright RiftbornAI. All Rights Reserved.
// EQS (Environment Query System) Tools Module
//
// Environment Query System is UE5's spatial reasoning framework for AI.
// Used to find locations, actors, and points of interest based on scoring tests.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * EQS Tools Module
 *
 * Provides tools for UE5's Environment Query System:
 * - create_eqs_query: Create an EQS query asset
 * - list_eqs_queries: List all EQS query assets
 * - get_eqs_query_info: Inspect an EQS query's generators and tests
 * - add_eqs_generator: Add a generator to an EQS query
 * - add_eqs_test: Add a test/scorer to an EQS query
 * - run_eqs_query: Validate an EQS query and optionally drive an EQS Testing Pawn preview
 * - list_eqs_generators: List available generator types
 * - list_eqs_tests: List available test types
 */
class RIFTBORNAI_API FEQSToolsModule : public TToolModuleBase<FEQSToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("EQSTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateEQSQuery(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListEQSQueries(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetEQSQueryInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddEQSGenerator(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddEQSTest(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RunEQSQuery(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListEQSGenerators(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListEQSTests(const FClaudeToolCall& Call);
};
