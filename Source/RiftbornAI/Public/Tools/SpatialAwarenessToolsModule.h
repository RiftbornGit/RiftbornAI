// Copyright RiftbornAI. All Rights Reserved.
// Spatial Awareness Tools — geometric scene validation without vision.
// Detects overlapping actors, floating objects, scale anomalies, flow errors, density gaps.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

class RIFTBORNAI_API FSpatialAwarenessToolsModule : public TToolModuleBase<FSpatialAwarenessToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("SpatialAwarenessTools"); }
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Phase 1: Geometric awareness
    static FClaudeToolResult Tool_DetectOverlappingActors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DetectFloatingActors(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DetectScaleAnomalies(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ValidateSplineFlow(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_MeasurePlacementDensity(const FClaudeToolCall& Call);

    // Phase 2: Verify-after-mutate
    static FClaudeToolResult Tool_VerifyPlacementBatch(const FClaudeToolCall& Call);

    // Phase 3: Level navigability and human-scale validation
    static FClaudeToolResult Tool_ValidateNavigability(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ValidateHumanScale(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ValidateStructuralPlausibility(const FClaudeToolCall& Call);

    // Phase 4: Reference-guided verification (impl in SpatialAwarenessToolsModule_ReferenceVerify.cpp)
    static FClaudeToolResult Tool_CompareToReferenceImage(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExtractLayoutFromReference(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_VerifyRomanAccuracy(const FClaudeToolCall& Call);

    // Registration helper called from RegisterTools (Phase 3, defined in _Navigability.cpp)
    static void RegisterNavigabilityTools(FClaudeToolRegistry& Registry, FSpatialAwarenessToolsModule& Module);

    // Registration helper called from RegisterTools (Phase 4, defined in _ReferenceVerify.cpp)
    static void RegisterReferenceVerifyTools(FClaudeToolRegistry& Registry, FSpatialAwarenessToolsModule& Module);

    // Phase 5: Gameplay validation (impl in SpatialAwarenessToolsModule_Gameplay.cpp)
    static FClaudeToolResult Tool_AnalyzeSightLines(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_MeasureGameplaySpaces(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ValidateWayfinding(const FClaudeToolCall& Call);

    // Registration helper called from RegisterTools (Phase 5, defined in _Gameplay.cpp)
    static void RegisterGameplayTools(FClaudeToolRegistry& Registry, FSpatialAwarenessToolsModule& Module);

    // Phase 6: Quality validation (impl in SpatialAwarenessToolsModule_Quality.cpp)
    static FClaudeToolResult Tool_AuditMaterialConsistency(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ValidateLightingCoverage(const FClaudeToolCall& Call);

    // Registration helper called from RegisterTools (Phase 6, defined in _Quality.cpp)
    static void RegisterQualityTools(FClaudeToolRegistry& Registry, FSpatialAwarenessToolsModule& Module);
};
