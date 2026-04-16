// Copyright RiftbornAI. All Rights Reserved.
// Claude AI Provider Implementation

#pragma once

#include "CoreMinimal.h"
#include "IAIProvider.h"

/**
 * Claude (Anthropic) AI Provider Implementation
 * Implements IAIProvider for Claude API integration.
 */
class RIFTBORNAI_API FClaudeProvider : public IAIProvider, public TSharedFromThis<FClaudeProvider>
{
public:
    FClaudeProvider();
    virtual ~FClaudeProvider() override;

    // ============================================================================
    // IAIProvider - Configuration
    // ============================================================================
    
    virtual void SetAPIKey(const FString& InAPIKey) override { APIKey = InAPIKey; }
    virtual void SetModel(const FString& InModel) override { Model = InModel; }
    virtual FString GetModel() const override { return Model; }
    virtual FString GetProviderName() const override { return TEXT("Claude"); }
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
    // IAIProvider - State Management
    // ============================================================================
    
    virtual void ClearHistory() override;
    virtual void CancelRequest() override;
    virtual bool IsCancelled() const override { return bCancelled; }

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
    // IAIProvider - Token Tracking
    // ============================================================================
    
    virtual int64 GetSessionInputTokens() const override { return TotalInputTokens; }
    virtual int64 GetSessionOutputTokens() const override { return TotalOutputTokens; }
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
    // Claude-specific Methods
    // ============================================================================
    
    virtual void SetSystemPrompt(const FString& Prompt) override;
    FString GetSystemPrompt() const { return SystemPrompt; }
    
    void SetEndpoint(const FString& Endpoint) { APIEndpoint = Endpoint; }
    void SetMaxTokens(int32 Tokens) { MaxTokens = Tokens; }
    
    // ============================================================================
    // History Management (Simple, Effective)
    // ============================================================================
    
    /** Compact a tool result to a one-line state delta (the RIGHT way to save tokens) */
    static FString CompactToolResult(const FString& ToolName, const FString& RawResult);
    
    /** Keep only last N tool exchanges in detail, collapse older ones */
    void PruneHistorySimple(int32 KeepLastN = 5);
    
    /** Get count of tool result messages in history */
    int32 CountToolResults() const;
    
    void SetTemperature(float Temp) { Temperature = Temp; }

private:
    // Configuration
    FString APIKey;
    FString APIEndpoint;
    FString Model;
    FString SystemPrompt;
    int32 MaxTokens;
    float Temperature;
    float RequestTimeout;
    
    // State
    bool bRequestInProgress;
    bool bCancelled;
    int64 TotalInputTokens;
    int64 TotalOutputTokens;

    // Prompt-cache telemetry (populated from Anthropic API's usage.*).
    // cache_creation: tokens that established a new cache entry (~1.25x cost).
    // cache_read:     tokens served from cache on this turn (~0.10x cost).
    int64 TotalCacheCreationTokens = 0;
    int64 TotalCacheReadTokens     = 0;

    // Conversation history for context
    TArray<TSharedPtr<FJsonObject>> ConversationHistory;
    
    // Tool call tracking
    TArray<FClaudeToolCall> LastToolCalls;
    
    // Request-scoped tool results
    mutable FCriticalSection ToolResultsLock;
    TMap<FString, TArray<FClaudeToolResult>> ToolResultsByRequest;
    static inline TArray<FClaudeToolResult> EmptyResults;
    FString CurrentRequestId;
    
    // HTTP request handle for cancellation
    TSharedPtr<class IHttpRequest> CurrentRequest;
    
    // Helper methods
    TSharedPtr<class FJsonObject> BuildRequestPayload(
        const FString& Message,
        const TArray<TSharedPtr<FJsonValue>>* Tools = nullptr
    );
    
    void ProcessResponse(
        TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
        bool bWasSuccessful,
        TFunction<void(const FString& Token)> OnToken,
        TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
        TFunction<void(bool bSuccess, const FString& Error)> OnComplete
    );
    
    TArray<TSharedPtr<FJsonValue>> ConvertToolsToClaudeFormat(const TArray<FClaudeTool>& Tools);
    
    // Model-aware cost calculation
    // Returns cost per 1K tokens based on the configured Model string
    float GetInputTokenCostPer1K() const;
    float GetOutputTokenCostPer1K() const;
};
