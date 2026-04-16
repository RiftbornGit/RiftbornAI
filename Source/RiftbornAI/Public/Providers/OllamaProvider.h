// Copyright RiftbornAI. All Rights Reserved.
// Ollama (Local LLM) AI Provider

#pragma once

#include "CoreMinimal.h"
#include "IAIProvider.h"

/**
 * Ollama AI Provider Implementation
 * Connects to a local Ollama server for free, private AI inference.
 * 
 * Benefits:
 * - Zero API cost
 * - Complete privacy (no data leaves your machine)
 * - Works offline
 * - Supports many models (Qwen, Llama, Mistral, etc.)
 */
class RIFTBORNAI_API FOllamaProvider : public IAIProvider, public TSharedFromThis<FOllamaProvider>
{
public:
    FOllamaProvider();
    virtual ~FOllamaProvider() override;

    // ============================================================================
    // IAIProvider - Configuration
    // ============================================================================
    
    virtual void SetAPIKey(const FString& InAPIKey) override;  // No-op for Ollama
    virtual void SetSystemPrompt(const FString& Prompt) override;
    virtual void SetModel(const FString& InModel) override { Model = InModel; }
    virtual FString GetModel() const override { return Model; }
    virtual FString GetProviderName() const override { return TEXT("Ollama"); }
    virtual bool IsConfigured() const override;

    /** Set the Ollama server endpoint (default: http://localhost:11434) */
    void SetEndpoint(const FString& InEndpoint) { Endpoint = InEndpoint; }
    FString GetEndpoint() const { return Endpoint; }

    // ============================================================================
    // IAIProvider - Simple Messaging
    // ============================================================================
    
    virtual void SendMessage(
        const FString& Message,
        TFunction<void(bool bSuccess, const FString& Response)> OnComplete
    ) override;

    // ============================================================================
    // IAIProvider - Agentic Tool Use
    // ============================================================================
    
    virtual void SendMessageWithTools(
        const FString& Message,
        const FString& RequestId,
        TFunction<void(bool bSuccess, const FString& Response, const FString& RequestId)> OnComplete,
        TFunction<void(const FString& Status)> OnProgress = nullptr
    ) override;
    
    virtual void SendMessageStreaming(
        const FString& Message,
        const TArray<TSharedPtr<FJsonValue>>& Tools,
        TFunction<void(const FString& Token)> OnToken,
        TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
        TFunction<void(bool bSuccess, const FString& Error)> OnComplete
    ) override;
    
    virtual void ContinueWithToolResult(
        const FString& ToolUseId,
        const FString& Result,
        const TArray<TSharedPtr<FJsonValue>>& Tools,
        TFunction<void(const FString& Token)> OnToken,
        TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
        TFunction<void(bool bSuccess, const FString& Error)> OnComplete
    ) override;
    
    virtual void ContinueWithMultipleToolResults(
        const TArray<FToolResultEntry>& ToolResults,
        const TArray<TSharedPtr<FJsonValue>>& Tools,
        TFunction<void(const FString& Token)> OnToken,
        TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
        TFunction<void(bool bSuccess, const FString& Error)> OnComplete
    ) override;

    // ============================================================================
    // IAIProvider - State Management
    // ============================================================================
    
    virtual void ClearHistory() override;
    virtual void CancelRequest() override;
    virtual bool IsCancelled() const override { return bRequestCancelled; }

    // ============================================================================
    // IAIProvider - Token Tracking
    // ============================================================================
    
    virtual int64 GetSessionInputTokens() const override { return SessionInputTokens; }
    virtual int64 GetSessionOutputTokens() const override { return SessionOutputTokens; }
    virtual float EstimateSessionCost() const override { return 0.0f; }  // Always free!
    virtual void ResetSessionTokens() override;

    // ============================================================================
    // IAIProvider - Tool Call Tracking
    // ============================================================================
    
    virtual const TArray<FClaudeToolCall>& GetLastToolCalls() const override { return LastToolCalls; }
    virtual void ClearLastToolCalls() override { LastToolCalls.Empty(); }

    // ============================================================================
    // IAIProvider - Tool Result Tracking (Request-Scoped)
    // ============================================================================
    
    virtual TArray<FClaudeToolResult> GetToolResultsForRequest(const FString& RequestId) const override;
    virtual void ClearToolResultsForRequest(const FString& RequestId) override;
    
    // Deprecated legacy API (returns empty, use request-scoped instead)
    virtual const TArray<FClaudeToolResult>& GetLastToolResults() const override { return EmptyResults; }
    virtual void ClearLastToolResults() override {}

    // ============================================================================
    // Ollama-Specific Methods
    // ============================================================================
    
    /** Check if the Ollama server is running */
    void CheckServerStatus(TFunction<void(bool bRunning)> OnResult);
    
    /** List available models on the server */
    void ListModels(TFunction<void(const TArray<FString>& Models)> OnResult);
    
    /** Pull a model from Ollama registry */
    void PullModel(const FString& ModelName, TFunction<void(bool bSuccess, const FString& Error)> OnComplete);

private:
    FString Endpoint;
    FString Model;
    FString ExternalSystemPrompt;  // Set via SetSystemPrompt(), injected into payloads
    
    TArray<TSharedPtr<FJsonObject>> ConversationHistory;
    TArray<FClaudeToolCall> LastToolCalls;  // Tool calls from last SendMessageWithTools
    
    // Request-scoped tool results (keyed by RequestId)
    mutable FCriticalSection ToolResultsLock;
    TMap<FString, TArray<FClaudeToolResult>> ToolResultsByRequest;
    TMap<FString, double> ToolResultsTimestamps;  // TTL tracking: RequestId -> CreationTime
    static inline TArray<FClaudeToolResult> EmptyResults;  // For deprecated API
    static constexpr double ToolResultsTTLSeconds = 300.0;  // 5 minute TTL for orphaned results
    
    // Current request being processed (for result storage)
    FString CurrentRequestId;
    
    /** Purge tool results older than TTL (call periodically) */
    void PurgeExpiredToolResults();
    
    bool bRequestCancelled = false;
    TSharedPtr<class IHttpRequest> CurrentRequest;
    
    int64 SessionInputTokens = 0;
    int64 SessionOutputTokens = 0;
    
    /** Build the message payload for Ollama API (tool-calling mode) */
    TSharedPtr<FJsonObject> BuildRequestPayload(const FString& UserMessage, bool bStream = false);
    
    /** Build the message payload for CHAT mode (no tools, simpler prompt) */
    TSharedPtr<FJsonObject> BuildChatPayload(const FString& UserMessage);

    /**
     * Prune conversation history to prevent unbounded memory growth.
     * Keeps the first user message (original intent) plus the last N messages.
     * @param KeepLastN Number of recent messages to retain (default: 10)
     */
    void PruneHistory(int32 KeepLastN = 10);

    /** Parse streaming response chunks */
    void ParseStreamChunk(const FString& Chunk, FString& OutContent, bool& bOutDone);
    
    /** Estimate token count (rough approximation) */
    int32 EstimateTokens(const FString& Text) const;
    
    /** Parse tool calls from text output (for models that output tool calls as text) */
    TArray<struct FClaudeToolCall> ParseToolCallsFromText(const FString& Text);
    
    // ============================================================================
    // Multi-Step Tool Chaining (Agentic Loop)
    // ============================================================================
    
    /** Maximum number of tool iterations before forcing completion */
    static constexpr int32 MaxToolIterations = 10;
    
    /** Current iteration count (reset on each SendMessageWithTools call) */
    int32 CurrentToolIteration = 0;
    
    /** Accumulated tool results for current session */
    FString AccumulatedToolResults;
    
    /** Execute a single iteration of the tool loop */
    void ExecuteToolLoopIteration(
        const FString& UserMessage,
        const FString& RequestId,
        TSharedPtr<TFunction<void(bool, const FString&, const FString&)>> OnComplete,
        TSharedPtr<TFunction<void(const FString&)>> OnProgress
    );
    
    /** Process tool calls from Ollama response and continue the loop */
    void ProcessToolCallsAndContinue(
        const TArray<TSharedPtr<FJsonValue>>& ToolCalls,
        const FString& UserMessage,
        const FString& RequestId,
        TSharedPtr<TFunction<void(bool, const FString&, const FString&)>> OnComplete,
        TSharedPtr<TFunction<void(const FString&)>> OnProgress
    );
    
    /** Store a tool result for the current request (thread-safe) */
    void StoreToolResult(const FString& RequestId, const FClaudeToolResult& Result);
};
