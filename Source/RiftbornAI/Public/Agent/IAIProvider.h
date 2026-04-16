// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Response from an AI provider
 */
struct FAIProviderResponse
{
	bool bSuccess = false;
	FString Content;
	FString ErrorMessage;
	TArray<FClaudeToolCall> ToolCalls;
	int32 InputTokens = 0;
	int32 OutputTokens = 0;
};

/** Concrete model/provider choice after Auto routing or fallback resolution. */
struct FResolvedProviderSelection
{
	bool bValid = false;
	bool bUsedRouter = false;
	FString CanonicalModelId;
	FString ProviderName;
	FString ProviderModelId;
	FString TierName;
	FString Reason;
};

// Typedef to avoid comma in macro expansion
typedef TMap<FString, FString> FToolArgumentsMap;

/**
 * Callback types for AI provider operations
 */
DECLARE_DELEGATE_TwoParams(FOnAIComplete, bool /*bSuccess*/, const FString& /*Response*/);
DECLARE_DELEGATE_OneParam(FOnAIProgress, const FString& /*Status*/);
DECLARE_DELEGATE_OneParam(FOnAIToken, const FString& /*Token*/);
DECLARE_DELEGATE_ThreeParams(FOnAIToolCall, const FString& /*ToolName*/, const FToolArgumentsMap& /*Args*/, const FString& /*ToolUseId*/);

/**
 * Abstract interface for AI providers (Claude, OpenAI, Ollama, etc.)
 * 
 * Implement this interface to add new AI backends without modifying existing code.
 * The interface supports both simple message passing and agentic tool use patterns.
 */
class RIFTBORNAI_API IAIProvider
{
public:
	virtual ~IAIProvider() = default;

	// ============================================================================
	// Configuration
	// ============================================================================
	
	/** Set API credentials */
	virtual void SetAPIKey(const FString& APIKey) = 0;
	
	/** Set the model to use */
	virtual void SetModel(const FString& Model) = 0;
	
	/** Get the current model */
	virtual FString GetModel() const = 0;
	
	/** Get the provider name (e.g., "Anthropic", "OpenAI", "Ollama") */
	virtual FString GetProviderName() const = 0;
	
	/** Check if the provider is properly configured and ready */
	virtual bool IsConfigured() const = 0;

	/**
	 * Set the system prompt / instruction for the provider.
	 * This is sent in the API's native "system" field (not as a user message).
	 * Providers that don't support system prompts can implement this as a no-op.
	 */
	virtual void SetSystemPrompt(const FString& Prompt) = 0;

	// ============================================================================
	// Simple Messaging (Non-Agentic)
	// ============================================================================
	
	/**
	 * Send a simple message and get a response
	 * @param Message - The user message
	 * @param OnComplete - Callback with success status and response/error
	 */
	virtual void SendMessage(
		const FString& Message,
		TFunction<void(bool bSuccess, const FString& Response)> OnComplete
	) = 0;

	// ============================================================================
	// Agentic Tool Use
	// ============================================================================
	
	/**
	 * Send a message with tool use support (agentic mode)
	 * The provider will execute tools and continue conversation automatically
	 * @param Message - The user message/request
	 * @param RequestId - Unique ID for this request (for correlating tool results)
	 * @param OnComplete - Called when the final response is ready (includes RequestId)
	 * @param OnProgress - Optional progress updates (e.g., "Executing tool X...")
	 */
	virtual void SendMessageWithTools(
		const FString& Message,
		const FString& RequestId,
		TFunction<void(bool bSuccess, const FString& Response, const FString& RequestId)> OnComplete,
		TFunction<void(const FString& Status)> OnProgress = nullptr
	) = 0;
	
	/**
	 * Send a message with fine-grained streaming callbacks
	 * @param Message - The user message
	 * @param Tools - JSON array of tool definitions
	 * @param OnToken - Called for each streaming token
	 * @param OnToolCall - Called when the AI wants to use a tool
	 * @param OnComplete - Called when the response is complete
	 */
	virtual void SendMessageStreaming(
		const FString& Message,
		const TArray<TSharedPtr<FJsonValue>>& Tools,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete
	) = 0;
	
	/**
	 * Continue conversation after tool execution
	 * @param ToolUseId - The tool use ID from OnToolCall
	 * @param Result - The tool execution result
	 * @param Tools - Same tool definitions
	 * @param OnToken, OnToolCall, OnComplete - Same callbacks
	 */
	virtual void ContinueWithToolResult(
		const FString& ToolUseId,
		const FString& Result,
		const TArray<TSharedPtr<FJsonValue>>& Tools,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete
	) = 0;
	
	/** A single tool result entry for batch submission */
	struct FToolResultEntry
	{
		FString ToolUseId;
		FString ToolName;
		FString Result;
	};
	
	/**
	 * Continue conversation after executing multiple tools
	 * Adds ALL tool results to conversation history in the provider's native format,
	 * then sends a single API request. This is required by providers like Anthropic
	 * which mandate that every tool_use block must have a matching tool_result block
	 * in the immediately following user message.
	 *
	 * Default implementation calls ContinueWithToolResult for the last entry
	 * (only correct for single-tool case). Providers MUST override for multi-tool.
	 *
	 * @param ToolResults - Array of (ToolUseId, Result) pairs
	 * @param Tools - Same tool definitions
	 * @param OnToken, OnToolCall, OnComplete - Same callbacks
	 */
	virtual void ContinueWithMultipleToolResults(
		const TArray<FToolResultEntry>& ToolResults,
		const TArray<TSharedPtr<FJsonValue>>& Tools,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete
	)
	{
		// Default: fall back to single tool result (works if only 1 result)
		if (ToolResults.Num() > 0)
		{
			const FToolResultEntry& Last = ToolResults.Last();
			ContinueWithToolResult(Last.ToolUseId, Last.Result, Tools, OnToken, OnToolCall, OnComplete);
		}
		else if (OnComplete)
		{
			OnComplete(false, TEXT("No tool results to continue with"));
		}
	}

	// ============================================================================
	// Multimodal (Vision)
	// ============================================================================
	
	/**
	 * Send a message with an attached image for multimodal analysis.
	 * Providers that support vision (Claude, GPT-4o, Gemini) implement natively.
	 * Others fall back to text-only with a note that the image was provided.
	 * 
	 * @param Message - The user message / question about the image
	 * @param ImagePath - Absolute path to the image file (PNG/JPEG)
	 * @param OnToken - Called for each streaming token
	 * @param OnComplete - Called when the response is complete
	 */
	virtual void SendMessageWithImage(
		const FString& Message,
		const FString& ImagePath,
		TFunction<void(const FString& Token)> OnToken,
		TFunction<void(bool bSuccess, const FString& Error)> OnComplete
	);

	// ============================================================================
	// State Management
	// ============================================================================
	
	/** Clear conversation history */
	virtual void ClearHistory() = 0;
	
	/** Cancel any in-flight requests */
	virtual void CancelRequest() = 0;
	
	/** Check if a request was cancelled */
	virtual bool IsCancelled() const = 0;

	// ============================================================================
	// Tool Call Tracking
	// ============================================================================
	
	/**
	 * Get the tool calls executed in the last SendMessageWithTools call
	 * This allows callers to inspect what tools were used and their results
	 * @return Array of tool calls with names, arguments, and results
	 */
	virtual const TArray<FClaudeToolCall>& GetLastToolCalls() const = 0;
	
	/**
	 * Clear the last tool calls array
	 */
	virtual void ClearLastToolCalls() = 0;

	// ============================================================================
	// Tool Result Tracking (Governance Data) - REQUEST-SCOPED
	// ============================================================================
	
	/**
	 * Get tool results for a specific request
	 * This is the source of truth for governance/policy/undo/proof data.
	 * UI MUST render from these results, never infer from strings.
	 * @param RequestId - The request ID to fetch results for
	 * @return Array of tool results with governance fields populated
	 */
	virtual TArray<FClaudeToolResult> GetToolResultsForRequest(const FString& RequestId) const = 0;
	
	/**
	 * Clear tool results for a specific request (call after UI has consumed them)
	 * @param RequestId - The request ID to clear
	 */
	virtual void ClearToolResultsForRequest(const FString& RequestId) = 0;
	
	/**
	 * @deprecated Use GetToolResultsForRequest instead
	 * Get the tool results from the last SendMessageWithTools call
	 */
	virtual const TArray<FClaudeToolResult>& GetLastToolResults() const = 0;
	
	/**
	 * @deprecated Use ClearToolResultsForRequest instead
	 */
	virtual void ClearLastToolResults() = 0;

	// ============================================================================
	// Token Tracking
	// ============================================================================
	
	/** Get total input tokens used this session */
	virtual int64 GetSessionInputTokens() const = 0;
	
	/** Get total output tokens used this session */
	virtual int64 GetSessionOutputTokens() const = 0;
	
	/** Estimate session cost in USD */
	virtual float EstimateSessionCost() const = 0;
	
	/** Reset session token counters */
	virtual void ResetSessionTokens() = 0;
};

/**
 * Factory for creating AI providers
 */
class RIFTBORNAI_API FAIProviderFactory
{
public:
	/** Create a provider by name */
	static TSharedPtr<IAIProvider> CreateProvider(const FString& ProviderName);
	
	/** Create the default provider based on settings */
	static TSharedPtr<IAIProvider> CreateDefaultProvider();
	
	/**
	 * Create a provider optimized for a specific query.
	 * Uses TieredModelRouter to classify query complexity and select the best model.
	 * Falls back to CreateDefaultProvider() if router is not configured.
	 * 
	 * @param UserQuery The query that will be sent to this provider
	 * @param ToolCount Number of tools available (more tools = heavier query)
	 * @return Configured provider with appropriate model selected
	 */
	static TSharedPtr<IAIProvider> CreateProviderForQuery(
		const FString& UserQuery,
		int32 ToolCount = 0,
		int32 ConversationTurnCount = 0);

	/**
	 * Resolve the concrete provider/model pair for a query without creating it.
	 * Used by the copilot UI so Auto mode and factory-created requests share
	 * one routing decision path.
	 */
	static FResolvedProviderSelection ResolveProviderForQuery(
		const FString& UserQuery,
		int32 ToolCount = 0,
		int32 ConversationTurnCount = 0);
	
	/** Get list of available provider names */
	static TArray<FString> GetAvailableProviders();
};
