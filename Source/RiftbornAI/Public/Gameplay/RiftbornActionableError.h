// Copyright Riftborn. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RiftbornActionableError.generated.h"

/**
 * Error severity levels
 */
UENUM(BlueprintType)
enum class ERiftbornErrorSeverity : uint8
{
	Info UMETA(DisplayName = "Info"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error"),
	Critical UMETA(DisplayName = "Critical")
};

/**
 * Error categories for grouping and filtering
 */
UENUM(BlueprintType)
enum class ERiftbornErrorCategory : uint8
{
	Bridge UMETA(DisplayName = "Bridge Connection"),
	AI UMETA(DisplayName = "AI Provider"),
	Blueprint UMETA(DisplayName = "Blueprint"),
	Asset UMETA(DisplayName = "Asset"),
	Transaction UMETA(DisplayName = "Transaction"),
	Verification UMETA(DisplayName = "Verification"),
	Python UMETA(DisplayName = "Python Execution"),
	Configuration UMETA(DisplayName = "Configuration"),
	Unknown UMETA(DisplayName = "Unknown")
};

/**
 * Actionable error with user-friendly message and resolution steps
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornActionableError
{
	GENERATED_BODY()
	
	/** Unique error code for programmatic handling */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ErrorCode;
	
	/** Human-readable error title */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Title;
	
	/** Detailed error description */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Description;
	
	/** Error category */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	ERiftbornErrorCategory Category = ERiftbornErrorCategory::Unknown;
	
	/** Error severity */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	ERiftbornErrorSeverity Severity = ERiftbornErrorSeverity::Error;
	
	/** Suggested resolution steps */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> ResolutionSteps;
	
	/** Related documentation URL (if any) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString DocumentationUrl;
	
	/** Original technical error (for logging) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString TechnicalDetails;
	
	/** Affected asset paths (if applicable) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FString> AffectedAssets;
	
	/** Whether this error is recoverable */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bRecoverable = true;
	
	/** Suggested auto-fix action (if available) */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString AutoFixAction;
	
	/** Timestamp when error occurred */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FDateTime Timestamp = FDateTime();
	
	FRiftbornActionableError()
		: Timestamp(FDateTime::Now())
	{
	}
	
	/** Create from raw error message */
	static FRiftbornActionableError FromRawError(const FString& RawError);
	
	/** Format as user-friendly string */
	FString FormatForUser() const;
	
	/** Format as detailed log string */
	FString FormatForLog() const;
	
	/** Get severity as string */
	FString GetSeverityString() const;
	
	/** Get category as string */
	FString GetCategoryString() const;
};

/**
 * Error registry - maps raw errors to actionable errors
 */
class RIFTBORNAI_API FRiftbornErrorRegistry
{
public:
	/** Get singleton instance */
	static FRiftbornErrorRegistry& Get();
	
	/** Register an error pattern */
	void RegisterPattern(
		const FString& Pattern,
		const FString& ErrorCode,
		const FString& Title,
		const FString& Description,
		ERiftbornErrorCategory Category,
		ERiftbornErrorSeverity Severity,
		const TArray<FString>& ResolutionSteps,
		bool bRecoverable = true
	);
	
	/** Convert raw error to actionable error */
	FRiftbornActionableError TranslateError(const FString& RawError) const;
	
	/** Get all registered error patterns */
	TArray<FString> GetRegisteredPatterns() const;
	
private:
	FRiftbornErrorRegistry();
	
	struct FErrorPattern
	{
		FString Pattern;
		FString ErrorCode;
		FString Title;
		FString Description;
		ERiftbornErrorCategory Category;
		ERiftbornErrorSeverity Severity;
		TArray<FString> ResolutionSteps;
		bool bRecoverable;
	};
	
	TArray<FErrorPattern> Patterns;
	mutable FCriticalSection RegistryMutex;
	
	void RegisterDefaultPatterns();
};

/**
 * Error result with multiple actionable errors
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornErrorResult
{
	GENERATED_BODY()
	
	/** Whether the operation was successful */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	bool bSuccess = true;
	
	/** All errors encountered */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	TArray<FRiftbornActionableError> Errors;
	
	/** Summary message */
	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Summary;
	
	/** Add an error */
	void AddError(const FRiftbornActionableError& Error)
	{
		Errors.Add(Error);
		bSuccess = false;
	}
	
	/** Add error from raw string */
	void AddErrorFromRaw(const FString& RawError)
	{
		AddError(FRiftbornActionableError::FromRawError(RawError));
	}
	
	/** Get highest severity error */
	FRiftbornActionableError GetHighestSeverityError() const;
	
	/** Format all errors for user display */
	FString FormatAllForUser() const;
	
	/** Check if any critical errors exist */
	bool HasCriticalErrors() const;
	
	/** Get errors by category */
	TArray<FRiftbornActionableError> GetErrorsByCategory(ERiftbornErrorCategory Category) const;
};
