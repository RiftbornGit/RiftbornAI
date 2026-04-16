// Copyright RiftbornAI. All Rights Reserved.
// AgentEventStream.h - Append-only event log for agent task execution

#pragma once

#include "CoreMinimal.h"
#include "Agent/AgentEvent.h"
#include "HAL/CriticalSection.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAgentEvent, const FAgentEvent&);

/**
 * FAgentEventStream - Thread-safe append-only event log
 * 
 * Design principles:
 * - Events are IMMUTABLE after appending
 * - Supports streaming (UI subscribes to new events)
 * - Persists to JSONL file for replay
 * - One stream per task
 */
class RIFTBORNAI_API FAgentEventStream : public TSharedFromThis<FAgentEventStream>
{
public:
	FAgentEventStream(const FGuid& InTaskId, const FString& InLogPath);
	~FAgentEventStream();
	
	// === Core Operations ===
	
	/** Append a new event (thread-safe) */
	void Append(const FAgentEvent& Event);
	
	/** Create and append an event in one call */
	FAgentEvent& EmitEvent(EAgentEventType Type, const FString& Summary);
	
	/** Get all events (thread-safe copy) */
	TArray<FAgentEvent> GetAllEvents() const;
	
	/** Get events since a specific index */
	TArray<FAgentEvent> GetEventsSince(int32 StartIndex) const;
	
	/** Get event count */
	int32 GetEventCount() const;
	
	/** Get most recent event of a type */
	TOptional<FAgentEvent> GetLastEventOfType(EAgentEventType Type) const;
	
	// === Task Context ===
	
	/** Set current step context (affects subsequent events) */
	void SetCurrentStep(const FGuid& StepId, int32 Iteration);
	
	/** Clear step context */
	void ClearCurrentStep();
	
	// === Streaming ===
	
	/** Subscribe to new events */
	FDelegateHandle Subscribe(const FOnAgentEvent::FDelegate& Delegate);
	
	/** Unsubscribe */
	void Unsubscribe(FDelegateHandle Handle);
	
	// === Persistence ===
	
	/** Flush to disk */
	void Flush();
	
	/** Load from existing file */
	static TSharedPtr<FAgentEventStream> LoadFromFile(const FString& LogPath);

	// === Broadcaster Integration ===
	
	/** Bind to global agent event broadcaster (canonical bus) */
	void BindToBroadcaster(bool bAllowUnscopedEvents = true);
	
	/** Unbind from broadcaster */
	void UnbindFromBroadcaster();
	
	// === Accessors ===
	
	FGuid GetTaskId() const { return TaskId; }
	FString GetLogPath() const { return LogPath; }
	
private:
	FGuid TaskId;
	FString LogPath;
	
	// Thread safety
	mutable FCriticalSection EventLock;
	
	// In-memory log (also written to file)
	TArray<FAgentEvent> Events;
	
	// Current context (set by runner)
	FGuid CurrentStepId;
	int32 CurrentIteration = 0;
	
	// Streaming
	FOnAgentEvent OnEventAppended;
	FDelegateHandle BroadcasterHandle;
	bool bAcceptUnscopedEvents = true;
	
	// Suppress duplicate broadcast echoes
	TSet<FGuid> SuppressedBroadcastIds;
	mutable FCriticalSection SuppressLock;
	
	// File handle for incremental writes
	TUniquePtr<IFileHandle> FileHandle;
	bool bFileOpen = false;
	
	/** Open file for appending */
	bool OpenFile();
	
	/** Write event to file */
	void WriteEventToFile(const FAgentEvent& Event);
	
	/** Append event with optional broadcast suppression */
	void AppendInternal(const FAgentEvent& Event);
	
	/** Receive events from broadcaster */
	void HandleBroadcastEvent(const FAgentEvent& Event);
};

/**
 * Convenience macros for emitting events
 */
#define AGENT_EVENT(Stream, Type, Summary) \
	if (Stream.IsValid()) { Stream->EmitEvent(EAgentEventType::Type, Summary); }

#define AGENT_EVENT_EX(Stream, Type, Summary, Lambda) \
	if (Stream.IsValid()) { \
		FAgentEvent& __evt = Stream->EmitEvent(EAgentEventType::Type, Summary); \
		Lambda(__evt); \
	}
