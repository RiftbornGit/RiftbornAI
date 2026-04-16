// Copyright RiftbornAI. All Rights Reserved.
// Collision & Trace Tools Module — Line traces, sweeps, overlaps, collision channels
//
// Every game needs collision queries for gameplay, AI sight, interaction
// checks, and physics queries. This module provides common trace and sweep operations.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Collision & Trace Tools Module
 *
 * Provides tools for collision queries:
 * - line_trace: Single line trace (raycast) by channel
 * - sphere_trace: Sphere sweep trace
 * - box_trace: Box sweep trace
 * - capsule_trace: Capsule sweep trace
 * - overlap_test: Multi-overlap query at a location
 * - set_collision_preset: Set collision preset on an actor's component
 * - get_collision_info: Get collision settings and key response channels of an actor
 */
class RIFTBORNAI_API FCollisionTraceToolsModule : public TToolModuleBase<FCollisionTraceToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("CollisionTraceTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_LineTrace(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SphereTrace(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_BoxTrace(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CapsuleTrace(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_OverlapTest(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetCollisionPreset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetCollisionInfo(const FClaudeToolCall& Call);

private:
    static FVector ParseVector(const FString& X, const FString& Y, const FString& Z);
    static ECollisionChannel ParseChannel(const FString& Channel);
};
