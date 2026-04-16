// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

/**
 * DEPRECATED: Legacy Blueprint Generator Chat Widget
 * 
 * This is the OLD chat interface that talks to the external Riftborn Companion
 * app via HTTP bridge (port 8766). It is NOT the governance-grade Copilot.
 * 
 * For the real Copilot with execution authority, use SRiftbornCopilotPanel.
 * 
 * This widget remains for backward compatibility with users who have
 * workflows built around the external companion app.
 * 
 * @deprecated Use SRiftbornCopilotPanel instead for governed tool execution.
 */
class SRiftbornChatWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SRiftbornChatWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Chat history display
    TSharedPtr<SMultiLineEditableTextBox> ChatHistory;
    
    // User input
    TSharedPtr<SEditableTextBox> UserInput;
    
    // Message buffer
    FString MessageHistory;
    
    // Handlers
    void OnSendMessage();
    FReply OnSendButtonClicked();
    FReply OnClearButtonClicked();
    
    // Process user commands via HTTP bridge
    void ProcessCommand(const FString& Command);
    
    // Send chat request to HTTP bridge
    void SendChatRequest(const FString& Message);
    
    // Poll job status for async operations
    void PollJobStatus(const FString& JobId);
    
    // Add message to chat
    void AddMessage(const FString& Sender, const FString& Message, const FLinearColor& Color = FLinearColor::White);
    
    // Quick action buttons
    FReply OnQuickAction_SimpleHealth();
    FReply OnQuickAction_FullCombat();
    FReply OnQuickAction_ActionBar();
};
