// Copyright RiftbornAI. All Rights Reserved.
// Sequencer Tools Module - Level Sequence creation and manipulation

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Sequencer Tools Module
 * Provides tools for creating and editing Level Sequences (cinematics).
 * 
 * Tools included:
 * - create_level_sequence: Create a new Level Sequence asset
 * - open_sequence: Open a sequence for editing
 * - add_sequence_track: Add an actor track to a sequence
 * - add_keyframe: Add a keyframe to a track
 * - play_sequence: Play a sequence in editor
 * - list_sequences: List all sequences in project
 */
class RIFTBORNAI_API FSequencerToolsModule : public TToolModuleBase<FSequencerToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("SequencerTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_CreateLevelSequence(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_OpenSequence(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddSequenceTrack(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddKeyframe(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PlaySequence(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_StopSequence(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListSequences(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetSequenceInfo(const FClaudeToolCall& Call);
    
};
