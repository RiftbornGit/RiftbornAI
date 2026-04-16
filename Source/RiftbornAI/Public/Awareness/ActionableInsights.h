// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * ActionableInsights - Automated game analysis with fix recommendations
 * 
 * Goes beyond just reporting state - provides actual actionable fixes:
 * - "Here's what's wrong and here's exactly how to fix it"
 * - Code snippets, Blueprint node suggestions, property changes
 * - Priority-ranked issues with estimated fix effort
 */

// Issue severity
UENUM()
enum class EIssueSeverity : uint8
{
    Critical,   // Game-breaking, must fix
    High,       // Significant impact on quality
    Medium,     // Noticeable issue
    Low,        // Minor polish item
    Info        // Just FYI
};

// Issue category 
UENUM()
enum class EIssueCategory : uint8
{
    Performance,
    Gameplay,
    Navigation,
    Visual,
    Audio,
    Animation,
    Physics,
    AI,
    UX,
    Code
};

// Detailed issue with fix
struct RIFTBORNAI_API FGameIssue
{
    FString Title;
    FString Description;
    EIssueSeverity Severity;
    EIssueCategory Category;
    FString Location;           // Actor name, file path, etc.
    TArray<FString> Fixes;      // Step by step fixes
    FString CodeSnippet;        // Example fix code
    float EstimatedFixTimeMinutes;
    
    TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// ACTIONABLE INSIGHT TOOLS
// ============================================================================

/**
 * Tool: analyze_performance_issues
 * Deep performance analysis with specific fixes
 */
class RIFTBORNAI_API FAnalyzePerformanceIssuesTool : public FAgentTool
{
public:
    FAnalyzePerformanceIssuesTool();
    virtual FString GetName() const override { return TEXT("analyze_performance_issues"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
    
private:
    TArray<FGameIssue> AnalyzeFPS(UWorld* World);
    TArray<FGameIssue> AnalyzeDrawCalls(UWorld* World);
    TArray<FGameIssue> AnalyzeMemory(UWorld* World);
    TArray<FGameIssue> AnalyzeTicking(UWorld* World);
};

/**
 * Tool: analyze_gameplay_issues  
 * Find broken gameplay systems with fixes
 */
class RIFTBORNAI_API FAnalyzeGameplayIssuesTool : public FAgentTool
{
public:
    FAnalyzeGameplayIssuesTool();
    virtual FString GetName() const override { return TEXT("analyze_gameplay_issues"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

/**
 * Tool: analyze_ai_issues
 * Find AI/navigation problems with fixes
 */
class RIFTBORNAI_API FAnalyzeAIIssuesTool : public FAgentTool
{
public:
    FAnalyzeAIIssuesTool();
    virtual FString GetName() const override { return TEXT("analyze_ai_issues"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

/**
 * Tool: get_fix_for_issue
 * Get detailed fix instructions for a specific issue type
 */
class RIFTBORNAI_API FGetFixForIssueTool : public FAgentTool
{
public:
    FGetFixForIssueTool();
    virtual FString GetName() const override { return TEXT("get_fix_for_issue"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

/**
 * Tool: run_full_analysis
 * Complete game health check with prioritized issues
 */
class RIFTBORNAI_API FRunFullAnalysisTool : public FAgentTool
{
public:
    FRunFullAnalysisTool();
    virtual FString GetName() const override { return TEXT("run_full_analysis"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration
class RIFTBORNAI_API FActionableInsightsTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
