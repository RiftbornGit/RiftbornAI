#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QualityOfLife.generated.h"

// ============================================================================
// INLINE SUGGESTIONS
// Backs the registered Tool_GenerateInlineSuggestions (generate_inline_suggestions).
// ============================================================================

UENUM(BlueprintType)
enum class ESuggestionType : uint8
{
	CodeCompletion,
	PropertySuggestion,
	FunctionSuggestion,
	ClassSuggestion,
	IncludeSuggestion,
	QuickFix,
	Refactoring
};

USTRUCT(BlueprintType)
struct FInlineSuggestion
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	ESuggestionType Type = ESuggestionType::CodeCompletion;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString SuggestionText;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 LineNumber = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 ColumnNumber = 0;

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	FString ReplacementText; // Full text to insert

	UPROPERTY(BlueprintReadWrite, Category = "RiftbornAI")
	int32 Priority = 0; // Higher = shown first
};

// ============================================================================
// QUALITY OF LIFE UTILITIES
// ============================================================================

struct FQualityOfLifeUtils
{
	/** Analyze a line of C++ source and produce inline suggestions (UPROPERTY macros,
	    virtual overrides, etc.). Called by Tool_GenerateInlineSuggestions. */
	static TArray<FInlineSuggestion> GenerateSuggestionsForContext(const FString& Code, int32 LineNumber, int32 Column);
};
