// Copyright RiftbornAI. All Rights Reserved.
// Undo/Rollback System - Track and revert AI tool actions

#pragma once

#include "CoreMinimal.h"

/**
 * Snapshot of state before an action
 */
struct RIFTBORNAI_API FActionSnapshot
{
	// What was changed
	FString TargetType;  // "Actor", "Blueprint", "File", etc.

	FString TargetPath;  // Actor label, asset path, file path

	FString SnapshotData;  // JSON serialized state before change

	// For actors: transform, properties
	FVector Location = FVector::ZeroVector;

	FRotator Rotation = FRotator::ZeroRotator;

	FVector Scale = FVector::OneVector;

	// For files: content
	FString FileContent;

	// For created objects: just need to delete on undo
	bool bWasCreated = false;
};

/**
 * A single undoable action
 */
struct RIFTBORNAI_API FUndoableAction
{
	FString ActionId;

	FString ToolName;

	FString Description;  // Human-readable description

	FDateTime Timestamp;

	FActionSnapshot BeforeSnapshot;

	TMap<FString, FString> ToolArguments;

	bool bCanUndo = true;

	FString FailureReason;  // If can't undo, why
};

/**
 * Action History Manager
 * Tracks all AI tool actions and enables undo/rollback
 */
class RIFTBORNAI_API FActionHistoryManager
{
public:
	static FActionHistoryManager& Get();

	// Record an action before execution
	FString BeginAction(const FString& ToolName, const TMap<FString, FString>& Args);

	// Capture state before action executes
	void CaptureBeforeState(const FString& ActionId, const FString& TargetType, const FString& TargetPath);

	// Mark action as completed (with optional result info)
	void CompleteAction(const FString& ActionId, bool bSuccess);

	// Undo operations
	bool CanUndo() const;
	bool UndoLastAction();
	bool UndoAction(const FString& ActionId);
	bool UndoLastN(int32 Count);

	// Get action history
	const TArray<FUndoableAction>& GetHistory() const { return ActionHistory; }
	TArray<FUndoableAction> GetRecentActions(int32 Count) const;

	// Check if specific action can be undone
	bool CanUndoAction(const FString& ActionId) const;
	FString GetUndoFailureReason(const FString& ActionId) const;

	// Clear history
	void ClearHistory();
	void TrimHistory(int32 MaxActions);

	// Settings
	// Hard ceiling — each action may carry file content / JSON snapshots,
	// so an uncapped history can balloon into hundreds of MB of heap.
	static constexpr int32 HardMaxHistorySize = 1000;
	void SetMaxHistorySize(int32 Max) { MaxHistorySize = FMath::Clamp(Max, 1, HardMaxHistorySize); }
	void SetEnabled(bool bEnable) { bEnabled = bEnable; }
	bool IsEnabled() const { return bEnabled; }

	// Danger level assessment
	enum class EDangerLevel : uint8
	{
		Safe,       // Read-only, no risk
		Low,        // Minor changes, easy to undo
		Medium,     // Moderate changes
		High,       // Significant changes, hard to undo
		Critical    // Irreversible or dangerous
	};
	
	static EDangerLevel GetToolDangerLevel(const FString& ToolName);
	static FString GetDangerWarning(const FString& ToolName);

private:
	FActionHistoryManager();

	// Undo implementations for different target types
	bool UndoActorAction(const FUndoableAction& Action);
	bool UndoBlueprintAction(const FUndoableAction& Action);
	bool UndoFileAction(const FUndoableAction& Action);
	bool UndoAssetAction(const FUndoableAction& Action);

	// Capture state helpers
	void CaptureActorState(FActionSnapshot& Snapshot, const FString& ActorLabel);
	void CaptureFileState(FActionSnapshot& Snapshot, const FString& FilePath);

	// Generate human-readable description
	FString GenerateDescription(const FString& ToolName, const TMap<FString, FString>& Args);

	TArray<FUndoableAction> ActionHistory;
	TMap<FString, int32> ActionIdToIndex;  // For fast lookup
	FString CurrentActionId;  // Action being recorded
	
	int32 MaxHistorySize = 50;
	bool bEnabled = true;
};

/**
 * Scoped action recorder
 * RAII helper for recording actions
 */
class RIFTBORNAI_API FScopedActionRecorder
{
public:
	FScopedActionRecorder(const FString& ToolName, const TMap<FString, FString>& Args);
	~FScopedActionRecorder();

	void SetSuccess(bool bSuccess);
	void CaptureTarget(const FString& TargetType, const FString& TargetPath);
	const FString& GetActionId() const { return ActionId; }

private:
	FString ActionId;
	bool bSuccess = false;
};

/**
 * Simplified Action History interface for UI usage
 * Wraps FActionHistoryManager with simpler API
 */
class RIFTBORNAI_API FActionHistory
{
public:
	FActionHistory() {}
	
	// Record an action (simplified)
	void RecordAction(const FString& ToolName, const TMap<FString, FString>& Args, const FString& Description)
	{
		FActionHistoryManager::Get().BeginAction(ToolName, Args);
		// Note: Simplified - doesn't capture full before state
	}
	
	// Check if undo is available
	bool CanUndo() const
	{
		return FActionHistoryManager::Get().CanUndo();
	}
	
	// Undo and return description
	bool UndoLastAction(FString& OutDescription)
	{
		TArray<FUndoableAction> Recent = FActionHistoryManager::Get().GetRecentActions(1);
		if (Recent.Num() > 0)
		{
			OutDescription = Recent[0].Description;
		}
		return FActionHistoryManager::Get().UndoLastAction();
	}
	
	// Get recent actions
	TArray<FUndoableAction> GetRecentActions(int32 Count) const
	{
		return FActionHistoryManager::Get().GetRecentActions(Count);
	}
};
