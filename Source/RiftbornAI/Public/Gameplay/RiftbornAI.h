// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

/**
 * RiftbornAI Editor Module
 * Unreal Engine editor assistant module backed by external tool and model providers
 */
class FRiftbornAIModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** Opens the AI Assistant window */
	void OpenAIAssistant();
	
	/** Returns true if running in unattended/commandlet mode */
	bool IsUnattendedMode() const { return bIsUnattendedMode; }

private:
	void RegisterMenus();
	bool TickEditorLoadDiagnostics(float DeltaTime);
	void UpdateBlueprintFixServiceState(bool bLogDecision);
	void OnCodexUIButtonClicked();
	void OnCreateArenaWizardButtonClicked();
	void OnCopilotPanelButtonClicked();
	void OnAIAssistantButtonClicked();
	void StartPythonBridgeWatcher();

	TSharedPtr<class FUICommandList> PluginCommands;
	TUniquePtr<class FRiftbornEditorLoadDiagnostics> EditorLoadDiagnostics;
	bool bBridgeWatcherInitialized = false;
	bool bBlueprintFixServiceStarted = false;
	bool bIsUnattendedMode = false;
	bool bStartupCompleted = false;
	bool bTabSpawnerRegistered = false;
	FTSTicker::FDelegateHandle ModalDismissHandle;
	FTSTicker::FDelegateHandle EditorLoadDiagnosticsHandle;
};
