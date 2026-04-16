// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgenticLoopRunner.h"
#include "ProjectMemoryManager.h"
#include "FileSystemTools.h"
#include "HotReloadIntegration.h"
#include "AgentSystemPrompt.h"
#include "AgentTransactionManager.h"
#include "ClaudeToolUse.h"

/**
 * Agent Bootstrap - Initializes and connects all agent subsystems
 * 
 * Call FAgentBootstrap::Initialize() on plugin startup to wire everything together.
 */
class RIFTBORNAI_API FAgentBootstrap
{
public:
    static FAgentBootstrap& Get();
    
    /** Initialize all agent subsystems */
    void Initialize();
    
    /** Shutdown all agent subsystems */
    void Shutdown();
    
    /** Check if agent is fully initialized */
    bool IsInitialized() const { return bInitialized; }
    
    /** Get summary of registered tools */
    FString GetToolSummary() const;
    
    /** Get status of all subsystems */
    FString GetSubsystemStatus() const;

    /** Re-register all tools (useful after Hot Reload to refresh function pointers) */
    void ReloadTools();
    
private:
    FAgentBootstrap() = default;
    
    bool bInitialized = false;
    FString ProjectPath;
    
    void InitializeProjectPath();
    void RegisterAllTools();
    void LoadProjectMemory();
    void BuildInitialContext();
};

/**
 * Convenience functions for common agent operations
 */
namespace AgentOps
{
    /** Execute a prompt through the full agentic loop */
    RIFTBORNAI_API void ExecutePrompt(
        const FString& UserPrompt,
        TFunction<void(const FString&)> OnComplete,
        TFunction<void(const FString&)> OnError = nullptr
    );
    
    /** Execute a prompt with transaction wrapping (auto-rollback on failure) */
    RIFTBORNAI_API void ExecutePromptWithTransaction(
        const FString& TransactionName,
        const FString& UserPrompt,
        TFunction<void(const FString&)> OnComplete,
        TFunction<void(const FString&)> OnError = nullptr
    );
    
    /** Quick Blueprint creation from natural language */
    RIFTBORNAI_API void CreateBlueprint(
        const FString& Description,
        TFunction<void(const FString& AssetPath)> OnComplete
    );
    
    /** Quick C++ class creation from natural language */
    RIFTBORNAI_API void CreateCppClass(
        const FString& Description,
        TFunction<void(const TArray<FString>& CreatedFiles)> OnComplete
    );
    
    /** Get agent capabilities as structured data */
    RIFTBORNAI_API FString GetCapabilities();
}
