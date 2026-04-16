// Copyright RiftbornAI. All Rights Reserved.
// Config Tools Module - Configuration and settings tools

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Config Tools Module
 * Provides tools for project configuration and settings.
 * 
 * Tools include:
 * - get_project_settings
 * - set_project_setting
 * - get_input_mappings
 * - set_input_mapping
 * - get_collision_settings
 * - set_collision_response
 * - get_game_mode
 * - set_game_mode
 */
class RIFTBORNAI_API FConfigToolsModule : public TToolModuleBase<FConfigToolsModule>
{
public:
    /** Module name for registration */
    static FString StaticModuleName() { return TEXT("ConfigTools"); }
    
    /** Register all Config tools with the registry */
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
};
