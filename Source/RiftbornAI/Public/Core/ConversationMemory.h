// Copyright RiftbornAI. All Rights Reserved.
// Conversation Memory - Persistent storage for chat sessions

#pragma once

#include "CoreMinimal.h"
#include "ConversationMemory.generated.h"

/**
 * A single message in conversation history
 */
USTRUCT()
struct FConversationMessage
{
	GENERATED_BODY()

	UPROPERTY()
	FString Role;  // "user" or "assistant"

	UPROPERTY()
	FString Content;

	UPROPERTY()
	FDateTime Timestamp = FDateTime();

	UPROPERTY()
	TArray<FString> ToolCalls;  // Tool names used in this message

	UPROPERTY()
	int32 InputTokens = 0;

	UPROPERTY()
	int32 OutputTokens = 0;
};

/**
 * A saved conversation session
 */
USTRUCT()
struct FConversationSession
{
	GENERATED_BODY()

	UPROPERTY()
	FString SessionId;

	UPROPERTY()
	FString Title;  // Auto-generated from first message

	UPROPERTY()
	FDateTime CreatedAt = FDateTime();

	UPROPERTY()
	FDateTime LastModified = FDateTime();

	UPROPERTY()
	TArray<FConversationMessage> Messages;

	UPROPERTY()
	int32 TotalInputTokens = 0;

	UPROPERTY()
	int32 TotalOutputTokens = 0;

	UPROPERTY()
	float EstimatedCost = 0.0f;

	UPROPERTY()
	FString ProjectName;

	UPROPERTY()
	FString LevelName;

	// Rich panel snapshot used to restore the Slate copilot exactly as it was.
	UPROPERTY()
	FString PanelStateJson;
};

/**
 * Conversation Memory Manager
 * Handles saving/loading conversation sessions to disk
 */
class RIFTBORNAI_API FConversationMemory
{
public:
	static FConversationMemory& Get();

	// Session management
	FString CreateNewSession();
	bool SaveSession(const FConversationSession& Session);
	bool LoadSession(const FString& SessionId, FConversationSession& OutSession);
	bool DeleteSession(const FString& SessionId);
	
	// Get all saved sessions (metadata only, not full messages)
	TArray<FConversationSession> GetAllSessions();
	
	// Get recent sessions
	TArray<FConversationSession> GetRecentSessions(int32 Count = 10);

	// Add message to current session
	void AddMessage(const FString& Role, const FString& Content, int32 InputTokens = 0, int32 OutputTokens = 0);
	void AddToolCall(const FString& ToolName);

	// Current session
	FConversationSession& GetCurrentSession() { return CurrentSession; }
	void SetCurrentSession(const FConversationSession& Session) { CurrentSession = Session; }
	bool HasUnsavedChanges() const { return bUnsavedChanges; }

	// Auto-save
	void EnableAutoSave(bool bEnable, float IntervalSeconds = 30.0f);
	void AutoSaveTick(float DeltaTime);

	// Export
	bool ExportSessionToMarkdown(const FString& SessionId, const FString& OutputPath);
	bool ExportSessionToJson(const FString& SessionId, const FString& OutputPath);

private:
	FConversationMemory();
	
	FString GetSessionsDirectory() const;
	FString GetSessionFilePath(const FString& SessionId) const;
	FString GenerateSessionId() const;
	FString GenerateSessionTitle(const FString& FirstMessage) const;

	FConversationSession CurrentSession;
	bool bUnsavedChanges = false;
	bool bAutoSaveEnabled = true;
	float AutoSaveInterval = 30.0f;
	float TimeSinceLastSave = 0.0f;
};
