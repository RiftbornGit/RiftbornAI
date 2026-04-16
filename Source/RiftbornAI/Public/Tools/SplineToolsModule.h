// Copyright RiftbornAI. All Rights Reserved.
// Spline Tools Module — Create and edit spline components and spline mesh actors
//
// Splines are commonly used for roads, rivers, cables, rails, paths, rollercoasters,
// and procedural placement along curves. This module provides spline creation and editing.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Spline Tools Module
 *
 * Provides tools for spline management:
 * - create_spline_actor: Spawn an actor with a spline component
 * - add_spline_point: Add a point to a spline at a given index
 * - remove_spline_point: Remove a spline point by index
 * - set_spline_point: Set position/tangent of a spline point
 * - set_spline_type: Set spline type (linear, curve, constant, clamped)
 * - get_spline_info: Get text or JSON spline authored state
 * - create_spline_mesh: Create a spline mesh component (mesh deformed along spline)
 * - set_spline_closed: Set whether the spline forms a closed loop
 * - sample_spline: Sample position/rotation/direction at distance or time along spline
 */
class RIFTBORNAI_API FSplineToolsModule : public TToolModuleBase<FSplineToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("SplineTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateSplineActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddSplinePoint(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RemoveSplinePoint(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSplinePoint(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSplineType(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetSplineInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateSplineMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSplineClosed(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SampleSpline(const FClaudeToolCall& Call);

private:
    static class USplineComponent* FindSplineOnActor(AActor* Actor);
};
