// TodoListSubsystem.h
// Persistent per-session todo list — the LLM writes it via the
// `update_todo_list` tool, the user sees it in the copilot panel, and it
// survives editor restart. Provider-agnostic.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

UENUM(BlueprintType)
enum class ERiftbornTodoStatus : uint8
{
	Pending    UMETA(DisplayName = "Pending"),
	InProgress UMETA(DisplayName = "In progress"),
	Completed  UMETA(DisplayName = "Completed"),
	Blocked    UMETA(DisplayName = "Blocked"),
	Cancelled  UMETA(DisplayName = "Cancelled"),
};

struct RIFTBORNAI_API FRiftbornTodoItem
{
	FString             Id;
	FString             Content;
	FString             ActiveForm;
	ERiftbornTodoStatus Status = ERiftbornTodoStatus::Pending;
	FDateTime           UpdatedAt;
};

/**
 * Process-wide todo list. Persists to
 *   Saved/RiftbornAI/todo_list.json
 * Thread-safe; change delegate fires on the game thread via FTSTicker.
 */
class RIFTBORNAI_API FRiftbornTodoList
{
public:
	static FRiftbornTodoList& Get();

	/** Replace the entire list atomically (matches Claude Code's TodoWrite semantics). */
	void ReplaceAll(const TArray<FRiftbornTodoItem>& NewItems);

	/** Snapshot current items. */
	TArray<FRiftbornTodoItem> GetItems() const;

	/** Clear all items. */
	void Clear();

	/** Parse todos from a JSON array produced by the LLM. Strict: returns
	 *  false with a populated error if the shape is wrong. */
	static bool ParseFromJson(const FString& JsonArrayText, TArray<FRiftbornTodoItem>& OutItems, FString& OutError);

	/** Serialize to JSON array (matches parse shape). */
	static FString SerializeToJson(const TArray<FRiftbornTodoItem>& Items);

	/** Status names accepted by ParseFromJson. */
	static ERiftbornTodoStatus ParseStatus(const FString& Raw);
	static FString             StatusToString(ERiftbornTodoStatus Status);

	/** Called every time the list changes. UI widgets bind to this to
	 *  refresh without polling. Fires on the game thread. */
	DECLARE_MULTICAST_DELEGATE(FOnTodoListChanged);
	FOnTodoListChanged OnTodoListChanged;

	/** Force persist to disk (normally auto-saved on change). */
	void SaveToDisk();

	/** Load persisted state (called automatically on first Get()). */
	void LoadFromDisk();

private:
	FRiftbornTodoList();

	FString PersistencePath() const;

	mutable FCriticalSection      Lock;
	TArray<FRiftbornTodoItem>     Items;
	bool                          bLoaded = false;
};
