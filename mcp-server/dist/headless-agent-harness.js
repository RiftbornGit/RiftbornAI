import * as crypto from "node:crypto";
import * as fs from "node:fs";
import * as path from "node:path";
import { fileURLToPath } from "node:url";
import { AdaptiveThrottle, } from "./adaptive-throttle.js";
import { resolveBridgeAuthToken } from "./auth-token.js";
import { retryableHttpRequest, } from "./bridge-reliability.js";
import { createManagedDispatcher } from "./dispatcher.js";
import { GENERATED_TOOLS } from "./generated-tools.js";
import { canonicalArgsJson } from "./governed-execution.js";
import { buildAllTools, MANUAL_TOOL_NAMES } from "./manual-tools.js";
import { PipelineTelemetry } from "./pipeline-telemetry.js";
import { PipelineTraceStore } from "./pipeline-trace.js";
import { ContextPropagator, ToolRouter, clampToolSearchResults, normalizeToolSearchQuery, } from "./pipeline-intelligence.js";
import { FailureBudget, ResponseDeltaTracker, SessionBookmarks, } from "./pipeline-refinements.js";
import { sanitizeIndexValue, toSafeRecord } from "./response-builders.js";
import { SceneChangeLog } from "./scene-safety.js";
import { SceneDiffTracker } from "./vision-intelligence.js";
import { createToolHandlers } from "./tool-handlers.js";
import { filterToolsByReadiness, getToolReadiness, } from "./tool-readiness.js";
import { ToolResolver } from "./tool-resolution.js";
import { LatencyTracker } from "./performance-intelligence.js";
import { ProgressTracker } from "./session-intelligence.js";
import { SchemaIntelligence } from "./schema-intelligence.js";
import { SessionTracker, } from "./system-enhancements.js";
import { getBlockedToolNameSet, getInternalToolNameSet, } from "./surface-manifest.js";
import { installWorkflowHandlers } from "./workflow-handlers.js";
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const PLUGIN_ROOT = path.resolve(__dirname, "../..");
const DEFAULT_OUTPUT_DIR = path.resolve(PLUGIN_ROOT, "artifacts/headless-harness");
const DEFAULT_MAX_ITERATIONS = 8;
const DEFAULT_OPENAI_MODEL = "gpt-5.4";
const OPENAI_RESPONSES_URL = "https://api.openai.com/v1/responses";
const OPENAI_TIMEOUT_MS = 120_000;
const DEFAULT_HARNESS_TOOL_TIERS = ["PRODUCTION", "BETA"];
const DEFAULT_SUBAGENT_TIMEOUT_SECONDS = 120;
const DEFAULT_HARNESS_INSTRUCTIONS = "You are RiftbornAI running in a headless harness. " +
    "No Unreal Editor is running. The provided tools are the real governed RiftbornAI tool surface, " +
    "but any Unreal-facing results are mocked and should be treated as authoritative for this run. " +
    "Use tools before answering when you need evidence. " +
    "In your final answer, explain which tool path you took, what you concluded, and any flaws, missing information, or uncertainties you found.";
function cloneStructured(value) {
    return sanitizeIndexValue(value);
}
function normalizeToolNameList(value) {
    if (!Array.isArray(value)) {
        return [];
    }
    const seen = new Set();
    const names = [];
    for (const entry of value) {
        if (typeof entry !== "string") {
            continue;
        }
        const trimmed = entry.trim();
        if (!trimmed || seen.has(trimmed)) {
            continue;
        }
        seen.add(trimmed);
        names.push(trimmed);
    }
    return names;
}
function normalizeStringList(value) {
    if (!Array.isArray(value)) {
        return [];
    }
    const seen = new Set();
    const entries = [];
    for (const entry of value) {
        if (typeof entry !== "string") {
            continue;
        }
        const trimmed = entry.trim();
        if (!trimmed || seen.has(trimmed)) {
            continue;
        }
        seen.add(trimmed);
        entries.push(trimmed);
    }
    return entries;
}
function normalizeRecord(value) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
        return undefined;
    }
    return toSafeRecord(value);
}
function stableKey(value) {
    return canonicalArgsJson((normalizeRecord(value) ?? {}));
}
function clampIterations(value) {
    if (typeof value !== "number" || !Number.isFinite(value)) {
        return DEFAULT_MAX_ITERATIONS;
    }
    return Math.min(Math.max(1, Math.trunc(value)), 50);
}
function normalizeIterationOverride(value) {
    if (typeof value !== "number" || !Number.isFinite(value)) {
        return undefined;
    }
    return Math.min(Math.max(1, Math.trunc(value)), 50);
}
function normalizePositiveInt(value) {
    if (typeof value !== "number" || !Number.isFinite(value)) {
        return undefined;
    }
    const normalized = Math.trunc(value);
    return normalized > 0 ? normalized : undefined;
}
function normalizeNonNegativeInt(value) {
    if (typeof value !== "number" || !Number.isFinite(value)) {
        return undefined;
    }
    const normalized = Math.trunc(value);
    return normalized >= 0 ? normalized : undefined;
}
function normalizeSubagentTimeoutSeconds(value) {
    if (typeof value !== "number" || !Number.isFinite(value)) {
        return DEFAULT_SUBAGENT_TIMEOUT_SECONDS;
    }
    return Math.min(Math.max(5, Math.trunc(value)), 600);
}
async function withTimeout(promise, timeoutMs, message) {
    let timer;
    try {
        return await Promise.race([
            promise,
            new Promise((_resolve, reject) => {
                timer = setTimeout(() => reject(new Error(message)), timeoutMs);
                timer.unref?.();
            }),
        ]);
    }
    finally {
        if (timer) {
            clearTimeout(timer);
        }
    }
}
function parseSubagentProfiles(raw) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        return undefined;
    }
    const profiles = Object.entries(toSafeRecord(raw))
        .map(([name, value]) => {
        const trimmedName = name.trim();
        if (!trimmedName) {
            return null;
        }
        return [trimmedName, parseSubagentProfile(value, trimmedName)];
    })
        .filter((entry) => entry !== null);
    if (profiles.length === 0) {
        return undefined;
    }
    return Object.fromEntries(profiles);
}
function parseSubagentProfile(raw, profileName) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        throw new Error(`subagent_profiles.${profileName} must be an object.`);
    }
    const value = toSafeRecord(raw);
    const toolTiers = Array.isArray(value.tool_tiers)
        ? value.tool_tiers
            .filter((entry) => (entry === "PRODUCTION"
            || entry === "BETA"
            || entry === "EXPERIMENTAL"
            || entry === "STUB"
            || entry === "DEPRECATED"))
        : undefined;
    const mockTools = Array.isArray(value.mock_tools)
        ? value.mock_tools.map((entry, index) => parseMockTool(entry, index))
        : undefined;
    const mockHttp = Array.isArray(value.mock_http)
        ? value.mock_http.map((entry, index) => parseMockHttp(entry, index))
        : undefined;
    const toolNames = normalizeToolNameList(value.tool_names);
    return {
        instructions: typeof value.instructions === "string" ? value.instructions.trim() || undefined : undefined,
        model: typeof value.model === "string" ? value.model.trim() || undefined : undefined,
        max_iterations: normalizeIterationOverride(value.max_iterations),
        max_output_tokens: normalizePositiveInt(value.max_output_tokens),
        reasoning_effort: typeof value.reasoning_effort === "string" ? value.reasoning_effort.trim() || undefined : undefined,
        tool_tiers: toolTiers && toolTiers.length > 0 ? toolTiers : undefined,
        tool_names: toolNames.length > 0 ? toolNames : undefined,
        mock_tools: mockTools,
        mock_http: mockHttp,
    };
}
function parseAgentTaskRuntime(raw) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        return undefined;
    }
    const value = toSafeRecord(raw);
    const executionMode = value.execution_mode === "deferred"
        ? "deferred"
        : value.execution_mode === "immediate"
            ? "immediate"
            : undefined;
    const progressionMode = value.progression_mode === "time"
        ? "time"
        : value.progression_mode === "poll"
            ? "poll"
            : undefined;
    const initialStatus = value.initial_status === "queued"
        ? "queued"
        : value.initial_status === "running"
            ? "running"
            : undefined;
    return {
        execution_mode: executionMode,
        progression_mode: progressionMode,
        complete_after_polls: normalizePositiveInt(value.complete_after_polls),
        start_after_ms: normalizeNonNegativeInt(value.start_after_ms),
        complete_after_ms: normalizePositiveInt(value.complete_after_ms),
        initial_status: initialStatus,
    };
}
function parseBenchmarkToolPathExpectation(raw) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        return undefined;
    }
    const value = toSafeRecord(raw);
    const requiredTools = normalizeToolNameList(value.required_tools);
    const orderedSubsequence = normalizeToolNameList(value.ordered_subsequence);
    const forbiddenTools = normalizeToolNameList(value.forbidden_tools);
    return {
        required_tools: requiredTools.length > 0 ? requiredTools : undefined,
        ordered_subsequence: orderedSubsequence.length > 0 ? orderedSubsequence : undefined,
        forbidden_tools: forbiddenTools.length > 0 ? forbiddenTools : undefined,
        ideal_total_calls: normalizePositiveInt(value.ideal_total_calls),
        max_total_calls: normalizePositiveInt(value.max_total_calls),
    };
}
function parseBenchmarkDelegationExpectation(raw) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        return undefined;
    }
    const value = toSafeRecord(raw);
    const expectedMode = value.expected_mode === "direct"
        || value.expected_mode === "subagent"
        || value.expected_mode === "agent_task"
        ? value.expected_mode
        : undefined;
    const childRequired = normalizeToolNameList(value.child_required_tools);
    const childOrdered = normalizeToolNameList(value.child_ordered_subsequence);
    const childForbidden = normalizeToolNameList(value.child_forbidden_tools);
    return {
        expected_mode: expectedMode,
        child_required_tools: childRequired.length > 0 ? childRequired : undefined,
        child_ordered_subsequence: childOrdered.length > 0 ? childOrdered : undefined,
        child_forbidden_tools: childForbidden.length > 0 ? childForbidden : undefined,
    };
}
function parseBenchmarkFinalOutputExpectation(raw) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        return undefined;
    }
    const value = toSafeRecord(raw);
    const requiredPhrases = normalizeStringList(value.required_phrases);
    const forbiddenPhrases = normalizeStringList(value.forbidden_phrases);
    return {
        required_phrases: requiredPhrases.length > 0 ? requiredPhrases : undefined,
        forbidden_phrases: forbiddenPhrases.length > 0 ? forbiddenPhrases : undefined,
    };
}
function parseBenchmarkExpectations(raw) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        return undefined;
    }
    const value = toSafeRecord(raw);
    const toolPath = parseBenchmarkToolPathExpectation(value.tool_path);
    const delegation = parseBenchmarkDelegationExpectation(value.delegation);
    const finalOutput = parseBenchmarkFinalOutputExpectation(value.final_output);
    const passThreshold = typeof value.pass_threshold === "number" && Number.isFinite(value.pass_threshold)
        ? Math.min(Math.max(value.pass_threshold, 0), 1)
        : undefined;
    if (!toolPath && !delegation && !finalOutput && passThreshold === undefined) {
        return undefined;
    }
    return {
        pass_threshold: passThreshold,
        tool_path: toolPath,
        delegation,
        final_output: finalOutput,
    };
}
export function parseHeadlessHarnessScenario(raw) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        throw new Error("Harness scenario must be a JSON object.");
    }
    const value = toSafeRecord(raw);
    const prompt = typeof value.prompt === "string" ? value.prompt.trim() : "";
    if (!prompt) {
        throw new Error("Harness scenario requires a non-empty 'prompt' string.");
    }
    const toolNames = normalizeToolNameList(value.tool_names);
    if (toolNames.length === 0) {
        throw new Error("Harness scenario requires a non-empty 'tool_names' array.");
    }
    const toolTiers = Array.isArray(value.tool_tiers)
        ? value.tool_tiers
            .filter((entry) => (entry === "PRODUCTION"
            || entry === "BETA"
            || entry === "EXPERIMENTAL"
            || entry === "STUB"
            || entry === "DEPRECATED"))
        : undefined;
    const mockTools = Array.isArray(value.mock_tools)
        ? value.mock_tools.map((entry, index) => parseMockTool(entry, index))
        : undefined;
    const mockHttp = Array.isArray(value.mock_http)
        ? value.mock_http.map((entry, index) => parseMockHttp(entry, index))
        : undefined;
    const subagentProfiles = parseSubagentProfiles(value.subagent_profiles);
    return {
        name: typeof value.name === "string" ? value.name.trim() || undefined : undefined,
        prompt,
        instructions: typeof value.instructions === "string" ? value.instructions.trim() || undefined : undefined,
        model: typeof value.model === "string" ? value.model.trim() || undefined : undefined,
        max_iterations: clampIterations(value.max_iterations),
        max_output_tokens: normalizePositiveInt(value.max_output_tokens),
        reasoning_effort: typeof value.reasoning_effort === "string" ? value.reasoning_effort.trim() || undefined : undefined,
        tool_tiers: toolTiers && toolTiers.length > 0 ? toolTiers : undefined,
        tool_names: toolNames,
        bridge_mode: value.bridge_mode === "live" ? "live" : "mock",
        bridge_host: typeof value.bridge_host === "string" ? value.bridge_host.trim() || undefined : undefined,
        bridge_http_port: normalizePositiveInt(value.bridge_http_port),
        mock_tools: mockTools,
        mock_http: mockHttp,
        max_subagent_depth: normalizePositiveInt(value.max_subagent_depth),
        subagent_profiles: subagentProfiles,
        agent_task_profile: value.agent_task_profile
            ? parseSubagentProfile(value.agent_task_profile, "agent_task_profile")
            : undefined,
        agent_task_runtime: parseAgentTaskRuntime(value.agent_task_runtime),
        benchmark: parseBenchmarkExpectations(value.benchmark),
    };
}
function parseMockTool(raw, index) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        throw new Error(`mock_tools[${index}] must be an object.`);
    }
    const value = toSafeRecord(raw);
    const tool = typeof value.tool === "string" ? value.tool.trim() || undefined : undefined;
    const bridgeTool = typeof value.bridge_tool === "string" ? value.bridge_tool.trim() || undefined : undefined;
    const topLevelTool = typeof value.top_level_tool === "string" ? value.top_level_tool.trim() || undefined : undefined;
    if (!tool && !bridgeTool && !topLevelTool) {
        throw new Error(`mock_tools[${index}] must define at least one of tool, bridge_tool, or top_level_tool.`);
    }
    return {
        tool,
        bridge_tool: bridgeTool,
        top_level_tool: topLevelTool,
        args: normalizeRecord(value.args),
        times: normalizePositiveInt(value.times),
        response: parseRiftbornResponse(value.response, `mock_tools[${index}].response`),
    };
}
function parseMockHttp(raw, index) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        throw new Error(`mock_http[${index}] must be an object.`);
    }
    const value = toSafeRecord(raw);
    const pathValue = typeof value.path === "string" ? value.path.trim() : "";
    if (!pathValue) {
        throw new Error(`mock_http[${index}] requires a non-empty 'path'.`);
    }
    const method = value.method === "GET" || value.method === "POST"
        ? value.method
        : "POST";
    return {
        method,
        path: pathValue,
        body: normalizeRecord(value.body),
        top_level_tool: typeof value.top_level_tool === "string" ? value.top_level_tool.trim() || undefined : undefined,
        times: normalizePositiveInt(value.times),
        response: parseRiftbornResponse(value.response, `mock_http[${index}].response`),
    };
}
function parseRiftbornResponse(raw, field) {
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        throw new Error(`${field} must be a RiftbornResponse object.`);
    }
    const value = toSafeRecord(raw);
    if (typeof value.ok !== "boolean") {
        throw new Error(`${field}.ok must be a boolean.`);
    }
    return cloneStructured(value);
}
function buildVisibleToolCatalog(tools, handlerNameSet) {
    const generatedToolNames = new Set(GENERATED_TOOLS.map((tool) => tool.name));
    return tools.map((tool) => {
        const readiness = getToolReadiness(tool.name, handlerNameSet, MANUAL_TOOL_NAMES, generatedToolNames);
        return {
            name: tool.name,
            description: tool.description ?? "",
            tier: readiness.tier,
            reason: readiness.reason ?? "",
        };
    });
}
class ScenarioMockBackend {
    toolMocks;
    httpMocks;
    logEntries = [];
    bridgeMode;
    bridgeConfig;
    constructor(scenario, options = {}) {
        this.bridgeMode = scenario.bridge_mode ?? "mock";
        this.bridgeConfig = options.bridgeConfig;
        this.toolMocks = (scenario.mock_tools ?? []).map((entry, index) => ({
            index,
            tool: entry.tool,
            bridgeTool: entry.bridge_tool,
            topLevelTool: entry.top_level_tool,
            argsKey: entry.args ? stableKey(entry.args) : undefined,
            remaining: entry.times ?? 1,
            response: cloneStructured(entry.response),
        }));
        this.httpMocks = (scenario.mock_http ?? []).map((entry, index) => ({
            index,
            method: entry.method ?? "POST",
            path: entry.path,
            bodyKey: entry.body ? stableKey(entry.body) : undefined,
            topLevelTool: entry.top_level_tool,
            remaining: entry.times ?? 1,
            response: cloneStructured(entry.response),
        }));
    }
    get log() {
        return this.logEntries.map((entry) => cloneStructured(entry));
    }
    async executeTool(bridgeTool, args, topLevelTool) {
        const argsKey = stableKey(args);
        const mock = this.toolMocks.find((candidate) => this.matchesToolMock(candidate, bridgeTool, topLevelTool, argsKey));
        if (mock) {
            mock.remaining -= 1;
        }
        const response = mock
            ? cloneStructured(mock.response)
            : await this.executeLiveBridgeTool(bridgeTool, args, topLevelTool);
        this.logEntries.push({
            kind: "tool",
            timestamp: Date.now(),
            top_level_tool: topLevelTool,
            bridge_tool: bridgeTool,
            args: cloneStructured(args),
            matched_mock: mock?.index,
            response: cloneStructured(response),
        });
        return response;
    }
    async httpRequest(method, requestPath, body, topLevelTool) {
        if (method === "POST"
            && requestPath === "/riftborn/tool"
            && body
            && typeof body.tool === "string") {
            return this.executeTool(body.tool, normalizeRecord(body.params) ?? normalizeRecord(body.args) ?? {}, topLevelTool);
        }
        const bodyKey = body ? stableKey(body) : undefined;
        const mock = this.httpMocks.find((candidate) => this.matchesHttpMock(candidate, method, requestPath, topLevelTool, bodyKey));
        if (mock) {
            mock.remaining -= 1;
        }
        const response = mock
            ? cloneStructured(mock.response)
            : await this.executeLiveHttpRequest(method, requestPath, body, topLevelTool);
        this.logEntries.push({
            kind: "http",
            timestamp: Date.now(),
            top_level_tool: topLevelTool,
            method,
            path: requestPath,
            body: body ? cloneStructured(body) : undefined,
            matched_mock: mock?.index,
            response: cloneStructured(response),
        });
        return response;
    }
    matchesToolMock(mock, bridgeTool, topLevelTool, argsKey) {
        if (mock.remaining <= 0) {
            return false;
        }
        if (mock.bridgeTool && mock.bridgeTool !== bridgeTool) {
            return false;
        }
        if (mock.topLevelTool && mock.topLevelTool !== topLevelTool) {
            return false;
        }
        if (mock.tool && mock.tool !== bridgeTool && mock.tool !== topLevelTool) {
            return false;
        }
        if (mock.argsKey && mock.argsKey !== argsKey) {
            return false;
        }
        return true;
    }
    async executeLiveBridgeTool(bridgeTool, args, topLevelTool) {
        if (this.bridgeMode !== "live") {
            return {
                ok: false,
                error: `No mocked bridge response matched tool '${bridgeTool}'` +
                    (topLevelTool ? ` from top-level tool '${topLevelTool}'` : "") +
                    ".",
            };
        }
        if (!this.bridgeConfig) {
            return {
                ok: false,
                error: `Live bridge mode is enabled, but bridge configuration was not resolved for '${bridgeTool}'.`,
            };
        }
        return retryableHttpRequest(this.bridgeConfig, "POST", "/riftborn/tool", { tool: bridgeTool, arguments: args }, OPENAI_TIMEOUT_MS);
    }
    async executeLiveHttpRequest(method, requestPath, body, topLevelTool) {
        if (this.bridgeMode !== "live") {
            return {
                ok: false,
                error: `No mocked HTTP response matched ${method} ${requestPath}` +
                    (topLevelTool ? ` from top-level tool '${topLevelTool}'` : "") +
                    ".",
            };
        }
        if (!this.bridgeConfig) {
            return {
                ok: false,
                error: `Live bridge mode is enabled, but bridge configuration was not resolved for ${method} ${requestPath}.`,
            };
        }
        return retryableHttpRequest(this.bridgeConfig, method, requestPath, body, OPENAI_TIMEOUT_MS);
    }
    matchesHttpMock(mock, method, requestPath, topLevelTool, bodyKey) {
        if (mock.remaining <= 0) {
            return false;
        }
        if (mock.method !== method || mock.path !== requestPath) {
            return false;
        }
        if (mock.topLevelTool && mock.topLevelTool !== topLevelTool) {
            return false;
        }
        if (mock.bodyKey && mock.bodyKey !== bodyKey) {
            return false;
        }
        return true;
    }
}
function createNoopSessionPersistence() {
    return {
        tryRestore: () => ({ restored: false, reason: "disabled" }),
        scheduleSave: () => undefined,
        saveNow: () => true,
        dispose: () => undefined,
        get stateFilePath() {
            return "headless://disabled";
        },
    };
}
function createHeadlessHarnessRuntime(scenario, callbacks = {}) {
    const blockedTools = getBlockedToolNameSet();
    const internalOnlyTools = getInternalToolNameSet();
    const generatedToolNames = new Set(GENERATED_TOOLS.map((tool) => tool.name));
    const resolvedAuth = resolveBridgeAuthToken({
        searchRoots: [PLUGIN_ROOT, process.cwd()],
    });
    const bridgeConfig = scenario.bridge_mode === "live"
        ? {
            host: scenario.bridge_host ?? process.env.RIFTBORN_HOST ?? "127.0.0.1",
            httpPort: scenario.bridge_http_port ?? normalizePositiveInt(process.env.RIFTBORN_HTTP_PORT) ?? 8767,
            authToken: resolvedAuth.token,
        }
        : undefined;
    const mockBackend = new ScenarioMockBackend(scenario, { bridgeConfig });
    const shippableTools = buildAllTools().filter((tool) => !blockedTools.has(tool.name) && !internalOnlyTools.has(tool.name));
    const sessionTracker = new SessionTracker();
    const contextPropagator = new ContextPropagator();
    const sceneDiffTracker = new SceneDiffTracker();
    const progressTracker = new ProgressTracker();
    const sceneChangeLog = new SceneChangeLog();
    const latencyTracker = new LatencyTracker();
    const adaptiveThrottle = new AdaptiveThrottle();
    const sessionBookmarks = new SessionBookmarks();
    const failureBudget = new FailureBudget();
    const responseDeltaTracker = new ResponseDeltaTracker();
    const pipelineTelemetry = new PipelineTelemetry();
    const pipelineTraceStore = new PipelineTraceStore();
    let sessionCallCount = 0;
    const sessionPersistence = createNoopSessionPersistence();
    const schemaIntel = new SchemaIntelligence(shippableTools);
    let toolResolver = null;
    let visibleTools = [];
    let rawDispatchManagedTool = null;
    const dispatchStack = [];
    const currentTopLevelTool = () => (dispatchStack.length > 0 ? dispatchStack[dispatchStack.length - 1] : undefined);
    const runDispatch = async (name, args = {}) => {
        if (!rawDispatchManagedTool) {
            throw new Error("Harness dispatcher is not initialized.");
        }
        dispatchStack.push(name);
        try {
            return await rawDispatchManagedTool(name, args);
        }
        finally {
            dispatchStack.pop();
        }
    };
    const toolHandlers = createToolHandlers({
        executeTool: (toolName, params) => mockBackend.executeTool(toolName, normalizeRecord(params) ?? {}, currentTopLevelTool()),
        dispatchTool: (toolName, params) => runDispatch(toolName, params ?? {}),
        httpRequest: (method, requestPath, body) => mockBackend.httpRequest(method, requestPath, normalizeRecord(body), currentTopLevelTool()),
        host: "127.0.0.1",
        httpPort: 8767,
        visibleTools: () => visibleTools,
    });
    toolHandlers.spawn_subagent = async (args) => {
        if (!callbacks.spawnSubagent) {
            return {
                ok: false,
                error: "spawn_subagent is not configured in this headless harness run.",
            };
        }
        return callbacks.spawnSubagent(normalizeRecord(args) ?? {});
    };
    toolHandlers.start_agent_task = async (args) => {
        if (!callbacks.startAgentTask) {
            return {
                ok: false,
                error: "start_agent_task is not configured in this headless harness run.",
            };
        }
        return callbacks.startAgentTask(normalizeRecord(args) ?? {});
    };
    toolHandlers.get_agent_task = async (args) => {
        if (!callbacks.getAgentTask) {
            return {
                ok: false,
                error: "get_agent_task is not configured in this headless harness run.",
            };
        }
        return callbacks.getAgentTask(normalizeRecord(args) ?? {});
    };
    toolHandlers.get_agentic_session = async (args) => {
        if (!callbacks.getAgenticSession) {
            return {
                ok: false,
                error: "get_agentic_session is not configured in this headless harness run.",
            };
        }
        return callbacks.getAgenticSession(normalizeRecord(args) ?? {});
    };
    toolHandlers.list_agent_tasks = async (args) => {
        if (!callbacks.listAgentTasks) {
            return {
                ok: false,
                error: "list_agent_tasks is not configured in this headless harness run.",
            };
        }
        return callbacks.listAgentTasks(normalizeRecord(args) ?? {});
    };
    toolHandlers.cancel_agent_task = async (args) => {
        if (!callbacks.cancelAgentTask) {
            return {
                ok: false,
                error: "cancel_agent_task is not configured in this headless harness run.",
            };
        }
        return callbacks.cancelAgentTask(normalizeRecord(args) ?? {});
    };
    installWorkflowHandlers({
        toolHandlers,
        dispatchManagedTool: (name, params) => runDispatch(name, params ?? {}),
        visibleTools: () => visibleTools,
        sessionTracker,
        contextPropagator,
        sceneChangeLog,
        progressTracker,
        sessionBookmarks,
        pipelineTelemetry,
        pipelineTraceStore,
    });
    toolHandlers.find_tools = async (args) => {
        const toolRouter = new ToolRouter(visibleTools);
        const results = toolRouter.search(normalizeToolSearchQuery(String(args.query || "")), clampToolSearchResults(args.max_results == null ? 20 : Number(args.max_results)));
        return { ok: true, result: { matches: results.length, tools: results } };
    };
    const allowedTiers = scenario.tool_tiers ?? DEFAULT_HARNESS_TOOL_TIERS;
    const handlerNameSet = new Set(Object.keys(toolHandlers));
    visibleTools = filterToolsByReadiness(shippableTools, allowedTiers, handlerNameSet, MANUAL_TOOL_NAMES, generatedToolNames);
    const requestedAllVisibleTools = scenario.tool_names.includes("*");
    const requestedTools = requestedAllVisibleTools
        ? new Set(visibleTools.map((tool) => tool.name))
        : new Set(scenario.tool_names);
    if (!requestedAllVisibleTools) {
        const missingTools = [...requestedTools].filter((toolName) => !visibleTools.some((tool) => tool.name === toolName));
        if (missingTools.length > 0) {
            throw new Error(`Harness scenario requested tools that are not on the visible readiness-gated surface: ${missingTools.join(", ")}.`);
        }
    }
    visibleTools = visibleTools.filter((tool) => requestedTools.has(tool.name));
    toolResolver = new ToolResolver(visibleTools);
    const visibleToolNames = new Set(visibleTools.map((tool) => tool.name));
    rawDispatchManagedTool = createManagedDispatcher({
        toolHandlers,
        generatedToolNames,
        blockedTools,
        internalOnlyTools,
        getVisibleToolNames: () => visibleToolNames,
        enableInternalTools: false,
        allowHiddenTools: false,
        executeTool: (toolName, params = {}) => mockBackend.executeTool(toolName, normalizeRecord(params) ?? {}, currentTopLevelTool()),
        executeToolDirect: (toolName, params) => mockBackend.executeTool(toolName, normalizeRecord(params) ?? {}, currentTopLevelTool()),
        requireSchemaIntel: () => schemaIntel,
        requireToolResolver: () => {
            if (!toolResolver) {
                throw new Error("Harness tool resolver is not initialized.");
            }
            return toolResolver;
        },
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
        digestInterval: 10,
    });
    return {
        visibleTools,
        dispatchTool: runDispatch,
        sessionTracker,
        pipelineTraceStore,
        mockBackend,
        toolCatalog: buildVisibleToolCatalog(visibleTools, handlerNameSet),
    };
}
function toolToOpenAIFunction(tool) {
    return {
        type: "function",
        name: tool.name,
        description: tool.description ?? "",
        parameters: cloneStructured(tool.inputSchema ?? { type: "object", properties: {} }),
    };
}
function extractTextParts(content) {
    if (typeof content === "string") {
        return [content];
    }
    if (!Array.isArray(content)) {
        return [];
    }
    const parts = [];
    for (const entry of content) {
        if (!entry || typeof entry !== "object" || Array.isArray(entry)) {
            continue;
        }
        const item = entry;
        const textValue = item.text;
        if (typeof textValue === "string" && textValue) {
            parts.push(textValue);
            continue;
        }
        if (textValue && typeof textValue === "object" && !Array.isArray(textValue)) {
            const nestedText = textValue.value;
            if (typeof nestedText === "string" && nestedText) {
                parts.push(nestedText);
            }
        }
    }
    return parts;
}
function normalizeFunctionCall(item) {
    if (item.type !== "function_call") {
        return null;
    }
    if (typeof item.name !== "string" || typeof item.call_id !== "string") {
        return null;
    }
    const argumentsText = typeof item.arguments === "string"
        ? item.arguments
        : JSON.stringify(item.arguments ?? {});
    let parsedArguments;
    let parseError;
    try {
        const parsed = JSON.parse(argumentsText);
        if (!parsed || typeof parsed !== "object" || Array.isArray(parsed)) {
            parseError = "Model emitted non-object JSON arguments.";
        }
        else {
            parsedArguments = toSafeRecord(parsed);
        }
    }
    catch (error) {
        parseError = error instanceof Error ? error.message : String(error);
    }
    return {
        id: typeof item.id === "string" ? item.id : undefined,
        callId: item.call_id,
        name: item.name,
        argumentsText,
        parsedArguments,
        parseError,
    };
}
function extractOutputText(outputItems) {
    const parts = [];
    for (const rawItem of outputItems) {
        if (!rawItem || typeof rawItem !== "object" || Array.isArray(rawItem)) {
            continue;
        }
        const item = rawItem;
        if (item.type === "message") {
            parts.push(...extractTextParts(item.content));
            continue;
        }
        if (item.type === "output_text" && typeof item.text === "string") {
            parts.push(item.text);
        }
    }
    return parts.join("\n").trim();
}
function extractOpenAIErrorText(payload, status) {
    if (!payload || typeof payload !== "object" || Array.isArray(payload)) {
        return `OpenAI Responses API failed with HTTP ${status}.`;
    }
    const record = payload;
    if (record.error && typeof record.error === "object" && !Array.isArray(record.error)) {
        const message = record.error.message;
        if (typeof message === "string" && message.trim()) {
            return message.trim();
        }
    }
    return `OpenAI Responses API failed with HTTP ${status}.`;
}
export class OpenAIResponsesClient {
    fetchImpl;
    apiKey;
    baseUrl;
    organization;
    project;
    constructor(options) {
        const apiKey = options?.apiKey ?? process.env.OPENAI_API_KEY;
        if (!apiKey) {
            throw new Error("OPENAI_API_KEY must be set to run the headless harness against the real OpenAI API.");
        }
        this.apiKey = apiKey;
        this.baseUrl = options?.baseUrl ?? process.env.OPENAI_BASE_URL ?? OPENAI_RESPONSES_URL;
        this.organization = options?.organization ?? process.env.OPENAI_ORGANIZATION;
        this.project = options?.project ?? process.env.OPENAI_PROJECT;
        this.fetchImpl = options?.fetchImpl ?? fetch;
    }
    async createResponse(request) {
        const clientRequestId = crypto.randomUUID();
        const payload = {
            model: request.model,
            instructions: request.instructions,
            input: cloneStructured(request.input),
            tools: cloneStructured(request.tools),
            tool_choice: "auto",
        };
        if (request.previousResponseId) {
            payload.previous_response_id = request.previousResponseId;
        }
        if (request.maxOutputTokens) {
            payload.max_output_tokens = request.maxOutputTokens;
        }
        if (request.reasoningEffort) {
            payload.reasoning = { effort: request.reasoningEffort };
        }
        const headers = {
            "Authorization": `Bearer ${this.apiKey}`,
            "Content-Type": "application/json",
            "X-Client-Request-Id": clientRequestId,
        };
        if (this.organization) {
            headers["OpenAI-Organization"] = this.organization;
        }
        if (this.project) {
            headers["OpenAI-Project"] = this.project;
        }
        const response = await this.fetchImpl(this.baseUrl, {
            method: "POST",
            headers,
            body: JSON.stringify(payload),
            signal: AbortSignal.timeout(OPENAI_TIMEOUT_MS),
        });
        const rawText = await response.text();
        let parsed = {};
        try {
            parsed = rawText ? JSON.parse(rawText) : {};
        }
        catch {
            if (!response.ok) {
                throw new Error(`OpenAI Responses API failed with HTTP ${response.status}: ${rawText.slice(0, 500)}`);
            }
            throw new Error("OpenAI Responses API returned invalid JSON.");
        }
        if (!response.ok) {
            throw new Error(extractOpenAIErrorText(parsed, response.status));
        }
        const raw = toSafeRecord(parsed);
        const rawOutputItems = Array.isArray(raw.output)
            ? raw.output.map((item) => cloneStructured(item))
            : [];
        const functionCalls = rawOutputItems
            .map((item) => (item && typeof item === "object" && !Array.isArray(item)
            ? normalizeFunctionCall(item)
            : null))
            .filter((entry) => entry !== null);
        const outputText = typeof raw.output_text === "string" && raw.output_text.trim()
            ? raw.output_text
            : extractOutputText(rawOutputItems);
        return {
            id: typeof raw.id === "string" ? raw.id : undefined,
            requestId: response.headers.get("x-request-id") ?? undefined,
            clientRequestId,
            outputText,
            functionCalls,
            usage: normalizeRecord(raw.usage),
            raw,
            rawOutputItems,
        };
    }
}
function buildFunctionOutputItems(executions) {
    return executions.map((entry) => ({
        type: "function_call_output",
        call_id: entry.call_id,
        output: JSON.stringify(entry.dispatch_result),
    }));
}
function mergeInstructionText(...parts) {
    const merged = parts
        .map((part) => (typeof part === "string" ? part.trim() : ""))
        .filter((part) => part.length > 0)
        .join("\n\n");
    return merged || undefined;
}
function buildSubagentScenario(parentScenario, callArgs, parentCallId) {
    const task = typeof callArgs.task === "string" ? callArgs.task.trim() : "";
    if (!task) {
        throw new Error("spawn_subagent requires a non-empty 'task' string.");
    }
    const profile = typeof callArgs.profile === "string" && callArgs.profile.trim()
        ? callArgs.profile.trim()
        : "editor_assistant";
    const profileConfig = parentScenario.subagent_profiles?.[profile] ?? parentScenario.subagent_profiles?.default;
    const includeSceneContext = callArgs.include_scene_context !== false;
    const systemPromptAddendum = typeof callArgs.system_prompt_addendum === "string"
        ? callArgs.system_prompt_addendum.trim() || undefined
        : undefined;
    return {
        profile,
        task,
        scenario: {
            name: `${parentScenario.name ?? "scenario"}-subagent-${profile}-${parentCallId.slice(-6)}`,
            prompt: task,
            instructions: mergeInstructionText(parentScenario.instructions, profileConfig?.instructions, includeSceneContext
                ? "Headless harness note: no live scene snapshot was auto-injected for this sub-agent; use tools to gather evidence."
                : undefined, systemPromptAddendum),
            model: profileConfig?.model ?? parentScenario.model,
            max_iterations: normalizeIterationOverride(callArgs.max_iterations)
                ?? profileConfig?.max_iterations
                ?? parentScenario.max_iterations,
            max_output_tokens: profileConfig?.max_output_tokens ?? parentScenario.max_output_tokens,
            reasoning_effort: profileConfig?.reasoning_effort ?? parentScenario.reasoning_effort,
            tool_tiers: profileConfig?.tool_tiers ?? parentScenario.tool_tiers,
            tool_names: profileConfig?.tool_names ?? parentScenario.tool_names,
            bridge_mode: parentScenario.bridge_mode,
            bridge_host: parentScenario.bridge_host,
            bridge_http_port: parentScenario.bridge_http_port,
            mock_tools: profileConfig?.mock_tools ?? parentScenario.mock_tools,
            mock_http: profileConfig?.mock_http ?? parentScenario.mock_http,
            max_subagent_depth: parentScenario.max_subagent_depth,
            subagent_profiles: parentScenario.subagent_profiles,
            agent_task_profile: parentScenario.agent_task_profile,
        },
    };
}
function buildAgentTaskScenario(parentScenario, callArgs, taskId) {
    const request = typeof callArgs.request === "string" ? callArgs.request.trim() : "";
    if (!request) {
        throw new Error("start_agent_task requires a non-empty 'request' string.");
    }
    const profileConfig = parentScenario.agent_task_profile ?? parentScenario.subagent_profiles?.default;
    return {
        name: `${parentScenario.name ?? "scenario"}-agent-task-${taskId.slice(-6)}`,
        prompt: request,
        instructions: mergeInstructionText(parentScenario.instructions, profileConfig?.instructions, "Headless harness note: this autonomous agent task is simulated locally and can be inspected with get_agent_task or get_agentic_session."),
        model: profileConfig?.model ?? parentScenario.model,
        max_iterations: normalizeIterationOverride(callArgs.max_iterations)
            ?? profileConfig?.max_iterations
            ?? parentScenario.max_iterations,
        max_output_tokens: profileConfig?.max_output_tokens ?? parentScenario.max_output_tokens,
        reasoning_effort: profileConfig?.reasoning_effort ?? parentScenario.reasoning_effort,
        tool_tiers: profileConfig?.tool_tiers ?? parentScenario.tool_tiers,
        tool_names: profileConfig?.tool_names ?? parentScenario.tool_names,
        bridge_mode: parentScenario.bridge_mode,
        bridge_host: parentScenario.bridge_host,
        bridge_http_port: parentScenario.bridge_http_port,
        mock_tools: profileConfig?.mock_tools ?? parentScenario.mock_tools,
        mock_http: profileConfig?.mock_http ?? parentScenario.mock_http,
        max_subagent_depth: parentScenario.max_subagent_depth,
        subagent_profiles: parentScenario.subagent_profiles,
        agent_task_profile: parentScenario.agent_task_profile,
    };
}
function summarizeSubagentTranscript(transcript) {
    return {
        depth: transcript.depth,
        scenario_name: transcript.scenario_name,
        stop_reason: transcript.stop_reason,
        iterations: transcript.iterations.length,
        tool_calls: transcript.tool_executions.length,
        tool_path: transcript.tool_executions.map((entry) => entry.requested_tool),
        final_output_text: transcript.final_output_text,
    };
}
function summarizeAgentTaskTranscript(transcript) {
    return {
        depth: transcript.depth,
        scenario_name: transcript.scenario_name,
        stop_reason: transcript.stop_reason,
        iterations: transcript.iterations.length,
        tool_calls: transcript.tool_executions.length,
        tool_path: transcript.tool_executions.map((entry) => entry.requested_tool),
        subagent_runs: transcript.subagent_runs.length,
        final_output_text: transcript.final_output_text,
    };
}
export async function runHeadlessAgentHarness(options) {
    const depth = options.depth ?? 0;
    const subagentRuns = [];
    const agentTaskRuns = [];
    const agentTasks = new Map();
    const agentTaskRuntime = options.scenario.agent_task_runtime ?? {};
    const clearAgentTaskTimers = (taskState) => {
        if (taskState.runningTimer) {
            clearTimeout(taskState.runningTimer);
            taskState.runningTimer = undefined;
        }
        if (taskState.completionTimer) {
            clearTimeout(taskState.completionTimer);
            taskState.completionTimer = undefined;
        }
    };
    const setAgentTaskStatus = (taskState, status) => {
        if (taskState.closed || taskState.status === status) {
            return;
        }
        taskState.status = status;
        taskState.updatedAt = new Date().toISOString();
        agentTaskRuns[taskState.runRecordIndex].status = status;
    };
    const finalizeAgentTaskState = (taskState, status, extra) => {
        clearAgentTaskTimers(taskState);
        taskState.status = status;
        taskState.updatedAt = new Date().toISOString();
        taskState.transcript = extra?.transcript;
        taskState.error = extra?.error;
        const runRecord = agentTaskRuns[taskState.runRecordIndex];
        runRecord.status = status;
        runRecord.transcript = extra?.transcript;
        runRecord.error = extra?.error;
    };
    const completeAgentTaskState = async (taskState) => {
        if (taskState.completionPromise) {
            return taskState.completionPromise;
        }
        taskState.completionPromise = (async () => {
            if (taskState.status === "completed" || taskState.status === "failed" || taskState.status === "canceled" || taskState.closed) {
                return;
            }
            if (!taskState.childScenario || !taskState.parentCallId) {
                finalizeAgentTaskState(taskState, "failed", {
                    error: "Headless agent task is missing child scenario context.",
                });
                return;
            }
            taskState.completionStarted = true;
            setAgentTaskStatus(taskState, "running");
            try {
                const childTranscript = await runHeadlessAgentHarness({
                    scenario: taskState.childScenario,
                    client: options.client,
                    model: taskState.childScenario.model,
                    maxIterations: taskState.childScenario.max_iterations,
                    depth: depth + 1,
                    parentCallId: taskState.parentCallId,
                });
                finalizeAgentTaskState(taskState, "completed", { transcript: childTranscript });
            }
            catch (error) {
                finalizeAgentTaskState(taskState, "failed", {
                    error: error instanceof Error ? error.message : String(error),
                });
            }
        })();
        return taskState.completionPromise;
    };
    const scheduleTimeBasedAgentTask = (taskState) => {
        if (taskState.closed || taskState.status === "completed" || taskState.status === "failed" || taskState.status === "canceled") {
            return;
        }
        if (taskState.status === "queued") {
            taskState.runningTimer = setTimeout(() => {
                if (taskState.closed || taskState.status !== "queued") {
                    return;
                }
                setAgentTaskStatus(taskState, "running");
            }, taskState.startAfterMs);
            taskState.runningTimer.unref?.();
        }
        taskState.completionTimer = setTimeout(() => {
            void completeAgentTaskState(taskState);
        }, taskState.completeAfterMs);
        taskState.completionTimer.unref?.();
    };
    const maybeProgressAgentTaskState = async (taskState) => {
        if (taskState.status === "completed" || taskState.status === "failed" || taskState.status === "canceled") {
            return;
        }
        if (!taskState.deferredExecution) {
            await completeAgentTaskState(taskState);
            return;
        }
        if (taskState.progressionMode === "time") {
            return;
        }
        if (taskState.remainingPolls > 0) {
            taskState.remainingPolls -= 1;
            taskState.updatedAt = new Date().toISOString();
            if (taskState.status === "queued") {
                setAgentTaskStatus(taskState, "running");
            }
            if (taskState.remainingPolls > 0) {
                return;
            }
        }
        await completeAgentTaskState(taskState);
    };
    const runtime = createHeadlessHarnessRuntime(options.scenario, {
        spawnSubagent: async (args) => {
            const maxDepth = options.scenario.max_subagent_depth ?? 2;
            if (depth >= maxDepth) {
                return {
                    ok: false,
                    error: `spawn_subagent refused because max sub-agent depth ${maxDepth} was reached.`,
                };
            }
            const childCallId = typeof args.__harness_parent_call_id === "string"
                ? args.__harness_parent_call_id
                : "unknown";
            const childIteration = typeof args.__harness_parent_iteration === "number"
                ? args.__harness_parent_iteration
                : 0;
            const normalizedArgs = { ...args };
            delete normalizedArgs.__harness_parent_call_id;
            delete normalizedArgs.__harness_parent_iteration;
            const childConfig = buildSubagentScenario(options.scenario, normalizedArgs, childCallId);
            const timeoutSeconds = normalizeSubagentTimeoutSeconds(normalizedArgs.timeout_seconds);
            const childTranscript = await withTimeout(runHeadlessAgentHarness({
                scenario: childConfig.scenario,
                client: options.client,
                model: childConfig.scenario.model,
                maxIterations: childConfig.scenario.max_iterations,
                depth: depth + 1,
                parentCallId: childCallId,
            }), timeoutSeconds * 1000, `spawn_subagent timed out after ${timeoutSeconds} seconds.`);
            subagentRuns.push({
                parent_iteration: childIteration,
                parent_call_id: childCallId,
                parent_tool: "spawn_subagent",
                depth: depth + 1,
                task: childConfig.task,
                profile: childConfig.profile,
                transcript: childTranscript,
            });
            return {
                ok: true,
                result: {
                    delegated: true,
                    profile: childConfig.profile,
                    task: childConfig.task,
                    transcript_index: subagentRuns.length - 1,
                    summary: summarizeSubagentTranscript(childTranscript),
                },
            };
        },
        startAgentTask: async (args) => {
            const maxDepth = options.scenario.max_subagent_depth ?? 2;
            if (depth >= maxDepth) {
                return {
                    ok: false,
                    error: `start_agent_task refused because max agent depth ${maxDepth} was reached.`,
                };
            }
            const parentCallId = typeof args.__harness_parent_call_id === "string"
                ? args.__harness_parent_call_id
                : "unknown";
            const parentIteration = typeof args.__harness_parent_iteration === "number"
                ? args.__harness_parent_iteration
                : 0;
            const request = typeof args.request === "string" ? args.request.trim() : "";
            if (!request) {
                return {
                    ok: false,
                    error: "start_agent_task requires a non-empty 'request' string.",
                };
            }
            const taskId = crypto.randomUUID();
            const sessionId = crypto.randomUUID();
            const createdAtMs = Date.now();
            const createdAt = new Date(createdAtMs).toISOString();
            const childScenario = buildAgentTaskScenario(options.scenario, args, taskId);
            const deferredExecution = agentTaskRuntime.execution_mode === "deferred";
            const progressionMode = deferredExecution
                ? (agentTaskRuntime.progression_mode === "time"
                    || typeof agentTaskRuntime.complete_after_ms === "number"
                    ? "time"
                    : "poll")
                : "poll";
            const initialStatus = deferredExecution
                ? (agentTaskRuntime.initial_status ?? "queued")
                : "running";
            const remainingPolls = deferredExecution
                ? (progressionMode === "poll" ? (agentTaskRuntime.complete_after_polls ?? 1) : 0)
                : 0;
            const startAfterMs = deferredExecution && progressionMode === "time"
                ? (agentTaskRuntime.start_after_ms ?? 0)
                : 0;
            const completeAfterMs = deferredExecution && progressionMode === "time"
                ? Math.max(agentTaskRuntime.complete_after_ms ?? 1_000, startAfterMs)
                : 0;
            const runRecordIndex = agentTaskRuns.push({
                parent_iteration: parentIteration,
                parent_call_id: parentCallId,
                parent_tool: "start_agent_task",
                depth: depth + 1,
                task_id: taskId,
                session_id: sessionId,
                request,
                status: initialStatus,
            }) - 1;
            const taskState = {
                taskId,
                sessionId,
                request,
                status: initialStatus,
                createdAt,
                createdAtMs,
                updatedAt: createdAt,
                childScenario,
                parentCallId,
                parentIteration,
                remainingPolls,
                deferredExecution,
                progressionMode,
                startAfterMs,
                completeAfterMs,
                completionStarted: false,
                closed: false,
                runRecordIndex,
            };
            agentTasks.set(taskId, taskState);
            if (!deferredExecution) {
                await completeAgentTaskState(taskState);
            }
            else if (progressionMode === "time") {
                scheduleTimeBasedAgentTask(taskState);
            }
            return {
                ok: true,
                result: {
                    task_id: taskId,
                    session_id: sessionId,
                    status: taskState.status,
                    accepted: true,
                    message: "Agent task recorded. Use get_agent_task or get_agentic_session for details.",
                },
            };
        },
        getAgentTask: async (args) => {
            const taskId = typeof args.task_id === "string" ? args.task_id.trim() : "";
            if (!taskId) {
                return {
                    ok: false,
                    error: "get_agent_task requires a non-empty 'task_id' string.",
                };
            }
            const taskState = agentTasks.get(taskId);
            if (!taskState) {
                return {
                    ok: false,
                    error: `No headless agent task found for task_id '${taskId}'.`,
                };
            }
            await maybeProgressAgentTaskState(taskState);
            return {
                ok: true,
                result: {
                    task_id: taskState.taskId,
                    session_id: taskState.sessionId,
                    request: taskState.request,
                    status: taskState.status,
                    created_at: taskState.createdAt,
                    updated_at: taskState.updatedAt,
                    error: taskState.error,
                    summary: taskState.transcript ? summarizeAgentTaskTranscript(taskState.transcript) : undefined,
                },
            };
        },
        getAgenticSession: async (args) => {
            const sessionId = typeof args.session_id === "string" ? args.session_id.trim() : "";
            if (!sessionId) {
                return {
                    ok: false,
                    error: "get_agentic_session requires a non-empty 'session_id' string.",
                };
            }
            const taskState = [...agentTasks.values()].find((entry) => entry.sessionId === sessionId);
            if (!taskState) {
                return {
                    ok: false,
                    error: `No headless agent session found for session_id '${sessionId}'.`,
                };
            }
            await maybeProgressAgentTaskState(taskState);
            return {
                ok: true,
                result: {
                    session_id: taskState.sessionId,
                    task_id: taskState.taskId,
                    request: taskState.request,
                    status: taskState.status,
                    created_at: taskState.createdAt,
                    updated_at: taskState.updatedAt,
                    summary: taskState.transcript ? summarizeAgentTaskTranscript(taskState.transcript) : undefined,
                    transcript: taskState.transcript ? {
                        iterations: taskState.transcript.iterations.length,
                        tool_executions: taskState.transcript.tool_executions,
                        final_output_text: taskState.transcript.final_output_text,
                    } : undefined,
                    error: taskState.error,
                },
            };
        },
        listAgentTasks: async (args) => {
            const maxResults = normalizePositiveInt(args.max_results) ?? 20;
            const tasks = [...agentTasks.values()]
                .sort((left, right) => right.createdAt.localeCompare(left.createdAt))
                .slice(0, maxResults)
                .map((taskState) => ({
                task_id: taskState.taskId,
                session_id: taskState.sessionId,
                request: taskState.request,
                status: taskState.status,
                created_at: taskState.createdAt,
                updated_at: taskState.updatedAt,
            }));
            return {
                ok: true,
                result: {
                    count: tasks.length,
                    tasks,
                },
            };
        },
        cancelAgentTask: async (args) => {
            const taskId = typeof args.task_id === "string" ? args.task_id.trim() : "";
            if (!taskId) {
                return {
                    ok: false,
                    error: "cancel_agent_task requires a non-empty 'task_id' string.",
                };
            }
            const taskState = agentTasks.get(taskId);
            if (!taskState) {
                return {
                    ok: false,
                    error: `No headless agent task found for task_id '${taskId}'.`,
                };
            }
            if (taskState.status === "completed" || taskState.status === "failed") {
                return {
                    ok: true,
                    result: {
                        task_id: taskState.taskId,
                        session_id: taskState.sessionId,
                        status: taskState.status,
                        canceled: false,
                        message: "Task already reached a terminal state before cancellation.",
                    },
                };
            }
            if (taskState.completionStarted) {
                return {
                    ok: true,
                    result: {
                        task_id: taskState.taskId,
                        session_id: taskState.sessionId,
                        status: taskState.status,
                        canceled: false,
                        message: "Task is already executing and can no longer be canceled cleanly.",
                    },
                };
            }
            finalizeAgentTaskState(taskState, "canceled");
            return {
                ok: true,
                result: {
                    task_id: taskState.taskId,
                    session_id: taskState.sessionId,
                    status: taskState.status,
                    canceled: true,
                },
            };
        },
    });
    const startedAt = new Date().toISOString();
    const toolSchemas = runtime.visibleTools.map(toolToOpenAIFunction);
    const iterations = [];
    const toolExecutions = [];
    const model = options.model ?? options.scenario.model ?? DEFAULT_OPENAI_MODEL;
    const maxIterations = options.maxIterations ?? options.scenario.max_iterations ?? DEFAULT_MAX_ITERATIONS;
    const instructions = options.scenario.instructions ?? DEFAULT_HARNESS_INSTRUCTIONS;
    let previousResponseId;
    let finalOutputText = "";
    let stopReason = "completed";
    for (let iteration = 1; iteration <= maxIterations; iteration++) {
        const input = iteration === 1
            ? [
                {
                    role: "user",
                    content: [
                        {
                            type: "input_text",
                            text: options.scenario.prompt,
                        },
                    ],
                },
            ]
            : buildFunctionOutputItems(toolExecutions.filter((entry) => entry.iteration === iteration - 1));
        const modelResponse = await options.client.createResponse({
            model,
            instructions,
            input,
            tools: toolSchemas,
            previousResponseId,
            maxOutputTokens: options.scenario.max_output_tokens,
            reasoningEffort: options.scenario.reasoning_effort,
        });
        previousResponseId = modelResponse.id ?? previousResponseId;
        iterations.push({
            iteration,
            response_id: modelResponse.id,
            request_id: modelResponse.requestId,
            client_request_id: modelResponse.clientRequestId,
            output_text: modelResponse.outputText,
            function_calls: modelResponse.functionCalls.map((call) => cloneStructured({ ...call })),
            usage: modelResponse.usage ? cloneStructured(modelResponse.usage) : undefined,
            raw_output_items: cloneStructured(modelResponse.rawOutputItems),
        });
        if (modelResponse.functionCalls.length === 0) {
            finalOutputText = modelResponse.outputText;
            break;
        }
        for (const call of modelResponse.functionCalls) {
            const traceCountBefore = runtime.pipelineTraceStore.size;
            const totalCallsBefore = runtime.sessionTracker.getHistory().total_calls;
            const mockLogBefore = runtime.mockBackend.log.length;
            const subagentRunCountBefore = subagentRuns.length;
            const agentTaskRunCountBefore = agentTaskRuns.length;
            const callArgs = call.parsedArguments
                ? cloneStructured(call.parsedArguments)
                : {};
            if (call.name === "spawn_subagent" || call.name === "start_agent_task") {
                callArgs.__harness_parent_call_id = call.callId;
                callArgs.__harness_parent_iteration = iteration;
            }
            const dispatchResult = call.parseError
                ? {
                    ok: false,
                    error: `Model emitted invalid JSON arguments for '${call.name}': ${call.parseError}`,
                }
                : await runtime.dispatchTool(call.name, callArgs);
            const newTraceCount = runtime.pipelineTraceStore.size - traceCountBefore;
            const newTotalCalls = runtime.sessionTracker.getHistory().total_calls - totalCallsBefore;
            const newMockEntries = runtime.mockBackend.log.length - mockLogBefore;
            const newSubagentRuns = subagentRuns.length - subagentRunCountBefore;
            const newAgentTaskRuns = agentTaskRuns.length - agentTaskRunCountBefore;
            toolExecutions.push({
                depth,
                iteration,
                call_id: call.callId,
                requested_tool: call.name,
                arguments_text: call.argumentsText,
                parsed_arguments: call.parsedArguments ? cloneStructured(call.parsedArguments) : undefined,
                parse_error: call.parseError,
                dispatch_result: cloneStructured(dispatchResult),
                session_entries: newTotalCalls > 0
                    ? cloneStructured(runtime.sessionTracker.recent(newTotalCalls))
                    : [],
                pipeline_traces: newTraceCount > 0
                    ? cloneStructured(runtime.pipelineTraceStore.recent(newTraceCount))
                    : [],
                mock_backend_log: newMockEntries > 0
                    ? cloneStructured(runtime.mockBackend.log.slice(-newMockEntries))
                    : [],
                subagent_run_indices: newSubagentRuns > 0
                    ? Array.from({ length: newSubagentRuns }, (_unused, index) => subagentRunCountBefore + index)
                    : undefined,
                agent_task_run_indices: newAgentTaskRuns > 0
                    ? Array.from({ length: newAgentTaskRuns }, (_unused, index) => agentTaskRunCountBefore + index)
                    : undefined,
            });
        }
        if (iteration === maxIterations) {
            stopReason = "max_iterations";
        }
    }
    for (const taskState of agentTasks.values()) {
        if (!taskState.completionPromise) {
            taskState.closed = true;
            clearAgentTaskTimers(taskState);
        }
    }
    const activeCompletionPromises = [...agentTasks.values()]
        .map((taskState) => taskState.completionPromise)
        .filter((promise) => promise !== undefined);
    if (activeCompletionPromises.length > 0) {
        await Promise.allSettled(activeCompletionPromises);
    }
    for (const taskState of agentTasks.values()) {
        taskState.closed = true;
        clearAgentTaskTimers(taskState);
    }
    const completedAt = new Date().toISOString();
    return {
        depth,
        parent_call_id: options.parentCallId,
        scenario_name: options.scenario.name ?? "unnamed-scenario",
        prompt: options.scenario.prompt,
        instructions,
        model,
        started_at: startedAt,
        completed_at: completedAt,
        stop_reason: stopReason,
        max_iterations: maxIterations,
        tool_names: [...options.scenario.tool_names],
        visible_tools: cloneStructured(runtime.toolCatalog),
        iterations,
        tool_executions: toolExecutions,
        subagent_runs: cloneStructured(subagentRuns),
        agent_task_runs: cloneStructured(agentTaskRuns),
        final_output_text: finalOutputText,
        session_history: cloneStructured(runtime.sessionTracker.getHistory()),
        pipeline_trace_report: cloneStructured(toSafeRecord(runtime.pipelineTraceStore.getReport({ count: Math.max(runtime.pipelineTraceStore.size, 1) }))),
        mock_backend_log: cloneStructured(runtime.mockBackend.log),
    };
}
function slugifyName(value) {
    const slug = value
        .toLowerCase()
        .replace(/[^a-z0-9]+/g, "-")
        .replace(/^-+|-+$/g, "");
    return slug || "scenario";
}
export function defaultHarnessTranscriptPath(scenarioName) {
    const stamp = new Date().toISOString().replace(/[:.]/g, "-");
    return path.resolve(DEFAULT_OUTPUT_DIR, `${slugifyName(scenarioName)}-${stamp}.json`);
}
export function writeHarnessTranscript(filePath, transcript) {
    fs.mkdirSync(path.dirname(filePath), { recursive: true });
    fs.writeFileSync(filePath, JSON.stringify(transcript, null, 2), "utf8");
    return filePath;
}
export function summarizeHarnessTranscript(transcript) {
    const requestedPath = transcript.tool_executions.map((entry) => entry.requested_tool).join(" -> ") || "(no tool calls)";
    const finalLine = transcript.final_output_text
        ? transcript.final_output_text.replace(/\s+/g, " ").slice(0, 280)
        : "(no final text returned)";
    return [
        `Scenario: ${transcript.scenario_name}`,
        `Model: ${transcript.model}`,
        `Depth: ${transcript.depth}`,
        `Iterations: ${transcript.iterations.length}`,
        `Tool calls: ${transcript.tool_executions.length}`,
        `Sub-agent runs: ${transcript.subagent_runs.length}`,
        `Agent-task runs: ${transcript.agent_task_runs.length}`,
        `Path: ${requestedPath}`,
        `Stop reason: ${transcript.stop_reason}`,
        `Final: ${finalLine}`,
    ].join("\n");
}
export function loadHeadlessHarnessScenario(filePath) {
    const raw = JSON.parse(fs.readFileSync(filePath, "utf8"));
    return parseHeadlessHarnessScenario(raw);
}