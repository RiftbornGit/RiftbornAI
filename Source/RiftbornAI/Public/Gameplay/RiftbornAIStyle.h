// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"

/**
 * FRiftbornAIStyle - Slate styling for the RiftbornAI plugin UI
 * 
 * Manages custom icons, brushes, and visual styles used throughout
 * the plugin's editor interface. Handles loading/unloading of style
 * resources and provides access to the style set.
 * 
 * Lifecycle:
 *   Initialize() - Call on module startup to register styles
 *   Shutdown() - Call on module shutdown to cleanup
 *   
 * Access:
 *   Get() - Returns the ISlateStyle interface for this plugin
 *   GetStyleSetName() - Returns the FName identifier "RiftbornAIStyle"
 */
class FRiftbornAIStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static const ISlateStyle& Get();
	static FName GetStyleSetName();
	static FSlateFontInfo GetUIFont(float Size, bool bBold = false);
	static FSlateFontInfo GetChatFont(float Size, bool bBold = false);
	static FSlateFontInfo GetMonoFont(float Size);
	static const FEditableTextBoxStyle& GetComposerTextBoxStyle();

	/** 1536x1024 hero splash used as the onboarding-panel banner. Shipped
	 *  via Resources/UI/RiftbornAI_Splash.png. Use with SImage(.Image(...)). */
	static const FSlateBrush* GetSplashBrush();

private:
	static TSharedRef< class FSlateStyleSet > Create();
	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};
