// Copyright RiftbornAI. All Rights Reserved.
// RomanCityToolsModule — Roman modular kit, street layout, district composition, audit, dressing, and review

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FRomanCityToolsModule
 * 
 * RomanCityToolsModule — Roman modular kit, street layout, district composition, audit, dressing, and review
 */
class RIFTBORNAI_API FRomanCityToolsModule : public TToolModuleBase<FRomanCityToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("RomanCityTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateRomanColumn(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateRomanArch(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateRomanWallBay(const FClaudeToolCall& Call);

    static FClaudeToolResult Tool_CreateRomanStreetGrid(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ComposeRomanForum(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ComposeRomanInsulaBlock(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ComposeRomanDistrict(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AuditRomanDistrict(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DressRomanDistrict(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ReviewRomanDistrict(const FClaudeToolCall& Call);

    // Phase 1: Building types (satellite: RomanCityToolsModule_Buildings.cpp)
    static FClaudeToolResult Tool_CreateRomanTemple(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateRomanAqueductSpan(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateRomanCityWall(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateRomanTabernaeRow(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateRomanTriumphalArch(const FClaudeToolCall& Call);

    // Phase 2: Roofing + Multi-Story (satellite: RomanCityToolsModule_Roofing.cpp)
    static FClaudeToolResult Tool_AddRomanRoof(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddRomanPediment(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_StackInsulaFloors(const FClaudeToolCall& Call);

    // Phase 4: Ground + Props + Materials (satellite: RomanCityToolsModule_Dressing.cpp)
    static FClaudeToolResult Tool_CreateRomanGroundSurface(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ScatterRomanProps(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyRomanMaterialVariety(const FClaudeToolCall& Call);
};
