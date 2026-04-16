// Copyright RiftbornAI. All Rights Reserved.
// Cloth Simulation Tools Module — Cloth painting, wind, simulation configuration

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Cloth Sim Tools Module
 *
 * Provides tools for cloth simulation:
 * - create_cloth_asset: Create a cloth simulation asset for a skeletal mesh
 * - set_cloth_config: Configure cloth simulation parameters (mass, stiffness, damping)
 * - set_cloth_wind: Configure wind-related cloth config properties on mesh cloth assets
 * - get_cloth_info: Query cloth assets and their configurations
 * - set_cloth_collision: Configure collision thickness and self-collision properties on cloth configs
 * - enable_cloth_on_component: Enable/disable cloth on a skeletal mesh component
 */
class RIFTBORNAI_API FClothSimToolsModule : public TToolModuleBase<FClothSimToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ClothSimTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateClothAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetClothConfig(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetClothWind(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetClothInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetClothCollision(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EnableClothOnComponent(const FClaudeToolCall& Call);
};
