// Copyright RiftbornAI. All Rights Reserved.
// Level Snapshot tools for world-state capture and restoration.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FLevelSnapshotToolsModule
 * 
 * Level Snapshot tools for world-state capture and restoration.
 */
class RIFTBORNAI_API FLevelSnapshotToolsModule : public TToolModuleBase<FLevelSnapshotToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("LevelSnapshotTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CaptureLevelSnapshot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RestoreLevelSnapshot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListLevelSnapshots(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLevelSnapshotInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CaptureDestructionSnapshot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RestoreDestructionSnapshot(const FClaudeToolCall& Call);
};
