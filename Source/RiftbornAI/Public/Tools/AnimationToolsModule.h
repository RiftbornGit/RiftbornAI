// AnimationToolsModule.h
// Animation management tools for RiftbornAI
// Tools: create_anim_blueprint, add_anim_state, add_anim_transition, play_animation_montage,
//        create_blend_space, set_anim_slot, get_skeleton_bones

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Animation Tools Module
 * 
 * Provides tools for creating and managing animations:
 * - create_anim_blueprint: Create new animation blueprint for a skeleton
 * - add_anim_state: Add state to animation state machine
 * - add_anim_transition: Add transition between states
 * - play_animation_montage: Play animation montage on actor
 * - create_blend_space: Create 1D or 2D blend space
 * - set_anim_slot: Set animation in a montage slot
 * - get_skeleton_bones: Get list of bones in a skeleton
 */
class RIFTBORNAI_API FAnimationToolsModule : public TToolModuleBase<FAnimationToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("AnimationTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_CreateAnimBlueprint(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddAnimState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddAnimTransition(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PlayAnimationMontage(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateBlendSpace(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetAnimSlot(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetSkeletonBones(const FClaudeToolCall& Call);
    
};
