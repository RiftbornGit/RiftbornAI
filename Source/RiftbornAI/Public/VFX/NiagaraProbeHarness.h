// Copyright Hivemind Studios. All Rights Reserved.
// NiagaraEnv.h - Reinforcement Learning Environment for Niagara VFX Authoring
// 
// ┌─────────────────────────────────────────────────────────────────────────┐
// │  STATUS: L0-LEARNING-PROVEN                                             │
// │  DATE: 2026-01-27                                                       │
// │  EVIDENCE: artifacts/proof/CI_GATE_SINGLE_OBJECTIVE.json                │
// │                                                                         │
// │  GOVERNANCE:                                                            │
// │  - Breaking changes REQUIRE ci/test_vfx_learning_gate.py to pass        │
// │  - Proof bundle required for any learning claim modification            │
// │  - See docs/LEARNING_CLAIM.md for exact claim and limitations           │
// └─────────────────────────────────────────────────────────────────────────┘
//
// This is the REAL learning loop. No theatre.
// Observe → Propose mutation → Execute primitive ops → Render → Score → Learn

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NiagaraProbeHarness.generated.h"

// Forward declarations
class AActor;
class UWorld;
class USceneCaptureComponent2D;
class UNiagaraComponent;

/**
 * Primitive Actions - The ONLY things the agent can do
 * Each action is deterministic and verifiable
 */
UENUM(BlueprintType)
enum class ENiagaraPrimitive : uint8
{
    // Module operations
    AddModule,           // Add a module to an emitter
    RemoveModule,        // Remove a module from an emitter
    
    // Parameter operations  
    SetFloatParam,       // Set a float parameter
    SetVectorParam,      // Set a vector parameter
    SetColorParam,       // Set a color parameter
    SetCurveParam,       // Set a curve parameter
    
    // Emitter operations
    AddEmitter,          // Add an emitter to system
    RemoveEmitter,       // Remove an emitter from system
    SetEmitterEnabled,   // Enable/disable emitter
    
    // Renderer operations
    SetRenderer,         // Change renderer type (sprite/mesh/ribbon)
    SetMaterial,         // Change material on renderer
    SetMaterialParam,    // Modify material parameter
    
    // Spawn operations
    SetSpawnRate,        // Directly set spawn rate
    SetSpawnBurst,       // Set burst spawn count
    
    // Lifetime operations
    SetLifetime,         // Set particle lifetime
    
    // No-op (for baseline)
    NoOp
};

/**
 * Action - A single mutation to apply
 * Fully serializable for replay and proof
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraAction
{
    GENERATED_BODY()
    
    // What primitive operation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ENiagaraPrimitive Primitive = ENiagaraPrimitive::NoOp;
    
    // Target path within the system (e.g., "Emitter0.SpawnRate")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString TargetPath;
    
    // Arguments (type depends on primitive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, FString> Args;
    
    // Unique ID for tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ActionId;
    
    // Timestamp when action was proposed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    double TimestampMs = 0.0;
    
    // Human-readable description
    FString ToString() const;
};

/**
 * Visual Metrics - Computed from rendered frames
 * These are the REAL observations, not guesses
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVisualMetrics
{
    GENERATED_BODY()
    
    // Coverage: what fraction of frame has particles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Coverage = 0.0f;
    
    // Edge density: amount of high-frequency detail
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float EdgeDensity = 0.0f;
    
    // Color distribution (histogram peaks)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FLinearColor DominantColor = FLinearColor::Black;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float ColorVariance = 0.0f;
    
    // Motion coherence (frame-to-frame optical flow consistency)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MotionCoherence = 0.0f;
    
    // Brightness distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MeanBrightness = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float BrightnessVariance = 0.0f;
    
    // Bounding box of effect in screen space
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FBox2D EffectBounds = FBox2D(FVector2D::ZeroVector, FVector2D::ZeroVector);
    
    // Frame hash for exact comparison
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString FrameHash;
};

/**
 * Performance Metrics - GPU/CPU cost of the effect
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraPerfMetrics
{
    GENERATED_BODY()
    
    // Particle count
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 ActiveParticles = 0;
    
    // GPU time in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float GPUTimeMs = 0.0f;
    
    // CPU time in milliseconds (simulation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float CPUTimeMs = 0.0f;
    
    // Memory usage in KB
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MemoryKB = 0.0f;
    
    // Draw calls contributed by this effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 DrawCalls = 0;
    
    // Triangle count
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Triangles = 0;
    
    // Overdraw estimate (pixels rendered / visible pixels)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float OverdrawRatio = 0.0f;
};

/**
 * Constraint Violations - What rules were broken
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FConstraintViolation
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ConstraintName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Severity = 0.0f; // 0-1, where 1 = fatal
};

/**
 * Observation - Complete state after an action
 * This is what the agent sees after each step
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraObservation
{
    GENERATED_BODY()
    
    // Step number in this episode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 StepIndex = 0;
    
    // The action that led to this state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FNiagaraAction PreviousAction;
    
    // Visual metrics from rendered frames
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVisualMetrics Visual;
    
    // Performance metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FNiagaraPerfMetrics Perf;
    
    // Current system snapshot (serialized graph state)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SystemSnapshot;
    
    // Constraint violations in current state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FConstraintViolation> Violations;
    
    // Is this a terminal state?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bTerminal = false;
    
    // Why terminal (if applicable)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString TerminalReason;
};

/**
 * Target Specification - What we're trying to achieve
 * Defines the reward function
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVFXTargetSpec
{
    GENERATED_BODY()
    
    // Human-readable goal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
    
    // Effect category (fire, smoke, explosion, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Category;
    
    // Visual targets (what we want to see)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float TargetCoverage = 0.0f;           // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float CoverageTolerance = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FLinearColor TargetDominantColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float ColorTolerance = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float TargetEdgeDensity = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float EdgeDensityTolerance = 0.15f;
    
    // Motion targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float TargetMotionCoherence = 0.5f;    // 0 = chaotic, 1 = smooth
    
    // Performance budget (hard constraints)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 MaxParticles = 10000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MaxGPUTimeMs = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MaxCPUTimeMs = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MaxMemoryKB = 1024.0f;
    
    // Reference frames (optional) - actual target images
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<UTexture2D*> ReferenceFrames;
    
    // Weights for reward calculation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float VisualWeight = 0.6f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float PerfWeight = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float ConstraintWeight = 0.2f;
};

/**
 * Step Result - The outcome of a single step
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraStepResult
{
    GENERATED_BODY()
    
    // The observation after the action
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FNiagaraObservation Observation;
    
    // Reward signal (-1 to 1, higher is better)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Reward = 0.0f;
    
    // Reward breakdown for debugging
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float VisualReward = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float PerfReward = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float ConstraintReward = 0.0f;
    
    // Did the action execute successfully?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bActionSucceeded = false;
    
    // Error message if action failed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ErrorMessage;
    
    // Time taken for this step (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float StepTimeMs = 0.0f;
};

/**
 * Episode - A complete sequence of steps
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraEpisode
{
    GENERATED_BODY()
    
    // Unique episode ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString EpisodeId;
    
    // Target we were optimizing for
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVFXTargetSpec Target;
    
    // Initial system (before any actions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString InitialSystemPath;
    
    // All steps in order
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FNiagaraStepResult> Steps;
    
    // Total reward accumulated
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float TotalReward = 0.0f;
    
    // Did we achieve the target?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bTargetAchieved = false;
    
    // Start/end timestamps
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FDateTime StartTime;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FDateTime EndTime;
};

/**
 * Environment Configuration
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FNiagaraEnvConfig
{
    GENERATED_BODY()
    
    // Render resolution for frame capture
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 RenderWidth = 256;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 RenderHeight = 256;
    
    // Number of frames to capture per step
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 FramesPerStep = 4;
    
    // Time between frames (simulated seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float FrameInterval = 0.1f;
    
    // Maximum steps per episode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 MaxStepsPerEpisode = 100;
    
    // Camera distance from effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float CameraDistance = 500.0f;
    
    // Camera angles (multiple views)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FRotator> CameraAngles;
    
    // Auto-terminate on constraint violation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bTerminateOnViolation = true;
    
    // Logging verbosity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bVerboseLogging = false;
};

/**
 * NiagaraEnv - The actual RL environment
 * 
 * This is where learning happens. Every action is:
 * 1. Executed on the real Niagara system
 * 2. Rendered to actual frames
 * 3. Scored against real metrics
 * 4. Logged for proof
 */
UCLASS(BlueprintType)
class RIFTBORNAI_API UNiagaraEnv : public UObject
{
    GENERATED_BODY()
    
public:
    UNiagaraEnv();
    
    /**
     * Get the singleton instance of NiagaraEnv
     * This prevents GPU memory leaks from creating multiple instances
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    static UNiagaraEnv* GetInstance();
    
    // ============================================================
    // CORE RL INTERFACE
    // ============================================================
    
    /**
     * Reset the environment to initial state
     * @param SystemPath - Asset path to Niagara system (or empty to create blank)
     * @param Target - What we're trying to achieve
     * @param Config - Environment configuration
     * @return Initial observation
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    FNiagaraObservation Reset(
        const FString& SystemPath,
        const FVFXTargetSpec& Target,
        const FNiagaraEnvConfig& Config);
    
    /**
     * Take a single step in the environment
     * This is the core loop: action → execute → render → score → observe
     * @param Action - The action to take
     * @return Step result with observation, reward, and terminal flag
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    FNiagaraStepResult Step(const FNiagaraAction& Action);
    
    /**
     * Get current observation without taking an action
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    FNiagaraObservation GetObservation();
    
    /**
     * Get the current episode (all steps so far)
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    FNiagaraEpisode GetCurrentEpisode() const { return CurrentEpisode; }
    
    /**
     * Check if episode is done
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    bool IsDone() const { return bEpisodeDone; }
    
    /**
     * Flush GPU resources to prevent memory accumulation during long runs.
     * Call this periodically between episodes to prevent GPU memory leaks.
     * Does NOT destroy active resources - just forces cleanup of pending work.
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    void FlushGPU();
    
    /**
     * Deep GPU flush - aggressively reclaims GPU memory by recreating render resources.
     * More expensive than FlushGPU() but recovers more memory.
     * Call this every N episodes when GPU memory is accumulating.
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    void DeepFlushGPU();
    
    // ============================================================
    // ACTION EXECUTION
    // ============================================================
    
    /**
     * Execute a primitive action on the current system
     * Returns true if action was valid and executed
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Actions")
    bool ExecuteAction(const FNiagaraAction& Action, FString& OutError);
    
    /**
     * Get list of valid actions from current state
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Actions")
    TArray<FNiagaraAction> GetValidActions() const;
    
    // ============================================================
    // OBSERVATION & METRICS
    // ============================================================
    
    /**
     * Render frames and compute visual metrics
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Observation")
    FVisualMetrics CaptureVisualMetrics();
    
    /**
     * Get current performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Observation")
    FNiagaraPerfMetrics GetPerfMetrics() const;
    
    /**
     * Check constraints and return violations
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Observation")
    TArray<FConstraintViolation> CheckConstraints() const;
    
    /**
     * Serialize current system state to snapshot
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Observation")
    FString GetSystemSnapshot() const;
    
    // ============================================================
    // REWARD CALCULATION
    // ============================================================
    
    /**
     * Compute reward from observation
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Reward")
    float ComputeReward(const FNiagaraObservation& Obs) const;
    
    /**
     * Compute visual similarity to target
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Reward")
    float ComputeVisualReward(const FVisualMetrics& Visual) const;
    
    /**
     * Compute performance reward (within budget = good)
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Reward")
    float ComputePerfReward(const FNiagaraPerfMetrics& Perf) const;
    
    /**
     * Compute constraint reward (violations = bad)
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Reward")
    float ComputeConstraintReward(const TArray<FConstraintViolation>& Violations) const;
    
    // ============================================================
    // DIAGNOSTICS
    // ============================================================
    
    /**
     * Get the responsiveness diagnostics from the last sanity test
     * Returns JSON with coverage_delta, brightness_delta, hash_changed, passed
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Diagnostics")
    FString GetResponsivenessDiagnostics() const;
    
    // ============================================================
    // PROOF & LOGGING
    // ============================================================
    
    /**
     * Save episode to proof bundle
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Proof")
    bool SaveEpisode(const FString& FilePath) const;
    
    /**
     * Load episode from file
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Proof")
    bool LoadEpisode(const FString& FilePath, FNiagaraEpisode& OutEpisode) const;
    
    /**
     * Replay an episode step by step
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Proof")
    bool ReplayEpisode(const FNiagaraEpisode& Episode, TArray<FNiagaraStepResult>& OutResults);
    
protected:
    // Current target specification
    UPROPERTY()
    FVFXTargetSpec CurrentTarget;
    
    // Environment configuration
    UPROPERTY()
    FNiagaraEnvConfig CurrentConfig;
    
    // Active Niagara component
    UPROPERTY()
    UNiagaraComponent* ActiveComponent;

    // Actor that owns the active Niagara component (for cleanup)
    UPROPERTY()
    AActor* NiagaraActor;
    
    // Scene capture for rendering
    UPROPERTY()
    USceneCaptureComponent2D* SceneCapture;
    
    // Actor that holds the scene capture (for cleanup)
    UPROPERTY()
    AActor* SceneCaptureActor;
    
    // Render target for frame capture
    UPROPERTY()
    UTextureRenderTarget2D* RenderTarget;
    
    // Current step index
    UPROPERTY()
    int32 CurrentStep;
    
    // Is episode done?
    UPROPERTY()
    bool bEpisodeDone;

    // Terminal reason (if episode was force-stopped)
    UPROPERTY()
    FString EpisodeTerminalReason;

    // Observation pipeline health flag (camera/target/metrics coupling)
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    bool bObservationPipelineBroken = false;

    // Diagnostics from the last responsiveness check
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float LastResponsivenessCoverageDelta = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    float LastResponsivenessBrightnessDelta = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    bool bLastResponsivenessHashChanged = false;
    
    // Black frame detection: tracks consecutive black captures for retry logic
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    int32 ConsecutiveBlackFrames = 0;
    
    // Flag set when black frame recovery fails - causes CAPTURE_BLACK_FRAME terminal
    UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
    bool bCaptureBlackFrameFailure = false;
    
    // Cached system to avoid reloading same asset repeatedly (GPU memory optimization)
    UPROPERTY()
    FString CachedSystemPath;
    
    UPROPERTY()
    UNiagaraSystem* CachedSystem = nullptr;
    
    // Current episode data
    UPROPERTY()
    FNiagaraEpisode CurrentEpisode;
    
    // Previous visual metrics (for delta computation)
    UPROPERTY()
    FVisualMetrics PreviousVisual;

public:
    // Clean up spawned actors and components
    virtual void BeginDestroy() override;
    
    // Manual cleanup - lightweight, reuses resources
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    void Cleanup();
    
    // Full cleanup - destroys all resources, called on destruction
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv")
    void FullCleanup();
    
private:
    // Resolve the best world context for this environment (PIE > Editor > Game)
    UWorld* GetEnvWorld() const;

    // Destroy any previously leaked env artifacts in the world (tagged or render-target owned)
    void PurgeLeakedEnvArtifacts(UWorld* World);

    // (Re)create render target + scene capture actor/component
    void SetupSceneCapture(UWorld* World);
    
    // Force recreate the SceneCapture (destroys existing first)
    void ForceRecreateSceneCapture(UWorld* World);

    // (Re)create a dedicated actor + Niagara component instance (never attach to WorldSettings)
    bool SetupNiagaraActor(UWorld* World, UNiagaraSystem* System, FString& OutError);

    // Place the capture camera relative to the active component bounds center
    bool UpdateCaptureTransformToActiveBounds();
    
    // Apply random initial variation to system parameters
    void ApplyInitialRandomization(UNiagaraSystem* System);
    
    // Sanity delta test: prove observation changes when the effect changes
    // If this fails, the episode is force-terminated with OBSERVATION_NOT_RESPONSIVE.
    bool RunObservationSanityDeltaTest(FString& OutError);
    
    // Advance simulation by one frame interval
    void TickSimulation(float DeltaTime);
    
    // Capture single frame to texture
    void CaptureFrame();
    
    // Compute metrics from render target
    FVisualMetrics ComputeMetricsFromTexture();
    
    // Hash a frame for comparison
    FString ComputeFrameHash(UTextureRenderTarget2D* Texture);
    
    // Sobel edge detection
    float ComputeEdgeDensity(const TArray<FColor>& Pixels, int32 Width, int32 Height);
    
    // Color histogram analysis
    void AnalyzeColorDistribution(const TArray<FColor>& Pixels, 
        FLinearColor& OutDominant, float& OutVariance);
    
    // Compute coverage (non-black pixels ratio) - uses known black background
    float ComputeCoverage(const TArray<FColor>& Pixels);
    
    // DEPRECATED: Mode-based coverage - inverts when particles dominate
    float ComputeCoverage_Legacy(const TArray<FColor>& Pixels);
    
    // Find effect bounds in screen space
    FBox2D ComputeEffectBounds(const TArray<FColor>& Pixels, int32 Width, int32 Height);
};

/**
 * Static utilities for NiagaraEnv
 */
UCLASS()
class RIFTBORNAI_API UNiagaraEnvUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    /**
     * Create a target spec for fire effects
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Targets")
    static FVFXTargetSpec CreateFireTarget();
    
    /**
     * Create a target spec for smoke effects
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Targets")
    static FVFXTargetSpec CreateSmokeTarget();
    
    /**
     * Create a target spec for explosion effects
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Targets")
    static FVFXTargetSpec CreateExplosionTarget();
    
    /**
     * Create default environment config
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Config")
    static FNiagaraEnvConfig CreateDefaultConfig();
    
    /**
     * Create an action from primitive + target + args
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Actions")
    static FNiagaraAction MakeAction(
        ENiagaraPrimitive Primitive,
        const FString& TargetPath,
        const TMap<FString, FString>& Args);
    
    /**
     * Serialize observation to JSON
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Serialization")
    static FString ObservationToJson(const FNiagaraObservation& Obs);
    
    /**
     * Serialize episode to JSON
     */
    UFUNCTION(BlueprintCallable, Category = "NiagaraEnv|Serialization")
    static FString EpisodeToJson(const FNiagaraEpisode& Episode);
};
