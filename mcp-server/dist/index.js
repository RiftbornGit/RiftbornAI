#!/usr/bin/env node
/**
 * RiftbornAI MCP Server
 *
 * Model Context Protocol server that bridges VS Code Copilot to Unreal Engine
 * via the RiftbornAI HTTP bridge.
 *
 * This file is the ES-module entry point: it wires singletons together,
 * installs tool handlers, and owns the main() bootstrap. Larger pieces
 * have been split into sibling modules:
 *   - bootstrap.ts          — env parsing + auth token resolution
 *   - response-builders.ts  — safe response builders + sanitizer helpers
 *   - workflow-handlers.ts  — index-level tool handler overrides
 */
import { Server } from "@modelcontextprotocol/sdk/server/index.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import { CallToolRequestSchema, ListToolsRequestSchema, } from "@modelcontextprotocol/sdk/types.js";
import { GENERATED_TOOLS } from "./generated-tools.js";
import { filterToolsByReadiness, getReadinessSummary, getToolReadiness, } from "./tool-readiness.js";
import { buildAllTools, CATEGORY_MAP, MANUAL_TOOL_NAMES } from "./manual-tools.js";
import { registerPromptHandlers } from "./mcp-prompts.js";
import { registerResourceHandlers } from "./mcp-resources.js";
import { getDefaultReadinessTierNames, getDefaultSurfaceStage, getBetaReleaseToolNameSet, } from "./surface-manifest.js";
import { createToolHandlers } from "./tool-handlers.js";
import { retryableHttpRequest, ToolExecutionQueue, sanitizeParams, BridgeHealthMonitor, classifyTimeout, } from "./bridge-reliability.js";
import { ReadCache, SessionTracker } from "./system-enhancements.js";
import { ToolRouter, clampToolSearchResults, normalizeToolSearchQuery, ContextPropagator } from "./pipeline-intelligence.js";
import { compressToolListing } from "./tool-compression.js";
import { SchemaIntelligence } from "./schema-intelligence.js";
import { ToolResolver } from "./tool-resolution.js";
import { SceneDiffTracker } from "./vision-intelligence.js";
import { ProgressTracker } from "./session-intelligence.js";
import { SceneChangeLog } from "./scene-safety.js";
import { LatencyTracker } from "./performance-intelligence.js";
import { AdaptiveThrottle } from "./adaptive-throttle.js";
import { SessionPersistence } from "./session-persistence.js";
import { SessionBookmarks, ResponseDeltaTracker, FailureBudget, } from "./pipeline-refinements.js";
import { PipelineTelemetry } from "./pipeline-telemetry.js";
import { PipelineTraceStore } from "./pipeline-trace.js";
import * as path from "path";
import { fileURLToPath } from "url";
import { loadBootstrapConfig } from "./bootstrap.js";
import { buildSafeCatchResponse, buildSafePlanWorkflowResponse, } from "./response-builders.js";
import { installWorkflowHandlers } from "./workflow-handlers.js";
import { createManagedDispatcher } from "./dispatcher.js";
import { getGovernedRouteTimeoutMs, getToolRiskTier, GovernedExecutionClient, requiresGovernedExecution, } from "./governed-execution.js";
// Re-export safe response builders so existing importers (index.test.ts)
// continue to work unchanged.
export { buildSafeCatchResponse, buildSafePlanWorkflowResponse };
// ESM-compatible __dirname
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
// ─── Configuration + auth resolution ──────────────────────────────────────────
const BOOT = loadBootstrapConfig(__filename);
const { RIFTBORN_HTTP_PORT, RIFTBORN_TCP_PORT, RIFTBORN_BRAIN_PORT, RIFTBORN_HOST, RIFTBORN_AUTH_TOKEN, ENABLE_INTERNAL_TOOLS, ALLOW_HIDDEN_TOOLS, DEV_MODE, INTERNAL_ONLY_TOOLS, BLOCKED_TOOLS, } = BOOT;
// =================== BRIDGE RELIABILITY ===================
const bridgeConfig = {
    host: RIFTBORN_HOST,
    httpPort: RIFTBORN_HTTP_PORT,
    authToken: RIFTBORN_AUTH_TOKEN,
};
// Probe bridge auth mode: if auth is disabled, don't send tokens
// (avoids RBAC rejection when token isn't registered in current UE session)
async function probeBridgeAuth() {
    if (!bridgeConfig.authToken)
        return;
    try {
        const url = `http://${bridgeConfig.host}:${bridgeConfig.httpPort}/riftborn/tool`;
        const resp = await fetch(url, {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                Authorization: `Bearer ${bridgeConfig.authToken}`,
            },
            body: JSON.stringify({ tool: "get_current_level", args: {} }),
            signal: AbortSignal.timeout(5000),
        });
        if (resp.status === 403) {
            const noAuth = await fetch(url, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ tool: "get_current_level", args: {} }),
                signal: AbortSignal.timeout(5000),
            });
            if (noAuth.ok) {
                bridgeConfig.authDisabled = true;
                console.error("[RiftbornAI] Bridge auth is disabled — skipping token to avoid RBAC rejection.");
            }
        }
    }
    catch {
        // Bridge not reachable yet — will be handled by health monitor
    }
}
probeBridgeAuth();
const governedExecutionClient = new GovernedExecutionClient(bridgeConfig);
// Execution queue: reads in parallel (max 3), mutations serial
const toolQueue = new ToolExecutionQueue();
// Health monitor: polls /riftborn/health every 30s, queues calls when disconnected
const healthMonitor = new BridgeHealthMonitor(bridgeConfig, async (toolName, params) => executeToolRouted(toolName, params));
// =================== SYSTEM / PIPELINE SINGLETONS ===================
const readCache = new ReadCache();
const sessionTracker = new SessionTracker();
const contextPropagator = new ContextPropagator();
const sceneDiffTracker = new SceneDiffTracker();
const progressTracker = new ProgressTracker();
let sessionCallCount = 0;
const DIGEST_INTERVAL = 10;
const sceneChangeLog = new SceneChangeLog();
const latencyTracker = new LatencyTracker();
const adaptiveThrottle = new AdaptiveThrottle();
const sessionBookmarks = new SessionBookmarks();
const failureBudget = new FailureBudget();
const responseDeltaTracker = new ResponseDeltaTracker();
const pipelineTelemetry = new PipelineTelemetry();
const pipelineTraceStore = new PipelineTraceStore();
const sessionPersistence = new SessionPersistence({
    contextPropagator,
    sessionTracker,
    sceneChangeLog,
    progressTracker,
});
let schemaIntel = null;
let toolResolver = null;
/**
 * Make HTTP request to RiftbornAI HTTP bridge.
 * Uses RetryableHttpRequest with exponential backoff (1s, 3s, 8s).
 */
async function httpRequest(method, path, body, timeoutMs) {
    return retryableHttpRequest(bridgeConfig, method, path, body, timeoutMs ?? 30_000);
}
/**
 * Execute a tool via HTTP bridge — direct call (no queue, no sanitization).
 * Used internally by the health monitor replay path.
 */
async function executeToolDirect(toolName, params) {
    const timeout = classifyTimeout(toolName);
    return httpRequest("POST", "/riftborn/tool", { tool: toolName, params }, timeout);
}
/**
 * Execute a tool through the correct bridge lane.
 * Safe reads stay on /riftborn/tool; mutating tools use governed /riftborn/agent/step.
 */
async function executeToolRouted(toolName, params) {
    if (!requiresGovernedExecution(toolName)) {
        return executeToolDirect(toolName, params);
    }
    return governedExecutionClient.executeTool(toolName, params, getToolRiskTier(toolName), classifyTimeout(toolName));
}
/**
 * Execute a tool via HTTP bridge.
 * Applies ParameterSanitizer, routes through ToolExecutionQueue,
 * and respects BridgeHealthMonitor status.
 */
/** Session-level license cache. Checked once per session; cleared on 403. */
let _licenseChecked = false;
let _licenseValid = true;
async function ensureLicensed() {
    if (_licenseChecked && _licenseValid)
        return true;
    if (_licenseChecked && !_licenseValid)
        return false;
    // Probe the bridge with a lightweight read-only tool.
    // If the bridge returns 403 license_required, cache the rejection.
    try {
        const probe = await executeToolDirect("get_error_summary", {});
        if (!probe.ok && typeof probe.error === "string" && probe.error.includes("license")) {
            _licenseChecked = true;
            _licenseValid = false;
            return false;
        }
        _licenseChecked = true;
        _licenseValid = true;
        return true;
    }
    catch {
        // Bridge unreachable — let the normal health monitor handle it
        return true;
    }
}
async function executeTool(toolName, params = {}) {
    // LICENSE GATE — fast reject if bridge already told us license is invalid
    if (_licenseChecked && !_licenseValid) {
        return {
            ok: false,
            error: "RiftbornAI license required. Install a valid license to use tools.",
        };
    }
    const status = healthMonitor.bridgeStatus();
    if (status === "disconnected") {
        const sanitized = sanitizeParams(toolName, params);
        const queued = healthMonitor.queueForReplay(toolName, sanitized);
        if (queued) {
            return queued;
        }
        return {
            ok: false,
            error: "Bridge is disconnected and replay queue is full. Ensure Unreal Editor is running with RiftbornAI plugin.",
        };
    }
    const sanitized = sanitizeParams(toolName, params);
    if (readCache.isCacheable(toolName)) {
        const cached = readCache.get(toolName, sanitized);
        if (cached)
            return cached;
    }
    else {
        readCache.invalidate();
    }
    const result = await toolQueue.enqueue(toolName, () => executeToolRouted(toolName, sanitized), getGovernedRouteTimeoutMs(toolName, classifyTimeout(toolName)));
    // Detect license rejection mid-session (e.g. license revoked while running)
    if (!result.ok && typeof result.error === "string" && result.error.includes("license")) {
        _licenseChecked = true;
        _licenseValid = false;
    }
    if (readCache.isCacheable(toolName) && result.ok) {
        readCache.set(toolName, sanitized, result);
    }
    return result;
}
const ALL_TOOLS = buildAllTools();
const SHIPPABLE_TOOLS = ALL_TOOLS.filter((tool) => {
    if (BLOCKED_TOOLS.has(tool.name)) {
        return false;
    }
    if (!ENABLE_INTERNAL_TOOLS && INTERNAL_ONLY_TOOLS.has(tool.name)) {
        return false;
    }
    return true;
});
const GENERATED_TOOL_NAMES = new Set(GENERATED_TOOLS.map(t => t.name));
// ─── Tool Readiness Gate ──────────────────────────────────────────────────────
const ALLOWED_TIERS = (process.env.RIFTBORN_TOOL_TIERS || getDefaultReadinessTierNames().join(",")).split(",").map(s => s.trim());
// Build the handler-name set once so the readiness heuristic can check it.
let HANDLER_NAME_SET = new Set();
// Lazily computed visible tools — populated after TOOL_HANDLERS is built.
let VISIBLE_TOOLS = [];
let VISIBLE_TOOL_NAMES = new Set();
function getToolCatalog() {
    const visible = new Set(VISIBLE_TOOLS.map((tool) => tool.name));
    return SHIPPABLE_TOOLS.map((tool) => {
        const entry = getToolReadiness(tool.name, HANDLER_NAME_SET, MANUAL_TOOL_NAMES, GENERATED_TOOL_NAMES);
        return {
            name: tool.name,
            description: tool.description || "",
            tier: entry.tier,
            visible: visible.has(tool.name),
        };
    });
}
function initReadinessGate() {
    const suppressReadinessLogs = process.argv.includes("--self-test")
        || process.env.RIFTBORN_SUPPRESS_READINESS_LOGS === "true";
    HANDLER_NAME_SET = new Set(Object.keys(TOOL_HANDLERS));
    VISIBLE_TOOLS = filterToolsByReadiness(SHIPPABLE_TOOLS, ALLOWED_TIERS, HANDLER_NAME_SET, MANUAL_TOOL_NAMES, GENERATED_TOOL_NAMES);
    // ── BETA-RELEASE LOCK ──
    // When the surface stage is `beta_release` (the public-Beta build), apply
    // a hard intersection with the curated beta-release list. End users see only
    // those names, period — even if the readiness tier filter let other
    // PRODUCTION tools through. RIFTBORN_DEV_MODE bypasses the lock so the
    // team can keep all ~700 tools while developing on top of the plugin.
    const stage = (process.env.RIFTBORN_SURFACE_STAGE || getDefaultSurfaceStage());
    const betaReleaseLockActive = stage === "beta_release" && !DEV_MODE;
    if (betaReleaseLockActive) {
        const BetaReleaseTools = getBetaReleaseToolNameSet();
        const before = VISIBLE_TOOLS.length;
        VISIBLE_TOOLS = VISIBLE_TOOLS.filter(t => BetaReleaseTools.has(t.name));
        if (!suppressReadinessLogs) {
            console.error(`[RiftbornAI]   BETA RELEASE LOCK: filtered ${before} → ${VISIBLE_TOOLS.length} (must equal ${BetaReleaseTools.size})`);
        }
        if (VISIBLE_TOOLS.length !== BetaReleaseTools.size) {
            // Loud failure — either a beta-release tool isn't in the registry yet, or
            // its readiness tier got demoted. The surface lock test catches this
            // pre-merge, but this runtime check is the last line of defense.
            const visibleSet = new Set(VISIBLE_TOOLS.map(t => t.name));
            const missing = [...BetaReleaseTools].filter(n => !visibleSet.has(n));
            console.error(`[RiftbornAI]   BETA RELEASE LOCK MISSING: ${missing.join(", ") || "(none)"}`);
        }
    }
    VISIBLE_TOOL_NAMES = new Set(VISIBLE_TOOLS.map(t => t.name));
    const summary = getReadinessSummary(SHIPPABLE_TOOLS, HANDLER_NAME_SET, MANUAL_TOOL_NAMES, GENERATED_TOOL_NAMES);
    const betaReleaseLockSuffix = betaReleaseLockActive ? ` (${getBetaReleaseToolNameSet().size}-tool beta lock active)` : "";
    if (!suppressReadinessLogs) {
        console.error(`[RiftbornAI] Tool Readiness Gate:`);
        console.error(`[RiftbornAI]   Surface stage: ${stage}${betaReleaseLockSuffix}`);
        console.error(`[RiftbornAI]   Dev mode (full surface): ${DEV_MODE}`);
        console.error(`[RiftbornAI]   Allowed tiers: ${ALLOWED_TIERS.join(", ")}`);
        console.error(`[RiftbornAI]   Generated tools exposed: ${process.env.RIFTBORN_EXPOSE_GENERATED_TOOLS === "true"}`);
        console.error(`[RiftbornAI]   Hidden tool calls allowed: ${ALLOW_HIDDEN_TOOLS}`);
        console.error(`[RiftbornAI]   Internal tools enabled: ${ENABLE_INTERNAL_TOOLS}`);
        console.error(`[RiftbornAI]   PRODUCTION:   ${summary.PRODUCTION}`);
        console.error(`[RiftbornAI]   BETA:         ${summary.BETA}`);
        console.error(`[RiftbornAI]   EXPERIMENTAL: ${summary.EXPERIMENTAL}`);
        console.error(`[RiftbornAI]   STUB:         ${summary.STUB}`);
        console.error(`[RiftbornAI]   DEPRECATED:   ${summary.DEPRECATED}`);
        console.error(`[RiftbornAI]   Visible: ${VISIBLE_TOOLS.length} / ${SHIPPABLE_TOOLS.length} total`);
    }
}
const TOOL_HANDLERS = createToolHandlers({
    executeTool,
    dispatchTool: (toolName, params) => dispatchManagedTool(toolName, params ?? {}),
    httpRequest,
    host: RIFTBORN_HOST,
    httpPort: RIFTBORN_HTTP_PORT,
});
function parseCliOptions(argv) {
    const args = new Set(argv);
    return {
        showHelp: args.has("--help") || args.has("-h"),
        showVersion: args.has("--version") || args.has("-v"),
        selfTest: args.has("--self-test"),
        json: args.has("--json"),
    };
}
function printCliHelp() {
    console.log(`RiftbornAI MCP Server\n\nUsage:\n  node dist/index.js              Start the stdio MCP server\n  node dist/index.js --self-test  Validate the packaged runtime and exit\n  node dist/index.js --version    Print version and exit\n  node dist/index.js --help       Show this help\n`);
}
function buildSelfTestResult() {
    initReadinessGate();
    const stage = process.env.RIFTBORN_SURFACE_STAGE || getDefaultSurfaceStage();
    const pluginRoot = path.resolve(__dirname, "../..");
    return {
        ok: true,
        version: "2.1.0",
        node: process.version,
        entry: __filename,
        plugin_root: pluginRoot,
        cwd: process.cwd(),
        surface_stage: stage,
        allowed_tiers: ALLOWED_TIERS,
        auth_token_present: Boolean(RIFTBORN_AUTH_TOKEN),
        shippable_tools: SHIPPABLE_TOOLS.length,
        visible_tools: VISIBLE_TOOLS.length,
        beta_release_tools: getBetaReleaseToolNameSet().size,
        bridge_host: RIFTBORN_HOST,
        bridge_http_port: RIFTBORN_HTTP_PORT,
        bridge_tcp_port: RIFTBORN_TCP_PORT,
    };
}
function requireSchemaIntel() {
    if (!schemaIntel) {
        throw new Error("Schema intelligence is not initialized.");
    }
    return schemaIntel;
}
function requireToolResolver() {
    if (!toolResolver) {
        throw new Error("Tool resolver is not initialized.");
    }
    return toolResolver;
}
const dispatchManagedTool = createManagedDispatcher({
    toolHandlers: TOOL_HANDLERS,
    generatedToolNames: GENERATED_TOOL_NAMES,
    blockedTools: BLOCKED_TOOLS,
    internalOnlyTools: INTERNAL_ONLY_TOOLS,
    getVisibleToolNames: () => VISIBLE_TOOL_NAMES,
    enableInternalTools: ENABLE_INTERNAL_TOOLS,
    allowHiddenTools: ALLOW_HIDDEN_TOOLS,
    executeTool,
    executeToolDirect,
    requireSchemaIntel,
    requireToolResolver,
    contextPropagator,
    sceneDiffTracker,
    progressTracker,
    sceneChangeLog,
    latencyTracker,
    adaptiveThrottle,
    failureBudget,
    responseDeltaTracker,
    pipelineTelemetry,
    pipelineTraceStore,
    sessionTracker,
    sessionPersistence,
    getCallCount: () => sessionCallCount,
    incrementCallCount: () => ++sessionCallCount,
    digestInterval: DIGEST_INTERVAL,
});
async function main() {
    // ── Restore persisted session state (if fresh) ──
    const restoreResult = sessionPersistence.tryRestore();
    if (restoreResult.restored) {
        console.error(`[RiftbornAI] Session state restored (age: ${Math.round(restoreResult.age_ms / 1000)}s)`);
    }
    else {
        console.error(`[RiftbornAI] No session state to restore (${restoreResult.reason})`);
    }
    // ── Startup health check ──
    const healthCheck = await httpRequest("GET", "/riftborn/health");
    if (healthCheck.ok) {
        console.error(`[RiftbornAI] Bridge connected (HTTP :${RIFTBORN_HTTP_PORT})`);
    }
    else {
        console.error("[RiftbornAI] Bridge NOT responding on port " + RIFTBORN_HTTP_PORT);
        console.error("[RiftbornAI]     Tools will fail until Unreal Editor is running with RiftbornAI plugin.");
        console.error("[RiftbornAI]     Check: curl http://127.0.0.1:" + RIFTBORN_HTTP_PORT + "/riftborn/health");
    }
    // ── Start bridge health monitor (polls every 30s, queues tools when disconnected) ──
    healthMonitor.start();
    console.error("[RiftbornAI] Bridge health monitor started (30s poll interval)");
    // Create MCP server
    const server = new Server({
        name: "riftborn-mcp",
        version: "2.1.0",
    }, {
        capabilities: {
            tools: {},
            resources: {},
            prompts: {},
        },
    });
    registerResourceHandlers(server, {
        allTools: SHIPPABLE_TOOLS,
        visibleTools: () => VISIBLE_TOOLS,
        getToolCatalog,
        categoryMap: CATEGORY_MAP,
        executeTool,
        httpRequest,
        host: RIFTBORN_HOST,
        httpPort: RIFTBORN_HTTP_PORT,
        tcpPort: RIFTBORN_TCP_PORT,
        brainPort: RIFTBORN_BRAIN_PORT,
        sessionTracker,
    });
    registerPromptHandlers(server);
    // =================== TOOLS ===================
    // Schema intelligence: type coercion + local validation
    schemaIntel = new SchemaIntelligence(ALL_TOOLS);
    // Tool resolver: fuzzy name resolution for wrong tool names
    toolResolver = new ToolResolver(ALL_TOOLS);
    // Install index-level tool handler overrides (workflow, plan, verify,
    // bookmarks, telemetry, trace, rollback).
    installWorkflowHandlers({
        toolHandlers: TOOL_HANDLERS,
        dispatchManagedTool,
        sessionTracker,
        contextPropagator,
        sceneChangeLog,
        progressTracker,
        sessionBookmarks,
        pipelineTelemetry,
        pipelineTraceStore,
    });
    // Initialize readiness after installing index-level overrides so manual
    // tools that only exist in index.ts are visible to the production surface.
    initReadinessGate();
    // Tool router: powers find_tools for keyword-based discovery.
    const toolRouter = new ToolRouter(VISIBLE_TOOLS);
    TOOL_HANDLERS["find_tools"] = async (args) => {
        const results = toolRouter.search(normalizeToolSearchQuery(String(args.query || "")), clampToolSearchResults(Number(args.max_results) || 20));
        return { ok: true, result: { matches: results.length, tools: results } };
    };
    // Compress tool listings to save ~40% of the ListTools payload.
    const COMPRESSED_TOOLS = compressToolListing(VISIBLE_TOOLS);
    // List tools handler
    server.setRequestHandler(ListToolsRequestSchema, async () => ({
        tools: COMPRESSED_TOOLS,
    }));
    // Call tool handler — dispatch through the managed execution path
    server.setRequestHandler(CallToolRequestSchema, async (request) => {
        const { name, arguments: args } = request.params;
        const result = await dispatchManagedTool(name, (args || {}));
        return {
            content: [
                {
                    type: "text",
                    text: JSON.stringify(result, null, 2),
                },
            ],
            isError: !result.ok,
        };
    });
    // Start server
    const transport = new StdioServerTransport();
    await server.connect(transport);
    // Graceful shutdown
    const shutdown = async () => {
        try {
            sessionPersistence.saveNow();
        }
        catch { /* best-effort */ }
        try {
            healthMonitor.stop();
        }
        catch { /* best-effort */ }
        process.exit(0);
    };
    process.on("SIGTERM", shutdown);
    process.on("SIGINT", shutdown);
    console.error("RiftbornAI MCP Server started");
}
process.on("unhandledRejection", (reason) => {
    console.error("[RiftbornAI] Unhandled rejection:", reason);
});
const isMainModule = process.argv[1]
    ? path.resolve(process.argv[1]) === __filename
    : false;
if (isMainModule) {
    const cli = parseCliOptions(process.argv.slice(2));
    if (cli.showHelp) {
        printCliHelp();
        process.exit(0);
    }
    if (cli.showVersion) {
        console.log("2.1.0");
        process.exit(0);
    }
    if (cli.selfTest) {
        try {
            const result = buildSelfTestResult();
            if (cli.json) {
                console.log(JSON.stringify(result));
            }
            else {
                console.log("RiftbornAI MCP self-test OK");
                console.log(`  Version: ${result.version}`);
                console.log(`  Visible tools: ${result.visible_tools}`);
                console.log(`  Plugin root: ${result.plugin_root}`);
            }
            process.exit(0);
        }
        catch (err) {
            console.error("[RiftbornAI] Self-test failed:", err);
            process.exit(1);
        }
    }
    main().catch((err) => {
        console.error("[RiftbornAI] Fatal startup error:", err);
        process.exit(1);
    });
}
//# sourceMappingURL=index.js.map