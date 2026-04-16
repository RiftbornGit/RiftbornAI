// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Event types from Python backend
 */
UENUM()
enum class EStreamEventType : uint8
{
    Plan,           // Planning/analyzing
    Diff,           // Code diff generated
    Compilation,    // Compilation status
    Rollback,       // Rollback triggered
    Analysis,       // Code analysis
    FixApplied,     // Fix applied
    Refactor,       // Refactoring operation
    Complete,       // Operation complete
    Error           // Error occurred
};

/**
 * Stream event from Python backend
 */
struct FStreamEvent
{
    EStreamEventType Type;
    FString Stage;
    FString Message;
    TMap<FString, FString> Data;
    
    FStreamEvent() : Type(EStreamEventType::Complete) {}
    
    static FStreamEvent FromJSON(const FString& JSON);
};

/**
 * Streaming event handler - Captures and processes events from Python backend
 */
class FStreamingEventHandler
{
public:
    FStreamingEventHandler();
    ~FStreamingEventHandler();
    
    /**
     * Process log line from Python backend
     * Looks for [STREAM_CHUNK], [STREAM_EVENT], [STREAM_COMPLETE] markers
     */
    void ProcessLogLine(const FString& LogLine);
    
    /**
     * Set delegates for different event types
     */
    DECLARE_DELEGATE_OneParam(FOnTextChunk, const FString& /*Chunk*/);
    DECLARE_DELEGATE_OneParam(FOnEvent, const FStreamEvent& /*Event*/);
    DECLARE_DELEGATE_TwoParams(FOnComplete, bool /*bSuccess*/, const FString& /*Message*/);
    
    FOnTextChunk OnTextChunk;
    FOnEvent OnEvent;
    FOnComplete OnComplete;
    
private:
    void HandleChunk(const FString& Chunk);
    void HandleEvent(const FString& EventJSON);
    void HandleComplete(const FString& ResultJSON);
};
