// Copyright RiftbornAI. All Rights Reserved.
// Game Persistence Tools Module - Save game creation and testing

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

// Forward declarations
class UBlueprint;
class USaveGame;

/**
 * Game Persistence Tools Module
 *
 * Tools for creating USaveGame Blueprint subclasses and testing save/load cycles:
 *
 * - create_save_game_blueprint: Create a USaveGame BP with typed properties
 * - test_save_load_cycle: During PIE, round-trip save/load and verify data integrity
 */
class RIFTBORNAI_API FGamePersistenceToolsModule : public TToolModuleBase<FGamePersistenceToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("GamePersistenceTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateSaveGameBlueprint(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_TestSaveLoadCycle(const FClaudeToolCall& Call);

private:
    // Map user-facing type strings to FEdGraphPinType for AddMemberVariable
    static bool ResolvePropertyType(const FString& TypeName, struct FEdGraphPinType& OutPinType);

    // Save a Blueprint package to disk
    static bool SaveBlueprintAsset(UBlueprint* Blueprint);
};
