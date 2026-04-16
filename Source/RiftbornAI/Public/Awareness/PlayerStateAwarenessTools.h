// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * Tools for player state awareness - input bindings, controller state, camera, inventory
 */

// Tool: get_input_bindings - Get current input action/axis mappings
class RIFTBORNAI_API FGetInputBindingsTool : public FAgentTool
{
public:
    FGetInputBindingsTool();
    virtual FString GetName() const override { return TEXT("get_input_bindings"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_player_state - Get comprehensive player state snapshot
class RIFTBORNAI_API FGetPlayerStateTool : public FAgentTool
{
public:
    FGetPlayerStateTool();
    virtual FString GetName() const override { return TEXT("get_player_state"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_camera_info - Get camera position, rotation, FOV, projection
class RIFTBORNAI_API FGetCameraInfoTool : public FAgentTool
{
public:
    FGetCameraInfoTool();
    virtual FString GetName() const override { return TEXT("get_camera_info"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_controller_type - Detect if using gamepad, keyboard/mouse, touch
class RIFTBORNAI_API FGetControllerTypeTool : public FAgentTool
{
public:
    FGetControllerTypeTool();
    virtual FString GetName() const override { return TEXT("get_controller_type"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_movement_input - Get current movement input vector
class RIFTBORNAI_API FGetMovementInputTool : public FAgentTool
{
public:
    FGetMovementInputTool();
    virtual FString GetName() const override { return TEXT("get_movement_input"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_all_players - Get info about all players in the game
class RIFTBORNAI_API FGetAllPlayersTool : public FAgentTool
{
public:
    FGetAllPlayersTool();
    virtual FString GetName() const override { return TEXT("get_all_players"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_player_viewport - Get player's viewport size and settings
class RIFTBORNAI_API FGetPlayerViewportTool : public FAgentTool
{
public:
    FGetPlayerViewportTool();
    virtual FString GetName() const override { return TEXT("get_player_viewport"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration wrapper class
class RIFTBORNAI_API FPlayerStateAwarenessTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
