// AutomatedPlaytester.h - AI-driven game testing
// Plays the game like a human would to find bugs, balance issues, and UX problems
// Part of the "Prompt → Full Game" testing infrastructure

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AutomatedPlaytester.generated.h"

/**
 * Playtester persona - different player types
 */
enum class EPlaytesterPersona : uint8
{
    // Skill levels
    Newbie,             // First time gamer
    Casual,             // Plays occasionally
    Core,               // Regular gamer
    Hardcore,           // Skilled gamer
    Speedrunner,        // Optimizes everything
    
    // Playstyles
    Explorer,           // Explores every corner
    Rusher,             // Goes straight to objectives
    Completionist,      // Does everything
    Social,             // Focuses on multiplayer/coop
    Creative,           // Builds and creates
    
    // Test roles
    QATester,           // Tries to break things
    BalanceTester,      // Looks for exploits
    AccessibilityTester // Tests accessibility features
};

/**
 * Test scenario definition
 */
struct FPlaytestScenario
{
    FString Name;
    FString Description;
    
    // Starting conditions
    FString StartLevel;
    TMap<FString, FString> StartState;  // Variable → Value
    
    // Goals
    TArray<FString> Objectives;
    float MaxDurationMinutes = 30.0f;
    
    // Persona
    EPlaytesterPersona Persona = EPlaytesterPersona::Core;
    
    // Focus areas
    bool bTestCombat = true;
    bool bTestProgression = true;
    bool bTestUI = true;
    bool bTestPerformance = true;
    bool bTestAccessibility = false;
};

/**
 * Single issue found during playtest
 */
struct FPlaytestIssue
{
    // Issue classification
    enum class ESeverity : uint8
    {
        Critical,       // Game-breaking
        Major,          // Significant problem
        Minor,          // Annoyance
        Suggestion      // Improvement idea
    };
    
    enum class ECategory : uint8
    {
        Bug,            // Something broken
        Balance,        // Too easy/hard
        UX,             // Confusing or frustrating
        Performance,    // FPS drops, hitches
        Accessibility,  // Hard to see/hear/use
        Polish          // Missing feedback, juice
    };
    
    ESeverity Severity = ESeverity::Minor;
    ECategory Category = ECategory::Bug;
    
    // Details
    FString Title;
    FString Description;
    FString StepsToReproduce;
    
    // Context
    FString Level;
    FVector Location = FVector::ZeroVector;
    float Timestamp = 0.0f;
    FString Screenshot;  // Path to screenshot
    
    // Reproduction
    float ReproductionRate = 1.0f;  // 0-1, how often it happens
    int32 OccurrenceCount = 1;
    
    // Auto-suggested fix
    FString SuggestedFix;
    float FixConfidence = 0.0f;
};

/**
 * Playtest session result
 */
struct FPlaytestResult
{
    FString SessionId;
    FDateTime StartTime;
    FDateTime EndTime;
    
    // Scenario
    FPlaytestScenario Scenario;
    EPlaytesterPersona Persona;
    
    // Completion
    bool bCompleted = false;
    TArray<FString> ObjectivesCompleted;
    float CompletionPercent = 0.0f;
    
    // Issues found
    TArray<FPlaytestIssue> Issues;
    int32 CriticalIssueCount = 0;
    int32 MajorIssueCount = 0;
    int32 MinorIssueCount = 0;
    
    // Metrics
    float TotalPlaytimeMinutes = 0.0f;
    int32 DeathCount = 0;
    int32 RetryCount = 0;
    TMap<FString, float> TimePerArea;  // Area → Minutes
    
    // Performance
    float AverageFPS = 0.0f;
    float MinFPS = 0.0f;
    TArray<FString> PerformanceHotspots;
    
    // Path taken
    TArray<FVector> MovementPath;
    TArray<TPair<float, FString>> Timeline;  // Timestamp → Event
    
    // Frustration detection
    float FrustrationScore = 0.0f;  // 0-1
    TArray<FString> FrustrationMoments;
    
    // Scores
    float OverallScore = 0.0f;  // 0-100
    float FunScore = 0.0f;
    float ChallengeScore = 0.0f;
    float PolishScore = 0.0f;
    
    // Get issues by severity
    TArray<FPlaytestIssue> GetIssuesBySeverity(FPlaytestIssue::ESeverity Severity) const;
    
    // Get issues by category  
    TArray<FPlaytestIssue> GetIssuesByCategory(FPlaytestIssue::ECategory Category) const;
    
    // Generate report
    FString GenerateReport() const;
    FString GenerateMarkdownReport() const;
};

/**
 * Playtest campaign - multiple sessions
 */
struct FPlaytestCampaign
{
    FString Name;
    FString Description;
    
    // Sessions
    TArray<FPlaytestScenario> Scenarios;
    TArray<EPlaytesterPersona> Personas;
    int32 SessionsPerCombination = 3;  // For statistical significance
    
    // Results
    TArray<FPlaytestResult> Results;
    
    // Aggregates
    float AverageCompletionRate = 0.0f;
    float AverageFrustration = 0.0f;
    TMap<FString, int32> IssueFrequency;  // Issue title → Count
    
    // Get most common issues
    TArray<FPlaytestIssue> GetTopIssues(int32 Count) const;
    
    // Get problem areas
    TArray<FString> GetProblemAreas() const;
};

/**
 * AI behavior during playtest
 */
struct FPlaytesterBehavior
{
    // Movement
    float MovementRandomness = 0.3f;  // 0=optimal, 1=random
    bool bExploreDeadEnds = true;
    float ExplorationBias = 0.5f;     // 0=objectives, 1=exploration
    
    // Combat
    float ReactionTime = 0.3f;        // Seconds
    float AimAccuracy = 0.7f;         // 0-1
    float AggressionLevel = 0.5f;     // 0=defensive, 1=aggressive
    bool bUseAbilities = true;
    bool bUseItems = true;
    
    // Decision making
    float OptimalChoiceRate = 0.5f;   // How often picks "best" option
    bool bReadAllDialogue = false;
    bool bInteractWithEverything = false;
    
    // Patience
    float GiveUpThreshold = 10.0f;    // Minutes stuck before giving up
    int32 MaxRetriesBeforeSkip = 5;
    
    // Apply persona preset */
    static FPlaytesterBehavior FromPersona(EPlaytesterPersona Persona);
};

/**
 * FAutomatedPlaytester - AI game tester
 * 
 * Plays the game like a human would, discovering:
 * - Bugs: Crashes, soft locks, broken mechanics
 * - Balance: Too easy, too hard, exploits
 * - UX: Confusing moments, missing feedback
 * - Performance: FPS drops, memory leaks
 * - Accessibility: Color blind issues, subtitle problems
 * 
 * Multiple personas test different playstyles:
 * - Newbie: Can they learn the game?
 * - Speedrunner: Can it be broken?
 * - Explorer: Is everything reachable?
 * - QATester: Deliberately breaks things
 * 
 * Generates detailed reports with:
 * - Prioritized issue list
 * - Video clips of problems
 * - Heat maps of death/frustration
 * - Suggested fixes
 */
class RIFTBORNAI_API FAutomatedPlaytester
{
public:
    static FAutomatedPlaytester& Get();
    
    // ========================================
    // PLAYTEST EXECUTION
    // ========================================
    
    /** Run a single playtest session */
    void RunPlaytest(
        const FPlaytestScenario& Scenario,
        TFunction<void(const FPlaytestResult&)> OnComplete,
        TFunction<void(float Progress, const FString& Status)> OnProgress = nullptr
    );
    
    /** Run a full campaign */
    void RunCampaign(
        const FPlaytestCampaign& Campaign,
        TFunction<void(const FPlaytestCampaign&)> OnComplete,
        TFunction<void(float Progress, const FString& Status)> OnProgress = nullptr
    );
    
    /** Stop current playtest */
    void StopPlaytest();
    
    /** Is playtest running? */
    bool IsPlaytesting() const { return bIsPlaytesting; }
    
    /** Get current progress */
    float GetProgress() const { return CurrentProgress; }

    // ========================================
    // SCENARIO CREATION
    // ========================================
    
    /** Create scenario for level */
    FPlaytestScenario CreateScenarioForLevel(const FString& LevelPath);
    
    /** Create full game scenario */
    FPlaytestScenario CreateFullGameScenario();
    
    /** Create stress test scenario */
    FPlaytestScenario CreateStressTestScenario();
    
    /** Create accessibility scenario */
    FPlaytestScenario CreateAccessibilityScenario();

    // ========================================
    // PERSONA MANAGEMENT
    // ========================================
    
    /** Get behavior for persona */
    FPlaytesterBehavior GetPersonaBehavior(EPlaytesterPersona Persona) const;
    
    /** Create custom persona */
    void RegisterCustomPersona(const FString& Name, const FPlaytesterBehavior& Behavior);

    // ========================================
    // ISSUE DETECTION
    // ========================================
    
    /** Add custom issue detector */
    void AddIssueDetector(const FString& Name, TFunction<TArray<FPlaytestIssue>(const FPlaytestResult&)> Detector);
    
    /** Run issue detectors on result */
    TArray<FPlaytestIssue> DetectIssues(const FPlaytestResult& Result);
    
    /** Auto-categorize issue */
    FPlaytestIssue::ECategory CategorizeIssue(const FString& Description);
    
    /** Auto-suggest fix */
    FString SuggestFix(const FPlaytestIssue& Issue);

    // ========================================
    // ANALYTICS
    // ========================================
    
    /** Generate heat map of deaths */
    void GenerateDeathHeatMap(const FPlaytestResult& Result, const FString& OutputPath);
    
    /** Generate frustration heat map */
    void GenerateFrustrationHeatMap(const FPlaytestResult& Result, const FString& OutputPath);
    
    /** Generate path visualization */
    void GeneratePathVisualization(const FPlaytestResult& Result, const FString& OutputPath);
    
    /** Generate time-per-area chart */
    void GenerateTimeChart(const FPlaytestResult& Result, const FString& OutputPath);

    // ========================================
    // REPORTING
    // ========================================
    
    /** Generate full report */
    void GenerateReport(const FPlaytestResult& Result, const FString& OutputPath);
    
    /** Generate campaign report */
    void GenerateCampaignReport(const FPlaytestCampaign& Campaign, const FString& OutputPath);
    
    /** Export to bug tracker format */
    FString ExportToJiraFormat(const TArray<FPlaytestIssue>& Issues);
    FString ExportToGitHubIssues(const TArray<FPlaytestIssue>& Issues);

    // ========================================
    // CONTINUOUS TESTING
    // ========================================
    
    /** Enable continuous testing mode */
    void EnableContinuousTesting(bool bEnabled);
    
    /** Run playtest on every PIE start */
    void SetTestOnPIE(bool bEnabled);
    
    /** Schedule nightly playtests */
    void ScheduleNightlyTests(const TArray<FPlaytestScenario>& Scenarios);

    // ========================================
    // RECORDING
    // ========================================
    
    /** Enable video recording */
    void SetVideoRecordingEnabled(bool bEnabled);
    
    /** Set screenshot capture for issues */
    void SetScreenshotCaptureEnabled(bool bEnabled);
    
    /** Get recording path */
    FString GetRecordingPath() const;

private:
    FAutomatedPlaytester() = default;
    
    // Playtest execution
    void TickPlaytest(float DeltaTime);
    void ProcessInput(float DeltaTime);
    void EvaluateSituation();
    void MakeDecision();
    void DetectIssuesRealtime();
    
    // Behavior
    FVector ChooseMovementTarget();
    bool ShouldAttack();
    bool ShouldUseAbility();
    FString ChooseDialogueOption();
    
    // Frustration detection
    float CalculateFrustration();
    void RecordFrustrationMoment(const FString& Reason);
    
    // Issue recording
    void RecordIssue(const FPlaytestIssue& Issue);
    void TakeIssueScreenshot(FPlaytestIssue& Issue);
    
    // State
    bool bIsPlaytesting = false;
    float CurrentProgress = 0.0f;
    
    FPlaytestScenario CurrentScenario;
    FPlaytesterBehavior CurrentBehavior;
    FPlaytestResult CurrentResult;
    
    // Execution context
    TWeakObjectPtr<APlayerController> TestPlayerController;
    float PlaytestStartTime = 0.0f;
    float TimeInCurrentArea = 0.0f;
    FString CurrentArea;
    
    // Tracking
    int32 StuckCounter = 0;
    FVector LastPosition = FVector::ZeroVector;
    float LastProgressTime = 0.0f;
    
    // Custom detectors
    TMap<FString, TFunction<TArray<FPlaytestIssue>(const FPlaytestResult&)>> CustomDetectors;
    
    // Custom personas
    TMap<FString, FPlaytesterBehavior> CustomPersonas;
    
    // Settings
    bool bVideoRecordingEnabled = false;
    bool bScreenshotEnabled = true;
    bool bContinuousTestingEnabled = false;
};

/**
 * Playtest controller - possesses AI during tests
 */
UCLASS()
class RIFTBORNAI_API APlaytestController : public APlayerController
{
    GENERATED_BODY()
    
public:
    /** Set behavior parameters */
    void SetBehavior(const FPlaytesterBehavior& Behavior);
    
    /** Get current state */
    FString GetCurrentState() const;
    
    /** Force issue (for testing) */
    void ForceIssue(const FPlaytestIssue& Issue);
    
protected:
    virtual void Tick(float DeltaTime) override;
    
private:
    FPlaytesterBehavior Behavior;
    
    // AI state
    FVector CurrentTarget = FVector::ZeroVector;
    float DecisionCooldown = 0.0f;
    TArray<FVector> RecentPositions;
};

/**
 * Quick playtest helpers
 */
namespace PlaytestHelpers
{
    /** Quick test current level */
    void QuickTestLevel(EPlaytesterPersona Persona = EPlaytesterPersona::Core);
    
    /** Quick test with all personas */
    void QuickTestAllPersonas();
    
    /** Find issues in level */
    TArray<FPlaytestIssue> FindIssuesInLevel(const FString& LevelPath);
    
    /** Estimate level difficulty */
    float EstimateDifficulty(const FString& LevelPath);
    
    /** Check level completable */
    bool IsLevelCompletable(const FString& LevelPath);
}
