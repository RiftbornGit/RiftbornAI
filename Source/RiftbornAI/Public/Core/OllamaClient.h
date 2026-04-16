// OllamaClient.h
// ACTUAL LLM integration - no faking, real HTTP calls to Ollama

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "Dom/JsonObject.h"

DECLARE_DELEGATE_TwoParams(FOnLLMResponse, bool /*bSuccess*/, const FString& /*Response*/);
DECLARE_DELEGATE_OneParam(FOnLLMStreamChunk, const FString& /*Chunk*/);

/**
 * LLM request configuration
 */
struct FLLMRequestConfig
{
    FString Model = TEXT("deepseek-coder:6.7b");  // Good for code
    float Temperature = 0.7f;
    int32 MaxTokens = 4096;
    bool bStream = false;
    TArray<FString> StopSequences;
    
    // Context window management
    int32 ContextWindow = 8192;
    bool bTruncateContext = true;
};

/**
 * A message in the conversation
 */
struct FLLMMessage
{
    FString Role;    // "system", "user", "assistant"
    FString Content;
    
    FLLMMessage() = default;
    FLLMMessage(const FString& InRole, const FString& InContent)
        : Role(InRole), Content(InContent) {}
};

/**
 * Full conversation context
 */
struct FLLMConversation
{
    TArray<FLLMMessage> Messages;
    FString SystemPrompt;
    int32 TotalTokensEstimate = 0;
    
    void AddUserMessage(const FString& Content)
    {
        Messages.Add(FLLMMessage(TEXT("user"), Content));
        TotalTokensEstimate += EstimateTokenCount(Content);
    }
    
    void AddAssistantMessage(const FString& Content)
    {
        Messages.Add(FLLMMessage(TEXT("assistant"), Content));
        TotalTokensEstimate += EstimateTokenCount(Content);
    }
    
    void Clear()
    {
        Messages.Empty();
        TotalTokensEstimate = EstimateTokenCount(SystemPrompt);
    }
    
    void TrimToFit(int32 MaxTokens)
    {
        while (TotalTokensEstimate > MaxTokens && Messages.Num() > 2)
        {
            TotalTokensEstimate -= EstimateTokenCount(Messages[0].Content);
            Messages.RemoveAt(0);
        }
    }
    
private:
    /** Estimate token count: ~3.3 chars/token for code, ~4 for English.
     *  Using 3 as conservative estimate since we deal mostly with code/JSON. */
    static int32 EstimateTokenCount(const FString& Text)
    {
        return FMath::Max(1, Text.Len() / 3);
    }
};

/**
 * LLM response with metadata
 */
struct FLLMResponse
{
    bool bSuccess = false;
    FString Content;
    FString ErrorMessage;
    
    // Metadata
    int32 PromptTokens = 0;
    int32 CompletionTokens = 0;
    float GenerationTimeSeconds = 0.0f;
    FString Model;
    
    // Parsed code blocks
    TArray<FString> CodeBlocks;
    
    void ParseCodeBlocks()
    {
        CodeBlocks.Empty();
        
        int32 StartIdx = 0;
        while ((StartIdx = Content.Find(TEXT("```"), ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIdx)) != INDEX_NONE)
        {
            int32 CodeStart = Content.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIdx);
            if (CodeStart == INDEX_NONE) break;
            CodeStart++;
            
            int32 CodeEnd = Content.Find(TEXT("```"), ESearchCase::CaseSensitive, ESearchDir::FromStart, CodeStart);
            if (CodeEnd == INDEX_NONE) break;
            
            FString Code = Content.Mid(CodeStart, CodeEnd - CodeStart).TrimEnd();
            if (!Code.IsEmpty())
            {
                CodeBlocks.Add(Code);
            }
            
            StartIdx = CodeEnd + 3;
        }
    }
};

/**
 * Real Ollama HTTP client
 * Actually calls Ollama API - no faking
 */
class RIFTBORNAI_API FOllamaClient
{
public:
    FOllamaClient();
    ~FOllamaClient();
    
    // === Configuration ===
    
    /** Set Ollama server URL (default: http://localhost:11434) */
    void SetServerURL(const FString& URL);
    
    /** Set default model */
    void SetModel(const FString& ModelName);
    
    /** Set system prompt for all conversations */
    void SetSystemPrompt(const FString& Prompt);
    
    /** Get current config */
    FLLMRequestConfig& GetConfig() { return Config; }
    
    // === Connection ===
    
    /** Check if Ollama is running */
    bool IsServerAvailable();
    
    /** Get list of available models */
    TArray<FString> GetAvailableModels();
    
    /** Pull a model if not available */
    bool PullModel(const FString& ModelName);
    
    // === Synchronous API ===
    
    /** Send a single prompt and wait for response */
    FLLMResponse Generate(const FString& Prompt);
    
    /** Send with full conversation context */
    FLLMResponse Chat(FLLMConversation& Conversation);
    
    // === Asynchronous API ===
    
    /** Send prompt asynchronously */
    void GenerateAsync(const FString& Prompt, FOnLLMResponse OnComplete);
    
    /** Chat asynchronously */
    void ChatAsync(FLLMConversation& Conversation, FOnLLMResponse OnComplete);
    
    /** Streaming generation (chunks returned as they arrive) */
    void GenerateStreaming(const FString& Prompt, FOnLLMStreamChunk OnChunk, FOnLLMResponse OnComplete);
    
    // === Utilities ===
    
    /** Estimate token count for text */
    static int32 EstimateTokens(const FString& Text);
    
    /** Cancel any pending requests */
    void CancelAllRequests();
    
    /** Check if a request is in progress */
    bool IsRequestInProgress() const { return bRequestInProgress; }
    
private:
    FString BuildGenerateRequestBody(const FString& Prompt);
    FString BuildChatRequestBody(const FLLMConversation& Conversation);
    FLLMResponse ParseResponse(const FString& JsonResponse);
    void HandleHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnLLMResponse Callback);
    
    FString ServerURL;
    FString DefaultModel;
    FString SystemPrompt;
    FLLMRequestConfig Config;
    
    TArray<TSharedPtr<IHttpRequest>> PendingRequests;
    bool bRequestInProgress = false;
    FCriticalSection RequestLock;
};

/**
 * Specialized prompts for Unreal Engine code generation
 */
class RIFTBORNAI_API FUEPromptBuilder
{
public:
    /** Build system prompt for UE code generation */
    static FString BuildSystemPrompt();
    
    /** Build prompt for creating a new class */
    static FString BuildCreateClassPrompt(
        const FString& ClassName,
        const FString& BaseClass,
        const FString& Description,
        const TArray<FString>& ExistingClasses
    );
    
    /** Build prompt for modifying existing code */
    static FString BuildModifyCodePrompt(
        const FString& ExistingCode,
        const FString& Modification
    );
    
    /** Build prompt for fixing compilation errors */
    static FString BuildFixErrorPrompt(
        const FString& Code,
        const FString& ErrorMessage
    );
    
    /** Build prompt for explaining code */
    static FString BuildExplainPrompt(const FString& Code);
    
    /** Build prompt for generating Blueprint logic description */
    static FString BuildBlueprintPrompt(const FString& Description);
    
    /** Add project context to any prompt */
    static FString AddProjectContext(
        const FString& Prompt,
        const TArray<FString>& RelevantFiles,
        const TMap<FString, FString>& FileContents
    );
};
