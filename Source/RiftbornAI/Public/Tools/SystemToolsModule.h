// Copyright RiftbornAI. All Rights Reserved.
// System Tools Module - Editor system and utility tools

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * System Tools Module
 * Provides tools for editor system operations and utilities.
 * 
 * Tools include:
 * - play_in_editor
 * - stop_play
 * - get_project_info
 * - get_tool_stats
 * - execute_console_command
 * - garbage_collect
 * - get_memory_stats
 * - get_fps_stats
 */
class RIFTBORNAI_API FSystemToolsModule : public TToolModuleBase<FSystemToolsModule>
{
public:
    /** Module name for registration */
    static FString StaticModuleName() { return TEXT("SystemTools"); }
    
    /** Register all System tools with the registry */
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
};
