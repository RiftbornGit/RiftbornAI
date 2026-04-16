// Copyright RiftbornAI. All Rights Reserved.
// Transaction Tools Module — MCP wrappers for FAgentTransactionManager undo/redo system.

#pragma once

#include "CoreMinimal.h"
#include "Tools/ToolModuleBase.h"

struct FClaudeToolCall;
struct FClaudeToolResult;
class FClaudeToolRegistry;

class RIFTBORNAI_API FTransactionToolsModule : public TToolModuleBase<FTransactionToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("TransactionTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_BeginTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CommitTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RollbackTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_UndoLastTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListTransactions(const FClaudeToolCall& Call);
};
