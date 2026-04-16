// Copyright RiftbornAI. All Rights Reserved.
// AssetValidationPipeline.h - Bulk asset validation against configurable budgets
//
// PURPOSE: Validates assets against configurable budgets before packaging:
//   - Texture resolution limits (max 4K, power-of-two enforcement)
//   - Mesh triangle budgets (per-LOD, total)
//   - LOD chain verification (LOD0 exists, reduction ratios, screen sizes)
//   - Material complexity (instruction count, texture samples, shader model)
//   - Naming convention enforcement
//   - Missing reference detection
//
// USAGE:
//   FAssetValidationPipeline Pipeline;
//   Pipeline.SetBudgets(Budgets);  // Or use platform-specific defaults
//   FAssetValidationReport Report = Pipeline.ValidateScope(TEXT("/Game/Characters/"));
//   // Report contains per-asset pass/fail with detailed violation descriptions

#pragma once

#include "CoreMinimal.h"
#include "ValidationFramework.h"

RIFTBORNAI_API DECLARE_LOG_CATEGORY_EXTERN(LogAssetValidation, Log, All);

/**
 * Category of validation check
 */
enum class EValidationCategory : uint8
{
    Texture,        // Resolution, format, power-of-two, streaming
    Mesh,           // Triangle count, vertex count, bounds
    LOD,            // LOD chain completeness, reduction ratios
    Material,       // Instruction count, texture samples, complexity
    NamingConvention,  // Asset naming standards
    Reference,      // Missing/broken references
    Memory,         // Estimated memory usage
    Shader,         // Shader complexity, permutation count
    Animation,      // Bone count, curve count, compression
    Audio,          // Sample rate, channel count, duration
    Blueprint,      // Node count, cast chains, nativization readiness
    DataTable       // Schema, referential integrity
};

/**
 * Single validation violation
 */
struct RIFTBORNAI_API FAssetViolation
{
    /** Asset path that has the violation */
    FString AssetPath;
    
    /** Human-readable description */
    FString Description;
    
    /** Category */
    EValidationCategory Category = EValidationCategory::Texture;
    
    /** Severity */
    EValidationSeverity Severity = EValidationSeverity::Warning;
    
    /** The metric name (e.g., "triangle_count", "texture_resolution") */
    FString MetricName;
    
    /** Actual value */
    double ActualValue = 0.0;
    
    /** Budget/limit value */
    double BudgetValue = 0.0;
    
    /** Percentage over budget (negative = under budget) */
    double OverBudgetPercent = 0.0;
    
    /** Suggested fix */
    FString SuggestedFix;
};

/**
 * Validation budgets — configurable per-platform
 */
struct RIFTBORNAI_API FAssetValidationBudgets
{
    // === TEXTURES ===
    int32 MaxTextureResolution = 4096;          // Max dimension (width or height)
    bool bRequirePowerOfTwo = true;             // Enforce power-of-two dimensions
    bool bRequireSquare = false;                // Enforce square textures
    int64 MaxTextureSizeBytes = 16 * 1024 * 1024; // 16 MB max per texture
    
    // === MESHES ===
    int32 MaxTrianglesLOD0 = 100000;            // Max tris for LOD0
    int32 MaxTrianglesLOD1 = 50000;             // Max tris for LOD1
    int32 MaxTrianglesLOD2 = 25000;             // Max tris for LOD2
    int32 MaxVerticesLOD0 = 65535;              // 16-bit index buffer limit
    float MinLODReductionRatio = 0.4f;          // Each LOD should be ≤40% of previous
    int32 MinRequiredLODs = 2;                  // Minimum number of LODs
    
    // === MATERIALS ===
    int32 MaxMaterialInstructions = 300;        // Max shader instructions
    int32 MaxTextureSamples = 16;               // Max texture samples per material
    int32 MaxMaterialPermutations = 64;         // Max shader permutations
    
    // === MEMORY ===
    int64 MaxAssetMemoryMB = 256;               // Max estimated runtime memory per asset
    int64 TotalBudgetMB = 2048;                 // Total memory budget for validated scope
    
    // === ANIMATION ===
    int32 MaxBoneCount = 256;                   // Max bones per skeleton
    int32 MaxAnimCurveCount = 100;              // Max curves per anim sequence
    
    // === BLUEPRINT ===
    int32 MaxBlueprintNodes = 500;              // Max nodes per Blueprint graph
    int32 MaxCastChainDepth = 3;                // Max nested casts
    
    // === NAMING ===
    /** Prefix rules: "StaticMesh" -> "SM_", "Texture2D" -> "T_", etc. */
    TMap<FString, FString> NamingPrefixRules;
    
    /** Create default budgets for a target platform */
    static FAssetValidationBudgets DefaultPC()
    {
        return FAssetValidationBudgets(); // Defaults are PC-tuned
    }
    
    static FAssetValidationBudgets DefaultConsole()
    {
        FAssetValidationBudgets B;
        B.MaxTextureResolution = 2048;
        B.MaxTrianglesLOD0 = 60000;
        B.MaxTrianglesLOD1 = 30000;
        B.MaxTrianglesLOD2 = 15000;
        B.MinRequiredLODs = 3;
        B.MaxMaterialInstructions = 200;
        B.MaxTextureSamples = 12;
        B.MaxAssetMemoryMB = 128;
        B.TotalBudgetMB = 1024;
        B.MaxBoneCount = 128;
        return B;
    }
    
    static FAssetValidationBudgets DefaultMobile()
    {
        FAssetValidationBudgets B;
        B.MaxTextureResolution = 1024;
        B.MaxTrianglesLOD0 = 20000;
        B.MaxTrianglesLOD1 = 10000;
        B.MaxTrianglesLOD2 = 5000;
        B.MinRequiredLODs = 3;
        B.MaxMaterialInstructions = 100;
        B.MaxTextureSamples = 8;
        B.MaxAssetMemoryMB = 32;
        B.TotalBudgetMB = 256;
        B.MaxBoneCount = 75;
        B.MaxBlueprintNodes = 200;
        return B;
    }
};

/**
 * Report from a validation run
 */
struct RIFTBORNAI_API FAssetValidationReport
{
    /** Scope that was validated (e.g., "/Game/Characters/") */
    FString Scope;
    
    /** All violations found */
    TArray<FAssetViolation> Violations;
    
    /** Total assets scanned */
    int32 TotalAssetsScanned = 0;
    
    /** Assets that passed all checks */
    int32 PassedCount = 0;
    
    /** Assets with at least one warning */
    int32 WarningCount = 0;
    
    /** Assets with at least one error */
    int32 ErrorCount = 0;
    
    /** Assets with at least one critical violation */
    int32 CriticalCount = 0;
    
    /** Total estimated memory usage of scanned assets (MB) */
    double TotalEstimatedMemoryMB = 0.0;
    
    /** When the validation was run */
    FDateTime Timestamp;
    
    /** Budgets used */
    FAssetValidationBudgets BudgetsUsed;
    
    /** Duration of the validation scan */
    float DurationSeconds = 0.0f;
    
    /** Did the scope pass validation? (No errors or criticals) */
    bool Passed() const { return ErrorCount == 0 && CriticalCount == 0; }
    
    /** Get violations filtered by category */
    TArray<FAssetViolation> GetViolationsByCategory(EValidationCategory Cat) const
    {
        TArray<FAssetViolation> Filtered;
        for (const FAssetViolation& V : Violations)
        {
            if (V.Category == Cat) Filtered.Add(V);
        }
        return Filtered;
    }
    
    /** Get violations filtered by severity */
    TArray<FAssetViolation> GetViolationsBySeverity(EValidationSeverity Sev) const
    {
        TArray<FAssetViolation> Filtered;
        for (const FAssetViolation& V : Violations)
        {
            if (V.Severity == Sev) Filtered.Add(V);
        }
        return Filtered;
    }
    
    /** Get summary string */
    FString GetSummary() const
    {
        return FString::Printf(
            TEXT("Asset Validation: %d scanned, %d passed, %d warnings, %d errors, %d critical | %.0f MB estimated | %s"),
            TotalAssetsScanned, PassedCount, WarningCount, ErrorCount, CriticalCount,
            TotalEstimatedMemoryMB,
            Passed() ? TEXT("✓ PASSED") : TEXT("✗ FAILED"));
    }
    
    /** Serialize to JSON for CI/CD integration */
    FString ToJson() const;
    
    /** Save report to disk */
    bool SaveToFile(const FString& FilePath) const;
};

/**
 * FAssetValidationPipeline
 * 
 * Main entry point for bulk asset validation.
 * 
 * Usage:
 *   FAssetValidationPipeline Pipeline;
 *   Pipeline.SetBudgets(FAssetValidationBudgets::DefaultConsole());
 *   FAssetValidationReport Report = Pipeline.ValidateScope(TEXT("/Game/"));
 *   
 *   if (!Report.Passed())
 *   {
 *       for (auto& V : Report.GetViolationsBySeverity(EValidationSeverity::Error))
 *       {
 *           UE_LOG(LogTemp, Error, TEXT("%s: %s"), *V.AssetPath, *V.Description);
 *       }
 *   }
 */
class RIFTBORNAI_API FAssetValidationPipeline
{
public:
    FAssetValidationPipeline();
    
    /** Set validation budgets */
    void SetBudgets(const FAssetValidationBudgets& InBudgets) { Budgets = InBudgets; }
    
    /** Get current budgets */
    const FAssetValidationBudgets& GetBudgets() const { return Budgets; }

    /** Set target platform for TRC/XR certification checks.
     *  Valid values: "PC", "PS5", "XSX", "Switch", "Mobile", "iOS", "Android"
     *  Empty string (default) disables platform-specific checks. */
    void SetTargetPlatform(const FString& Platform) { TargetPlatform = Platform; }
    
    /**
     * Validate all assets under a given scope path.
     * @param ScopePath Content path like "/Game/Characters/" or "/Game/"
     * @param bRecursive Include subdirectories (default true)
     * @return Validation report with all violations
     */
    FAssetValidationReport ValidateScope(const FString& ScopePath, bool bRecursive = true);
    
    /**
     * Validate a single asset by path.
     * @param AssetPath Full asset path (e.g., "/Game/Meshes/SM_Hero")
     * @return Violations for this single asset
     */
    TArray<FAssetViolation> ValidateAsset(const FString& AssetPath);
    
    /**
     * Quick check: does this scope pass validation?
     * Stops on first error for speed.
     */
    bool QuickCheck(const FString& ScopePath);

    /** Register validation tools with the tool registry */
    static void RegisterTools();

private:
    FAssetValidationBudgets Budgets;
    FString TargetPlatform;  // For TRC/XR certification checks
    
    // Per-asset-type validators
    TArray<FAssetViolation> ValidateTexture(const FString& AssetPath, class UTexture* Texture);
    TArray<FAssetViolation> ValidateStaticMesh(const FString& AssetPath, class UStaticMesh* Mesh);
    TArray<FAssetViolation> ValidateMaterial(const FString& AssetPath, class UMaterialInterface* Material);
    TArray<FAssetViolation> ValidateBlueprint(const FString& AssetPath, class UBlueprint* Blueprint);
    TArray<FAssetViolation> ValidateSkeletalMesh(const FString& AssetPath, class USkeletalMesh* SkMesh);
    TArray<FAssetViolation> ValidateAnimSequence(const FString& AssetPath, class UAnimSequence* AnimSeq);
    TArray<FAssetViolation> ValidateNamingConvention(const FString& AssetPath, class UObject* Asset);
    TArray<FAssetViolation> CheckBrokenReferences(const FString& AssetPath, class UObject* Asset);

    // =========================================================================
    // Console TRC/XR Validation (Gap #11 + #12)
    // Platform-specific compliance checks beyond budgets
    // =========================================================================

    /**
     * Validate console certification requirements.
     * Checks textures for streaming mips, BC format compatibility,
     * meshes for vertex count within 16-bit index limits,
     * materials for mobile shader model compatibility, etc.
     * 
     * @param AssetPath Asset being validated
     * @param Asset The loaded asset
     * @param InTargetPlatform "PS5", "XSX", "Switch", "Mobile"
     * @return Platform-specific violations with Critical severity
     */
    TArray<FAssetViolation> ValidatePlatformCertification(
        const FString& AssetPath, class UObject* Asset, const FString& InTargetPlatform);
};
