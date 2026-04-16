// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * Tools for audio awareness - querying sounds, audio components, ambient zones
 */

// Tool: get_playing_sounds - List all currently playing sounds
class RIFTBORNAI_API FGetPlayingSoundsTool : public FAgentTool
{
public:
    FGetPlayingSoundsTool();
    virtual FString GetName() const override { return TEXT("get_playing_sounds"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_audio_components - List audio components in the scene
class RIFTBORNAI_API FGetAudioComponentsTool : public FAgentTool
{
public:
    FGetAudioComponentsTool();
    virtual FString GetName() const override { return TEXT("get_audio_components"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_ambient_zones - Get all ambient sound zones
class RIFTBORNAI_API FGetAmbientZonesTool : public FAgentTool
{
public:
    FGetAmbientZonesTool();
    virtual FString GetName() const override { return TEXT("get_ambient_zones"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_reverb_volumes - Get reverb volumes and their settings
class RIFTBORNAI_API FGetReverbVolumesTool : public FAgentTool
{
public:
    FGetReverbVolumesTool();
    virtual FString GetName() const override { return TEXT("get_reverb_volumes"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_audio_at_location - Get audio info at a specific location
class RIFTBORNAI_API FGetAudioAtLocationTool : public FAgentTool
{
public:
    FGetAudioAtLocationTool();
    virtual FString GetName() const override { return TEXT("get_audio_at_location"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_sound_classes - Get all sound classes and their settings
class RIFTBORNAI_API FGetSoundClassesTool : public FAgentTool
{
public:
    FGetSoundClassesTool();
    virtual FString GetName() const override { return TEXT("get_sound_classes"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_music_state - Get current music playback state
class RIFTBORNAI_API FGetMusicStateTool : public FAgentTool
{
public:
    FGetMusicStateTool();
    virtual FString GetName() const override { return TEXT("get_music_state"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration wrapper class
class RIFTBORNAI_API FAudioAwarenessTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
