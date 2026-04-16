// NPCLifecycleToolsModule.h
// NPC lifecycle helpers: spawn a Character, optionally wire AI control, then configure patrol.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FNPCLifecycleToolsModule
 * 
 * NPC lifecycle helpers: spawn a Character, optionally wire AI control, then configure patrol.
 */
class RIFTBORNAI_API FNPCLifecycleToolsModule : public TToolModuleBase<FNPCLifecycleToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("NPCLifecycle"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// spawn_npc: Spawns character, assigns AI controller, optionally runs behavior tree
	static FClaudeToolResult Tool_SpawnNPC(const FClaudeToolCall& Call);

	// configure_npc_patrol: Creates patrol points and assigns them to NPC blackboard
	static FClaudeToolResult Tool_ConfigureNPCPatrol(const FClaudeToolCall& Call);

	// get_npc_status: Reports AI state, current BT node, blackboard values, perception
	static FClaudeToolResult Tool_GetNPCStatus(const FClaudeToolCall& Call);

	// despawn_npc: Cleanly removes NPC and its AI controller
	static FClaudeToolResult Tool_DespawnNPC(const FClaudeToolCall& Call);

	// spawn_npc_group: Spawns multiple NPCs at positions with shared BT
	static FClaudeToolResult Tool_SpawnNPCGroup(const FClaudeToolCall& Call);

	// create_patrol_behavior_tree: Creates a patrol BT + blackboard scaffold asset
	static FClaudeToolResult Tool_CreatePatrolBehaviorTree(const FClaudeToolCall& Call);

private:
	static class AAIController* GetOrCreateAIController(class APawn* Pawn);
	static bool AssignBehaviorTree(class AAIController* Controller, const FString& BTPath);
	static bool SetupPerception(class AAIController* Controller, bool bSight, bool bHearing, bool bDamage);
};
