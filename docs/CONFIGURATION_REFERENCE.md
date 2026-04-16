# RiftbornAI — Configuration Reference

**Last updated**: 2026-04-06

This document only covers configuration that still exists in the checked-in code. Historical Brain API and UGDB settings were removed from this reference because they are not part of the live product contract.

It also excludes speculative 3D/audio generation provider config that is not wired to any shipped implementation in the current repository.

---

## 1. UE Plugin Settings

Validated from:

- `Source/RiftbornAI/Public/Gameplay/RiftbornSettings.h`
- `Config/DefaultRiftbornAI.ini`

Configure these in `Project Settings -> Plugins -> RiftbornAI`.

### Bridge

| Setting | Default | Meaning |
|------|---------|---------|
| `BridgePort` | `8765` | TCP bridge port for low-level UE Python bridge |
| `HttpBridgePort` | `8766` | Plugin HTTP bridge setting that still exists in UE settings; do not assume it is the primary external product route |
| `ConnectionTimeoutSeconds` | `30.0` | Bridge connection timeout |
| `CommandTimeoutSeconds` | `60.0` | Command execution timeout |
| `MaxPoolConnections` | `4` | Max pooled bridge connections |
| `MinPoolConnections` | `1` | Min pooled bridge connections |
| `bAutoStartBridge` | `true` | Auto-start bridge services on editor launch |

### AI Provider

| Setting | Default | Meaning |
|------|---------|---------|
| `DefaultProvider` | `Local` | Default AI backend |
| `ClaudeModel` | `ClaudeOpus4` | Anthropic model enum (`claude-opus-4-7` by default) |
| `OpenAIModel` | `gpt-5-mini` | OpenAI model string |
| `GeminiModel` | `gemini-1.5-flash` | Gemini model string |
| `OllamaEndpoint` | `http://127.0.0.1:11434` | Local provider endpoint |
| `OllamaModel` | `minimax-m2.5:cloud` | Local model name |
| `MaxResponseTokens` | `16384` | Response token cap |
| `MaxTokensPerRequest` | `16384` | Back-compat request token cap |
| `Temperature` | `0.7` | Sampling temperature |
| `RequestTimeoutSeconds` | `300.0` | Provider request timeout |

### Rate limiting and resilience

| Setting | Default |
|------|---------|
| `bEnableRateLimiting` | `true` |
| `RequestsPerMinute` | `0` |
| `bEnableCircuitBreaker` | `true` |
| `CircuitBreakerThreshold` | `5` |
| `CircuitBreakerCooldown` | `60.0` |

### Snapshot and recovery

| Setting | Default |
|------|---------|
| `SnapshotDirectory` | `RiftbornAI/Snapshots` |
| `SnapshotRetentionDays` | `7` |
| `MaxSnapshotSizeMB` | `100` |
| `bAutoSnapshot` | `true` |

### Logging and security

| Setting | Default |
|------|---------|
| `LogLevel` | `Info` |
| `LogRetentionDays` | `7` |
| `bMirrorToUELog` | `true` |
| `HealthCheckIntervalSeconds` | `30.0` |
| `bEnableSandbox` | `true` |
| `PythonExecutionTimeoutSeconds` | `30.0` |
| `bSandboxAllowFileRead` | `true` |

### UI and editor-performance controls

Validated examples:

- `bShowHealthIndicator`
- `bShowMetricsInStatusBar`
- `DefaultWindowWidth`
- `DefaultWindowHeight`
- `bEnableBlueprintFixService`
- `bSuspendBlueprintFixServiceDuringHeavyCompiles`
- `BlueprintFixServicePendingCompileThreshold`
- `BlueprintFixServiceUsedPhysicalMemoryThresholdGB`
- `bEnableEditorLoadWarnings`

### Cost controls

Validated examples:

- `bLiteMode`
- `bDualModelMode`
- `bMinimalTools`
- `bCompactPrompt`
- `MaxConversationTurns`
- `SessionSpendingCapUSD`
- `PerRequestSpendingCapUSD`
- `MaxAgenticIterations`
- `bLogCostPerCall`
- `bAutoUpgradeModels`

---

## 2. Python Bridge Config

Validated from:

- `Bridge/riftborn_config.py`

The current bridge-side Python config is intentionally small:

| Field | Default | Source |
|------|---------|---------|
| `bridge_host` | `127.0.0.1` | `RIFTBORN_BRIDGE_HOST` |
| `threaded_http_port` | `8767` | `RIFTBORN_THREADED_HTTP_PORT` |
| `brain_data_dir` | `Bridge/artifacts/brain_data` | computed path |

Notes:

- `threaded_http_port` is the validated live HTTP bridge setting in the current Python config module.
- `brain_data_dir` still exists as a storage path, but that does not imply a standalone Brain API server is part of the shipped runtime.
- The current external product path uses the governed threaded HTTP bridge on `8767`, even though UE settings still expose an `HttpBridgePort` field.

---

## 3. Port Map

Validated live/declared ports in the current tree:

| Port | Owner | Meaning |
|------|------|---------|
| `8765` | UE TCP bridge | Low-level UE Python bridge |
| `8766` | UE plugin settings | HTTP bridge setting exposed by `URiftbornSettings` |
| `8767` | Python bridge / MCP routing | Primary governed HTTP bridge used by the external product path |
| `11434` | Ollama | Local LLM provider endpoint |

Important:

- Older docs that listed `8768` as a required Brain API port or `8780` as a required UGDB port were describing legacy sidecars, not the live product contract.
- The existence of both `8766` and `8767` in checked-in settings/docs reflects an implementation transition. Treat `8767` as the primary governed external route unless runtime evidence for your branch says otherwise.

---

## 4. Environment Variables

Validated examples from code and config:

| Variable | Purpose |
|------|---------|
| `RIFTBORN_BRIDGE_HOST` | Override Python bridge host |
| `RIFTBORN_THREADED_HTTP_PORT` | Override Python bridge HTTP port |
| `RIFTBORN_AUTH_TOKEN` | Preferred explicit bridge auth token for MCP clients and bridge helpers |
| `RIFTBORN_API_KEY` | Back-compat auth token alias |
| `RIFTBORN_DEV_TOKEN` | Local-development auth token alias |
| `RIFTBORN_DEV_TOKEN_FILE` | Optional explicit path to a `.dev_token` file |
| `ANTHROPIC_API_KEY` | Anthropic key fallback |
| `OPENAI_API_KEY` | OpenAI key fallback |
| `GEMINI_API_KEY` | Gemini key fallback |

Other runtime flags still used in the current tree include:

- `RIFTBORN_PRODUCT_MODE`
- `RIFTBORN_ENABLE_INTERNAL_TOOLS`
- `RIFTBORN_PROOF_MODE`

Bridge auth token discovery notes:

- Python bridge helpers and local test utilities now resolve auth from `RIFTBORN_AUTH_TOKEN`, then `RIFTBORN_API_KEY`, then `RIFTBORN_DEV_TOKEN`.
- If no env var is set, they look for `Saved/RiftbornAI/.dev_token` near the project or via `RIFTBORN_DEV_TOKEN_FILE`.
- The MCP server uses the same env aliases and `.dev_token` fallback so external agents no longer need a separate env-only setup in local sessions.

---

## 5. Removed From This Reference

The following were intentionally removed from this document because they are not validated as part of the live product contract:

- removed Brain API sidecar port settings
- `plan_arbiter.py` tier tables
- `ugdb_port` / `ugdb_url`
- standalone “Living Mind” sidecar config tables

If you see those names elsewhere in the repo, treat them as legacy references to clean up, not authoritative configuration.
