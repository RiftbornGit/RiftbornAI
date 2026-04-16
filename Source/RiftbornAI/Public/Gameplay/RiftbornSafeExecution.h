// Copyright 2024 Riftborn. All Rights Reserved.
// RiftbornSafeExecution.h - Safe execution wrappers

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "Misc/ScopeLock.h"
#include "RiftbornLog.h"

/**
 * Safe execution result with full error context
 */
struct FSafeExecutionResult
{
    bool bSuccess = false;
    FString Result;
    FString ErrorMessage;
    FString ErrorCode;
    double ExecutionTimeMs = 0.0;
    FString StackTrace;
    
    static FSafeExecutionResult Success(const FString& InResult)
    {
        FSafeExecutionResult R;
        R.bSuccess = true;
        R.Result = InResult;
        return R;
    }
    
    static FSafeExecutionResult Failure(const FString& Code, const FString& Message)
    {
        FSafeExecutionResult R;
        R.bSuccess = false;
        R.ErrorCode = Code;
        R.ErrorMessage = Message;
        return R;
    }
};

/**
 * Execution statistics for monitoring
 */
struct FExecutionStats
{
    FThreadSafeCounter TotalExecutions;
    FThreadSafeCounter SuccessfulExecutions;
    FThreadSafeCounter FailedExecutions;
    FThreadSafeCounter TimeoutExecutions;
    TAtomic<double> TotalExecutionTimeMs;
    TAtomic<double> MaxExecutionTimeMs;
    
    void RecordExecution(bool bSuccess, bool bTimeout, double TimeMs)
    {
        TotalExecutions.Increment();
        if (bSuccess) SuccessfulExecutions.Increment();
        else if (bTimeout) TimeoutExecutions.Increment();
        else FailedExecutions.Increment();
        
        TotalExecutionTimeMs.Store(TotalExecutionTimeMs.Load() + TimeMs);
        if (TimeMs > MaxExecutionTimeMs.Load())
        {
            MaxExecutionTimeMs.Store(TimeMs);
        }
    }
    
    double GetAverageTimeMs() const
    {
        int32 Total = TotalExecutions.GetValue();
        return Total > 0 ? TotalExecutionTimeMs.Load() / Total : 0.0;
    }
    
    double GetSuccessRate() const
    {
        int32 Total = TotalExecutions.GetValue();
        return Total > 0 ? (double)SuccessfulExecutions.GetValue() / Total : 1.0;
    }
};

/**
 * Input validation helpers
 */
namespace RiftbornValidation
{
    // Validate a UE content path
    inline bool IsValidContentPath(const FString& Path, FString& OutError)
    {
        if (Path.IsEmpty())
        {
            OutError = TEXT("Path is empty");
            return false;
        }
        
        // Must start with /Game/, /Engine/, or /Script/
        if (!Path.StartsWith(TEXT("/Game/")) && 
            !Path.StartsWith(TEXT("/Engine/")) && 
            !Path.StartsWith(TEXT("/Script/")))
        {
            OutError = FString::Printf(TEXT("Invalid path prefix. Must start with /Game/, /Engine/, or /Script/. Got: %s"), *Path.Left(50));
            return false;
        }
        
        // Check for path traversal attacks
        if (Path.Contains(TEXT("..")) || Path.Contains(TEXT("./")) || Path.Contains(TEXT("\\")))
        {
            OutError = TEXT("Path contains invalid traversal characters");
            return false;
        }
        
        // Check for null bytes
        if (Path.Contains(TEXT("\0")))
        {
            OutError = TEXT("Path contains null bytes");
            return false;
        }
        
        return true;
    }
    
    // Validate a filesystem path (for sandboxed operations only)
    inline bool IsValidFilePath(const FString& Path, const FString& AllowedRoot, FString& OutError)
    {
        if (Path.IsEmpty())
        {
            OutError = TEXT("File path is empty");
            return false;
        }
        
        // Normalize path
        FString NormalizedPath = FPaths::ConvertRelativePathToFull(Path);
        FString NormalizedRoot = FPaths::ConvertRelativePathToFull(AllowedRoot);
        
        // Must be under allowed root
        if (!NormalizedPath.StartsWith(NormalizedRoot))
        {
            OutError = FString::Printf(TEXT("Path is outside allowed directory: %s"), *AllowedRoot);
            return false;
        }
        
        // Check for path traversal
        if (Path.Contains(TEXT("..")))
        {
            OutError = TEXT("Path traversal (..) not allowed");
            return false;
        }
        
        return true;
    }
    
    // Validate string input (general purpose)
    inline bool IsValidString(const FString& Input, int32 MaxLength, FString& OutError)
    {
        if (Input.Len() > MaxLength)
        {
            OutError = FString::Printf(TEXT("String exceeds maximum length of %d characters"), MaxLength);
            return false;
        }
        
        // Check for null bytes
        if (Input.Contains(TEXT("\0")))
        {
            OutError = TEXT("String contains null bytes");
            return false;
        }
        
        return true;
    }
    
    // Validate numeric range
    template<typename T>
    inline bool IsInRange(T Value, T Min, T Max, const FString& ParamName, FString& OutError)
    {
        if (Value < Min || Value > Max)
        {
            OutError = FString::Printf(TEXT("%s must be between %s and %s"), 
                *ParamName, *LexToString(Min), *LexToString(Max));
            return false;
        }
        return true;
    }
    
    // Validate required parameter exists
    inline bool RequireParameter(const TMap<FString, FString>& Args, const FString& ParamName, FString& OutError)
    {
        if (!Args.Contains(ParamName) || Args[ParamName].IsEmpty())
        {
            OutError = FString::Printf(TEXT("Required parameter '%s' is missing"), *ParamName);
            return false;
        }
        return true;
    }
}

/**
 * Safe execution wrapper - wraps any function with error handling, timing, and logging
 */
class FRiftbornSafeExecutor
{
public:
    // Default timeout in seconds
    static constexpr double DefaultTimeoutSeconds = 30.0;
    
    // Execute a function safely with full error handling
    template<typename Func>
    static FSafeExecutionResult Execute(
        const FString& OperationName,
        Func&& Function,
        double TimeoutSeconds = DefaultTimeoutSeconds)
    {
        FSafeExecutionResult Result;
        double StartTime = FPlatformTime::Seconds();
        
        RIFTBORN_LOG(Log, TEXT("SafeExec: Starting '%s' (timeout: %.1fs)"), *OperationName, TimeoutSeconds);
        
        // Note: UE4/5 doesn't have standard C++ exceptions enabled by default
        // We use structured error checking instead
        
        // Execute the function
        bool bTimedOut = false;
        FString ExecutionResult;
        FString ExecutionError;
        bool bSuccess = false;
        
        // Timeout enforcement: We use a watchdog timer approach.
        // Full preemptive timeout requires running in a separate thread, which is
        // problematic for UE API calls that must run on the game thread.
        // Instead, we:
        // 1. Record start time before execution
        // 2. Check elapsed time after execution
        // 3. Mark as timed out if exceeded (post-hoc timeout detection)
        // 4. Tools should call CheckTimeout() periodically for cooperative cancellation
        //
        // For truly async operations, use ExecuteAsync() with a cancellation token.
        
        bSuccess = Function(ExecutionResult, ExecutionError);
        
        // Post-hoc timeout detection
        double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
        if (ElapsedSeconds > TimeoutSeconds)
        {
            bTimedOut = true;
            RIFTBORN_LOG(Warning, TEXT("SafeExec: '%s' exceeded timeout (%.1fs > %.1fs) - marking as timeout"),
                *OperationName, ElapsedSeconds, TimeoutSeconds);
        }
        
        Result.ExecutionTimeMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
        
        if (bSuccess)
        {
            Result.bSuccess = true;
            Result.Result = ExecutionResult;
            RIFTBORN_LOG(Log, TEXT("SafeExec: '%s' completed in %.2fms"), *OperationName, Result.ExecutionTimeMs);
        }
        else
        {
            Result.bSuccess = false;
            Result.ErrorMessage = ExecutionError;
            Result.ErrorCode = TEXT("EXECUTION_FAILED");
            RIFTBORN_LOG(Warning, TEXT("SafeExec: '%s' failed in %.2fms: %s"), 
                *OperationName, Result.ExecutionTimeMs, *ExecutionError);
        }
        
        // Update global stats
        GetStats().RecordExecution(bSuccess, bTimedOut, Result.ExecutionTimeMs);
        
        return Result;
    }
    
    // Execute with retry logic
    template<typename Func>
    static FSafeExecutionResult ExecuteWithRetry(
        const FString& OperationName,
        Func&& Function,
        int32 MaxRetries = 3,
        double RetryDelaySeconds = 0.5,
        double TimeoutSeconds = DefaultTimeoutSeconds)
    {
        FSafeExecutionResult Result;
        
        for (int32 Attempt = 0; Attempt <= MaxRetries; Attempt++)
        {
            if (Attempt > 0)
            {
                RIFTBORN_LOG(Log, TEXT("SafeExec: Retrying '%s' (attempt %d/%d)"), 
                    *OperationName, Attempt + 1, MaxRetries + 1);
                FPlatformProcess::Sleep(RetryDelaySeconds);
            }
            
            Result = Execute(OperationName, Forward<Func>(Function), TimeoutSeconds);
            
            if (Result.bSuccess)
            {
                return Result;
            }
            
            // Don't retry on validation errors or permanent failures
            if (Result.ErrorCode == TEXT("VALIDATION_ERROR") || 
                Result.ErrorCode == TEXT("NOT_FOUND") ||
                Result.ErrorCode == TEXT("PERMISSION_DENIED"))
            {
                break;
            }
        }
        
        return Result;
    }
    
    // Get global execution statistics
    static FExecutionStats& GetStats()
    {
        static FExecutionStats Stats;
        return Stats;
    }
    
    // Cooperative timeout context for long-running operations
    // Tools can call CheckTimeout() periodically to enable cooperative cancellation
    struct FTimeoutContext
    {
        double StartTime = 0.0;
        double TimeoutSeconds = 30.0;
        bool bCancelled = false;
        
        FTimeoutContext(double InTimeoutSeconds = 30.0)
            : StartTime(FPlatformTime::Seconds())
            , TimeoutSeconds(InTimeoutSeconds)
            , bCancelled(false)
        {
        }
        
        // Check if operation should be cancelled (call periodically in loops)
        bool ShouldCancel() const
        {
            if (bCancelled) return true;
            return (FPlatformTime::Seconds() - StartTime) > TimeoutSeconds;
        }
        
        // Get remaining time
        double GetRemainingSeconds() const
        {
            double Elapsed = FPlatformTime::Seconds() - StartTime;
            return FMath::Max(0.0, TimeoutSeconds - Elapsed);
        }
        
        // Request cancellation
        void Cancel() { bCancelled = true; }
    };
    
    // Thread-local timeout context for cooperative cancellation
    static FTimeoutContext*& GetCurrentTimeoutContext()
    {
        static thread_local FTimeoutContext* Context = nullptr;
        return Context;
    }
    
    // Check if current operation should be cancelled (call from within tools)
    static bool CheckTimeout()
    {
        FTimeoutContext* Ctx = GetCurrentTimeoutContext();
        return Ctx ? Ctx->ShouldCancel() : false;
    }
    
    // Get remaining time for current operation
    static double GetRemainingTime()
    {
        FTimeoutContext* Ctx = GetCurrentTimeoutContext();
        return Ctx ? Ctx->GetRemainingSeconds() : DBL_MAX;
    }
    
    // Generate stats report
    static FString GetStatsReport()
    {
        const FExecutionStats& Stats = GetStats();
        return FString::Printf(
            TEXT("Executions: %d (Success: %d, Failed: %d, Timeout: %d)\n")
            TEXT("Success Rate: %.2f%%\n")
            TEXT("Avg Time: %.2fms, Max Time: %.2fms"),
            Stats.TotalExecutions.GetValue(),
            Stats.SuccessfulExecutions.GetValue(),
            Stats.FailedExecutions.GetValue(),
            Stats.TimeoutExecutions.GetValue(),
            Stats.GetSuccessRate() * 100.0,
            Stats.GetAverageTimeMs(),
            Stats.MaxExecutionTimeMs.Load()
        );
    }
};

/**
 * Circuit breaker for external service calls
 */
class FRiftbornCircuitBreakerGuard
{
public:
    enum class EState : uint8
    {
        Closed,     // Normal operation
        Open,       // Failing, reject all calls
        HalfOpen    // Testing if service recovered
    };
    
    FRiftbornCircuitBreakerGuard(
        const FString& InServiceName,
        int32 InFailureThreshold = 5,
        double InResetTimeSeconds = 60.0)
        : ServiceName(InServiceName)
        , FailureThreshold(InFailureThreshold)
        , ResetTimeSeconds(InResetTimeSeconds)
        , CurrentState(EState::Closed)
        , FailureCount(0)
        , LastFailureTime(0)
    {
    }
    
    // Check if request should be allowed
    bool AllowRequest()
    {
        FScopeLock Lock(&CriticalSection);
        
        switch (CurrentState)
        {
            case EState::Closed:
                return true;
                
            case EState::Open:
            {
                double Now = FPlatformTime::Seconds();
                if (Now - LastFailureTime >= ResetTimeSeconds)
                {
                    CurrentState = EState::HalfOpen;
                    RIFTBORN_LOG(Log, TEXT("CircuitBreaker '%s': Transitioning to HalfOpen"), *ServiceName);
                    return true; // Allow one test request
                }
                return false;
            }
            
            case EState::HalfOpen:
                return true; // Allow test request
        }
        
        return false;
    }
    
    // Record successful call
    void RecordSuccess()
    {
        FScopeLock Lock(&CriticalSection);
        
        if (CurrentState == EState::HalfOpen)
        {
            CurrentState = EState::Closed;
            FailureCount = 0;
            RIFTBORN_LOG(Log, TEXT("CircuitBreaker '%s': Recovered, transitioning to Closed"), *ServiceName);
        }
    }
    
    // Record failed call
    void RecordFailure()
    {
        FScopeLock Lock(&CriticalSection);
        
        FailureCount++;
        LastFailureTime = FPlatformTime::Seconds();
        
        if (CurrentState == EState::HalfOpen || FailureCount >= FailureThreshold)
        {
            CurrentState = EState::Open;
            RIFTBORN_LOG(Warning, TEXT("CircuitBreaker '%s': Opening circuit after %d failures"), 
                *ServiceName, FailureCount);
        }
    }
    
    EState GetState() const { return CurrentState; }
    int32 GetFailureCount() const { return FailureCount; }
    
private:
    FString ServiceName;
    int32 FailureThreshold;
    double ResetTimeSeconds;
    EState CurrentState;
    int32 FailureCount;
    double LastFailureTime;
    FCriticalSection CriticalSection;
};

/**
 * Rate limiter for API calls
 */
class FRiftbornRateLimiter
{
public:
    FRiftbornRateLimiter(int32 InMaxRequests, double InWindowSeconds)
        : MaxRequests(InMaxRequests)
        , WindowSeconds(InWindowSeconds)
    {
    }
    
    // Check if request should be allowed, returns true if allowed
    bool TryAcquire()
    {
        FScopeLock Lock(&CriticalSection);
        
        double Now = FPlatformTime::Seconds();
        
        // Remove expired timestamps
        while (RequestTimestamps.Num() > 0 && Now - RequestTimestamps[0] > WindowSeconds)
        {
            RequestTimestamps.RemoveAt(0);
        }
        
        // Check if under limit
        if (RequestTimestamps.Num() < MaxRequests)
        {
            RequestTimestamps.Add(Now);
            return true;
        }
        
        return false;
    }
    
    // Get remaining requests in window
    int32 GetRemainingRequests() const
    {
        return FMath::Max(0, MaxRequests - RequestTimestamps.Num());
    }
    
    // Get time until next request allowed (in seconds)
    double GetTimeUntilAvailable() const
    {
        if (RequestTimestamps.Num() < MaxRequests)
        {
            return 0.0;
        }
        
        double Now = FPlatformTime::Seconds();
        double OldestRequest = RequestTimestamps[0];
        return FMath::Max(0.0, WindowSeconds - (Now - OldestRequest));
    }
    
private:
    int32 MaxRequests;
    double WindowSeconds;
    TArray<double> RequestTimestamps;
    FCriticalSection CriticalSection;
};

/**
 * Macro for safe tool execution with automatic error handling
 */
#define RIFTBORN_SAFE_TOOL_BEGIN(ToolName) \
    FClaudeToolResult __SafeResult; \
    __SafeResult.ToolUseId = Call.ToolUseId; \
    const FString __ToolName = TEXT(#ToolName); \
    double __StartTime = FPlatformTime::Seconds();

#define RIFTBORN_SAFE_TOOL_END() \
    __SafeResult.ExecutionTimeMs = (FPlatformTime::Seconds() - __StartTime) * 1000.0; \
    if (!__SafeResult.bSuccess) { \
        RIFTBORN_LOG(Warning, TEXT("Tool '%s' failed in %.2fms: %s"), *__ToolName, __SafeResult.ExecutionTimeMs, *__SafeResult.ErrorMessage); \
    } else { \
        RIFTBORN_LOG(Log, TEXT("Tool '%s' completed in %.2fms"), *__ToolName, __SafeResult.ExecutionTimeMs); \
    } \
    return __SafeResult;

#define RIFTBORN_VALIDATE_REQUIRED(ParamName) \
    { \
        FString __ValError; \
        if (!RiftbornValidation::RequireParameter(Call.Arguments, TEXT(#ParamName), __ValError)) { \
            __SafeResult.bSuccess = false; \
            __SafeResult.ErrorMessage = __ValError; \
            __SafeResult.ErrorCode = TEXT("VALIDATION_ERROR"); \
            RIFTBORN_SAFE_TOOL_END(); \
        } \
    }

#define RIFTBORN_VALIDATE_PATH(PathVar) \
    { \
        FString __ValError; \
        if (!RiftbornValidation::IsValidContentPath(PathVar, __ValError)) { \
            __SafeResult.bSuccess = false; \
            __SafeResult.ErrorMessage = __ValError; \
            __SafeResult.ErrorCode = TEXT("VALIDATION_ERROR"); \
            RIFTBORN_SAFE_TOOL_END(); \
        } \
    }

#define RIFTBORN_CHECK_NULL(Ptr, ErrorMsg) \
    if (!(Ptr)) { \
        __SafeResult.bSuccess = false; \
        __SafeResult.ErrorMessage = TEXT(ErrorMsg); \
        __SafeResult.ErrorCode = TEXT("NULL_POINTER"); \
        RIFTBORN_SAFE_TOOL_END(); \
    }

#define RIFTBORN_CHECK_EDITOR() \
    if (!GEditor) { \
        __SafeResult.bSuccess = false; \
        __SafeResult.ErrorMessage = TEXT("Editor is not available"); \
        __SafeResult.ErrorCode = TEXT("NO_EDITOR"); \
        RIFTBORN_SAFE_TOOL_END(); \
    }

#define RIFTBORN_CHECK_WORLD() \
    RIFTBORN_CHECK_EDITOR(); \
    UWorld* __World = GEditor->GetEditorWorldContext().World(); \
    if (!__World) { \
        __SafeResult.bSuccess = false; \
        __SafeResult.ErrorMessage = TEXT("No world loaded"); \
        __SafeResult.ErrorCode = TEXT("NO_WORLD"); \
        RIFTBORN_SAFE_TOOL_END(); \
    }
