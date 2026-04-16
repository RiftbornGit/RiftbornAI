# LLM Provider System

> Deep-dive into the multi-provider AI architecture: abstract interface, four provider implementations, circuit-breaker failover, and tiered model routing.

---

## Architecture Overview

RiftbornAI supports **four LLM providers** behind a unified abstract interface (`IAIProvider`). A **circuit-breaker failover** system (`FProviderFailover`) ensures requests are handled seamlessly even when providers go down. A **tiered model router** (`FTieredModelRouter`) classifies queries by complexity and routes them to the most cost/latency-efficient provider.

```
User Query
    ↓
FTieredModelRouter::ClassifyQuery()     ← Local heuristic routing
    ↓ (Fast / Standard / Heavy)
FAIProviderFactory::CreateProviderForQuery()
    ↓
IAIProvider::SendMessageWithTools()
    ↓ (on failure)
FProviderFailover::SendWithToolsFailover()
    ↓ (tries next healthy provider)
Claude → OpenAI → Gemini → Ollama
```

**Source files:**
- `Source/RiftbornAI/Public/IAIProvider.h` — Abstract interface (301 lines)
- `Source/RiftbornAI/Public/Providers/ClaudeProvider.h` — Anthropic Claude
- `Source/RiftbornAI/Public/Providers/OpenAIProvider.h` — OpenAI GPT
- `Source/RiftbornAI/Public/Providers/GeminiProvider.h` — Google Gemini
- `Source/RiftbornAI/Public/Providers/OllamaProvider.h` — Local Ollama
- `Source/RiftbornAI/Public/ProviderFailover.h` — Circuit-breaker failover (237 lines)
- `Source/RiftbornAI/Public/TieredModelRouter.h` — Query complexity routing (184 lines)
- `Source/RiftbornAI/Public/ClaudeAPIClient.h` — Low-level Claude API client (285 lines)
- `Source/RiftbornAI/Public/OllamaClient.h` — Low-level Ollama client (253 lines)

---

## IAIProvider — Abstract Interface

Every provider implements this interface identically, making providers interchangeable.

### Configuration Methods

```cpp
virtual void SetAPIKey(const FString& APIKey) = 0;
virtual void SetModel(const FString& Model) = 0;
virtual FString GetModel() const = 0;
virtual FString GetProviderName() const = 0;   // "Claude", "OpenAI", "Gemini", "Ollama"
virtual bool IsConfigured() const = 0;
virtual void SetSystemPrompt(const FString& Prompt) = 0;
```

### Three Communication Modes

#### 1. Simple Messaging (Non-Agentic)

```cpp
virtual void SendMessage(
    const FString& Message,
    TFunction<void(bool bSuccess, const FString& Response)> OnComplete
) = 0;
```

Single request/response. No tool use. Good for classification, summarization, or simple Q&A.

#### 2. Agentic Tool Use

```cpp
virtual void SendMessageWithTools(
    const FString& Message,
    const FString& RequestId,
    TFunction<void(bool bSuccess, const FString& Response, const FString& RequestId)> OnComplete,
    TFunction<void(const FString& Status)> OnProgress = nullptr
) = 0;
```

Tool-loop mode. The provider executes tools and continues the conversation automatically until the task is complete. The `RequestId` correlates tool results for governance tracking.

#### 3. Fine-Grained Streaming

```cpp
virtual void SendMessageStreaming(
    const FString& Message,
    const TArray<TSharedPtr<FJsonValue>>& Tools,
    TFunction<void(const FString& Token)> OnToken,
    TFunction<void(const FString& ToolName, const TMap<FString, FString>& Args, const FString& ToolUseId)> OnToolCall,
    TFunction<void(bool bSuccess, const FString& Error)> OnComplete
) = 0;
```

Token-by-token streaming with tool call callbacks. Allows UI to show real-time typing and tool execution.

### Multi-Tool Continuation

Critical for providers like Anthropic that mandate every `tool_use` block must have a matching `tool_result` in the immediately following user message:

```cpp
virtual void ContinueWithMultipleToolResults(
    const TArray<FToolResultEntry>& ToolResults,
    const TArray<TSharedPtr<FJsonValue>>& Tools,
    // ... callbacks
);
```

Default implementation falls back to single-tool for providers that don't support batch tool results.

### Multimodal (Vision)

```cpp
virtual void SendMessageWithImage(
    const FString& Message,
    const FString& ImagePath,     // Absolute path to PNG/JPEG
    TFunction<void(const FString& Token)> OnToken,
    TFunction<void(bool bSuccess, const FString& Error)> OnComplete
);
```

Supported natively by Claude, GPT-4o, and Gemini Pro Vision. Others fall back to text-only.

### Token Tracking

Every provider tracks session-level token usage:

```cpp
virtual int64 GetSessionInputTokens() const = 0;
virtual int64 GetSessionOutputTokens() const = 0;
virtual float EstimateSessionCost() const = 0;    // USD estimate
virtual void ResetSessionTokens() = 0;
```

### Tool Result Tracking (Request-Scoped)

Governance requires knowing exactly which tools were executed for each request:

```cpp
virtual TArray<FClaudeToolResult> GetToolResultsForRequest(const FString& RequestId) const = 0;
virtual void ClearToolResultsForRequest(const FString& RequestId) = 0;
```

This is the **source of truth** for governance/policy/undo/proof data. UI must render from these results, never infer from strings.

---

## Provider Implementations

### FClaudeProvider (Anthropic)

| Feature | Detail |
|---------|--------|
| **Endpoint** | `https://api.anthropic.com/v1/messages` (configurable) |
| **Models** | Opus, Sonnet, Haiku (via `SetModel()`) |
| **Vision** | ✅ Native multimodal support |
| **Tool Use** | ✅ Native, with multi-tool continuation |
| **Streaming** | ✅ SSE streaming |
| **Cost Tracking** | ✅ Model-aware (`GetInputTokenCostPer1K()`) |
| **History Pruning** | `PruneHistorySimple(KeepLastN=5)` — collapses old tool exchanges to one-line deltas |
| **Token Compaction** | `CompactToolResult()` — reduces verbose tool results to state deltas |

**Unique Features:**
- Model-specific cost calculation per 1K tokens
- History compaction that keeps last N tool exchanges in detail, collapses older ones
- Request-scoped tool results with thread-safe storage (`FCriticalSection`)

### FOpenAIProvider (OpenAI)

| Feature | Detail |
|---------|--------|
| **Endpoint** | `https://api.openai.com/v1/chat/completions` (configurable) |
| **Models** | GPT-4, GPT-4-Turbo, GPT-4o |
| **Vision** | ✅ Native (GPT-4o) |
| **Tool Use** | ✅ Native function calling |
| **Streaming** | ✅ SSE streaming |
| **History Pruning** | `PruneHistory(KeepLastN=10)` |

### FGeminiProvider (Google)

| Feature | Detail |
|---------|--------|
| **Endpoint** | Google Generative AI API |
| **Models** | Gemini Pro, Gemini Pro Vision, Gemini 1.5 Pro, Gemini 1.5 Flash |
| **Vision** | ✅ Native multimodal |
| **Tool Use** | ✅ Function calling (converted from Claude format) |
| **System Prompt** | Mapped to `systemInstruction` field |
| **Tool Conversion** | `ConvertToolsToGeminiFormat()` — translates Claude tool definitions |
| **History Pruning** | `PruneHistory(KeepLastN=10)` |

### FOllamaProvider (Local)

| Feature | Detail |
|---------|--------|
| **Endpoint** | `http://localhost:11434` (configurable) |
| **Models** | Any Ollama model (Qwen, Llama, Mistral, etc.) |
| **Cost** | No hosted model fee from RiftbornAI; local compute cost still applies |
| **Privacy** | Local-model path; requests do not need to leave the machine unless the surrounding workflow adds remote services |
| **Offline** | ✅ Works without internet |
| **Vision** | ❌ Text-only fallback |
| **Tool Use** | ✅ Text-parsed tool calls (`ParseToolCallsFromText()`) |
| **Agentic Loop** | Up to `MaxToolIterations = 10` per request |
| **History Pruning** | `PruneHistory(KeepLastN=10)` with original intent preservation |
| **TTL Cleanup** | `PurgeExpiredToolResults()` — 5-minute TTL for orphaned results |

**Unique Features:**
- `CheckServerStatus()` — verify Ollama is running
- `ListModels()` — enumerate available models
- `PullModel()` — download models from Ollama registry
- Text-based tool call parsing for models that output tool calls as text
- Built-in agentic loop with iteration tracking

---

## FAIProviderFactory

Creates and configures providers:

```cpp
// Create by name
TSharedPtr<IAIProvider> provider = FAIProviderFactory::CreateProvider("Claude");

// Create default (from settings)
TSharedPtr<IAIProvider> provider = FAIProviderFactory::CreateDefaultProvider();

// Create optimal for a specific query (uses TieredModelRouter)
TSharedPtr<IAIProvider> provider = FAIProviderFactory::CreateProviderForQuery(
    "Move the player start to 0,0,100",
    /*ToolCount=*/ 200
);

// List available
TArray<FString> providers = FAIProviderFactory::GetAvailableProviders();
```

---

## FProviderFailover — Circuit Breaker

Wraps `IAIProvider` with automatic cross-provider failover. Users should **never** see "Claude is down" — they should see their request handled seamlessly by a backup provider.

### Health Tracking (FProviderHealth)

Per-provider health state:

| Field | Description |
|-------|-------------|
| `bHealthy` | Current health status |
| `ConsecutiveFailures` | Reset to 0 on success |
| `UNHEALTHY_THRESHOLD` | **3 consecutive failures** → mark unhealthy |
| `CooldownSeconds` | **60 seconds** before retrying unhealthy provider |
| `AverageLatencySeconds` | Rolling average TTFT |
| `LastError` | Most recent error message |

### Failover Chain

Default order: **Claude → OpenAI → Gemini → Ollama**

Configurable via `SetFailoverOrder()`. On each request:

1. Try preferred/primary provider
2. On failure, update health tracking
3. If unhealthy, skip to next provider in chain
4. Unhealthy providers with expired cooldowns get retried
5. If all providers exhausted, return error

### API

```cpp
// Simple message with failover
FProviderFailover::Get().SendWithFailover(
    "Move actor to origin",
    [](const FFailoverResult& Result) {
        // Result.ProviderUsed, Result.ProvidersAttempted, Result.AttemptLog
    },
    "Claude"  // preferred
);

// Agentic with failover
FProviderFailover::Get().SendWithToolsFailover(
    Message, RequestId, OnComplete, OnProgress, "Claude"
);

// Manual health management
FProviderFailover::Get().SetProviderHealthy("Claude", false);
FProviderFailover::Get().ReportSuccess("OpenAI", 1.2f);
FProviderFailover::Get().ReportFailure("Claude", "Rate limited");

// Get next healthy alternative
TSharedPtr<IAIProvider> backup = FProviderFailover::Get().GetNextHealthyProvider("Claude");
```

### FFailoverResult

```cpp
struct FFailoverResult {
    bool bSuccess;
    FString ProviderUsed;          // Which provider actually handled it
    int32 ProvidersAttempted;      // How many were tried
    TArray<FString> AttemptLog;    // Per-attempt details
    FString Response;
    float TotalTimeSeconds;
};
```

---

## FTieredModelRouter — Query Complexity Routing

One major latency optimization is avoiding heavy models for obviously simple requests. The exact distribution depends on project shape, provider availability, and how the agent is being used.

### Query Tiers

| Tier | Target TTFT | Use Case | Example Models |
|------|------------|----------|----------------|
| **Fast** | <2s | Single-tool, property get/set, navigation, info | Ollama qwen3:14b, Haiku |
| **Standard** | <5s | Multi-step but predictable, common patterns | Sonnet, GPT-4-Turbo |
| **Heavy** | <15s | Code generation, complex reasoning, multi-file | Opus, GPT-4o |

### Local Classification

Classification is done locally using keyword patterns and heuristics, so it does not require an extra model call:

```cpp
FQueryClassification result = FTieredModelRouter::Get().ClassifyQuery(
    "Move the player start forward 100 units",
    /*ToolCount=*/ 200,
    /*ConversationTurnCount=*/ 0
);
// result.Tier = EQueryTier::Fast
// result.Confidence = 0.9
// result.MatchedPattern = "move * forward"
```

**Fast-tier detection**: Keyword patterns for simple operations (move, set, get, list, show, etc.)

**Heavy-tier detection**: Keywords indicating complexity (generate, create from scratch, refactor, analyze, implement) plus heuristics like high tool count or long conversation.

### Model Configuration

```cpp
FTierModelConfig config;
config.ProviderName = "Ollama";
config.ModelName = "qwen3:14b";
config.Temperature = 0.1f;
config.MaxTokens = 2048;
config.ExpectedTTFTSeconds = 1.5f;

FTieredModelRouter::Get().SetModelForTier(EQueryTier::Fast, config);
```

`AutoConfigure()` detects available providers (Ollama models, API keys) and assigns optimally.

### Latency Monitoring

Records actual TTFT per tier for monitoring and auto-tuning:

```cpp
FTieredModelRouter::Get().RecordLatency(EQueryTier::Fast, 1.2f, /*bSuccess=*/true);
float avg = FTieredModelRouter::Get().GetAverageLatency(EQueryTier::Fast);
```

Keeps a rolling window of the last 50 latencies per tier with failure tracking.

---

## Low-Level Clients

### ClaudeAPIClient.h (285 lines)

Direct Claude API integration used internally by `FClaudeProvider`:

| Feature | Detail |
|---------|--------|
| **Streaming** | SSE (Server-Sent Events) |
| **Tool Use** | Full tool call/result cycle |
| **Multi-Tool Continuation** | Batch tool results in single user message |
| **Token Tracking** | Per-request input/output counts |
| **Cost Estimation** | Model-aware pricing |
| **Model Constants** | Opus, Haiku, Ollama model strings |

### OllamaClient.h (253 lines)

Local LLM client via Ollama HTTP API:

| Feature | Detail |
|---------|--------|
| **Endpoint** | `http://localhost:11434` |
| **Streaming** | NDJSON streaming |
| **Tool Use** | Text-parsed tool calls |
| **Server Management** | Status check, model listing, model pull |
| **Agentic Loop** | Built-in iteration with max 10 tool calls |

---

## Provider Comparison Matrix

| Capability | Claude | OpenAI | Gemini | Ollama |
|-----------|--------|--------|--------|--------|
| Simple Messages | ✅ | ✅ | ✅ | ✅ |
| Tool Use (Native) | ✅ | ✅ | ✅ | ✅ (text-parsed) |
| Multi-Tool Batch | ✅ | ✅ | ✅ | ✅ |
| Streaming | ✅ | ✅ | ✅ | ✅ |
| Vision/Multimodal | ✅ | ✅ | ✅ | ❌ |
| System Prompt | ✅ | ✅ | ✅ (systemInstruction) | ✅ |
| Token Tracking | ✅ | ✅ | ✅ | ✅ (estimated) |
| Cost | $$$ | $$ | $ | **Free** |
| Offline | ❌ | ❌ | ❌ | ✅ |
| Privacy | Cloud | Cloud | Cloud | Local-model path |
| History Pruning | ✅ (compact) | ✅ | ✅ | ✅ |
| Request-Scoped Results | ✅ | ✅ | ✅ | ✅ (with TTL) |

---

## Cross-References

- **Agent System**: Uses providers for LLM inference — see [AGENT_SYSTEM.md](AGENT_SYSTEM.md)
- **Configuration**: Provider settings in RiftbornSettings — see [CONFIGURATION_REFERENCE.md](CONFIGURATION_REFERENCE.md)
- **Tool System**: Providers execute tools from the registry — see [TOOL_SYSTEM.md](TOOL_SYSTEM.md)
- **Governance**: Tool results feed governance proofs — see [GOVERNANCE_AND_SECURITY.md](GOVERNANCE_AND_SECURITY.md)
