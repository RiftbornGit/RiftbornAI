// Copyright RiftbornAI. All Rights Reserved.
// Gameplay Camera asset and component orchestration tools.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FGameplayCamerasToolsModule
 * 
 * Gameplay Camera asset and component orchestration tools.
 */
class RIFTBORNAI_API FGameplayCamerasToolsModule : public TToolModuleBase<FGameplayCamerasToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("GameplayCamerasTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateGameplayCameraAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateCameraRigAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddGameplayCameraComponent(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetGameplayCameraAssetInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetGameplayCameraAsset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ActivatePersistentCameraRig(const FClaudeToolCall& Call);
};
