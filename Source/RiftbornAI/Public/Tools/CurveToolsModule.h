// Copyright RiftbornAI. All Rights Reserved.
// Curve Tools Module — Float, vector, color curves and rich curve editing

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Curve Tools Module
 *
 * Provides tools for curve asset management:
 * - create_curve_float: Create a UCurveFloat asset
 * - create_curve_vector: Create a UCurveVector asset
 * - create_curve_linear_color: Create a UCurveLinearColor asset
 * - add_curve_key: Add a key to a curve at given time with value
 * - remove_curve_key: Remove a key from a curve by index
 * - set_curve_interp: Set interpolation mode for a key (linear, cubic, constant)
 * - get_curve_info: Get text or JSON rich-curve state from a curve
 * - evaluate_curve: Evaluate a curve at a given time
 * - create_curve_table: Create a UCurveTable from data
 */
class RIFTBORNAI_API FCurveToolsModule : public TToolModuleBase<FCurveToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("CurveTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateCurveFloat(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateCurveVector(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateCurveLinearColor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddCurveKey(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RemoveCurveKey(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetCurveInterp(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetCurveInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateCurve(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateCurveTable(const FClaudeToolCall& Call);
};
