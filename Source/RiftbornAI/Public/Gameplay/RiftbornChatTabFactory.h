// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

/**
 * FRiftbornChatTabFactory - Factory for creating RiftbornAI chat tabs
 * 
 * Creates and manages the AI chat interface tab within asset editors
 * and other workflow-oriented applications. The tab provides:
 * - Natural language input for AI commands
 * - Response display area
 * - Tool execution feedback
 * 
 * Can be embedded in Blueprint editors, Material editors, etc.
 * to provide context-aware AI assistance.
 * 
 * @see SRiftbornChatWidget for the actual tab content
 */
class FRiftbornChatTabFactory : public FWorkflowTabFactory
{
public:
    FRiftbornChatTabFactory(TSharedPtr<class FAssetEditorToolkit> InHostingApp);
    
    virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
    
    static const FName TabID;
};
