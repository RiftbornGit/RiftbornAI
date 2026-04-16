// Copyright RiftbornAI. All Rights Reserved.
// Camera Tools Module — Create and configure cameras, spring arms, camera shakes
//
// Games often need cinematic cameras. This module provides scripted control
// over camera actors, cine cameras, spring arms, and camera shake effects.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Camera Tools Module
 *
 * Provides tools for camera management:
 * - spawn_camera: Spawn a camera or cine camera actor
 * - set_camera_properties: Configure FOV, aspect ratio, focus, filmback
 * - create_spring_arm: Add a spring arm component to an actor
 * - set_spring_arm_properties: Configure length, lag, collision, offset
 * - add_camera_shake: Trigger or create a camera shake effect
 * - get_camera_info: Query text or JSON camera actor state
 */
class RIFTBORNAI_API FCameraToolsModule : public TToolModuleBase<FCameraToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("CameraTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_SpawnCamera(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetCameraProperties(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateSpringArm(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSpringArmProperties(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddCameraShake(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetCameraInfo(const FClaudeToolCall& Call);

};
