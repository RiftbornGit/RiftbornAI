// BatchToolsModule.h
// Batch operation tools for bulk actor/asset manipulation

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * BatchToolsModule provides bulk operation tools for:
 * - Batch delete actors by pattern
 * - Batch move actors by pattern  
 * - Batch move/rename assets
 * - Batch clone actors
 * - Batch material application
 * - Performance monitoring
 *
 * Uses FBatchOperations API with FBatchFilter for pattern matching.
 */
namespace BatchToolsModule
{
	/** Register all batch operation tools */
	void RegisterTools();
	
	// Individual tool handlers
	FClaudeToolResult Tool_BatchDeleteActors(const FClaudeToolCall& Call);
	FClaudeToolResult Tool_BatchMoveActors(const FClaudeToolCall& Call);
	FClaudeToolResult Tool_BatchMoveAssets(const FClaudeToolCall& Call);
	FClaudeToolResult Tool_BatchCloneActors(const FClaudeToolCall& Call);
	FClaudeToolResult Tool_BatchSetMaterial(const FClaudeToolCall& Call);
	FClaudeToolResult Tool_GetPerformanceReport(const FClaudeToolCall& Call);
}
