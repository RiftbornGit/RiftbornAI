// Copyright RiftbornAI. All Rights Reserved.
// DataTable Tools Module - DataTable creation and manipulation

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * DataTable Tools Module
 * Provides tools for creating and editing DataTable assets.
 * 
 * Tools included:
 * - create_datatable: Create a new DataTable from a row struct
 * - add_datatable_row: Add a row to a DataTable
 * - get_datatable_rows: Get all rows from a DataTable
 * - delete_datatable_row: Remove a row from a DataTable
 * - list_datatables: List all DataTables in project
 * - get_datatable_info: Get text or JSON authored table state
 */
class RIFTBORNAI_API FDataTableToolsModule : public TToolModuleBase<FDataTableToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("DataTableTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_CreateDataTable(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddDataTableRow(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetDataTableRows(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DeleteDataTableRow(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListDataTables(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetDataTableInfo(const FClaudeToolCall& Call);
};
