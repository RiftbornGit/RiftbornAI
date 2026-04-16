// AudioAwareness.h - Audio and sound understanding for AI Agents
// Provides agents with awareness of sounds, music, ambient audio, and audio zones

#pragma once

#include "CoreMinimal.h"
#include "AudioAwareness.generated.h"

// ============================================================================
// AUDIO ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EAudioCategory : uint8
{
    Music,
    Ambient,
    SFX,
    Voice,
    UI,
    Foley,
    Weather,
    Combat,
    Unknown
};

UENUM(BlueprintType)
enum class EMusicState : uint8
{
    None,
    Exploration,
    Combat,
    Tension,
    Victory,
    Defeat,
    Menu,
    Cinematic,
    Custom
};

UENUM(BlueprintType)
enum class EAmbientZone : uint8
{
    None,
    Indoor,
    Outdoor,
    Cave,
    Underwater,
    Forest,
    City,
    Desert,
    Snow,
    Industrial,
    Custom
};

// ============================================================================
// AUDIO DATA STRUCTURES
// ============================================================================

/** Information about a playing sound */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPlayingSoundInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SoundName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SoundAssetPath;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAudioCategory Category = EAudioCategory::Unknown;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Volume = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Pitch = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsLooping = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIs3D = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float AttenuationRadius = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float TimeRemaining = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString OwnerActorName;
};

/** Audio zone information */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAudioZoneInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ZoneName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAmbientZone ZoneType = EAmbientZone::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Center = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Extent = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float ReverbAmount = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AmbientSoundName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Priority = 0;
};

/** Current audio state summary */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAudioState
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EMusicState MusicState = EMusicState::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString CurrentMusicTrack;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAmbientZone CurrentAmbientZone = EAmbientZone::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MasterVolume = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MusicVolume = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float SFXVolume = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 ActiveSoundCount = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FPlayingSoundInfo> PlayingSounds;
    
    FString GetDescription() const;
};

// ============================================================================
// AUDIO AWARENESS SYSTEM
// ============================================================================

/**
 * Audio Awareness System
 * 
 * Provides agents with understanding of:
 * - Currently playing sounds
 * - Music state (combat, exploration, etc.)
 * - Ambient sound zones
 * - Audio component states
 * - Reverb zones and audio effects
 * 
 * Usage:
 *   FAudioAwareness& AA = FAudioAwareness::Get();
 *   FAudioState State = AA.GetCurrentAudioState();
 *   TArray<FPlayingSoundInfo> Sounds = AA.GetPlayingSounds();
 */
class RIFTBORNAI_API FAudioAwareness
{
public:
    static FAudioAwareness& Get();
    
    // =========================================================================
    // AUDIO STATE
    // =========================================================================
    
    /** Get complete audio state */
    FAudioState GetCurrentAudioState() const;
    
    /** Get current music state */
    EMusicState GetMusicState() const;
    
    /** Get current ambient zone at position */
    EAmbientZone GetAmbientZoneAtPosition(const FVector& Position) const;
    
    // =========================================================================
    // PLAYING SOUNDS
    // =========================================================================
    
    /** Get all currently playing sounds */
    TArray<FPlayingSoundInfo> GetPlayingSounds() const;
    
    /** Get sounds by category */
    TArray<FPlayingSoundInfo> GetSoundsByCategory(EAudioCategory Category) const;
    
    /** Get sounds near a position */
    TArray<FPlayingSoundInfo> GetSoundsNearPosition(const FVector& Position, float Radius) const;
    
    /** Check if a specific sound is playing */
    bool IsSoundPlaying(const FString& SoundName) const;
    
    /** Get sound count by category */
    int32 GetSoundCount(EAudioCategory Category) const;
    
    // =========================================================================
    // AUDIO COMPONENTS
    // =========================================================================
    
    /** Get all audio components in level */
    TArray<FPlayingSoundInfo> GetAudioComponentsInLevel() const;
    
    /** Get audio components on actor */
    TArray<FPlayingSoundInfo> GetAudioComponentsOnActor(AActor* Actor) const;
    
    // =========================================================================
    // AUDIO ZONES
    // =========================================================================
    
    /** Get all audio zones in level */
    TArray<FAudioZoneInfo> GetAudioZones() const;
    
    /** Get audio zone at position */
    FAudioZoneInfo GetAudioZoneAtPosition(const FVector& Position) const;
    
    /** Get reverb settings at position */
    float GetReverbAtPosition(const FVector& Position) const;
    
    // =========================================================================
    // AUDIO SETTINGS
    // =========================================================================
    
    /** Get master volume */
    float GetMasterVolume() const;
    
    /** Get music volume */
    float GetMusicVolume() const;
    
    /** Get SFX volume */
    float GetSFXVolume() const;
    
    /** Get listener position */
    FVector GetListenerPosition() const;
    
    // =========================================================================
    // MUSIC
    // =========================================================================
    
    /** Get current music track name */
    FString GetCurrentMusicTrack() const;
    
    /** Check if music is playing */
    bool IsMusicPlaying() const;
    
    /** Get music intensity (0-1) */
    float GetMusicIntensity() const;
    
    // =========================================================================
    // UTILITY
    // =========================================================================
    
    /** Convert audio category to string */
    static FString CategoryToString(EAudioCategory Category);
    
    /** Convert music state to string */
    static FString MusicStateToString(EMusicState State);
    
    /** Convert ambient zone to string */
    static FString AmbientZoneToString(EAmbientZone Zone);
    
private:
    FAudioAwareness();
    
    // Track music state
    EMusicState CurrentMusicState = EMusicState::None;
    FString CurrentTrackName;
};
