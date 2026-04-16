// Copyright RiftbornAI. All Rights Reserved.
// OpenAI AI Provider

#pragma once

#include "CoreMinimal.h"
#include "IAIProvider.h"
#include "Interfaces/IHttpRequest.h"

/**
 * OpenAI AI Provider Implementation
 * Connects to OpenAI API for GPT model inference.
 * 
 * Supports:
 * - GPT-4, GPT-4-Turbo, GPT-4o
 * - Tool/function calling
 * - Streaming responses
 */
class RIFTBORNAI_API FOpenAIProvider : public IAIProvider, public TSharedFromThis<FOpenAIProvider>
{
public:
    FOpenAIProvider();
    virtual ~FOpenAIProvider() override;

    // ============================================================================
    // IAIProvider - Configuration
    // ============================================================================
    
    virtual void SetAPIKey(const FString& InAPIKey) override { APIKey = InAPIKey; }
    virtual void SetModel(const FString& InModel) override { Model = InModel; }
    virtual FString GetModel() const override { return Model; }
    virtual FString GetProviderName() const override { return TEXT("OpenAI"); }
    virtual bool IsConfigured() const override { return !APIKey.IsEmpty(); }

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
    // IAIProvider - Multimodal (Vision)
    // ============================================================================
    
    virtual void SendMessageWithImage(
        const FString& Message,
        const FString& ImagePath,
        TFunction<void(const FString& Token)> OnToken,
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
    virtual float EstimateSessionCost() const override;
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
    
    // Deprecated legacy API
    virtual const TArray<FClaudeToolResult>& GetLastToolResults() const override { return EmptyResults; }
    virtual void ClearLastToolResults() override {}

    // ============================================================================
    // OpenAI-Specific Methods
    // ============================================================================
    
    void SetEndpoint(const FString& InEndpoint) { Endpoint = InEndpoint; }
    FString GetEndpoint() const { return Endpoint; }
    
    virtual void SetSystemPrompt(const FString& Prompt) override;
    FString GetSystemPrompt() const { return SystemPrompt; }
    
    void SetMaxTokens(int32 Tokens) { MaxTokens = Tokens; }
    void SetTemperature(float Temp) { Temperature = Temp; }

private:
    // Build request payload
    TSharedPtr<FJsonObject> BuildRequestPayload(const FString& Message, const TArray<TSharedPtr<FJsonValue>>* Tools = nullptr);

    // Parse response
    bool ParseResponse(const FString& ResponseBody, FString& OutContent, TArray<FClaudeToolCall>& OutToolCalls);

    /**
     * Prune conversation history to prevent unbounded memory growth.
     * Keeps the first user message (original intent) plus the last N messages.
     * @param KeepLastN Number of recent messages to retain (default: 10)
     */
    void PruneHistory(int32 KeepLastN = 10);

    // Configuration
    FString APIKey;
    FString Endpoint;
    FString Model;
    FString SystemPrompt;
    int32 MaxTokens;
    float Temperature;
    
    // Request state
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
    bool bRequestCancelled = false;
    
    // Conversation history
    TArray<TSharedPtr<FJsonObject>> ConversationHistory;
    
    // Tool call tracking
    TArray<FClaudeToolCall> LastToolCalls;
    
    // Map ToolUseId ("call_xxx") -> ToolName for CompactToolResult resolution
    TMap<FString, FString> ToolUseIdToName;
    
    // Request-scoped tool results
    mutable FCriticalSection ToolResultsLock;
    TMap<FString, TArray<FClaudeToolResult>> ToolResultsByRequest;
    static inline TArray<FClaudeToolResult> EmptyResults;
    FString CurrentRequestId;
    
    // Token tracking
    int64 SessionInputTokens = 0;
    int64 SessionOutputTokens = 0;
};
