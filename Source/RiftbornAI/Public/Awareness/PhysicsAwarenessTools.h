// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * Tools for physics awareness - simulation state, bodies, collisions, constraints
 */

// Tool: get_physics_state - Get overall physics simulation state
class RIFTBORNAI_API FGetPhysicsStateTool : public FAgentTool
{
public:
    FGetPhysicsStateTool();
    virtual FString GetName() const override { return TEXT("get_physics_state"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_physics_bodies - List physics bodies in scene
class RIFTBORNAI_API FGetPhysicsBodiesTool : public FAgentTool
{
public:
    FGetPhysicsBodiesTool();
    virtual FString GetName() const override { return TEXT("get_physics_bodies"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_actor_physics - Get physics info for specific actor
class RIFTBORNAI_API FGetActorPhysicsTool : public FAgentTool
{
public:
    FGetActorPhysicsTool();
    virtual FString GetName() const override { return TEXT("get_actor_physics"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_collision_channels - Get collision channels and profiles
class RIFTBORNAI_API FGetCollisionChannelsTool : public FAgentTool
{
public:
    FGetCollisionChannelsTool();
    virtual FString GetName() const override { return TEXT("get_collision_channels"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_physics_constraints - List physics constraints
class RIFTBORNAI_API FGetPhysicsConstraintsTool : public FAgentTool
{
public:
    FGetPhysicsConstraintsTool();
    virtual FString GetName() const override { return TEXT("get_physics_constraints"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: overlap_test - Test for overlapping bodies at location
class RIFTBORNAI_API FOverlapTestTool : public FAgentTool
{
public:
    FOverlapTestTool();
    virtual FString GetName() const override { return TEXT("overlap_test"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_physics_materials - Get all physics materials and their properties
class RIFTBORNAI_API FGetPhysicsMaterialsTool : public FAgentTool
{
public:
    FGetPhysicsMaterialsTool();
    virtual FString GetName() const override { return TEXT("get_physics_materials"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: sweep_test - Perform a physics sweep test
class RIFTBORNAI_API FSweepTestTool : public FAgentTool
{
public:
    FSweepTestTool();
    virtual FString GetName() const override { return TEXT("sweep_test"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration wrapper class
class RIFTBORNAI_API FPhysicsAwarenessTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
