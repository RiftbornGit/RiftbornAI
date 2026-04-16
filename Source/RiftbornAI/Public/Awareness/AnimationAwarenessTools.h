// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * Tools for animation awareness - montages, blend states, animation graphs, notifies
 */

// Tool: get_animation_state - Get animation state for an actor
class RIFTBORNAI_API FGetAnimationStateTool : public FAgentTool
{
public:
    FGetAnimationStateTool();
    virtual FString GetName() const override { return TEXT("get_animation_state"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_playing_montages - Get all currently playing montages
class RIFTBORNAI_API FGetPlayingMontagesTool : public FAgentTool
{
public:
    FGetPlayingMontagesTool();
    virtual FString GetName() const override { return TEXT("get_playing_montages"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_skeletal_meshes - List all skeletal mesh components in scene
class RIFTBORNAI_API FGetSkeletalMeshesTool : public FAgentTool
{
public:
    FGetSkeletalMeshesTool();
    virtual FString GetName() const override { return TEXT("get_skeletal_meshes"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_anim_blueprint_state - Get animation blueprint variable states
class RIFTBORNAI_API FGetAnimBlueprintStateTool : public FAgentTool
{
public:
    FGetAnimBlueprintStateTool();
    virtual FString GetName() const override { return TEXT("get_anim_blueprint_state"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_blend_space_state - Get blend space parameter values
class RIFTBORNAI_API FGetBlendSpaceStateTool : public FAgentTool
{
public:
    FGetBlendSpaceStateTool();
    virtual FString GetName() const override { return TEXT("get_blend_space_state"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_bone_transforms - Get bone transforms for skeletal mesh
class RIFTBORNAI_API FGetBoneTransformsTool : public FAgentTool
{
public:
    FGetBoneTransformsTool();
    virtual FString GetName() const override { return TEXT("get_bone_transforms"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_anim_notifies - Get animation notifies and their states
class RIFTBORNAI_API FGetAnimNotifiesTool : public FAgentTool
{
public:
    FGetAnimNotifiesTool();
    virtual FString GetName() const override { return TEXT("get_anim_notifies"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration wrapper class
class RIFTBORNAI_API FAnimationAwarenessTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
