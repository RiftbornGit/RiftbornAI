// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "ClaudeToolUse.h"

/**
 * Claude API Response type
 */
enum class EClaudeResponseType : uint8
{
	Text,           // Normal text response
	ToolUse,        // Claude wants to use tools
	Error           // API error
};

/**
 * Claude API Response (parsed)
 */
struct FClaudeResponse
{
	EClaudeResponseType Type = EClaudeResponseType::Text;
	FString TextContent;
	TArray<FClaudeToolCall> ToolCalls;
	FString ErrorMessage;
	FString RawJson;
};

/**
 * Claude API Client with Tool Use Support
 * Handles communication with Anthropic's Claude API including function calling
 */
class RIFTBORNAI_API FClaudeAPIClient
{
public:
	FClaudeAPIClient();
	~FClaudeAPIClient();

	// Type alias for tool arguments (needed to avoid macro comma issues)
	using FToolArgsMap = TMap<FString, FString>;

	// Callback types for streaming API
	DECLARE_DELEGATE_OneParam(FOnToken, const FString& /*Token*/);
	DECLARE_DELEGATE_ThreeParams(FOnToolCall, const FString& /*ToolName*/, const FToolArgsMap& /*Args*/, const FString& /*ToolUseId*/);
	DECLARE_DELEGATE_TwoParams(FOnComplete, bool /*bSuccess*/, const FString& /*Error*/);

	/**
	 * Send a message to Claude and get a response (basic mode)
	 * @param UserMessage - The message/request from the user
	 * @param OnComplete - Callback when response is received
	 * @param OnStreamChunk - Optional callback for streaming chunks
	 */
	void SendMessage(
		const FString& UserMessage,
		TFunction<void(bool bSuccess, const FString& Response)> OnComplete,
		TFunction<void(const FString& Chunk)> OnStreamChunk = nullptr
	);

	/**
	 * Send a message with tool use support - AGENTIC MODE (simple callback version)
	 * Claude can call tools, receive results, and continue conversation
	 * @param UserMessage - The message/request from the user
	 * @param OnComplete - Callback when final response is ready
	 * @param OnProgress - Optional callback for progress updates
	 */
	void SendMessageWithTools(
		const FString& UserMessage,
		TFunction<void(bool bSuccess, const FString& Response)> OnComplete,
		TFunction<void(const FString& Status)> OnProgress = nullptr
	);
	
	/**
	 * Send message with streaming and tool callbacks - FULL STREAMING AGENTIC MODE
	 * Provides fine-grained callbacks for tokens, tool calls, and completion
	 */
	void SendMessageWithTools(
		const FString& UserMessage,
		const TArray<TSharedPtr<FJsonValue>>& Tools,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete
	);
	
	/**
	 * Continue conversation after tool result (DEPRECATED - use ContinueWithMultipleToolResults)
	 */
	void ContinueWithToolResult(
		const FString& ToolUseId,
		const FString& Result,
		const TArray<TSharedPtr<FJsonValue>>& Tools,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete
	);

	/**
	 * Info about a tool call that was executed, used to build proper Claude API messages.
	 * Contains everything needed for both the assistant tool_use block and user tool_result block.
	 */
	struct FCompletedToolCall
	{
		FString ToolUseId;
		FString ToolName;
		TMap<FString, FString> Arguments;
		FString ResultString;
	};

	/**
	 * Continue conversation after ALL tool results from a multi-tool response.
	 * Claude requires every tool_use in an assistant message to have a matching tool_result.
	 * This method builds proper JSON: assistant message with tool_use blocks,
	 * followed by user message with tool_result blocks.
	 */
	void ContinueWithMultipleToolResults(
		const TArray<FCompletedToolCall>& CompletedCalls,
		const TArray<TSharedPtr<FJsonValue>>& Tools,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete
	);

	/**
	 * Set the API key for Claude
	 */
	void SetAPIKey(const FString& InAPIKey);

	/**
	 * Get current conversation history
	 */
	const TArray<TPair<FString, FString>>& GetHistory() const { return ConversationHistory; }

	/**
	 * Clear conversation history
	 */
	void ClearHistory();

	/**
	 * Enable/disable tool use for agentic mode
	 */
	void SetToolsEnabled(bool bEnabled) { bToolsEnabled = bEnabled; }
	bool AreToolsEnabled() const { return bToolsEnabled; }

	// ===== Token Tracking =====
	
	/** Get input tokens used in this session */
	int64 GetSessionInputTokens() const { return SessionInputTokens; }
	
	/** Get output tokens used in this session */
	int64 GetSessionOutputTokens() const { return SessionOutputTokens; }
	
	/** Get total tokens (input + output) for this session */
	int64 GetSessionTotalTokens() const { return SessionInputTokens + SessionOutputTokens; }
	
	/** Get last request's input tokens */
	int32 GetLastInputTokens() const { return LastInputTokens; }
	
	/** Get last request's output tokens */
	int32 GetLastOutputTokens() const { return LastOutputTokens; }
	
	/** Reset session token counters */
	void ResetSessionTokens();
	
	/** Set maximum input tokens per session (0 = unlimited) */
	void SetMaxSessionTokens(int64 InMaxTokens) { MaxSessionInputTokens = InMaxTokens; }
	
	/** Get maximum session tokens */
	int64 GetMaxSessionTokens() const { return MaxSessionInputTokens; }
	
	/** Check if session token limit would be exceeded by estimated next request */
	bool WouldExceedTokenLimit(int32 EstimatedInputTokens = 4000) const;
	
	/** Estimate cost in USD for session tokens (Claude Opus pricing) */
	float EstimateSessionCost() const;
	
	/** Cancel any in-flight request */
	void CancelCurrentRequest();
	
	/** Check if request was cancelled */
	bool IsCancelled() const { return bCancelled; }
	
	/** Set model to use for next request */
	void SetModel(const FString& InModel) { Model = InModel; }
	
	/** Get current model */
	const FString& GetModel() const { return Model; }
	
	/** Model constants */
	static const FString ModelOpus;   // claude-opus-4-6
	static const FString ModelHaiku;  // claude-haiku-4-5-20251001
	static const FString ModelOllama; // qwen3:14b (local)
	
	/** Set to use Ollama backend instead of Claude API */
	void SetUseOllama(bool bUseOllama, const FString& OllamaEndpoint = TEXT("http://localhost:11434"), const FString& OllamaModelName = TEXT("qwen3:14b"));
	bool IsUsingOllama() const { return bUseOllamaBackend; }
	
	/**
	 * Query classification result
	 */
	enum class EQueryType : uint8
	{
		CodeGeneration,    // Needs Opus - create code, blueprints, complex logic
		ToolExecution,     // Needs Opus - execute tools, modify UE
		SimpleChat,        // Haiku OK - questions, explanations, help
		Classification     // Internal - used for Haiku classification call
	};
	
	/**
	 * Classify a query to determine which model to use
	 * Returns quickly via local heuristics, or uses Haiku for ambiguous cases
	 */
	EQueryType ClassifyQuery(const FString& Query) const;
	
	/**
	 * Get recommended model for a query type
	 */
	static FString GetModelForQueryType(EQueryType Type);

private:
	void OnResponseReceived(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bWasSuccessful,
		TFunction<void(bool, const FString&)>&& OnComplete
	);

	void OnToolResponseReceived(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bWasSuccessful,
		TFunction<void(bool, const FString&)> OnComplete,
		TFunction<void(const FString&)> OnProgress,
		int32 ToolIterations
	);

	FString BuildRequestBody(const FString& UserMessage, bool bIncludeTools = false) const;
	FString BuildToolResultMessage(const TArray<FClaudeToolResult>& Results) const;
	FString BuildSystemPromptWithContext() const;
	FClaudeResponse ParseResponse(const FString& JSONResponse) const;
	FString ExtractResponse(const FString& JSONResponse) const;

	FString APIKey;
	FString APIEndpoint;
	FString Model;
	bool bStreamingEnabled;
	bool bToolsEnabled;
	
	// Ollama backend support
	bool bUseOllamaBackend = false;
	FString OllamaEndpointUrl;
	FString OllamaModelName;
	
	// Max tool iterations to prevent infinite loops
	static constexpr int32 MaxToolIterations = 10;
	
	// Conversation history (User, Assistant pairs) - legacy flat format
	TArray<TPair<FString, FString>> ConversationHistory;
	
	// JSON conversation history - proper structured messages for Claude API
	// This tracks tool_use/tool_result content blocks correctly
	TArray<TSharedPtr<FJsonObject>> JsonConversationHistory;
	
	// HTTP module
	FHttpModule* HttpModule;
	
	// Current in-flight request (for cancellation)
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
	bool bCancelled = false;
	
	// Streaming state
	FString CurrentStreamBuffer;
	TFunction<void(const FString&)> OnStreamChunkCallback;
	
	// Token tracking
	int64 SessionInputTokens = 0;
	int64 SessionOutputTokens = 0;
	int32 LastInputTokens = 0;
	int32 LastOutputTokens = 0;
	int64 MaxSessionInputTokens = 100000; // Default 100k input token limit (~$1.50)
	
	/** Parse and track tokens from API response JSON */
	void ParseAndTrackTokens(const TSharedPtr<FJsonObject>& JsonObject);
};
