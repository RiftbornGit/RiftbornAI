// Copyright RiftbornAI. All Rights Reserved.
// PIE Input Injection Tools - Simulate player input during Play-In-Editor

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

// Forward declarations
class APlayerController;
class UWorld;

/**
 * PIE Input Tools Module
 * Injects input actions, key presses, and mouse movement into the PIE session.
 * All tools require PIE to be running (start_pie first).
 */
class RIFTBORNAI_API FPIEInputToolsModule : public TToolModuleBase<FPIEInputToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("PIEInputTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    /** Inject an Enhanced Input action into the PIE player controller */
    static FClaudeToolResult Tool_InjectInputAction(const FClaudeToolCall& Call);

    /** Simulate a raw key press during PIE */
    static FClaudeToolResult Tool_InjectKeyPress(const FClaudeToolCall& Call);

    /** Simulate mouse movement during PIE */
    static FClaudeToolResult Tool_InjectMouseMove(const FClaudeToolCall& Call);

private:
    /** Get the PIE world, or nullptr if PIE is not running. */
    static UWorld* GetPIEWorld();

    /** Get the first local player controller in PIE, or nullptr. */
    static APlayerController* GetPIEPlayerController();
};
