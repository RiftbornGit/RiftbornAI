// Copyright RiftbornAI. All Rights Reserved.
// Sound Tools Module - Audio asset creation and playback

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Sound Tools Module
 * Provides tools for working with audio assets and sound playback.
 * 
 * Tools included:
 * - create_sound_cue: Create a SoundCue from SoundWaves
 * - get_sound_info: Inspect authored SoundWave or SoundCue state
 * - play_sound_at_location: Play a sound at a world location
 * - play_sound_2d: Play a 2D sound (UI, music)
 * - spawn_audio_component: Spawn a persistent audio component
 * - list_sounds: List all sound assets
 */
class RIFTBORNAI_API FSoundToolsModule : public TToolModuleBase<FSoundToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("SoundTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_CreateSoundCue(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetSoundInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PlaySoundAtLocation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PlaySound2D(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnAudioComponent(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_StopAllSounds(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListSounds(const FClaudeToolCall& Call);
    
};
