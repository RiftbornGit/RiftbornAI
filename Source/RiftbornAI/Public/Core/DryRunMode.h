// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Dry-Run Mode Support for Destructive Operations
 * 
 * When dry_run=true, tools will:
 * 1. Validate all inputs
 * 2. Check permissions and preconditions
 * 3. Report what WOULD happen
 * 4. NOT execute the actual operation
 * 
 * This is a safety feature for:
 * - Previewing destructive operations
 * - Testing automation scripts
 * - Training/debugging AI agents
 */

/**
 * Dry-run mode settings
 */
struct RIFTBORNAI_API FDryRunSettings
{
    /** Global flag to force all operations into dry-run mode */
    static bool bGlobalDryRunMode;
    
    /** Enable global dry-run mode (all destructive ops become previews) */
    static void EnableGlobalDryRun() { bGlobalDryRunMode = true; }
    
    /** Disable global dry-run mode */
    static void DisableGlobalDryRun() { bGlobalDryRunMode = false; }
    
    /** Check if dry-run is active (global or per-request) */
    static bool IsDryRunActive(const FClaudeToolCall& Call)
    {
        if (bGlobalDryRunMode) return true;
        
        // Check for dry_run parameter in the call
        const FString* DryRun = Call.Arguments.Find(TEXT("dry_run"));
        if (DryRun && (DryRun->Equals(TEXT("true"), ESearchCase::IgnoreCase) || *DryRun == TEXT("1")))
        {
            return true;
        }
        return false;
    }
};

/**
 * Dry-run result builder
 */
class RIFTBORNAI_API FDryRunResult
{
public:
    FDryRunResult(const FString& ToolName, const FString& ToolUseId)
        : ToolName(ToolName)
        , ToolUseId(ToolUseId)
    {
        WouldPerform.Add(TEXT("Dry-run mode active - no changes will be made"));
    }
    
    /** Add what the operation would do */
    FDryRunResult& WouldDo(const FString& Action)
    {
        WouldPerform.Add(Action);
        return *this;
    }
    
    /** Add a validation check that passed */
    FDryRunResult& ValidationPassed(const FString& Check)
    {
        Validations.Add(FString::Printf(TEXT("✓ %s"), *Check));
        return *this;
    }
    
    /** Add a validation check that failed */
    FDryRunResult& ValidationFailed(const FString& Check, const FString& Reason)
    {
        Validations.Add(FString::Printf(TEXT("✗ %s: %s"), *Check, *Reason));
        bHasValidationErrors = true;
        return *this;
    }
    
    /** Add a warning */
    FDryRunResult& Warning(const FString& Warning)
    {
        Warnings.Add(Warning);
        return *this;
    }
    
    /** Add affected items (files, actors, assets, etc.) */
    FDryRunResult& AffectedItem(const FString& Item)
    {
        AffectedItems.Add(Item);
        return *this;
    }
    
    /** Set the reversibility of the operation */
    FDryRunResult& IsReversible(bool bReversible, const FString& RevertMethod = TEXT(""))
    {
        bCanBeReversed = bReversible;
        RevertMethodDescription = RevertMethod;
        return *this;
    }
    
    /** Build the final result */
    FClaudeToolResult Build() const
    {
        FClaudeToolResult Result;
        Result.ToolUseId = ToolUseId;
        Result.bSuccess = !bHasValidationErrors;
        
        TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
        Json->SetBoolField(TEXT("dry_run"), true);
        Json->SetBoolField(TEXT("would_succeed"), !bHasValidationErrors);
        Json->SetStringField(TEXT("tool"), ToolName);
        
        // Add what would be performed
        TArray<TSharedPtr<FJsonValue>> WouldPerformArray;
        for (const FString& Action : WouldPerform)
        {
            WouldPerformArray.Add(MakeShared<FJsonValueString>(Action));
        }
        Json->SetArrayField(TEXT("would_perform"), WouldPerformArray);
        
        // Add validations
        if (Validations.Num() > 0)
        {
            TArray<TSharedPtr<FJsonValue>> ValidationsArray;
            for (const FString& V : Validations)
            {
                ValidationsArray.Add(MakeShared<FJsonValueString>(V));
            }
            Json->SetArrayField(TEXT("validations"), ValidationsArray);
        }
        
        // Add warnings
        if (Warnings.Num() > 0)
        {
            TArray<TSharedPtr<FJsonValue>> WarningsArray;
            for (const FString& W : Warnings)
            {
                WarningsArray.Add(MakeShared<FJsonValueString>(W));
            }
            Json->SetArrayField(TEXT("warnings"), WarningsArray);
        }
        
        // Add affected items
        if (AffectedItems.Num() > 0)
        {
            TArray<TSharedPtr<FJsonValue>> AffectedArray;
            for (const FString& Item : AffectedItems)
            {
                AffectedArray.Add(MakeShared<FJsonValueString>(Item));
            }
            Json->SetArrayField(TEXT("affected_items"), AffectedArray);
            Json->SetNumberField(TEXT("affected_count"), AffectedItems.Num());
        }
        
        // Add reversibility info
        TSharedPtr<FJsonObject> ReversibilityJson = MakeShared<FJsonObject>();
        ReversibilityJson->SetBoolField(TEXT("reversible"), bCanBeReversed);
        if (!RevertMethodDescription.IsEmpty())
        {
            ReversibilityJson->SetStringField(TEXT("revert_method"), RevertMethodDescription);
        }
        Json->SetObjectField(TEXT("reversibility"), ReversibilityJson);
        
        // Serialize
        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);
        
        Result.Result = JsonString;
        return Result;
    }
    
private:
    FString ToolName;
    FString ToolUseId;
    TArray<FString> WouldPerform;
    TArray<FString> Validations;
    TArray<FString> Warnings;
    TArray<FString> AffectedItems;
    bool bHasValidationErrors = false;
    bool bCanBeReversed = false;
    FString RevertMethodDescription;
};

/**
 * Macro to check dry-run mode and return early if active
 * Use at the start of destructive tool implementations
 */
#define CHECK_DRY_RUN_AND_RETURN(Call, ToolName, DryRunResultBuilder) \
    do { \
        if (FDryRunSettings::IsDryRunActive(Call)) { \
            return DryRunResultBuilder.Build(); \
        } \
    } while(0)

/**
 * List of tools that are considered destructive
 */
namespace DestructiveTools
{
    static const TArray<FString> Names = {
        TEXT("delete_actor"),
        TEXT("delete_asset"),
        TEXT("remove_component"),
        TEXT("delete_file"),
        TEXT("batch_delete"),
        TEXT("clear_level"),
        TEXT("destroy_actor"),
        TEXT("remove_blueprint_node"),
        TEXT("delete_blueprint_variable"),
        TEXT("delete_blueprint_function"),
        TEXT("remove_pcg_node"),
        TEXT("delete_niagara_emitter"),
        TEXT("delete_data_table_row"),
        TEXT("delete_sound_cue"),
        TEXT("delete_animation_notify"),
        TEXT("remove_ai_task")
    };
    
    /** Check if a tool name is destructive */
    inline bool IsDestructive(const FString& ToolName)
    {
        return Names.Contains(ToolName);
    }
}
