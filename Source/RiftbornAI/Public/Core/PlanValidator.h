// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
//
// Plan Validator - Runtime validation of plan JSON against schema requirements
// PROOF MODE: Plans MUST pass validation before execution

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Result of plan validation
 */
struct RIFTBORNAI_API FPlanValidationResult
{
    bool bValid = false;
    TArray<FString> Errors;
    TArray<FString> Warnings;
    
    bool IsValid() const { return bValid && Errors.Num() == 0; }
    
    FString GetSummary() const
    {
        if (IsValid())
        {
            return Warnings.Num() > 0 
                ? FString::Printf(TEXT("Valid with %d warnings"), Warnings.Num())
                : TEXT("Valid");
        }
        return FString::Printf(TEXT("Invalid: %d errors"), Errors.Num());
    }
    
    void AddError(const FString& Error)
    {
        Errors.Add(Error);
        bValid = false;
    }
    
    void AddWarning(const FString& Warning)
    {
        Warnings.Add(Warning);
    }
};

/**
 * Plan Validator - validates plan JSON against schema requirements
 * 
 * In PROOF mode, plans MUST:
 * - Have valid JSON structure
 * - Include required fields: tools array
 * - Each step must have: tool name, valid arguments matching tool schema
 * - Dependencies must reference valid step indices
 * - Output bindings must be valid
 */
class RIFTBORNAI_API FPlanValidator
{
public:
    static FPlanValidator& Get();
    
    /**
     * Validate a plan JSON object
     * @param PlanJson - The plan JSON object to validate
     * @return Validation result with errors/warnings
     */
    FPlanValidationResult ValidatePlan(const TSharedPtr<FJsonObject>& PlanJson) const;
    
    /**
     * Validate a plan JSON string
     * @param PlanJsonStr - JSON string to parse and validate
     * @return Validation result with errors/warnings
     */
    FPlanValidationResult ValidatePlanString(const FString& PlanJsonStr) const;
    
    /**
     * Check if PROOF mode is enabled (stricter validation)
     */
    static bool IsProofModeEnabled();
    
    /**
     * Compute SHA256 hash of a plan JSON object
     * Used for proof bundles to bind execution to specific plan
     * @param PlanJson - The plan JSON object to hash
     * @return Hex-encoded SHA256 hash
     */
    static FString ComputePlanHash(const TSharedPtr<FJsonObject>& PlanJson);
    
    /**
     * Compute SHA256 hash of a plan JSON string
     * @param PlanJsonStr - The plan JSON string to hash
     * @return Hex-encoded SHA256 hash
     */
    static FString ComputePlanHashString(const FString& PlanJsonStr);
    
private:
    FPlanValidator() = default;
    
    /** Validate the tools array */
    void ValidateToolsArray(const TArray<TSharedPtr<FJsonValue>>& Tools, FPlanValidationResult& Result) const;
    
    /** Validate a single step */
    void ValidateStep(int32 Index, const TSharedPtr<FJsonObject>& StepJson, int32 TotalSteps, FPlanValidationResult& Result) const;
    
    /** Validate step arguments against tool contract schema */
    void ValidateStepArguments(int32 Index, const FString& ToolName, const TSharedPtr<FJsonObject>& Args, FPlanValidationResult& Result) const;
    
    /** Validate dependencies reference valid steps */
    void ValidateDependencies(int32 Index, const TArray<TSharedPtr<FJsonValue>>& Deps, int32 TotalSteps, FPlanValidationResult& Result) const;
    
    /** Validate output bindings format */
    void ValidateOutputBindings(int32 Index, const TSharedPtr<FJsonObject>& Bindings, FPlanValidationResult& Result) const;
};
