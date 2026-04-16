// Copyright 2024 Riftborn. All Rights Reserved.
// RiftbornTestFramework.h - Production test framework for tool validation

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ClaudeToolUse.h"

/**
 * Test result with detailed information
 */
struct FRiftbornTestResult
{
    FString TestName;
    bool bPassed = false;
    FString FailureReason;
    double ExecutionTimeMs = 0.0;
    TArray<FString> Logs;
    
    void Log(const FString& Message)
    {
        Logs.Add(Message);
    }
};

/**
 * Test case definition
 */
struct FRiftbornToolTestCase
{
    FString TestName;
    FString ToolName;
    TMap<FString, FString> Arguments;
    bool bExpectSuccess = true;
    FString ExpectedErrorCode;
    TFunction<bool(const FClaudeToolResult&)> CustomValidator;
};

/**
 * Test suite for a group of related tests
 */
class FRiftbornToolTestSuite
{
public:
    FRiftbornToolTestSuite(const FString& InSuiteName)
        : SuiteName(InSuiteName)
    {
    }
    
    // Add a test case
    void AddTest(const FRiftbornToolTestCase& TestCase)
    {
        TestCases.Add(TestCase);
    }
    
    // Add success test
    void AddSuccessTest(
        const FString& TestName,
        const FString& ToolName,
        const TMap<FString, FString>& Args)
    {
        FRiftbornToolTestCase Test;
        Test.TestName = TestName;
        Test.ToolName = ToolName;
        Test.Arguments = Args;
        Test.bExpectSuccess = true;
        TestCases.Add(Test);
    }
    
    // Add failure test (expect specific error)
    void AddFailureTest(
        const FString& TestName,
        const FString& ToolName,
        const TMap<FString, FString>& Args,
        const FString& ExpectedError = TEXT(""))
    {
        FRiftbornToolTestCase Test;
        Test.TestName = TestName;
        Test.ToolName = ToolName;
        Test.Arguments = Args;
        Test.bExpectSuccess = false;
        Test.ExpectedErrorCode = ExpectedError;
        TestCases.Add(Test);
    }
    
    // Run all tests
    TArray<FRiftbornTestResult> RunAll()
    {
        TArray<FRiftbornTestResult> Results;
        
        for (const FRiftbornToolTestCase& Test : TestCases)
        {
            Results.Add(RunTest(Test));
        }
        
        return Results;
    }
    
    // Run a single test
    FRiftbornTestResult RunTest(const FRiftbornToolTestCase& Test)
    {
        FRiftbornTestResult Result;
        Result.TestName = FString::Printf(TEXT("%s::%s"), *SuiteName, *Test.TestName);
        
        double StartTime = FPlatformTime::Seconds();
        
        // Build tool call
        FClaudeToolCall Call;
        Call.ToolName = Test.ToolName;
        Call.Arguments = Test.Arguments;
        Call.ToolUseId = FGuid::NewGuid().ToString();
        
        Result.Log(FString::Printf(TEXT("Executing tool: %s"), *Test.ToolName));
        
        // Execute tool
        FClaudeToolResult ToolResult = FClaudeToolRegistry::Get().ExecuteTool(Call);
        
        Result.ExecutionTimeMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
        
        // Validate result
        if (Test.bExpectSuccess)
        {
            if (ToolResult.bSuccess)
            {
                Result.bPassed = true;
                Result.Log(TEXT("✓ Tool succeeded as expected"));
            }
            else
            {
                Result.bPassed = false;
                Result.FailureReason = FString::Printf(
                    TEXT("Expected success but got error: %s"),
                    *ToolResult.ErrorMessage);
            }
        }
        else // Expect failure
        {
            if (!ToolResult.bSuccess)
            {
                if (Test.ExpectedErrorCode.IsEmpty() || 
                    ToolResult.ErrorMessage.Contains(Test.ExpectedErrorCode))
                {
                    Result.bPassed = true;
                    Result.Log(TEXT("✓ Tool failed as expected"));
                }
                else
                {
                    Result.bPassed = false;
                    Result.FailureReason = FString::Printf(
                        TEXT("Expected error '%s' but got '%s'"),
                        *Test.ExpectedErrorCode, *ToolResult.ErrorMessage);
                }
            }
            else
            {
                Result.bPassed = false;
                Result.FailureReason = TEXT("Expected failure but tool succeeded");
            }
        }
        
        // Run custom validator if provided
        if (Result.bPassed && Test.CustomValidator)
        {
            if (!Test.CustomValidator(ToolResult))
            {
                Result.bPassed = false;
                Result.FailureReason = TEXT("Custom validation failed");
            }
        }
        
        return Result;
    }
    
    FString GetSuiteName() const { return SuiteName; }
    int32 GetTestCount() const { return TestCases.Num(); }
    
private:
    FString SuiteName;
    TArray<FRiftbornToolTestCase> TestCases;
};

/**
 * Test runner for all suites
 */
class FRiftbornTestRunner
{
public:
    static FRiftbornTestRunner& Get()
    {
        static FRiftbornTestRunner Instance;
        return Instance;
    }
    
    // Register a test suite
    void RegisterSuite(TSharedPtr<FRiftbornToolTestSuite> Suite)
    {
        Suites.Add(Suite);
    }
    
    // Run all registered suites
    TArray<FRiftbornTestResult> RunAllSuites()
    {
        TArray<FRiftbornTestResult> AllResults;
        
        for (const TSharedPtr<FRiftbornToolTestSuite>& Suite : Suites)
        {
            TArray<FRiftbornTestResult> SuiteResults = Suite->RunAll();
            AllResults.Append(SuiteResults);
        }
        
        return AllResults;
    }
    
    // Generate report
    FString GenerateReport(const TArray<FRiftbornTestResult>& Results)
    {
        int32 Passed = 0;
        int32 Failed = 0;
        double TotalTime = 0.0;
        
        FString Report;
        Report += TEXT("═══════════════════════════════════════════\n");
        Report += TEXT("       RIFTBORN AI TEST REPORT\n");
        Report += TEXT("═══════════════════════════════════════════\n\n");
        
        for (const FRiftbornTestResult& Result : Results)
        {
            if (Result.bPassed)
            {
                Passed++;
                Report += FString::Printf(TEXT("✓ PASS: %s (%.2fms)\n"), 
                    *Result.TestName, Result.ExecutionTimeMs);
            }
            else
            {
                Failed++;
                Report += FString::Printf(TEXT("✗ FAIL: %s (%.2fms)\n"), 
                    *Result.TestName, Result.ExecutionTimeMs);
                Report += FString::Printf(TEXT("  Reason: %s\n"), *Result.FailureReason);
            }
            TotalTime += Result.ExecutionTimeMs;
        }
        
        Report += TEXT("\n═══════════════════════════════════════════\n");
        Report += FString::Printf(TEXT("SUMMARY: %d passed, %d failed, %d total\n"), 
            Passed, Failed, Results.Num());
        Report += FString::Printf(TEXT("Total execution time: %.2fms\n"), TotalTime);
        Report += FString::Printf(TEXT("Pass rate: %.1f%%\n"), 
            Results.Num() > 0 ? (float)Passed / Results.Num() * 100.0f : 0.0f);
        Report += TEXT("═══════════════════════════════════════════\n");
        
        return Report;
    }
    
private:
    TArray<TSharedPtr<FRiftbornToolTestSuite>> Suites;
};

/**
 * Built-in test suites for critical tools
 */
namespace RiftbornTests
{
    // Input validation tests
    inline TSharedPtr<FRiftbornToolTestSuite> CreateInputValidationSuite()
    {
        auto Suite = MakeShared<FRiftbornToolTestSuite>(TEXT("InputValidation"));
        
        // Empty parameter tests
        Suite->AddFailureTest(
            TEXT("spawn_actor_empty_class"),
            TEXT("spawn_actor"),
            {}, // No arguments
            TEXT("required")
        );
        
        Suite->AddFailureTest(
            TEXT("create_blueprint_empty_name"),
            TEXT("create_blueprint"),
            {{TEXT("parent_class"), TEXT("Actor")}}, // Missing name
            TEXT("required")
        );
        
        // Invalid path tests
        Suite->AddFailureTest(
            TEXT("open_blueprint_invalid_path"),
            TEXT("open_blueprint"),
            {{TEXT("path"), TEXT("../../../etc/passwd")}},
            TEXT("Invalid")
        );
        
        Suite->AddFailureTest(
            TEXT("find_assets_path_traversal"),
            TEXT("find_assets"),
            {{TEXT("search_query"), TEXT("../../Windows/System32")}},
            TEXT("")
        );
        
        return Suite;
    }
    
    // Null handling tests
    inline TSharedPtr<FRiftbornToolTestSuite> CreateNullHandlingSuite()
    {
        auto Suite = MakeShared<FRiftbornToolTestSuite>(TEXT("NullHandling"));
        
        // Non-existent assets
        Suite->AddFailureTest(
            TEXT("open_nonexistent_blueprint"),
            TEXT("open_blueprint"),
            {{TEXT("path"), TEXT("/Game/NonExistent/BP_DoesNotExist")}},
            TEXT("not found")
        );
        
        Suite->AddFailureTest(
            TEXT("delete_nonexistent_actor"),
            TEXT("delete_actor"),
            {{TEXT("label"), TEXT("Actor_That_Does_Not_Exist_12345")}},
            TEXT("")
        );
        
        return Suite;
    }
    
    // Basic functionality tests
    inline TSharedPtr<FRiftbornToolTestSuite> CreateBasicFunctionalitySuite()
    {
        auto Suite = MakeShared<FRiftbornToolTestSuite>(TEXT("BasicFunctionality"));
        
        // List operations (should always work)
        Suite->AddSuccessTest(
            TEXT("get_level_actors"),
            TEXT("get_level_actors"),
            {}
        );
        
        Suite->AddSuccessTest(
            TEXT("get_registered_tools"),
            TEXT("get_registered_tools"),
            {}
        );
        
        return Suite;
    }
    
    // Register all built-in test suites
    inline void RegisterBuiltInSuites()
    {
        FRiftbornTestRunner& Runner = FRiftbornTestRunner::Get();
        Runner.RegisterSuite(CreateInputValidationSuite());
        Runner.RegisterSuite(CreateNullHandlingSuite());
        Runner.RegisterSuite(CreateBasicFunctionalitySuite());
    }
}
