// Copyright RiftbornAI. All Rights Reserved.
// GapClosureIntegration - Wires new competitive gap closure systems into existing infrastructure
//
// This file connects:
//   1. FTieredModelRouter → IAIProvider pipeline (auto-select model by complexity)
//   2. FMultiFileRefactor → AgentToolRegistry (register multi_file_edit, extract_to_new_file tools)
//   3. FAutoSuggestionTrigger → BlueprintCopilotService (auto-trigger on graph events)
//   4. FFirstRunSetup → Module startup (zero-config onboarding)

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"

/**
 * Static integration class that wires the competitive gap closure systems
 * into the existing RiftbornAI infrastructure.
 * 
 * Called from FRiftbornAIModule::StartupModule() after core services are initialized.
 * Reverse cleanup called from ShutdownModule().
 */
class RIFTBORNAI_API FGapClosureIntegration
{
public:
	/**
	 * Wire all gap closure systems into the running infrastructure.
	 * Must be called AFTER:
	 *   - FAgentBootstrap::Initialize() (tool registries exist)
	 *   - FBlueprintCopilotService::RegisterCommands() (copilot service alive)
	 *   - FBlueprintFocusTracker::Initialize() (focus tracking active)
	 */
	static void Initialize();
	
	/**
	 * Tear down all gap closure system connections.
	 * Must be called BEFORE core services shut down.
	 */
	static void Shutdown();
	
	/** Has Initialize() been called this session? */
	static bool IsInitialized() { return bInitialized; }

private:
	/** Wire FFirstRunSetup — auto-detect Ollama, Python, API keys (Gap #5) */
	static void InitializeFirstRunSetup();
	
	/** Wire FTieredModelRouter — complexity-based model selection (Gap #2) */
	static void InitializeTieredRouter();
	
	/** Wire FMultiFileRefactor tools into AgentToolRegistry (Gap #6) */
	static void RegisterMultiFileRefactorTools();
	
	/** Wire FAutoSuggestionTrigger into BlueprintCopilotService (Gap #1) */
	static void InitializeAutoSuggestionTrigger();
	
	/** Wire FProviderFailover — cross-provider auto-failover (Gap #9) */
	static void InitializeProviderFailover();

	/** Run deferred first-run setup after editor startup settles */
	static bool RunDeferredFirstRunSetup(float DeltaTime);
	
	static bool bInitialized;
	static FTSTicker::FDelegateHandle DeferredFirstRunSetupHandle;
};
