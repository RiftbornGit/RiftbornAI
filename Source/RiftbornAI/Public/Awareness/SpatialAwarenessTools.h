// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * Tools for spatial awareness - querying actor positions, distances, visibility, navigation
 */

// Tool: get_actors_in_radius - Find all actors within a radius of a point
class RIFTBORNAI_API FGetActorsInRadiusTool : public FAgentTool
{
public:
    FGetActorsInRadiusTool();
    virtual FString GetName() const override { return TEXT("get_actors_in_radius"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_actor_transform - Get position, rotation, scale of an actor
class RIFTBORNAI_API FGetActorTransformTool : public FAgentTool
{
public:
    FGetActorTransformTool();
    virtual FString GetName() const override { return TEXT("get_actor_transform"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_distance_between - Calculate distance between two actors
class RIFTBORNAI_API FGetDistanceBetweenTool : public FAgentTool
{
public:
    FGetDistanceBetweenTool();
    virtual FString GetName() const override { return TEXT("get_distance_between"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: check_line_of_sight - Test visibility between two points
class RIFTBORNAI_API FCheckLineOfSightTool : public FAgentTool
{
public:
    FCheckLineOfSightTool();
    virtual FString GetName() const override { return TEXT("check_line_of_sight"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: raycast - Perform a raycast and return hit info
class RIFTBORNAI_API FRaycastTool : public FAgentTool
{
public:
    FRaycastTool();
    virtual FString GetName() const override { return TEXT("raycast"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_navmesh_info - Query navigation mesh at a point
class RIFTBORNAI_API FGetNavMeshInfoTool : public FAgentTool
{
public:
    FGetNavMeshInfoTool();
    virtual FString GetName() const override { return TEXT("get_navmesh_info"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: find_path - Find navigation path between two points
class RIFTBORNAI_API FFindPathTool : public FAgentTool
{
public:
    FFindPathTool();
    virtual FString GetName() const override { return TEXT("find_path"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_spatial_relationships - Get relationships between actors (overlapping, containing, near)
class RIFTBORNAI_API FGetSpatialRelationshipsTool : public FAgentTool
{
public:
    FGetSpatialRelationshipsTool();
    virtual FString GetName() const override { return TEXT("get_spatial_relationships"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_world_bounds - Get the bounds of the current level/world
class RIFTBORNAI_API FGetWorldBoundsTool : public FAgentTool
{
public:
    FGetWorldBoundsTool();
    virtual FString GetName() const override { return TEXT("get_world_bounds"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration wrapper class
class RIFTBORNAI_API FSpatialAwarenessTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
