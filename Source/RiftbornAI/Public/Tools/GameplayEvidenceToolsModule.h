// Copyright RiftbornAI. All Rights Reserved.
// GameplayEvidenceToolsModule - Tier-A gameplay loop verification tools

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Components/SphereComponent.h"

/**
 * Gameplay Evidence Tools Module
 * 
 * Provides tools for Tier-A gameplay loop verification:
 * - get_arena_game_state: Read GameState properties from PIE
 * - get_gameplay_evidence: Comprehensive evidence snapshot for proof bundles
 * - verify_gameplay_loop: Check if spawn→interact→feedback→terminal exists
 * - simulate_player_action: Trigger game events for testing
 * - inject_nonce_overlay: Display cryptographic nonce on screen for visual proof
 * - clear_nonce_overlay: Remove nonce overlay from screen
 * - spawn_projectile: Spawn a projectile actor in PIE
 * - fire_projectile_from_player: Fire projectile from player's camera
 */
class RIFTBORNAI_API FGameplayEvidenceToolsModule
{
public:
	static void RegisterTools(FClaudeToolRegistry& Registry);

	// Tool handlers
	static FClaudeToolResult Tool_GetArenaGameState(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetGameplayEvidence(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_VerifyGameplayLoop(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SimulatePlayerAction(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_InjectNonceOverlay(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ClearNonceOverlay(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SpawnProjectile(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_FireProjectileFromPlayer(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetArenaTelemetry(const FClaudeToolCall& Call);

private:
	// Helper to register tools with proper error handling
	static void RegisterToolInternal(FClaudeToolRegistry& Registry, const FClaudeTool& Tool, FOnExecuteTool Handler)
	{
		Registry.RegisterTool(Tool, Handler);
	}
	
	// Nonce overlay state
	static FString CurrentNonce;
	static bool bNonceOverlayActive;
};
