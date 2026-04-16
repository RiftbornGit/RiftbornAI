// BlueprintPatternRecognizer.h
// Recognizes common blueprint and code patterns from natural language

#pragma once

#include "CoreMinimal.h"

/**
 * Common gameplay patterns
 */
enum class EGameplayPattern : uint8
{
    None,
    
    // Movement Patterns
    TopDownMovement,        // Click to move, diablo style
    ThirdPersonMovement,    // Over the shoulder camera
    FirstPersonMovement,    // FPS style
    PlatformerMovement,     // 2D/3D platformer
    VehicleMovement,        // Car/plane/boat
    FlyingMovement,         // Free flight
    
    // Combat Patterns
    MeleeCombo,             // Combo attack system
    RangedShooting,         // Gun/bow shooting
    AbilityCooldown,        // Ability with cooldown
    DamageOverTime,         // DoT effect
    AreaOfEffect,           // AOE damage/heal
    Projectile,             // Spawned projectile
    Hitscan,                // Instant raycast
    
    // AI Patterns
    PatrolBehavior,         // Walk between points
    ChaseBehavior,          // Follow target
    FleeBehavior,           // Run from target
    GroupBehavior,          // Swarm/flock
    BossPattern,            // Phase-based boss
    
    // UI Patterns
    HealthBar,              // Health display
    Inventory,              // Item grid
    DialogueSystem,         // Conversation UI
    Minimap,                // Map overlay
    QuestTracker,           // Objective list
    
    // System Patterns
    SaveLoadGame,           // Persistence
    ObjectPooling,          // Object reuse
    SpawnSystem,            // Enemy/item spawning
    LootDrops,              // Random item drops
    LevelProgression,       // XP/leveling
    AchievementSystem,      // Achievements
    
    // Interaction Patterns
    InteractPrompt,         // "Press E to interact"
    PickupItem,             // Collectible
    Door,                   // Open/close door
    Trigger,                // Trigger volume
    Checkpoint,             // Save point
};

/**
 * Pattern match result with confidence
 */
struct FPatternMatch
{
    EGameplayPattern Pattern = EGameplayPattern::None;
    float Confidence = 0.0f;
    FString PatternName;
    FString Description;
    TArray<FString> RequiredClasses;
    TArray<FString> RequiredComponents;
    TArray<FString> SuggestedNodes;
    TArray<FString> ImplementationSteps;
};

/**
 * Common code structure patterns
 */
struct FCodePattern
{
    FString PatternName;
    FString Description;
    TArray<FString> Keywords;
    FString CppTemplate;
    FString BlueprintImplementation;
    TArray<FString> BestPractices;
};

/**
 * Recognizes gameplay and code patterns from descriptions
 */
class RIFTBORNAI_API FBlueprintPatternRecognizer
{
public:
    FBlueprintPatternRecognizer();
    
    /** Recognize patterns from a description */
    TArray<FPatternMatch> RecognizePatterns(const FString& Description) const;
    
    /** Get best matching pattern */
    FPatternMatch GetBestPattern(const FString& Description) const;
    
    /** Get implementation details for a pattern */
    FPatternMatch GetPatternDetails(EGameplayPattern Pattern) const;
    
    /** Get code template for pattern */
    FCodePattern GetCodePattern(const FString& PatternName) const;
    
    /** Suggest patterns for a game genre */
    TArray<FPatternMatch> GetPatternsForGenre(const FString& Genre) const;
    
    /** Check if description implies a pattern */
    bool ImpliesPattern(const FString& Description, EGameplayPattern Pattern) const;
    
private:
    void InitializePatterns();
    void InitializeCodeTemplates();
    
    TMap<EGameplayPattern, FPatternMatch> PatternDatabase;
    TMap<FString, FCodePattern> CodeTemplates;
    TMap<FString, TArray<EGameplayPattern>> GenrePatterns;
};
