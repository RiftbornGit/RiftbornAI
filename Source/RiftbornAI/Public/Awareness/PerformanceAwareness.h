// PerformanceAwareness.h - Performance metrics for AI Agents

#pragma once

#include "CoreMinimal.h"
#include "PerformanceAwareness.generated.h"

UENUM(BlueprintType)
enum class EPerformanceLevel : uint8
{
    Excellent,    // 60+ FPS, no hitches
    Good,         // 45-60 FPS
    Acceptable,   // 30-45 FPS
    Poor,         // 20-30 FPS
    Critical      // <20 FPS
};

UENUM(BlueprintType)
enum class EBottleneckType : uint8
{
    None,
    CPU_GameThread,
    CPU_RenderThread,
    GPU,
    Memory,
    Streaming,
    Physics,
    Animation,
    AI,
    Network
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornFrameMetrics
{
    GENERATED_BODY()
    
    UPROPERTY() float FPS = 60.0f;
    UPROPERTY() float FrameTimeMs = 16.67f;
    UPROPERTY() float GameThreadMs = 0.0f;
    UPROPERTY() float RenderThreadMs = 0.0f;
    UPROPERTY() float GPUTimeMs = 0.0f;
    UPROPERTY() int32 DrawCalls = 0;
    UPROPERTY() int32 Triangles = 0;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMemoryMetrics
{
    GENERATED_BODY()
    
    UPROPERTY() float UsedMemoryMB = 0.0f;
    UPROPERTY() float AvailableMemoryMB = 0.0f;
    UPROPERTY() float TextureMemoryMB = 0.0f;
    UPROPERTY() float MeshMemoryMB = 0.0f;
    UPROPERTY() float AudioMemoryMB = 0.0f;
    UPROPERTY() int32 LoadedTextures = 0;
    UPROPERTY() int32 StreamingTextures = 0;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FSceneMetrics
{
    GENERATED_BODY()
    
    UPROPERTY() int32 ActorCount = 0;
    UPROPERTY() int32 VisibleActors = 0;
    UPROPERTY() int32 StaticMeshComponents = 0;
    UPROPERTY() int32 SkeletalMeshComponents = 0;
    UPROPERTY() int32 LightCount = 0;
    UPROPERTY() int32 ShadowCastingLights = 0;
    UPROPERTY() int32 ParticleSystems = 0;
    UPROPERTY() int32 DecalCount = 0;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPerformanceState
{
    GENERATED_BODY()
    
    UPROPERTY() EPerformanceLevel Level = EPerformanceLevel::Good;
    UPROPERTY() EBottleneckType Bottleneck = EBottleneckType::None;
    UPROPERTY() FRiftbornFrameMetrics Frame;
    UPROPERTY() FMemoryMetrics Memory;
    UPROPERTY() FSceneMetrics Scene;
    UPROPERTY() TArray<FString> Warnings;
    UPROPERTY() TArray<FString> Recommendations;
    
    FString GetDescription() const;
};

class RIFTBORNAI_API FPerformanceAwareness
{
public:
    static FPerformanceAwareness& Get();
    
    FPerformanceState GetCurrentState() const;
    FRiftbornFrameMetrics GetFrameMetrics() const;
    FMemoryMetrics GetMemoryMetrics() const;
    FSceneMetrics GetSceneMetrics() const;
    
    EPerformanceLevel GetPerformanceLevel() const;
    EBottleneckType GetBottleneck() const;
    TArray<FString> GetOptimizationRecommendations() const;
    
    float GetFPS() const;
    float GetFrameTime() const;
    float GetGPUTime() const;
    int32 GetDrawCalls() const;
    
    bool IsGPUBound() const;
    bool IsCPUBound() const;
    bool HasHitches() const;
    
    static FString PerformanceLevelToString(EPerformanceLevel Level);
    static FString BottleneckToString(EBottleneckType Bottleneck);
    
private:
    FPerformanceAwareness();
    mutable FPerformanceState CachedState;
    mutable double LastUpdateTime = 0.0;
};
