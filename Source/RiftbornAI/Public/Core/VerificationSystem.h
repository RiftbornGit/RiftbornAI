// VerificationSystem.h - Compile & Runtime Verification for Generated Code
// Ensures all generated code compiles, links, and functions correctly

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "VerificationSystem.generated.h"

// ============================================================================
// VERIFICATION RESULT TYPES
// ============================================================================

UENUM(BlueprintType)
enum class EVerificationStatus : uint8
{
    NotRun,
    Running,
    Passed,
    Failed,
    Warning,
    Skipped
};

UENUM(BlueprintType)
enum class EVerificationType : uint8
{
    Compile,            // C++ compiles without errors
    Link,               // Links successfully
    HeaderCheck,        // Headers are valid and includable
    SyntaxCheck,        // Basic syntax validation
    BlueprintCompile,   // Blueprint compiles
    AssetValidation,    // Asset is valid
    RuntimeTest,        // Actually runs in game
    IntegrationTest     // Works with other systems
};

// ============================================================================
// VERIFICATION STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct FCompileError
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString FilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 LineNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Column = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ErrorCode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsWarning = false;
};

USTRUCT(BlueprintType)
struct FVerificationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EVerificationType VerificationType = EVerificationType::Compile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EVerificationStatus Status = EVerificationStatus::NotRun;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FCompileError> Errors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float DurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FDateTime Timestamp = FDateTime();
};

USTRUCT(BlueprintType)
struct FVerificationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ReportId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FVerificationResult> Results;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 TotalTests = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 PassedTests = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 FailedTests = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 WarningTests = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float TotalDurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FDateTime GeneratedAt = FDateTime();

    bool AllPassed() const { return FailedTests == 0; }
    float PassRate() const { return TotalTests > 0 ? (float)PassedTests / TotalTests * 100.0f : 0.0f; }
};

USTRUCT(BlueprintType)
struct FCodeMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 LinesOfCode = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 NumClasses = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 NumFunctions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 NumIncludes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 CyclomaticComplexity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> Dependencies;
};

// ============================================================================
// VERIFICATION SYSTEM
// ============================================================================

class RIFTBORNAI_API FVerificationSystem
{
public:
    static FVerificationSystem& Get()
    {
        static FVerificationSystem Instance;
        return Instance;
    }

    // ========================================================================
    // PRIMARY VERIFICATION
    // ========================================================================

    // Verify a single file compiles
    FVerificationResult VerifyFile(const FString& FilePath);

    // Verify multiple files compile together
    FVerificationResult VerifyFiles(const TArray<FString>& FilePaths);

    // Verify entire plugin compiles
    FVerificationResult VerifyPlugin(const FString& PluginName);

    // Verify a Blueprint asset
    FVerificationResult VerifyBlueprint(const FString& BlueprintPath);

    // Run all verification on generated content
    FVerificationReport RunFullVerification(const TArray<FString>& GeneratedFiles);

    // ========================================================================
    // STATIC ANALYSIS
    // ========================================================================

    // Check header is valid (includes exist, syntax OK)
    FVerificationResult ValidateHeader(const FString& HeaderPath);

    // Check for common C++ issues
    TArray<FCompileError> StaticAnalyze(const FString& FilePath);

    // Get code metrics
    FCodeMetrics AnalyzeCodeMetrics(const FString& FilePath);

    // Check for missing includes
    TArray<FString> FindMissingIncludes(const FString& FilePath);

    // Check for circular dependencies
    TArray<FString> FindCircularDependencies(const TArray<FString>& FilePaths);

    // ========================================================================
    // QUICK CHECKS
    // ========================================================================

    // Quick syntax check without full compile
    bool QuickSyntaxCheck(const FString& Code);

    // Check if a class exists
    bool ClassExists(const FString& ClassName);

    // Check if an include path is valid
    bool IncludeExists(const FString& IncludePath);

    // ========================================================================
    // RUNTIME TESTS
    // ========================================================================

    // Create and test-spawn an actor
    FVerificationResult TestSpawnActor(const FString& ActorClassName);

    // Test a component can be added
    FVerificationResult TestComponent(const FString& ComponentClassName);

    // Test a function can be called
    FVerificationResult TestFunctionCall(const FString& ClassName, const FString& FunctionName);

    // ========================================================================
    // REPORTING
    // ========================================================================

    // Generate human-readable report
    FString GenerateReportText(const FVerificationReport& Report);

    // Generate JSON report
    FString GenerateReportJson(const FVerificationReport& Report);

    // Save report to file
    bool SaveReport(const FVerificationReport& Report, const FString& FilePath);

    // Get last verification report
    const FVerificationReport& GetLastReport() const { return LastReport; }

private:
    FVerificationSystem() = default;

    // Internal helpers
    bool RunCompiler(const FString& FilePath, TArray<FCompileError>& OutErrors);
    void ParseCompilerOutput(const FString& Output, TArray<FCompileError>& OutErrors);
    FString GetProjectBuildCommand() const;
    FString GetUnrealBuildToolPath() const;

    FVerificationReport LastReport;
};
