// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "RiftbornAIStyle.h"

/**
 * FRiftbornAICommands - UI command bindings for the RiftbornAI plugin
 * 
 * Registers editor commands that can be bound to keyboard shortcuts,
 * menu items, and toolbar buttons. Currently includes:
 * - OpenPluginWindow: Opens the main RiftbornAI chat/assistant window
 * 
 * Usage:
 *   FRiftbornAICommands::Register();
 *   auto& Commands = FRiftbornAICommands::Get();
 *   // Bind Commands.OpenPluginWindow to UI elements
 */
class FRiftbornAICommands : public TCommands<FRiftbornAICommands>
{
public:
	FRiftbornAICommands()
		: TCommands<FRiftbornAICommands>(TEXT("RiftbornAI"), NSLOCTEXT("Contexts", "RiftbornAI", "RiftbornAI Plugin"), NAME_None, FRiftbornAIStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
	TSharedPtr< FUICommandInfo > TriggerBlueprintSuggestion;  // Ctrl+Space in BP editor
};
