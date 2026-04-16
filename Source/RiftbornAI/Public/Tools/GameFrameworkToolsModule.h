// Copyright RiftbornAI. All Rights Reserved.
// Game Framework Tools Module - GameMode/Pawn/Controller wiring and scaffolding

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

// Forward declarations
class UBlueprint;
class AGameModeBase;

/**
 * Game Framework Tools Module
 *
 * Tools for wiring and inspecting the UE5 game framework class chain:
 *
 * - configure_game_framework: Wire GameMode -> Pawn -> Controller -> HUD -> GameState -> PlayerState
 * - get_game_framework_info: Read current world framework configuration
 * - scaffold_game_system: Generate complete game systems (save_load, score_tracker, checkpoint, respawn)
 */
class RIFTBORNAI_API FGameFrameworkToolsModule : public TToolModuleBase<FGameFrameworkToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("GameFrameworkTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_ConfigureGameFramework(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetGameFrameworkInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ScaffoldGameSystem(const FClaudeToolCall& Call);

private:
    // Load a UClass from a Blueprint path or native class name
    static UClass* LoadClassFromPath(const FString& ClassPath);

    // Set a class property on a GameMode CDO
    static bool SetGameModeClassProperty(UBlueprint* GameModeBP, const TCHAR* PropertyName, UClass* NewClass);

    // Scaffold helpers per system type
    static FClaudeToolResult ScaffoldSaveLoad(const FString& Prefix, const FString& Destination, const FString& ToolUseId);
    static FClaudeToolResult ScaffoldScoreTracker(const FString& Prefix, const FString& Destination, const FString& ToolUseId);
    static FClaudeToolResult ScaffoldCheckpoint(const FString& Prefix, const FString& Destination, const FString& ToolUseId);
    static FClaudeToolResult ScaffoldRespawn(const FString& Prefix, const FString& Destination, const FString& ToolUseId);

    // Create a Blueprint with a given parent class, add variables and events, compile and save
    struct FBlueprintSpec
    {
        FString Name;
        FString ParentClass; // e.g. "Actor", "SaveGame"
        FString Destination; // e.g. "/Game/Blueprints"
        TArray<TPair<FString, FString>> Variables; // {Name, TypeString}
        TArray<FString> CustomEvents; // event names to create
    };
    static UBlueprint* CreateBlueprintFromSpec(const FBlueprintSpec& Spec, FString& OutError);

    // Save a Blueprint package to disk
    static bool SaveBlueprintAsset(UBlueprint* Blueprint);
};
