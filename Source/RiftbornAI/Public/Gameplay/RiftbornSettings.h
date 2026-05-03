// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RiftbornSettings.generated.h"

/**
 * AI Provider options
 */
UENUM(BlueprintType)
enum class ERiftbornAIProvider : uint8
{
	Anthropic UMETA(DisplayName = "Anthropic (Claude)"),
	OpenAI UMETA(DisplayName = "OpenAI (GPT)"),
	Gemini UMETA(DisplayName = "Google Gemini (Legacy)"),
	Local UMETA(DisplayName = "Local (Ollama)"),
	/** Use the Claude Code CLI as the reasoning backend. Anthropic auth is
	 *  delegated to Claude Code itself (subscription seat, no per-token API
	 *  charge). Tools route through the running RiftbornAI HTTP MCP bridge. */
	ClaudeCode UMETA(DisplayName = "Claude Code (CLI / subscription)")
};

/**
 * Claude model selection
 */
UENUM(BlueprintType)
enum class ERiftbornClaudeModel : uint8
{
	ClaudeOpus4 UMETA(DisplayName = "Claude Opus 4.7 (Most Capable)"),
	ClaudeSonnet4 UMETA(DisplayName = "Claude Sonnet 4.6 (Balanced)"),
	ClaudeHaiku UMETA(DisplayName = "Claude Haiku 4.5 (Legacy -> Sonnet 4.6)")
};

/**
 * Log level options
 */
UENUM(BlueprintType)
enum class ERiftbornLogLevel : uint8
{
	Debug UMETA(DisplayName = "Debug (Verbose)"),
	Info  UMETA(DisplayName = "Info"),
	Warn  UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error Only"),
	Fatal UMETA(DisplayName = "Fatal")
};

/**
 * RiftbornAI Plugin Settings
 *
 * Configure in: Project Settings > Plugins > RiftbornAI
 * Saved to: Config/DefaultRiftbornAI.ini
 */
UCLASS(config=RiftbornAI, defaultconfig, meta=(DisplayName="RiftbornAI"))
class RIFTBORNAI_API URiftbornSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URiftbornSettings();

	/** Get the singleton settings object */
	static URiftbornSettings* Get();

	// ==========================================================================
	// Bridge Settings
	// ==========================================================================

	/** TCP port for Python bridge connection */
	UPROPERTY(config, EditAnywhere, Category = "Bridge", meta = (ClampMin = "1024", ClampMax = "65535"))
	int32 BridgePort = 8765;

	/** HTTP port for external bridge communication (MCP server, VS Code) */
	UPROPERTY(config, EditAnywhere, Category = "Bridge", meta = (ClampMin = "1024", ClampMax = "65535"))
	int32 HttpBridgePort = 8766;

	/** Connection timeout in seconds */
	UPROPERTY(config, EditAnywhere, Category = "Bridge", meta = (ClampMin = "1.0", ClampMax = "120.0"))
	float ConnectionTimeoutSeconds = 30.0f;

	/** Command execution timeout in seconds */
	UPROPERTY(config, EditAnywhere, Category = "Bridge", meta = (ClampMin = "1.0", ClampMax = "600.0"))
	float CommandTimeoutSeconds = 60.0f;

	/** Maximum connections in pool */
	UPROPERTY(config, EditAnywhere, Category = "Bridge", meta = (ClampMin = "1", ClampMax = "16"))
	int32 MaxPoolConnections = 4;

	/** Minimum connections to keep alive */
	UPROPERTY(config, EditAnywhere, Category = "Bridge", meta = (ClampMin = "0", ClampMax = "4"))
	int32 MinPoolConnections = 1;

	/** Auto-start Python bridge when editor starts.
	 *  Default OFF — most installs don't need the legacy Python TCP bridge;
	 *  the in-process HTTP bridge handles all governed tool execution. Turn
	 *  this on only if you're using Python-script-based tools that connect
	 *  to the TCP bridge on port 8765. When OFF, the bridge monitor also
	 *  stops pinging so the editor doesn't freeze for ~2 s waiting on TCP
	 *  timeouts every backoff window. */
	UPROPERTY(config, EditAnywhere, Category = "Bridge")
	bool bAutoStartBridge = false;

	// ==========================================================================
	// Beta-Release Surface Lock
	// ==========================================================================

	/** When true, the in-editor copilot exposes ONLY the current beta-release
	 *  tool set (200 hardened tools in the public Beta) that ship with the
	 *  public Beta. Defaults to true — every build ships locked to the
	 *  200-tool beta surface. Internal dev builds override via the
	 *  RIFTBORN_DEV_MODE env var (any truthy value: 1/true/yes/on). The
	 *  setting is hidden from end users (AdvancedDisplay) so they can't
	 *  accidentally unlock the full surface. */
	UPROPERTY(config, meta = (DisplayName = "Lock to beta-release tool set"))
	bool bBetaReleaseLockActive = true;

	/** Developer override — when true, exposes EVERY registered tool to the
	 *  copilot regardless of bBetaReleaseLockActive. For internal use only.
	 *  Override via RIFTBORN_DEV_MODE env var (any truthy value: 1/true/yes/on).
	 *  Hidden from end users. */
	UPROPERTY(config, meta = (DisplayName = "Developer mode (expose all tools)"))
	bool bDeveloperMode = false;

	// ==========================================================================
	// AI Provider Settings
	// ==========================================================================

	/** DEPRECATED: provider is chosen via the copilot's in-panel model picker.
	 *  Kept as `config` (no `EditAnywhere`) so legacy values still load from
	 *  DefaultRiftbornAI.ini, but no longer surfaced in Project Settings —
	 *  it caused confusion when the displayed default (Local/Ollama) didn't
	 *  match the model the user actually selected in the copilot. */
	UPROPERTY(config)
	ERiftbornAIProvider DefaultProvider = ERiftbornAIProvider::Local;

	/** Claude copilot model to use (supported values normalize to Sonnet 4.6 or Opus 4.7) */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|Anthropic", meta = (EditCondition = "DefaultProvider == ERiftbornAIProvider::Anthropic"))
	ERiftbornClaudeModel ClaudeModel = ERiftbornClaudeModel::ClaudeOpus4;

	/** Direct Claude model ID override. If non-empty, this value is sent verbatim to the
	 *  Anthropic API and takes precedence over the ClaudeModel enum. Populate from
	 *  dynamic model discovery (FModelDiscovery) to use new models without recompiling. */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|Anthropic", meta = (EditCondition = "DefaultProvider == ERiftbornAIProvider::Anthropic", DisplayName = "Claude Model ID Override"))
	FString ClaudeModelIdOverride;

	/** When true, attach cache_control: ephemeral markers to the system prompt and
	 *  the last tool in the tools array. First turn pays cache_creation tokens;
	 *  every subsequent turn pays ~10% of the input cost for cache reads. Major
	 *  cost + latency win for tool-heavy sessions (1000+ registered tools).
	 *  Disable only for debugging or provider-compatibility issues. */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|Anthropic", meta = (DisplayName = "Enable Prompt Caching"))
	bool bEnablePromptCaching = true;

	/** Anthropic API Key (Claude) - Leave empty to use ANTHROPIC_API_KEY env var */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|API Keys", meta = (PasswordField = true))
	FString AnthropicApiKey;

	/** OpenAI API Key - Leave empty to use OPENAI_API_KEY env var */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|API Keys", meta = (PasswordField = true))
	FString OpenAIApiKey;

	/** Legacy Gemini API Key. Retained for compatibility, but not used by the supported copilot picker. Leave empty to use GEMINI_API_KEY env var. */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|Legacy Gemini", meta = (PasswordField = true, AdvancedDisplay))
	FString GeminiApiKey;

	/** OpenAI model to use for the copilot (GPT-5.4 is the supported cloud OpenAI target) */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|OpenAI")
	FString OpenAIModel = TEXT("gpt-5.4");

	/** Legacy Gemini model. Retained for compatibility, but not used by the supported copilot picker. */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|Legacy Gemini", meta = (AdvancedDisplay))
	FString GeminiModel = TEXT("gemini-1.5-flash");

	/** DEPRECATED: see DefaultProvider above. Hidden from UI — the copilot
	 *  model picker is the source of truth for provider selection. */
	UPROPERTY(config)
	FString PreferredProvider;

	/** Local Ollama endpoint URL */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|Local")
	FString OllamaEndpoint = TEXT("http://127.0.0.1:11434");

	/** Local model name for Ollama (used as a fallback when discovery has not populated installed models yet) */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|Local")
	FString OllamaModel = TEXT("qwen2.5-coder:7b");

	/** Absolute path to the Claude Code CLI (`claude` / `claude.cmd`).
	 *  Leave empty to auto-detect via PATH. Used only when DefaultProvider
	 *  is set to ClaudeCode. */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider|Claude Code", meta = (EditCondition = "DefaultProvider == ERiftbornAIProvider::ClaudeCode"))
	FString ClaudeCodePath;

	/** Maximum tokens for AI response */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider", meta = (ClampMin = "100", ClampMax = "128000"))
	int32 MaxResponseTokens = 16384;

	/** Maximum tokens per API request (same as MaxResponseTokens, for backwards compatibility) */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider", meta = (ClampMin = "100", ClampMax = "128000"))
	int32 MaxTokensPerRequest = 16384;

	/** Temperature for AI responses (0.0 = deterministic, 1.0 = creative) */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Temperature = 0.7f;

	/** Request timeout in seconds (Claude with 735 tools needs ~300s) */
	UPROPERTY(config, EditAnywhere, Category = "AI Provider", meta = (ClampMin = "10.0", ClampMax = "600.0"))
	float RequestTimeoutSeconds = 300.0f;

	// ==========================================================================
	// Rate Limiting
	// ==========================================================================

	/** Enable rate limiting for AI requests */
	UPROPERTY(config, EditAnywhere, Category = "Rate Limiting")
	bool bEnableRateLimiting = true;

	/** Requests per minute limit (0 = use provider default) */
	UPROPERTY(config, EditAnywhere, Category = "Rate Limiting", meta = (ClampMin = "0", ClampMax = "1000"))
	int32 RequestsPerMinute = 0;

	/** Enable circuit breaker */
	UPROPERTY(config, EditAnywhere, Category = "Rate Limiting")
	bool bEnableCircuitBreaker = true;

	/** Failures before circuit opens */
	UPROPERTY(config, EditAnywhere, Category = "Rate Limiting", meta = (ClampMin = "1", ClampMax = "20"))
	int32 CircuitBreakerThreshold = 5;

	/** Circuit breaker cooldown in seconds */
	UPROPERTY(config, EditAnywhere, Category = "Rate Limiting", meta = (ClampMin = "10.0", ClampMax = "600.0"))
	float CircuitBreakerCooldown = 60.0f;

	// ==========================================================================
	// Snapshot & Recovery
	// ==========================================================================

	/** Directory for snapshots (relative to Saved/) */
	UPROPERTY(config, EditAnywhere, Category = "Snapshot")
	FString SnapshotDirectory = TEXT("RiftbornAI/Snapshots");

	/** Days to keep snapshots */
	UPROPERTY(config, EditAnywhere, Category = "Snapshot", meta = (ClampMin = "1", ClampMax = "90"))
	int32 SnapshotRetentionDays = 7;

	/** Maximum snapshot size in MB (0 = unlimited) */
	UPROPERTY(config, EditAnywhere, Category = "Snapshot", meta = (ClampMin = "0", ClampMax = "1000"))
	int32 MaxSnapshotSizeMB = 100;

	/** Auto-create snapshot before risky operations */
	UPROPERTY(config, EditAnywhere, Category = "Snapshot")
	bool bAutoSnapshot = true;

	// ==========================================================================
	// Logging & Observability
	// ==========================================================================

	/** Minimum log level for file output */
	UPROPERTY(config, EditAnywhere, Category = "Logging")
	ERiftbornLogLevel LogLevel = ERiftbornLogLevel::Info;

	/** Days to keep log files */
	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (ClampMin = "1", ClampMax = "90"))
	int32 LogRetentionDays = 7;

	/** Mirror structured logs to UE_LOG */
	UPROPERTY(config, EditAnywhere, Category = "Logging")
	bool bMirrorToUELog = true;

	/** Log directory (relative to Saved/Logs/) */
	UPROPERTY(config, EditAnywhere, Category = "Logging")
	FString LogDirectory = TEXT("RiftbornAI");

	/** Health check interval in seconds */
	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (ClampMin = "5.0", ClampMax = "300.0"))
	float HealthCheckIntervalSeconds = 30.0f;

	// ==========================================================================
	// Sandbox & Security
	// ==========================================================================

	/** Enable Python code sandbox */
	UPROPERTY(config, EditAnywhere, Category = "Security")
	bool bEnableSandbox = true;

	/** Python execution timeout in seconds */
	UPROPERTY(config, EditAnywhere, Category = "Security", meta = (ClampMin = "5.0", ClampMax = "300.0"))
	float PythonExecutionTimeoutSeconds = 30.0f;

	/** Additional blocked Python modules (comma-separated) */
	UPROPERTY(config, EditAnywhere, Category = "Security")
	FString AdditionalBlockedModules;

	/** Allow file reading in sandbox */
	UPROPERTY(config, EditAnywhere, Category = "Security")
	bool bSandboxAllowFileRead = true;

	// ==========================================================================
	// UI Settings
	// ==========================================================================

	/** Show health status indicator in Codex window */
	UPROPERTY(config, EditAnywhere, Category = "UI")
	bool bShowHealthIndicator = true;

	/** Show metrics in status bar */
	UPROPERTY(config, EditAnywhere, Category = "UI")
	bool bShowMetricsInStatusBar = false;

	/** Default window width */
	UPROPERTY(config, EditAnywhere, Category = "UI", meta = (ClampMin = "400", ClampMax = "2000"))
	int32 DefaultWindowWidth = 800;

	/** Default window height */
	UPROPERTY(config, EditAnywhere, Category = "UI", meta = (ClampMin = "300", ClampMax = "1500"))
	int32 DefaultWindowHeight = 600;

	// ==========================================================================
	// Updates
	// ==========================================================================

	/** Check GitHub Releases for newer RiftbornAI builds after editor startup settles.
	 *  Default ON: just notification, no download or install action without further opt-in. */
	UPROPERTY(config, EditAnywhere, Category = "Updates")
	bool bEnableAutoUpdateChecks = true;

	/** Include GitHub prereleases in the update feed. Default OFF — beta users
	 *  should opt into prerelease channels explicitly so a poisoned beta tag
	 *  can't auto-replace stable plugin code. */
	UPROPERTY(config, EditAnywhere, Category = "Updates", meta = (EditCondition = "bEnableAutoUpdateChecks"))
	bool bIncludePrereleaseUpdates = false;

	/** Download newer packaged releases automatically after they are detected.
	 *  Default OFF — opt-in only. Auto-downloading from GitHub releases is a
	 *  supply-chain pre-stage; we keep the bytes off disk until the user agrees. */
	UPROPERTY(config, EditAnywhere, Category = "Updates", meta = (EditCondition = "bEnableAutoUpdateChecks"))
	bool bAutoDownloadUpdates = false;

	/** After an update has been downloaded, queue the installer so it applies automatically the next time the editor closes.
	 *  Default OFF — must be explicitly opted into. Even with download approved,
	 *  installation requires another consent step because the installer runs
	 *  PowerShell with -ExecutionPolicy Bypass and replaces the entire plugin
	 *  folder; that's effectively code-execution. Pair with
	 *  bRequireSignedReleaseManifests=true if you flip this on. */
	UPROPERTY(config, EditAnywhere, Category = "Updates", meta = (EditCondition = "bEnableAutoUpdateChecks && bAutoDownloadUpdates"))
	bool bAutoApplyDownloadedUpdateOnNextRestart = false;

	/** Relaunch the current project after the queued updater finishes replacing the plugin.
	 *  Default OFF — relaunch is a user choice, not the updater's. */
	UPROPERTY(config, EditAnywhere, Category = "Updates", meta = (EditCondition = "bEnableAutoUpdateChecks && bAutoDownloadUpdates && bAutoApplyDownloadedUpdateOnNextRestart"))
	bool bAutoRelaunchAfterUpdateInstall = false;

	/** Require a signed release manifest (SHA-256 hashes for each asset, signed
	 *  by the embedded RiftbornAI release key) before any downloaded update
	 *  may be installed. When ON, an asset that is missing from the manifest
	 *  or whose SHA-256 doesn't match is rejected and the install queue is
	 *  cleared. Default ON — flipping this off opts out of the supply-chain
	 *  defense and should require explicit policy approval. */
	UPROPERTY(config, EditAnywhere, Category = "Updates", meta = (EditCondition = "bEnableAutoUpdateChecks"))
	bool bRequireSignedReleaseManifests = true;

	/** Minimum number of hours between automatic release checks. */
	UPROPERTY(config, EditAnywhere, Category = "Updates", meta = (EditCondition = "bEnableAutoUpdateChecks", ClampMin = "1", ClampMax = "168"))
	int32 UpdateCheckIntervalHours = 24;

	/** GitHub repository that publishes packaged RiftbornAI release zips. */
	UPROPERTY(config, EditAnywhere, Category = "Updates", meta = (EditCondition = "bEnableAutoUpdateChecks", AdvancedDisplay))
	FString ReleaseRepository = TEXT("RiftbornGit/RiftbornAI");

	/** Internal state: last UTC time an automatic release check was attempted. */
	UPROPERTY(config)
	FString LastUpdateCheckUtc;

	/** Internal state: newest version the user chose to skip. */
	UPROPERTY(config)
	FString SkippedUpdateVersion;

	/** Internal state: downloaded update version waiting for install. */
	UPROPERTY(config)
	FString PendingDownloadedUpdateVersion;

	/** Internal state: absolute zip path for the downloaded update package. */
	UPROPERTY(config)
	FString PendingDownloadedUpdateZipPath;

	// ==========================================================================
	// Editor Performance
	// ==========================================================================

	/** Enable Blueprint compile-error suggestions in the editor. */
	UPROPERTY(config, EditAnywhere, Category = "Editor Performance")
	bool bEnableBlueprintFixService = true;

	/** Suspend Blueprint compile-error suggestions while the editor is under heavy compile or memory pressure. */
	UPROPERTY(config, EditAnywhere, Category = "Editor Performance", meta = (EditCondition = "bEnableBlueprintFixService"))
	bool bSuspendBlueprintFixServiceDuringHeavyCompiles = true;

	/** Suspend Blueprint compile-error suggestions when the async asset compile backlog reaches this size. */
	UPROPERTY(config, EditAnywhere, Category = "Editor Performance", meta = (EditCondition = "bEnableBlueprintFixService && bSuspendBlueprintFixServiceDuringHeavyCompiles", ClampMin = "1", ClampMax = "5000"))
	int32 BlueprintFixServicePendingCompileThreshold = 128;

	/** Suspend Blueprint compile-error suggestions when editor physical memory use reaches this threshold in GB. */
	UPROPERTY(config, EditAnywhere, Category = "Editor Performance", meta = (EditCondition = "bEnableBlueprintFixService && bSuspendBlueprintFixServiceDuringHeavyCompiles", ClampMin = "1.0", ClampMax = "128.0"))
	float BlueprintFixServiceUsedPhysicalMemoryThresholdGB = 24.0f;

	/** Emit warnings when the current editor session starts triggering expensive asset rebuilds. */
	UPROPERTY(config, EditAnywhere, Category = "Editor Performance")
	bool bEnableEditorLoadWarnings = true;

	/** Warn when a logged mesh rebuild asks for at least this much memory. */
	UPROPERTY(config, EditAnywhere, Category = "Editor Performance", meta = (EditCondition = "bEnableEditorLoadWarnings", ClampMin = "256.0", ClampMax = "65536.0"))
	float HeavyAssetBuildWarningThresholdMB = 2048.0f;

	/** Warn when repeated texture builds for the same context exceed this count in the current session. */
	UPROPERTY(config, EditAnywhere, Category = "Editor Performance", meta = (EditCondition = "bEnableEditorLoadWarnings", ClampMin = "8", ClampMax = "1000"))
	int32 TextureBuildStormWarningThreshold = 96;

	// ==========================================================================
	// Cost Optimization
	// ==========================================================================

	/** Use Lite Mode for lower costs (local keyword routing, shorter prompts). Recommended for general use. */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization")
	bool bLiteMode = true;

	/** Legacy cost optimization path. May use an internal lightweight routing model, but does not expose extra copilot picker options. */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization")
	bool bDualModelMode = true;

	/** Legacy internal routing toggle. This does not add Haiku back to the supported copilot picker. */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization", meta = (EditCondition = "!bLiteMode && !bDualModelMode"))
	bool bUseHaikuRouting = true;

	/** Only send essential tools (spawn, move, delete, python). ~80% fewer input tokens. */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization", meta = (EditCondition = "bLiteMode"))
	bool bMinimalTools = true;

	/** Use compact system prompt. ~50% fewer system tokens. */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization", meta = (EditCondition = "bLiteMode"))
	bool bCompactPrompt = true;

	/** Max conversation turns to keep in context (older turns are dropped). 0 = unlimited. */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization", meta = (ClampMin = "0", ClampMax = "50"))
	int32 MaxConversationTurns = 10;

	/** HARD SESSION SPENDING CAP in USD. Once this amount is reached, ALL Claude API calls are blocked until session reset. 0 = unlimited (DANGEROUS). */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization|Spending Limits", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float SessionSpendingCapUSD = 0.50f;

	/** HARD PER-REQUEST SPENDING CAP in USD. A single agentic loop cannot exceed this. 0 = unlimited. */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization|Spending Limits", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float PerRequestSpendingCapUSD = 0.25f;

	/** Max agentic loop iterations per request (safety valve against runaway loops). */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization|Spending Limits", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxAgenticIterations = 12;

	/** Log estimated cost after each API call */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization|Spending Limits")
	bool bLogCostPerCall = true;

	/** Automatically upgrade local or legacy lightweight selections to stronger supported copilot models for complex requests. */
	UPROPERTY(config, EditAnywhere, Category = "Cost Optimization")
	bool bAutoUpgradeModels = true;

	// ==========================================================================
	// Project Rules — Per-project constraints injected into the AI context.
	// Customers write their own .md rule files to customize AI behavior
	// without modifying plugin code.
	// ==========================================================================

	/** Enable project-specific rules injection into the AI system prompt.
	 *  Rules are loaded from the ProjectRulesDirectory on session start. */
	UPROPERTY(config, EditAnywhere, Category = "Project Rules")
	bool bEnableProjectRules = true;

	/** Directory containing .md rule files. Relative to project root.
	 *  All .md files in this directory (non-recursive) are loaded and injected
	 *  into the system prompt as project constraints.
	 *  Default: Config/RiftbornAI/Rules */
	UPROPERTY(config, EditAnywhere, Category = "Project Rules", meta = (EditCondition = "bEnableProjectRules"))
	FString ProjectRulesDirectory = TEXT("Config/RiftbornAI/Rules");

	/** Directory containing user/studio .md rule files loaded after project rules.
	 *  Relative paths resolve under the user's settings directory. */
	UPROPERTY(config, EditAnywhere, Category = "Project Rules", meta = (EditCondition = "bEnableProjectRules"))
	FString UserRulesDirectory = TEXT("RiftbornAI/Rules");

	/** Maximum total characters of project rules to inject.
	 *  Rules are loaded alphabetically; files that exceed the budget are skipped.
	 *  This prevents accidentally blowing the context window. */
	UPROPERTY(config, EditAnywhere, Category = "Project Rules", meta = (EditCondition = "bEnableProjectRules", ClampMin = "0", ClampMax = "50000"))
	int32 ProjectRulesMaxChars = 16000;

	// ==========================================================================
	// Skill Packs — Authorable markdown playbooks injected when relevant.
	// Bundled plugin packs live under Config/SkillPacks. Projects can add their
	// own packs under the directory below to override or extend them.
	// ==========================================================================

	/** Enable authorable skill/workflow pack injection into the AI prompt. */
	UPROPERTY(config, EditAnywhere, Category = "Skill Packs")
	bool bEnableSkillPacks = true;

	/** Directory containing per-project .md skill packs. Relative to project root.
	 *  Files in this directory override bundled packs with the same specialist or
	 *  pack name. Default: Config/RiftbornAI/Skills */
	UPROPERTY(config, EditAnywhere, Category = "Skill Packs", meta = (EditCondition = "bEnableSkillPacks"))
	FString ProjectSkillPacksDirectory = TEXT("Config/RiftbornAI/Skills");

	/** Directory containing user/studio .md skill packs loaded after project packs.
	 *  Relative paths resolve under the user's settings directory. */
	UPROPERTY(config, EditAnywhere, Category = "Skill Packs", meta = (EditCondition = "bEnableSkillPacks"))
	FString UserSkillPacksDirectory = TEXT("RiftbornAI/SkillPacks");

	/** Maximum total characters of loaded skill-pack content across layers. */
	UPROPERTY(config, EditAnywhere, Category = "Skill Packs", meta = (EditCondition = "bEnableSkillPacks", ClampMin = "0", ClampMax = "50000"))
	int32 SkillPacksMaxChars = 12000;

	/** Maximum number of matching skill packs to inject into one prompt. */
	UPROPERTY(config, EditAnywhere, Category = "Skill Packs", meta = (EditCondition = "bEnableSkillPacks", ClampMin = "1", ClampMax = "8"))
	int32 MaxSkillPacksPerPrompt = 3;

	// ==========================================================================
	// Agent Manifests — Authorable specialist agents for spawn_subagent.
	// Bundled plugin manifests live under Config/Agents. Projects can add their
	// own manifests under the directory below to override or extend them.
	// ==========================================================================

	/** Enable authored agent manifests for specialist subagent routing. */
	UPROPERTY(config, EditAnywhere, Category = "Agent Manifests")
	bool bEnableAgentManifests = true;

	/** Directory containing per-project .md agent manifests. Relative to project root.
	 *  Files in this directory override bundled manifests with the same alias or
	 *  manifest name. Default: Config/RiftbornAI/Agents */
	UPROPERTY(config, EditAnywhere, Category = "Agent Manifests", meta = (EditCondition = "bEnableAgentManifests"))
	FString ProjectAgentManifestsDirectory = TEXT("Config/RiftbornAI/Agents");

	/** Directory containing user/studio .md agent manifests loaded after project manifests.
	 *  Relative paths resolve under the user's settings directory. */
	UPROPERTY(config, EditAnywhere, Category = "Agent Manifests", meta = (EditCondition = "bEnableAgentManifests"))
	FString UserAgentManifestsDirectory = TEXT("RiftbornAI/Agents");

	/** Maximum total characters of loaded agent-manifest content across layers. */
	UPROPERTY(config, EditAnywhere, Category = "Agent Manifests", meta = (EditCondition = "bEnableAgentManifests", ClampMin = "0", ClampMax = "50000"))
	int32 AgentManifestsMaxChars = 12000;

	// ==========================================================================
	// Helper Methods
	// ==========================================================================

	/** Get the effective API key for a provider (checks settings then env var) */
	FString GetEffectiveApiKey(ERiftbornAIProvider Provider) const;

	/** Get provider name as string */
	static FString ProviderToString(ERiftbornAIProvider Provider);

	/** Get Claude model API string from enum */
	static FString GetClaudeModelString(ERiftbornClaudeModel Model);

	/** Get the effective Claude model ID (override takes precedence over enum).
	 *  Trims whitespace from the override; validates against discovered models
	 *  when available. Falls back to the enum mapping on empty/unknown override. */
	FString GetEffectiveClaudeModelId() const;

	/** Get the effective OpenAI model ID (validates against discovered models when available). */
	FString GetEffectiveOpenAIModelId() const;

	/** Get the effective Ollama model ID (validates against discovered models when available). */
	FString GetEffectiveOllamaModelId() const;

	/** Convert log level enum to ERiftbornLogLevel (from RiftbornLogger.h) */
	int32 GetLogLevelValue() const;

	// UDeveloperSettings interface
	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FName GetSectionName() const override { return TEXT("RiftbornAI"); }

#if WITH_EDITOR
	virtual FText GetSectionText() const override { return NSLOCTEXT("RiftbornAI", "SettingsSection", "RiftbornAI"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("RiftbornAI", "SettingsDesc", "Configure RiftbornAI plugin settings"); }
#endif
};
