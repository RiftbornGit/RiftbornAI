// Copyright RiftbornAI. All Rights Reserved.
// ErrorMessageEnhancer.h - Improves error messages with context, suggestions, and documentation links

#pragma once

#include "CoreMinimal.h"

/**
 * Error categories for grouping related errors
 */
enum class EErrorCategory : uint8
{
    Blueprint,
    Actor,
    Asset,
    World,
    Python,
    Material,
    Security,
    Parameter,
    FileIO,
    PCG,
    Animation,
    Level,
    Component,
    Network,
    Unknown
};

/**
 * Enhanced error information with context and recovery options
 */
struct RIFTBORNAI_API FEnhancedError
{
    // Original error message
    FString OriginalMessage;
    
    // Enhanced message with more context
    FString EnhancedMessage;
    
    // Category for grouping
    EErrorCategory Category = EErrorCategory::Unknown;
    
    // Error code for tracking (e.g., "RIFT-001")
    FString ErrorCode;
    
    // What the user can do to fix this
    TArray<FString> SuggestedActions;
    
    // Related documentation links
    TArray<FString> DocLinks;
    
    // Related tools that might help
    TArray<FString> RelatedTools;
    
    // Common causes of this error
    TArray<FString> CommonCauses;
    
    // Whether this is a transient error that might resolve itself
    bool bIsTransient = false;
    
    // Whether this error is recoverable
    bool bIsRecoverable = true;
    
    // Build the full enhanced message string
    FString GetFullMessage() const;
};

/**
 * Registry of known error patterns and their enhancements
 */
struct FErrorPattern
{
    // Pattern to match in error message (supports wildcards)
    FString Pattern;
    
    // Category for this error type
    EErrorCategory Category;
    
    // Error code prefix
    FString CodePrefix;
    
    // Template for enhanced message (can use {original} placeholder)
    FString EnhancedTemplate;
    
    // Suggested actions
    TArray<FString> SuggestedActions;
    
    // Documentation links
    TArray<FString> DocLinks;
    
    // Related tools
    TArray<FString> RelatedTools;
    
    // Common causes
    TArray<FString> CommonCauses;
    
    // Is this error transient?
    bool bIsTransient = false;
    
    // Is this error recoverable?
    bool bIsRecoverable = true;
};

/**
 * Singleton class that enhances error messages with context and suggestions
 * 
 * Usage:
 *     FString EnhancedMsg = FErrorMessageEnhancer::Get().Enhance("No world available");
 *     // Returns: "RIFT-W001: No world available
 *     //           Context: The editor world is not ready for actor operations.
 *     //           Suggestions: 1) Wait for level to finish loading, 2) Open a level first
 *     //           Related tools: get_current_level, open_level"
 */
class RIFTBORNAI_API FErrorMessageEnhancer
{
public:
    static FErrorMessageEnhancer& Get();
    
    /**
     * Enhance a raw error message with context, suggestions, and documentation
     * @param RawMessage The original error message
     * @param ToolName Optional tool name for additional context
     * @return Enhanced error information
     */
    FEnhancedError Enhance(const FString& RawMessage, const FString& ToolName = TEXT(""));
    
    /**
     * Get just the enhanced message string (convenience method)
     */
    FString EnhanceToString(const FString& RawMessage, const FString& ToolName = TEXT(""));
    
    /**
     * Register a custom error pattern
     */
    void RegisterPattern(const FErrorPattern& Pattern);
    
    /**
     * Get error statistics (for debugging/metrics)
     */
    TMap<EErrorCategory, int32> GetErrorStatistics() const;
    
    /**
     * Clear error statistics
     */
    void ClearStatistics();
    
    /**
     * Get category display name
     */
    static FString GetCategoryName(EErrorCategory Category);

    // Constructor is public for MakeUnique, but use Get() for access
    FErrorMessageEnhancer();

private:
    void InitializePatterns();
    
    // Find best matching pattern for an error message
    const FErrorPattern* FindMatchingPattern(const FString& Message) const;
    
    // Generate unique error code
    FString GenerateErrorCode(EErrorCategory Category, int32 Index) const;
    
    // Registered error patterns
    TArray<FErrorPattern> Patterns;
    
    // Error occurrence statistics
    TMap<EErrorCategory, int32> ErrorStats;
    mutable FCriticalSection StatsLock;
    
    // Error code index per category
    TMap<EErrorCategory, int32> CodeIndices;
    
    // Singleton instance
    static TUniquePtr<FErrorMessageEnhancer> Instance;
};

/**
 * Macro to enhance error messages in tool results
 * Usage: ENHANCE_ERROR(Result, "Original error message", "tool_name");
 */
#define ENHANCE_ERROR(Result, Message, ToolName) \
    Result.ErrorMessage = FErrorMessageEnhancer::Get().EnhanceToString(Message, ToolName)

/**
 * Macro for quick error enhancement without tool name
 */
#define ENHANCED_ERROR(Message) \
    FErrorMessageEnhancer::Get().EnhanceToString(Message)
