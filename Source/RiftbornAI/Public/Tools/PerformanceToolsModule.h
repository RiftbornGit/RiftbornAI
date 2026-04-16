// Copyright RiftbornAI. All Rights Reserved.
// Performance Profiling Tools Module
//
// Wraps UE5's built-in performance monitoring into the copilot tool framework.
// FPS, memory, GPU, draw calls, render stats, bottleneck detection,
// texture streaming stats, and per-actor render cost analysis.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Performance Profiling Tools Module
 *
 * Provides real-time performance analysis tools:
 * - get_fps_stats: Frame rate, frame time, percentiles
 * - get_memory_stats: Process/GPU/texture memory usage
 * - get_render_stats: Draw calls, triangles, material count
 * - get_gpu_stats: GPU timing, utilization, bottleneck type
 * - get_performance_snapshot: Comprehensive all-in-one snapshot
 * - get_actor_render_cost: Per-actor triangle/material/draw call cost
 * - get_performance_bottlenecks: Identify CPU/GPU/memory bottlenecks
 * - get_texture_streaming_stats: Texture pool, streaming budget, over-budget textures
 * - profile_scene_complexity: Scene complexity analysis (overdraw, shader cost)
 * - get_stat_group: Structured stat readout for Memory, Unit/FPS, and RHI groups
 * - analyze_scene_cost: Weighted scene-cost audit with top expensive actors
 * - analyze_perceptual_cost: Split cost into protected-quality and elastic regions
 * - get_visibility_budget_status: Inspect the scene-wide view-aware visibility budgeter
 * - configure_visibility_budget: Configure buffered FOV and turn-prediction prewarm
 * - get_actor_significance_policy: Inspect adaptive scene budget policy on an actor
 * - set_actor_significance_policy: Attach/configure adaptive scene budget policy on an actor
 * - apply_performance_budget: Apply a scene-wide adaptive budget preset to matching actors
 * - define_protected_quality_zone: Keep visible hero content at full quality while still allowing off-screen savings
 * - verify_quality_preservation: Audit protected actors for visible quality regressions
 * - optimize_streaming_residency: Tighten streaming residency and visibility prewarm to reduce VRAM/RAM pressure
 * - apply_distant_material_distillation: Push elastic distant actors onto cheaper LOD/material presentation
 * - schedule_shadow_updates: Focus adaptive budgeting on shadow-heavy actors
 * - generate_foliage_impostors: Build a merged distant proxy mesh asset from foliage-like actors
 * - optimize_open_world_scene: One-shot orchestration of the perception-preserving optimization stack
 */
class RIFTBORNAI_API FPerformanceToolsModule : public TToolModuleBase<FPerformanceToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("PerformanceTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_GetFpsStats(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetMemoryStats(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetRenderStats(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetGpuStats(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetPerformanceSnapshot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetActorRenderCost(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetPerformanceBottlenecks(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetTextureStreamingStats(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ProfileSceneComplexity(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetStatGroup(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AnalyzeSceneCost(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AnalyzePerceptualCost(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetVisibilityBudgetStatus(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConfigureVisibilityBudget(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetActorSignificancePolicy(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetActorSignificancePolicy(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyPerformanceBudget(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DefineProtectedQualityZone(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_VerifyQualityPreservation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_OptimizeStreamingResidency(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyDistantMaterialDistillation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ScheduleShadowUpdates(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateFoliageImpostors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_OptimizeOpenWorldScene(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AnalyzeDrawCalls(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CaptureTraceBookmark(const FClaudeToolCall& Call);

};
