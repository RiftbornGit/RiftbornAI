// Copyright RiftbornAI. All Rights Reserved.
// Multi-Turn Memory - Persistent conversation context across turns

#pragma once

#include "CoreMinimal.h"

/**
 * Entity mentioned in conversation
 */
struct RIFTBORNAI_API FConversationEntity
{
	FString Name;              // "warrior", "the cube", "BP_Champion"
	FString ResolvedPath;      // /Game/Champions/BP_Warrior
	FString Type;              // Character, Actor, Blueprint
	int32 MentionCount = 0;
	int32 LastMentionTurn = 0;
	float Relevance = 1.0f;    // Decays over time
};

/**
 * Action taken in conversation
 */
struct RIFTBORNAI_API FConversationAction
{
	int32 Turn;
	FString ToolName;
	TMap<FString, FString> Arguments;
	bool bSuccess;
	FString Result;
	FString Description;       // Human-readable: "Spawned BP_Warrior at origin"
};

/**
 * Topic/intent being discussed
 */
struct RIFTBORNAI_API FConversationTopic
{
	FString Topic;             // "character setup", "level design", "combat system"
	int32 StartTurn;
	int32 LastTurn;
	TArray<FString> RelatedEntities;
	TArray<FString> RelatedTools;
};

/**
 * Complete conversation state
 */
struct RIFTBORNAI_API FMultiTurnConversationState
{
	int32 CurrentTurn = 0;
	TArray<FConversationEntity> Entities;
	TArray<FConversationAction> Actions;
	TArray<FConversationTopic> Topics;
	
	// Quick access
	FConversationEntity LastSubject;
	FConversationEntity LastObject;
	FString LastIntent;
	FString LastToolUsed;
	
	// Pending state
	bool bAwaitingConfirmation = false;
	FString PendingAction;
	
	// User preferences learned
	TMap<FString, FString> UserPreferences;  // "spawn_height" -> "100"
};

/**
 * Multi-Turn Conversation Memory
 * Maintains context across conversation turns for coherent multi-step interactions.
 */
class RIFTBORNAI_API FMultiTurnMemory
{
public:
	static FMultiTurnMemory& Get();
	
	// =========================================================================
	// TURN MANAGEMENT
	// =========================================================================
	
	/** Start a new turn */
	void BeginTurn(const FString& UserMessage);
	
	/** End current turn */
	void EndTurn(const FString& AssistantResponse);
	
	/** Get current turn number */
	int32 GetCurrentTurn() const { return State.CurrentTurn; }
	
	// =========================================================================
	// ENTITY TRACKING
	// =========================================================================
	
	/** Record entity mention */
	void RecordEntity(const FString& Name, const FString& ResolvedPath, const FString& Type);
	
	/** Get most relevant entity */
	const FConversationEntity* GetMostRelevantEntity(const FString& TypeHint = TEXT("")) const;
	
	/** Resolve pronoun to entity */
	const FConversationEntity* ResolvePronoun(const FString& Pronoun) const;
	
	/** Get entity by name (fuzzy) */
	const FConversationEntity* FindEntity(const FString& Name) const;
	
	// =========================================================================
	// ACTION TRACKING
	// =========================================================================
	
	/** Record action taken */
	void RecordAction(const FString& ToolName, const TMap<FString, FString>& Args, bool bSuccess, const FString& Result);
	
	/** Get last N actions */
	TArray<FConversationAction> GetRecentActions(int32 Count = 5) const;
	
	/** Get actions for entity */
	TArray<FConversationAction> GetActionsForEntity(const FString& EntityName) const;
	
	/** Can we undo last action? */
	bool CanUndo() const;
	
	/** Get undo description */
	FString GetUndoDescription() const;
	
	// =========================================================================
	// TOPIC TRACKING
	// =========================================================================
	
	/** Update current topic */
	void UpdateTopic(const FString& Topic);
	
	/** Get current topic */
	FString GetCurrentTopic() const;
	
	/** Is topic related to previous? */
	bool IsRelatedTopic(const FString& Topic) const;
	
	// =========================================================================
	// CONTEXT GENERATION
	// =========================================================================
	
	/** Generate context summary for AI */
	FString GenerateContextSummary() const;
	
	/** Generate short context for tool selection */
	FString GenerateShortContext() const;
	
	/** Get conversation history formatted for LLM */
	FString GetFormattedHistory(int32 MaxTurns = 5) const;
	
	// =========================================================================
	// STATE MANAGEMENT
	// =========================================================================
	
	/** Clear conversation state */
	void Clear();
	
	/** Save state to disk */
	void SaveState(const FString& SlotName);
	
	/** Load state from disk */
	bool LoadState(const FString& SlotName);
	
	/** Get full state */
	const FMultiTurnConversationState& GetState() const { return State; }

private:
	FMultiTurnMemory() = default;
	void DecayRelevance();
	
	FMultiTurnConversationState State;
	TArray<TPair<FString, FString>> MessageHistory;  // role, content
};
