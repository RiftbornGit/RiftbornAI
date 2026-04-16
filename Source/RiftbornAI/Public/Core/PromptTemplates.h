// Copyright 2025 RiftbornAI. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PromptTemplates.generated.h"

/**
 * Category of prompt template
 */
UENUM(BlueprintType)
enum class EPromptCategory : uint8
{
    LevelDesign        UMETA(DisplayName = "Level Design"),
    Gameplay           UMETA(DisplayName = "Gameplay Mechanics"),
    Performance        UMETA(DisplayName = "Performance Optimization"),
    AI                 UMETA(DisplayName = "AI & NPCs"),
    Animation          UMETA(DisplayName = "Animation"),
    Audio              UMETA(DisplayName = "Audio"),
    Physics            UMETA(DisplayName = "Physics"),
    UI                 UMETA(DisplayName = "UI/UX"),
    Multiplayer        UMETA(DisplayName = "Multiplayer/Networking"),
    Graphics           UMETA(DisplayName = "Graphics & Rendering"),
    Blueprint          UMETA(DisplayName = "Blueprints"),
    CPlusPlus          UMETA(DisplayName = "C++ Code"),
    Debug              UMETA(DisplayName = "Debugging"),
    Balance            UMETA(DisplayName = "Game Balance"),
    General            UMETA(DisplayName = "General")
};

/**
 * A single prompt template
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPromptTemplate
{
    GENERATED_BODY()

    // Template ID
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString TemplateId;

    // Display name
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Name;

    // Description of what this prompt helps with
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Description;

    // Category
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    EPromptCategory Category = EPromptCategory::General;

    // The actual prompt text with {placeholders}
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString PromptText;

    // Parameters that need to be filled in
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> Parameters;

    // Example filled prompt
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Example;

    // Keywords for search
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> Keywords;

    // Usage count for popularity
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    int32 UsageCount = 0;
};

/**
 * Context gathered from the project for prompt templates
 * NOTE: Named differently from FPromptProjectContext in AgentSystemPrompt.h
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPromptProjectContext
{
    GENERATED_BODY()

    // Current level name
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString CurrentLevel;

    // Selected actor(s)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> SelectedActors;

    // Open Blueprint (if any)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString OpenBlueprint;

    // Recent errors
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> RecentErrors;

    // Performance snapshot
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float CurrentFPS = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float MemoryUsageMB = 0.0f;

    // Active game mode
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString GameMode;

    // Player class
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString PlayerClass;

    // Custom context data
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TMap<FString, FString> CustomData;
};

/**
 * Result from processing a prompt
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPromptResult
{
    GENERATED_BODY()

    // Was processing successful
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    bool bSuccess = false;

    // The generated response
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    FString Response;

    // Actionable suggestions
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> Suggestions;

    // Code snippets if applicable
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> CodeSnippets;

    // Blueprint node suggestions
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> BlueprintNodes;

    // Relevant documentation links
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> DocLinks;

    // Follow-up questions
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    TArray<FString> FollowUpQuestions;
};

/**
 * Prompt Templates Manager - provides pre-built prompts for game development
 */
class RIFTBORNAI_API FPromptTemplates
{
public:
    static FPromptTemplates& Get();

    // Template management
    void RegisterTemplate(const FPromptTemplate& Template);
    FPromptTemplate GetTemplate(const FString& TemplateId) const;
    TArray<FPromptTemplate> GetTemplatesByCategory(EPromptCategory Category) const;
    TArray<FPromptTemplate> SearchTemplates(const FString& Query) const;
    TArray<FPromptTemplate> GetPopularTemplates(int32 Count = 10) const;
    TArray<FPromptTemplate> GetAllTemplates() const { return Templates; }

    // Prompt execution
    FString FillTemplate(const FString& TemplateId, const TMap<FString, FString>& Parameters);
    FPromptResult ProcessPrompt(const FString& Prompt, const FPromptProjectContext& Context);
    FString BuildContextualPrompt(const FString& BasePrompt, UWorld* World);

    // Context gathering
    FPromptProjectContext GatherContext(UWorld* World);
    void AddCustomContext(const FString& Key, const FString& Value);

    // Quick prompts (pre-filled templates for common tasks)
    FString GetPerformanceAnalysisPrompt(UWorld* World);
    FString GetLevelDesignReviewPrompt(UWorld* World);
    FString GetBlueprintOptimizationPrompt(const FString& BlueprintName);
    FString GetAIBehaviorPrompt(const FString& AIClassName);
    FString GetBalanceAnalysisPrompt(const FString& EntityName);
    FString GetDebugHelpPrompt(const FString& ErrorMessage);

    // Natural language processing
    FPromptTemplate SuggestTemplate(const FString& UserInput);
    TArray<FString> ExtractParameters(const FString& UserInput);

private:
    FPromptTemplates();
    void InitializeDefaultTemplates();

    TArray<FPromptTemplate> Templates;
    TMap<FString, FString> CustomContextData;
};
