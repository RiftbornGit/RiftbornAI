// Copyright RiftbornAI. All Rights Reserved.
// Widget & UI Animation Tools Module

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Widget/UMG Tools Module
 * Provides tools for creating and animating UMG widgets
 */
class RIFTBORNAI_API FWidgetToolsModule : public TToolModuleBase<FWidgetToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("WidgetTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateWidget(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddWidgetChild(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddWidgetAnimation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PlayWidgetAnimation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetProperty(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetPosition(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetSize(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetAnchors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetVisibility(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetIsEnabled(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetFocusable(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetNavigationRule(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetNavigationRoutingPolicy(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetNavigationMethod(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetWidgetNavigationState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyWidgetInteractionState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddWidgetPropertyBinding(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RemoveWidgetPropertyBinding(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetTextBlockText(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetTextBlockFontSize(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetProgressBarPercent(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetImageBrush(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateWidgetFromJson(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetWidgetInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetWidgetEditorContext(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListWidgetTree(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CompileWidgetBlueprint(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RenameWidget(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListWidgets(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddWidgetToViewport(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_VerifyWidgetBlueprintLayout(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLiveViewportWidgets(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_VerifyWidgetPresentInPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AssertWidgetVisibleInPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWidgetFocusInPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetFocusedWidgetInPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SimulateWidgetNavigationInPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_VerifyWidgetFocusTraversalInPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RunWidgetInteractionScriptInPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_VerifyHudFlowInPIE(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RunUIFlowTest(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CaptureUIState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CaptureWidgetAtResolution(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RunHudResolutionSweep(const FClaudeToolCall& Call);
};
