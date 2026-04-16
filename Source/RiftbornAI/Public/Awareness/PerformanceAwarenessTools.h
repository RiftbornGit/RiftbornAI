// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * Tools for performance awareness - FPS, memory, draw calls, GPU stats
 */

// Tool: get_fps_stats - Get frame rate statistics
class RIFTBORNAI_API FGetFpsStatsTool : public FAgentTool
{
public:
    FGetFpsStatsTool();
    virtual FString GetName() const override { return TEXT("get_fps_stats"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_memory_stats - Get memory usage statistics
class RIFTBORNAI_API FGetMemoryStatsTool : public FAgentTool
{
public:
    FGetMemoryStatsTool();
    virtual FString GetName() const override { return TEXT("get_memory_stats"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_render_stats - Get scene-level render estimates (draw calls, triangles, etc.)
class RIFTBORNAI_API FGetRenderStatsTool : public FAgentTool
{
public:
    FGetRenderStatsTool();
    virtual FString GetName() const override { return TEXT("get_render_stats"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_gpu_stats - Get RHI info and heuristic GPU bottleneck data
class RIFTBORNAI_API FGetGpuStatsTool : public FAgentTool
{
public:
    FGetGpuStatsTool();
    virtual FString GetName() const override { return TEXT("get_gpu_stats"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_performance_snapshot - Get comprehensive performance snapshot
class RIFTBORNAI_API FGetPerformanceSnapshotTool : public FAgentTool
{
public:
    FGetPerformanceSnapshotTool();
    virtual FString GetName() const override { return TEXT("get_performance_snapshot"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_actor_render_cost - Get rendering cost of specific actor
class RIFTBORNAI_API FGetActorRenderCostTool : public FAgentTool
{
public:
    FGetActorRenderCostTool();
    virtual FString GetName() const override { return TEXT("get_actor_render_cost"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_performance_bottlenecks - Identify performance bottlenecks
class RIFTBORNAI_API FGetPerformanceBottlenecksTool : public FAgentTool
{
public:
    FGetPerformanceBottlenecksTool();
    virtual FString GetName() const override { return TEXT("get_performance_bottlenecks"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_streaming_stats - Get level/texture streaming statistics
class RIFTBORNAI_API FGetStreamingStatsTool : public FAgentTool
{
public:
    FGetStreamingStatsTool();
    virtual FString GetName() const override { return TEXT("get_streaming_stats"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration wrapper class
class RIFTBORNAI_API FPerformanceAwarenessTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
