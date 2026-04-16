// Copyright RiftbornAI. All Rights Reserved.
// ValidationFramework.h - Pre-flight validation system

#pragma once

#include "CoreMinimal.h"

/**
 * Validation severity levels
 */
enum class EValidationSeverity : uint8
{
    Info,       // Informational, not a problem
    Warning,    // Potential issue, operation can proceed
    Error,      // Problem found, operation may fail
    Critical    // Blocking issue, operation cannot proceed
};

/**
 * Single validation issue/finding
 */
struct RIFTBORNAI_API FValidationIssue
{
    EValidationSeverity Severity = EValidationSeverity::Info;
    FString Category;       // e.g., "Asset", "Blueprint", "Material"
    FString Code;           // e.g., "ASSET_NOT_FOUND", "CIRCULAR_REF"
    FString Message;        // Human-readable description
    FString Context;        // Path, asset name, etc.
    FString Suggestion;     // How to fix it
    FString SuggestedFix;   // Alias for Suggestion (backward compatibility)
    
    bool IsBlocking() const { return Severity == EValidationSeverity::Critical; }
    bool IsError() const { return Severity >= EValidationSeverity::Error; }
};

/**
 * Result of a validation check
 */
struct RIFTBORNAI_API FValidationResult
{
    bool bPassed = true;
    TArray<FValidationIssue> Issues;
    double ValidationTimeMs = 0.0;
    
    /** Check if validation passed (no critical/error issues) */
    bool CanProceed() const
    {
        for (const FValidationIssue& Issue : Issues)
        {
            if (Issue.IsBlocking()) return false;
        }
        return true;
    }
    
    /** Get count of issues by severity */
    int32 CountBySeverity(EValidationSeverity Severity) const
    {
        int32 Count = 0;
        for (const FValidationIssue& Issue : Issues)
        {
            if (Issue.Severity == Severity) Count++;
        }
        return Count;
    }
    
    /** Add an issue */
    void AddIssue(const FValidationIssue& Issue)
    {
        Issues.Add(Issue);
        if (Issue.IsError())
        {
            bPassed = false;
        }
    }
    
    /** Merge another result into this one */
    void Merge(const FValidationResult& Other)
    {
        Issues.Append(Other.Issues);
        if (!Other.bPassed) bPassed = false;
        ValidationTimeMs += Other.ValidationTimeMs;
    }
    
    /** Convert to human-readable summary */
    FString ToSummary() const;
    
    /** Convert to JSON */
    FString ToJson() const;
};

/** Helper to construct a validation issue in one call */
RIFTBORNAI_API FValidationIssue MakeValidationIssue(
    EValidationSeverity Severity,
    const FString& Category,
    const FString& Code,
    const FString& Message,
    const FString& Context = TEXT(""),
    const FString& Suggestion = TEXT(""));

// Forward declaration
struct FValidationFramework;

/**
 * Base class for validators
 */
struct RIFTBORNAI_API IValidator
{
    virtual ~IValidator() = default;
    virtual FString GetName() const = 0;
    virtual FString GetCategory() const = 0;
    virtual FValidationResult Validate() = 0;
};

/**
 * Convenience validator wrappers for ClaudeToolUse compatibility
 * Implementation is in ValidationFramework.cpp
 */
struct RIFTBORNAI_API FBlueprintValidator
{
    FValidationResult Validate(const FString& BlueprintPath);
};

struct RIFTBORNAI_API FMaterialValidator
{
    FValidationResult Validate(const FString& MaterialPath);
};

struct RIFTBORNAI_API FDependencyValidator
{
    FValidationResult Validate(const FString& AssetPath);
};

struct RIFTBORNAI_API FFilesystemValidator
{
    FValidationResult Validate(const FString& Path);
};

/**
 * Validation Framework - Pre-flight checks before expensive operations
 * 
 * Usage:
 *   FValidationFramework& Validator = FValidationFramework::Get();
 *   
 *   // Check if an asset exists
 *   FValidationResult Result = Validator.ValidateAssetExists("/Game/Materials/M_Metal");
 *   
 *   // Check if a Blueprint compiles
 *   Result = Validator.ValidateBlueprintCompiles("/Game/Blueprints/BP_MyActor");
 *   
 *   // Run all checks for level generation
 *   Result = Validator.ValidateLevelGenerationPrerequisites(LevelSpec);
 *   
 *   if (Result.CanProceed())
 *   {
 *       // Safe to proceed
 *   }
 */
struct RIFTBORNAI_API FValidationFramework
{
    /** Singleton access */
    static FValidationFramework& Get();
    
    // ========================================================================
    // ASSET VALIDATORS
    // ========================================================================
    
    /**
     * Validate that an asset exists at the given path
     * @param AssetPath Content path (e.g., "/Game/Materials/M_Metal")
     * @param ExpectedClass Optional class filter (e.g., UMaterial::StaticClass())
     */
    FValidationResult ValidateAssetExists(
        const FString& AssetPath,
        UClass* ExpectedClass = nullptr);
    
    /**
     * Validate multiple assets exist
     */
    FValidationResult ValidateAssetsExist(
        const TArray<FString>& AssetPaths,
        UClass* ExpectedClass = nullptr);
    
    /**
     * Validate an asset can be loaded (not corrupted)
     */
    FValidationResult ValidateAssetLoadable(const FString& AssetPath);
    
    // ========================================================================
    // BLUEPRINT VALIDATORS
    // ========================================================================
    
    /**
     * Validate a Blueprint compiles without errors
     */
    FValidationResult ValidateBlueprintCompiles(const FString& BlueprintPath);
    
    /**
     * Validate a Blueprint has required variables
     */
    FValidationResult ValidateBlueprintHasVariables(
        const FString& BlueprintPath,
        const TArray<FString>& RequiredVariables);
    
    /**
     * Validate a Blueprint has required functions
     */
    FValidationResult ValidateBlueprintHasFunctions(
        const FString& BlueprintPath,
        const TArray<FString>& RequiredFunctions);
    
    // ========================================================================
    // MATERIAL VALIDATORS
    // ========================================================================
    
    /**
     * Validate a material exists and has expected parameters
     */
    FValidationResult ValidateMaterial(
        const FString& MaterialPath,
        const TArray<FString>& ExpectedParameters = TArray<FString>());
    
    /**
     * Validate a material instance has a valid parent
     */
    FValidationResult ValidateMaterialInstance(const FString& MaterialInstancePath);
    
    // ========================================================================
    // LEVEL VALIDATORS
    // ========================================================================
    
    /**
     * Validate the editor is in a valid state for level operations
     */
    FValidationResult ValidateEditorState();
    
    /**
     * Validate a level can be saved
     */
    FValidationResult ValidateLevelCanSave();
    
    // ========================================================================
    // DEPENDENCY VALIDATORS
    // ========================================================================
    
    /**
     * Check for circular dependencies between assets
     */
    FValidationResult ValidateNoCircularDependencies(const TArray<FString>& AssetPaths);
    
    /**
     * Validate all references from an asset can be resolved
     */
    FValidationResult ValidateAssetReferences(const FString& AssetPath);
    
    // ========================================================================
    // FILE SYSTEM VALIDATORS
    // ========================================================================
    
    /**
     * Validate a file path is writable
     */
    FValidationResult ValidatePathWritable(const FString& FilePath);
    
    /**
     * Validate a source file exists
     */
    FValidationResult ValidateSourceFileExists(const FString& SourcePath);
    
    /**
     * Validate project structure is valid
     */
    FValidationResult ValidateProjectStructure();
    
    // ========================================================================
    // COMPOSITE VALIDATORS
    // ========================================================================
    
    /**
     * Run all validators in a category
     */
    FValidationResult ValidateCategory(const FString& Category);
    
    /**
     * Run a custom set of validators
     */
    FValidationResult ValidateCustom(const TArray<TSharedPtr<IValidator>>& Validators);
    
private:
    // Helper to create issues
    FValidationIssue MakeIssue(
        EValidationSeverity Severity,
        const FString& Category,
        const FString& Code,
        const FString& Message,
        const FString& Context = TEXT(""),
        const FString& Suggestion = TEXT(""));
};
