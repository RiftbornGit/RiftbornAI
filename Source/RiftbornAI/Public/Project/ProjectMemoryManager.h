// Copyright RiftbornAI. All Rights Reserved.
// ProjectMemoryManager - Persistent memory across sessions
//
// This enables the agent to remember:
// - What assets were created
// - User preferences
// - Common patterns
// - Project structure

#pragma once

#include "CoreMinimal.h"

// =============================================================================
// MEMORY HYGIENE CONSTANTS (2026-02-20)
//
// Prevents prompt injection via persisted tool results, stale data poisoning,
// and tampered memory files. Applied on WRITE (RecordConversation) and on
// READ (GetRecentConversationContext / BuildSystemPromptContext).
// =============================================================================
namespace MemoryHygiene
{
	/** Max bytes for any single field stored in memory (truncated on write) */
	static constexpr int32 MaxFieldBytes = 2048;

	/** Max bytes for user messages (shorter — these are human-typed) */
	static constexpr int32 MaxUserMessageBytes = 1024;

	/** Conversation entries older than this are evicted on load */
	static constexpr int32 EntryTTLDays = 30;

	/** Pinned context entries older than this (no timestamp, so by count) */
	static constexpr int32 MaxPinnedContext = 20;

	/** HMAC signature file alongside memory.json */
	static constexpr const TCHAR* SignatureFileExtension = TEXT(".sig");

	/**
	 * Sanitize a string before storing in memory.
	 *
	 * Strips:
	 *  - JSON-like tool_use blocks (prevent re-injection as tool calls)
	 *  - System prompt override patterns ("<|system|>", "[INST]", etc.)
	 *  - Excessive whitespace/newlines
	 *  - Truncates to MaxBytes
	 */
	RIFTBORNAI_API FString Sanitize(const FString& Raw, int32 MaxBytes = MaxFieldBytes);

	/** Compute HMAC-SHA256 of content using a session-derived key */
	RIFTBORNAI_API FString ComputeHMAC(const FString& Content, const FString& Key);

	/** Verify HMAC matches content */
	RIFTBORNAI_API bool VerifyHMAC(const FString& Content, const FString& ExpectedHMAC, const FString& Key);

	/** Get or derive the HMAC key for this machine (stable across sessions) */
	RIFTBORNAI_API FString GetMachineKey();
};

/**
 * Memory entry for a conversation turn
 */
struct RIFTBORNAI_API FConversationEntry
{
    FDateTime Timestamp;
    FString UserMessage;
    FString ActionTaken;
    FString Result;
    TArray<FString> ToolsCalled;
    bool bSuccess = true;
};

/**
 * Memory entry for created assets
 */
struct RIFTBORNAI_API FCreatedAssetEntry
{
    FDateTime CreatedAt;
    FString AssetPath;
    FString AssetType;  // Blueprint, Widget, Level, etc.
    FString Description;
    FString CreationPrompt;  // What the user asked for
};

/**
 * User preferences learned over time
 */
struct RIFTBORNAI_API FUserPreferences
{
    FString PreferredNamingConvention;  // "PascalCase", "snake_case", etc.
    FString PreferredCodingStyle;
    TMap<FString, FString> CommonPatterns;  // e.g., "health component" → "UHealthComponent"
    TArray<FString> FrequentlyUsedTools;
    TMap<FString, int32> ToolUsageCount;
};

/**
 * Error that occurred and how it was resolved
 */
struct RIFTBORNAI_API FErrorEntry
{
    FDateTime OccurredAt;
    FString ErrorMessage;
    FString Context;
    FString Resolution;
    bool bAutoResolved = false;
};

/**
 * Cached project structure understanding
 */
struct RIFTBORNAI_API FProjectStructureCache
{
    FDateTime LastUpdated;
    
    TArray<FString> BlueprintClasses;
    TArray<FString> CppClasses;
    TArray<FString> Levels;
    TArray<FString> Widgets;
    TArray<FString> DataAssets;
    
    FString DefaultGameMode;
    FString DefaultPawn;
    FString DefaultPlayerController;
    
    TMap<FString, FString> AssetToParentClass;  // Quick parent lookup
};

/**
 * Complete project memory
 */
struct RIFTBORNAI_API FProjectMemory
{
    // Meta
    FString ProjectName;
    FString ProjectPath;
    FDateTime FirstSession;
    FDateTime LastSession;
    int32 TotalSessions = 0;
    int32 TotalTokensUsed = 0;
    
    // Conversation history (last N entries, TTL-evicted on load)
    TArray<FConversationEntry> ConversationHistory;
    static constexpr int32 MaxConversationHistory = 100;
    
    // HMAC signature of last successful save (verified on load)
    FString LastVerifiedHMAC;
    
    // Created assets
    TArray<FCreatedAssetEntry> CreatedAssets;
    
    // User preferences
    FUserPreferences Preferences;
    
    // Errors for learning
    TArray<FErrorEntry> Errors;
    static constexpr int32 MaxErrors = 50;
    
    // Project structure cache
    FProjectStructureCache StructureCache;
    
    // Important context (pinned info)
    TArray<FString> PinnedContext;
    
    // Semantic embeddings for similarity search (future)
    // TArray<FVector> ConversationEmbeddings;
};

/**
 * FProjectMemoryManager
 * 
 * Manages persistent memory for a project:
 * - Saves to Saved/RiftbornAI/memory.json
 * - Loads on plugin startup
 * - Provides context for LLM prompts
 */
class RIFTBORNAI_API FProjectMemoryManager
{
public:
    static FProjectMemoryManager& Get();
    
    // =========================================================================
    // LIFECYCLE
    // =========================================================================
    
    /** Initialize for a project */
    void Initialize(const FString& ProjectPath);
    
    /** Save memory to disk */
    void Save();
    
    /** Force reload from disk */
    void Reload();
    
    // =========================================================================
    // CONVERSATION TRACKING
    // =========================================================================
    
    /** Record a conversation turn */
    void RecordConversation(
        const FString& UserMessage,
        const FString& ActionTaken,
        const FString& Result,
        const TArray<FString>& ToolsCalled,
        bool bSuccess
    );
    
    /** Get recent conversation history as context string */
    FString GetRecentConversationContext(int32 NumEntries = 5) const;

    /** Get the most relevant conversation history for a specific request. */
    FString GetTaskScopedConversationContext(const FString& Query, int32 NumEntries = 3) const;
    
    // =========================================================================
    // ASSET TRACKING
    // =========================================================================
    
    /** Record a created asset */
    void RecordCreatedAsset(
        const FString& AssetPath,
        const FString& AssetType,
        const FString& Description,
        const FString& CreationPrompt
    );
    
    /** Get assets created by the agent */
    TArray<FCreatedAssetEntry> GetCreatedAssets(const FString& TypeFilter = TEXT("")) const;
    
    /** Check if an asset was created by the agent */
    bool WasAssetCreatedByAgent(const FString& AssetPath) const;
    
    // =========================================================================
    // PREFERENCES
    // =========================================================================
    
    /** Record tool usage (for preference learning) */
    void RecordToolUsage(const FString& ToolName);
    
    /** Get user preferences */
    const FUserPreferences& GetPreferences() const { return Memory.Preferences; }
    
    /** Set a preference */
    void SetPreference(const FString& Key, const FString& Value);
    
    /** Get most frequently used tools */
    TArray<FString> GetFrequentTools(int32 TopN = 10) const;
    
    // =========================================================================
    // ERROR TRACKING
    // =========================================================================
    
    /** Record an error */
    void RecordError(const FString& ErrorMessage, const FString& Context);
    
    /** Record error resolution */
    void RecordErrorResolution(const FString& ErrorMessage, const FString& Resolution, bool bAutoResolved);
    
    /** Find similar past errors */
    TArray<FErrorEntry> FindSimilarErrors(const FString& ErrorMessage, int32 MaxResults = 3) const;
    
    /** Get most recent resolved errors (for cross-session learning) */
    TArray<FErrorEntry> GetRecentErrors(int32 MaxResults = 5) const;
    
    // =========================================================================
    // PROJECT STRUCTURE
    // =========================================================================
    
    /** Refresh project structure cache */
    void RefreshProjectStructure();
    
    /** Record a newly created C++ class */
    void RecordCppClassCreated(const FString& ClassName, const FString& ParentClass, const TArray<FString>& FilePaths);
    
    /** Get cached structure */
    const FProjectStructureCache& GetProjectStructure() const { return Memory.StructureCache; }
    
    /** Quick lookup: does this class exist? */
    bool ClassExists(const FString& ClassName) const;
    
    /** Quick lookup: get parent class */
    FString GetParentClass(const FString& ClassName) const;
    
    // =========================================================================
    // CONTEXT BUILDING
    // =========================================================================
    
    /** Build system prompt context from memory */
    FString BuildSystemPromptContext() const;
    
    /** Get pinned context */
    const TArray<FString>& GetPinnedContext() const { return Memory.PinnedContext; }
    
    /** Add pinned context */
    void PinContext(const FString& Context);
    
    /** Remove pinned context */
    void UnpinContext(const FString& Context);
    
    // =========================================================================
    // STATISTICS
    // =========================================================================
    
    /** Get session count */
    int32 GetSessionCount() const { return Memory.TotalSessions; }
    
    /** Get total tokens used */
    int32 GetTotalTokens() const { return Memory.TotalTokensUsed; }
    
    /** Increment token usage */
    void AddTokenUsage(int32 Tokens);
    
private:
    FProjectMemoryManager() = default;
    
    FProjectMemory Memory;
    FString MemoryFilePath;
    bool bInitialized = false;
    
    void LoadFromDisk();
    void SaveToDisk();
    void MigrateIfNeeded();
    
    // =========================================================================
    // MEMORY HYGIENE (2026-02-20)
    // =========================================================================
    
    /** Evict conversation entries older than MemoryHygiene::EntryTTLDays */
    void EvictStaleEntries();
    
    /** Verify HMAC of loaded memory file; returns false if tampered */
    bool VerifyMemoryIntegrity(const FString& JsonContent) const;
    
    /** Write HMAC signature file alongside memory.json */
    void WriteIntegritySignature(const FString& JsonContent) const;
    
    /** Get the signature file path */
    FString GetSignatureFilePath() const;
    
    /** True if last load detected tampering (informational, not blocking) */
    bool bLastLoadTampered = false;
};
