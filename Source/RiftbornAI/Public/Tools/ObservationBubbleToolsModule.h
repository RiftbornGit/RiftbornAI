// Copyright RiftbornAI. All Rights Reserved.
// Observation Bubble Tools — Create and manage observation-driven simulation bubbles

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Observation Bubble Tools Module
 *
 * Exposes the observation bubble system to Claude/MCP:
 * - create_observation_bubble: Configure and enable the bubble system
 * - get_bubble_state: Query zone counts, FPS, radius scale, transitions
 * - set_bubble_radius: Update active/peripheral/dormant radii
 * - set_bubble_mode: Switch between fixed/performance/manual adaptation
 * - register_bubble_actors: Bulk-register actors by class, tag, or all
 * - get_actor_observation_zone: Query a specific actor's zone and distance
 * - inspect_bubble_live: Rich diagnostic dump of actors per zone
 */
class RIFTBORNAI_API FObservationBubbleToolsModule : public TToolModuleBase<FObservationBubbleToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ObservationBubbleTools"); }
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateObservationBubble(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetBubbleState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetBubbleRadius(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetBubbleMode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RegisterBubbleActors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetActorObservationZone(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InspectBubbleLive(const FClaudeToolCall& Call);
};
