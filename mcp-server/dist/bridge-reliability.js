/**
 * Bridge Reliability Layer for RiftbornAI MCP Server
 *
 * Production-grade retry logic, concurrency control, parameter validation,
 * health monitoring, and timeout classification for the UE5 HTTP bridge.
 */
// ---------------------------------------------------------------------------
// 5. TimeoutClassifier — per-category timeout mapping
// ---------------------------------------------------------------------------
const VISION_PATTERNS = [
    "screenshot", "observe", "analyze_scene", "capture_viewport",
    "look_at_and_capture", "take_screenshot",
];
const BUILD_PATTERNS = [
    "build_lighting", "trigger_live_coding", "hot_reload_cpp",
    "run_ubt", "cook_project", "package_project",
    "run_quick_playtest", "validate_assets", "validate_blueprint_health",
    "validate_for_packaging", "audit_net_replication",
    "build_navmesh", "compile_blueprint",
];
const CODEGEN_PATTERNS = [
    "generate_actor_class", "generate_character_class", "generate_component_class",
    "generate_gamemode_class", "generate_subsystem_class", "generate_replicated_actor",
    "create_character_from_third_person",
    "create_behavior_tree", "create_gameplay_ability", "create_level",
    "make_character_playable",
];
const QUERY_PATTERNS = [
    "get_", "list_", "describe_", "find_", "search_", "resolve_", "is_",
];
/**
 * Determine the appropriate HTTP timeout for a tool based on its name pattern.
 *
 * - Vision tools (screenshot, observe, analyze): 90s — depends on AI vision provider latency.
 * - Build tools (compile, lighting, navmesh): 300s — UE5 builds can be slow.
 * - Codegen tools (generate_*, create_character): 120s — C++ code generation.
 * - Query tools (get_*, list_*, find_*, search_*, resolve_*, is_*): 15s — fast reads.
 * - Default: 60s for unrecognized tools.
 *
 * @param toolName - The registered tool name (case-insensitive matching).
 * @returns Timeout in milliseconds.
 */
export function classifyTimeout(toolName) {
    const lower = toolName.toLowerCase();
    for (const p of VISION_PATTERNS) {
        if (lower.includes(p))
            return 90_000;
    }
    for (const p of BUILD_PATTERNS) {
        if (lower.includes(p))
            return 300_000;
    }
    for (const p of CODEGEN_PATTERNS) {
        if (lower.includes(p))
            return 120_000;
    }
    for (const p of QUERY_PATTERNS) {
        if (lower.startsWith(p))
            return 15_000;
    }
    return 60_000;
}
// ---------------------------------------------------------------------------
// 3. ParameterSanitizer — validates and clamps params before bridge dispatch
// ---------------------------------------------------------------------------
const NUMERIC_LIMIT = 1e7;
const STRING_LIMIT = 65_536;
const ARRAY_LIMIT = 1_000;
const COUNT_LIMIT = 10_000;
const MAX_SANITIZE_DEPTH = 12;
const MAX_ERROR_BODY_BYTES = 64 * 1024;
/** Field names exempt from the [-1e7, 1e7] clamp (e.g. world coordinates). */
const NUMERIC_WHITELIST = new Set([
    // Landscape / world coordinates can legitimately exceed 1e7 in UE5
    "x", "y", "z",
    "loc_x", "loc_y", "loc_z",
    "start_x", "start_y", "start_z",
    "end_x", "end_y", "end_z",
    "center_x", "center_y", "center_z",
    "target_x", "target_y", "target_z",
    "offset_x", "offset_y", "offset_z",
    "extent_x", "extent_y", "extent_z",
]);
const COUNT_PATTERN = /^(count|num_)/;
function clampNumber(value, min, max) {
    if (!Number.isFinite(value))
        return 0;
    return Math.max(min, Math.min(max, value));
}
/**
 * Validate and clamp tool parameters before dispatching to the UE5 HTTP bridge.
 *
 * Protections applied:
 * - **NaN/Infinity** → replaced with 0.
 * - **Numeric clamp** → non-whitelisted numbers clamped to ±1e7 (world coordinates are exempt).
 * - **Count fields** (`count`, `num_*`) → capped at 10,000 to prevent runaway spawning.
 * - **Long strings** → truncated to 64KB.
 * - **Large arrays** → truncated to 1,000 items.
 * - **Nested objects** → recursively sanitized.
 * - **Booleans, null, undefined** → passed through unchanged.
 *
 * @param toolName - Tool name (used for warning messages).
 * @param params - Raw parameter object from MCP client.
 * @returns A new object with all values validated and clamped.
 */
export function sanitizeParams(toolName, params) {
    return sanitizeObject(toolName, params, {
        depth: 0,
        seen: new WeakSet(),
    });
}
function sanitizeObject(toolName, params, state) {
    const out = {};
    for (const [key, value] of Object.entries(params)) {
        out[key] = sanitizeValue(toolName, key, value, state);
    }
    return out;
}
function sanitizeValue(toolName, key, value, state) {
    if (value === null || value === undefined)
        return value;
    if (typeof value === "number") {
        // Reject NaN / Infinity
        if (!Number.isFinite(value)) {
            console.error(`[RiftbornAI][Sanitizer] WARN: ${toolName}.${key} was ${value}, replaced with 0`);
            return 0;
        }
        // Count / num_* hard cap
        if (COUNT_PATTERN.test(key) && value > COUNT_LIMIT) {
            console.error(`[RiftbornAI][Sanitizer] WARN: ${toolName}.${key}=${value} exceeds count limit ${COUNT_LIMIT}, clamped`);
            return COUNT_LIMIT;
        }
        // General numeric clamp (unless whitelisted)
        if (!NUMERIC_WHITELIST.has(key)) {
            const clamped = clampNumber(value, -NUMERIC_LIMIT, NUMERIC_LIMIT);
            if (clamped !== value) {
                console.error(`[RiftbornAI][Sanitizer] WARN: ${toolName}.${key}=${value} clamped to ${clamped}`);
            }
            return clamped;
        }
        return value;
    }
    if (typeof value === "string") {
        if (value.length > STRING_LIMIT) {
            console.error(`[RiftbornAI][Sanitizer] WARN: ${toolName}.${key} string truncated from ${value.length} to ${STRING_LIMIT} chars`);
            return value.substring(0, STRING_LIMIT);
        }
        return value;
    }
    if (Array.isArray(value)) {
        if (state.depth >= MAX_SANITIZE_DEPTH) {
            console.error(`[RiftbornAI][Sanitizer] WARN: ${toolName}.${key} exceeded max nesting depth ${MAX_SANITIZE_DEPTH}, replaced with null`);
            return null;
        }
        if (value.length > ARRAY_LIMIT) {
            console.error(`[RiftbornAI][Sanitizer] WARN: ${toolName}.${key} array truncated from ${value.length} to ${ARRAY_LIMIT} items`);
            return value.slice(0, ARRAY_LIMIT).map((v, i) => sanitizeValue(toolName, `${key}[${i}]`, v, {
                ...state,
                depth: state.depth + 1,
            }));
        }
        return value.map((v, i) => sanitizeValue(toolName, `${key}[${i}]`, v, {
            ...state,
            depth: state.depth + 1,
        }));
    }
    if (typeof value === "object") {
        if (state.depth >= MAX_SANITIZE_DEPTH) {
            console.error(`[RiftbornAI][Sanitizer] WARN: ${toolName}.${key} exceeded max nesting depth ${MAX_SANITIZE_DEPTH}, replaced with null`);
            return null;
        }
        const obj = value;
        if (state.seen.has(obj)) {
            console.error(`[RiftbornAI][Sanitizer] WARN: ${toolName}.${key} contains a circular reference, replaced with null`);
            return null;
        }
        state.seen.add(obj);
        try {
            return sanitizeObject(toolName, obj, {
                ...state,
                depth: state.depth + 1,
            });
        }
        finally {
            state.seen.delete(obj);
        }
    }
    // booleans and other primitives pass through
    return value;
}
// ---------------------------------------------------------------------------
// 1. RetryableHttpRequest — wraps raw fetch with retry + backoff
// ---------------------------------------------------------------------------
const RETRY_COUNT = 3;
const BACKOFF_MS = [1_000, 3_000, 8_000];
/** HTTP status codes that warrant a retry. */
const RETRYABLE_STATUS = new Set([502, 503, 504]);
/** Error message substrings that indicate transient network failures. */
const RETRYABLE_ERRORS = [
    "ECONNREFUSED",
    "ECONNRESET",
    "ETIMEDOUT",
    "UND_ERR_CONNECT_TIMEOUT",
    "TimeoutError",
    "abort",
    "network",
    "socket hang up",
];
function isRetryableError(msg) {
    const lower = msg.toLowerCase();
    return RETRYABLE_ERRORS.some((e) => lower.includes(e.toLowerCase()));
}
export function isDisconnectedBridgeError(msg) {
    if (!msg)
        return false;
    return isRetryableError(msg) || /bridge is disconnected/i.test(msg);
}
async function extractBridgeErrorBody(response) {
    const contentLength = response.headers.get("content-length");
    if (contentLength) {
        const declaredLength = Number(contentLength);
        if (Number.isFinite(declaredLength) && declaredLength > MAX_ERROR_BODY_BYTES) {
            return undefined;
        }
    }
    let text;
    try {
        text = await response.text();
    }
    catch {
        return undefined;
    }
    if (!text || text.length > MAX_ERROR_BODY_BYTES) {
        return undefined;
    }
    try {
        const parsed = JSON.parse(text);
        if (typeof parsed.error === "string") {
            return parsed.error.substring(0, 512);
        }
        if (typeof parsed.result === "string") {
            return parsed.result.substring(0, 512);
        }
    }
    catch {
        // Body wasn't JSON — fall through to generic message.
    }
    return undefined;
}
function sleep(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}
const BRIDGE_HOST_PATTERN = /^(?:localhost|(?:\d{1,3}\.){3}\d{1,3}|\[[0-9a-f:.]+\]|[a-z0-9](?:[a-z0-9.-]*[a-z0-9])?)$/i;
function buildBridgeUrl(config, urlPath) {
    const host = typeof config.host === "string" ? config.host.trim() : "";
    if (!host || host.length > 255 || /[\/\\\s]/.test(host) || !BRIDGE_HOST_PATTERN.test(host)) {
        return { ok: false, error: "Bridge host is invalid; expected a local hostname or IP address." };
    }
    if (!Number.isInteger(config.httpPort) || config.httpPort < 1 || config.httpPort > 65_535) {
        return { ok: false, error: "Bridge HTTP port is invalid; expected an integer between 1 and 65535." };
    }
    if (typeof urlPath !== "string" ||
        !urlPath.startsWith("/") ||
        /[\r\n\0\\]/.test(urlPath) ||
        urlPath.includes("://")) {
        return { ok: false, error: "Bridge URL path is invalid; expected an absolute local path." };
    }
    return { ok: true, url: `http://${host}:${config.httpPort}${urlPath}` };
}
/**
 * Make an HTTP request to the UE5 bridge with automatic retry and exponential backoff.
 *
 * Retries on HTTP 502/503/504 and transient network errors (ECONNREFUSED, ETIMEDOUT, etc.).
 * Non-retryable errors (400, 404, 500, JSON parse failures) return immediately.
 * Parses the C++ bridge's `{"ok":false,"error":"..."}` response format on error status codes.
 *
 * @param config - Bridge connection config (host, port, auth token).
 * @param method - HTTP method ("GET" or "POST").
 * @param urlPath - URL path (e.g., "/riftborn/exec").
 * @param body - Optional JSON body (POST only).
 * @param timeoutMs - Request timeout in milliseconds (default 30s).
 * @returns RiftbornResponse with `ok` flag and `result` or `error`.
 */
export async function retryableHttpRequest(config, method, urlPath, body, timeoutMs = 30_000) {
    const target = buildBridgeUrl(config, urlPath);
    if (!target.ok) {
        return { ok: false, error: target.error };
    }
    const url = target.url;
    let lastError = { ok: false, error: "No attempts made" };
    for (let attempt = 0; attempt <= RETRY_COUNT; attempt++) {
        if (attempt > 0) {
            const delay = BACKOFF_MS[attempt - 1] ?? BACKOFF_MS[BACKOFF_MS.length - 1];
            console.error(`[RiftbornAI][Retry] Attempt ${attempt + 1}/${RETRY_COUNT + 1} for ${method} ${urlPath} after ${delay}ms backoff`);
            await sleep(delay);
        }
        try {
            const options = {
                method,
                headers: {
                    "Content-Type": "application/json",
                },
                signal: AbortSignal.timeout(timeoutMs),
            };
            if (config.authToken && !config.authDisabled) {
                options.headers.Authorization = `Bearer ${config.authToken}`;
            }
            if (body && method === "POST") {
                options.body = JSON.stringify(body);
            }
            const response = await fetch(url, options);
            // Non-retryable HTTP errors — parse body for real error message
            if (!response.ok) {
                const status = response.status;
                // Try to extract the actual error from the JSON body.
                // The C++ bridge returns {"ok":false,"error":"..."} even on 500.
                const bodyError = await extractBridgeErrorBody(response);
                if (RETRYABLE_STATUS.has(status)) {
                    lastError = {
                        ok: false,
                        error: bodyError ?? `Bridge returned HTTP ${status} (retryable)`,
                    };
                    continue; // retry
                }
                // 400, 404, 500, etc. — do not retry, but surface the real error
                return { ok: false, error: bodyError ?? `Bridge returned HTTP ${status}` };
            }
            // Validate JSON content type
            const contentType = response.headers.get("content-type") || "";
            if (!contentType.includes("application/json") &&
                !contentType.includes("text/json")) {
                return {
                    ok: false,
                    error: `Bridge returned non-JSON Content-Type: ${contentType.substring(0, 100)}`,
                };
            }
            // Guard against oversized responses (10 MiB limit)
            const MAX_RESPONSE_BYTES = 10 * 1024 * 1024;
            const declaredLen = parseInt(response.headers.get("content-length") || "0", 10);
            if (Number.isFinite(declaredLen) && declaredLen > MAX_RESPONSE_BYTES) {
                return {
                    ok: false,
                    error: `Bridge response too large (${declaredLen} bytes, limit ${MAX_RESPONSE_BYTES}).`,
                };
            }
            const data = await response.json();
            // Auto-confirm: if bridge returns a confirmation request (safe mode),
            // automatically confirm and return the real result
            if (!data.ok && data.confirmation?.token) {
                try {
                    const confirmUrl = `http://${config.host}:${config.httpPort}/riftborn/confirm`;
                    const confirmHeaders = { "Content-Type": "application/json" };
                    if (config.authToken && !config.authDisabled) {
                        confirmHeaders.Authorization = `Bearer ${config.authToken}`;
                    }
                    const confirmResp = await fetch(confirmUrl, {
                        method: "POST",
                        headers: confirmHeaders,
                        body: JSON.stringify({ token: data.confirmation.token }),
                        signal: AbortSignal.timeout(timeoutMs),
                    });
                    if (confirmResp.ok) {
                        const confirmed = await confirmResp.json();
                        return confirmed;
                    }
                }
                catch {
                    // Confirmation failed — return original response
                }
            }
            return data;
        }
        catch (error) {
            const msg = error instanceof Error ? error.message : String(error);
            if (isRetryableError(msg)) {
                lastError = {
                    ok: false,
                    error: `HTTP request failed: ${msg.substring(0, 256)}`,
                };
                continue; // retry
            }
            // Non-retryable error (e.g. JSON parse failure, unexpected)
            return {
                ok: false,
                error: `HTTP request failed: ${msg.substring(0, 256)}`,
            };
        }
    }
    // All retries exhausted
    return lastError;
}
// ---------------------------------------------------------------------------
// 2. ToolExecutionQueue — serial mutations, parallel reads
// ---------------------------------------------------------------------------
const MAX_CONCURRENT_READS = 3;
const MAX_QUEUE_DEPTH = 50;
const READ_PREFIXES = ["get_", "list_", "describe_", "find_", "search_", "resolve_"];
function isReadTool(toolName) {
    const lower = toolName.toLowerCase();
    return READ_PREFIXES.some((p) => lower.startsWith(p));
}
/**
 * Concurrency controller for tool execution against the UE5 game thread.
 *
 * **Reads** (get_, list_, describe_, find_, search_, resolve_) run in parallel up to 3 concurrent.
 * **Mutations** (everything else) run serially to avoid game-thread contention.
 * Each item has a per-tool timeout based on {@link classifyTimeout}.
 * Queue depth is capped at 50 to prevent unbounded backlog.
 */
export class ToolExecutionQueue {
    mutationQueue = [];
    mutationRunning = false;
    readSemaphore = 0;
    readQueue = [];
    get queueDepth() {
        return this.mutationQueue.length + this.readQueue.length;
    }
    /**
     * Enqueue a tool execution. Reads run in parallel (up to MAX_CONCURRENT_READS),
     * mutations run serially to avoid game-thread contention.
     */
    enqueue(toolName, execute, timeoutMs) {
        if (this.queueDepth >= MAX_QUEUE_DEPTH) {
            return Promise.resolve({
                ok: false,
                error: `Tool execution queue full (${MAX_QUEUE_DEPTH} pending). Try again later.`,
            });
        }
        const effectiveTimeout = timeoutMs ?? classifyTimeout(toolName);
        return new Promise((resolve, reject) => {
            const item = {
                execute,
                resolve,
                reject,
                toolName,
                timeoutMs: effectiveTimeout,
            };
            if (isReadTool(toolName)) {
                this.readQueue.push(item);
                this.drainReads();
            }
            else {
                this.mutationQueue.push(item);
                this.drainMutations();
            }
        });
    }
    drainReads() {
        while (this.readSemaphore < MAX_CONCURRENT_READS && this.readQueue.length > 0) {
            const item = this.readQueue.shift();
            this.readSemaphore++;
            this.runWithTimeout(item).finally(() => {
                this.readSemaphore--;
                this.drainReads();
            });
        }
    }
    async drainMutations() {
        if (this.mutationRunning || this.mutationQueue.length === 0)
            return;
        this.mutationRunning = true;
        try {
            while (this.mutationQueue.length > 0) {
                const item = this.mutationQueue.shift();
                await this.runWithTimeout(item);
            }
        }
        finally {
            this.mutationRunning = false;
            // If items were enqueued during execution, drain again
            if (this.mutationQueue.length > 0) {
                void this.drainMutations();
            }
        }
    }
    async runWithTimeout(item) {
        let timer;
        try {
            const result = await Promise.race([
                item.execute(),
                new Promise((_, reject) => {
                    timer = setTimeout(() => reject(new Error(`Tool '${item.toolName}' timed out after ${item.timeoutMs}ms`)), item.timeoutMs);
                }),
            ]);
            item.resolve(result);
        }
        catch (err) {
            const msg = err instanceof Error ? err.message : String(err);
            item.resolve({ ok: false, error: msg.substring(0, 512) });
        }
        finally {
            if (timer !== undefined)
                clearTimeout(timer);
        }
    }
}
// ---------------------------------------------------------------------------
// 4. BridgeHealthMonitor — periodic health polling + offline queue
// ---------------------------------------------------------------------------
const HEALTH_POLL_INTERVAL = 30_000;
const MAX_OFFLINE_QUEUE = 10;
/**
 * Monitors the UE5 HTTP bridge with periodic health checks and offline replay.
 *
 * Polls the bridge's `/riftborn/health` endpoint every 30 seconds.
 * Tracks three states: "connected", "degraded" (bridge responds with errors), "disconnected".
 * When disconnected, tool calls can be queued (up to 10) for automatic replay on reconnect.
 * Stale queued items (>2 minutes old) are discarded on replay.
 */
export class BridgeHealthMonitor {
    status = "disconnected";
    pollTimer;
    offlineQueue = [];
    config;
    schemaVersionChecked = false;
    onReplayTool;
    constructor(config, onReplayTool) {
        this.config = config;
        this.onReplayTool = onReplayTool;
    }
    /** Current bridge status. */
    bridgeStatus() {
        return this.status;
    }
    /** Start periodic health polling. */
    start() {
        if (this.pollTimer)
            return;
        // Do an immediate check then start interval
        void this.check();
        this.pollTimer = setInterval(() => void this.check(), HEALTH_POLL_INTERVAL);
        // Allow Node to exit even if interval is running
        if (this.pollTimer && typeof this.pollTimer === "object" && "unref" in this.pollTimer) {
            this.pollTimer.unref();
        }
    }
    /** Stop health polling. */
    stop() {
        if (this.pollTimer) {
            clearInterval(this.pollTimer);
            this.pollTimer = undefined;
        }
    }
    /**
     * Queue a tool call for replay when the bridge reconnects.
     * Returns a promise that resolves when the tool is eventually executed,
     * or rejects if the queue is full.
     */
    queueForReplay(toolName, params) {
        if (this.offlineQueue.length >= MAX_OFFLINE_QUEUE) {
            return null; // caller should return "bridge disconnected" error
        }
        return new Promise((resolve) => {
            this.offlineQueue.push({
                toolName,
                params,
                resolve,
                enqueuedAt: Date.now(),
            });
            console.error(`[RiftbornAI][Health] Queued '${toolName}' for replay (${this.offlineQueue.length}/${MAX_OFFLINE_QUEUE})`);
        });
    }
    async check() {
        try {
            const resp = await retryableHttpRequest(this.config, "GET", "/riftborn/health", undefined, 5_000);
            const previousStatus = this.status;
            if (resp.ok) {
                this.status = "connected";
                // Validate schema version on first successful health check
                if (!this.schemaVersionChecked) {
                    this.schemaVersionChecked = true;
                    const bridgeVersion = resp.schema_version;
                    try {
                        const { GENERATED_TOOLS_SCHEMA_VERSION } = await import("./generated-tools.js");
                        if (bridgeVersion && bridgeVersion !== GENERATED_TOOLS_SCHEMA_VERSION) {
                            console.error(`[RiftbornAI][Health] WARNING: Schema version mismatch — ` +
                                `bridge reports ${bridgeVersion}, MCP expects ${GENERATED_TOOLS_SCHEMA_VERSION}. ` +
                                `Tool schemas may be stale. Run Scripts/generate_mcp_bindings.py to sync.`);
                        }
                    }
                    catch { /* generated-tools import failure is non-fatal */ }
                }
            }
            else if (isDisconnectedBridgeError(resp.error)) {
                this.status = "disconnected";
            }
            else {
                this.status = "degraded";
            }
            // If we just reconnected, replay queued items
            if (previousStatus === "disconnected" && this.status === "connected") {
                console.error(`[RiftbornAI][Health] Bridge reconnected — replaying ${this.offlineQueue.length} queued tool(s)`);
                await this.replayQueue();
            }
        }
        catch (err) {
            const prevStatus = this.status;
            this.status = "disconnected";
            if (prevStatus !== "disconnected") {
                const msg = err instanceof Error ? err.message : String(err);
                console.error(`[RiftbornAI][Health] Bridge disconnected: ${msg.substring(0, 256)}`);
            }
        }
    }
    async replayQueue() {
        const items = this.offlineQueue.splice(0);
        const REPLAY_TIMEOUT = 120_000; // 2 min max age for queued items
        for (const item of items) {
            const age = Date.now() - item.enqueuedAt;
            if (age > REPLAY_TIMEOUT) {
                console.error(`[RiftbornAI][Health] Discarding stale queued '${item.toolName}' (age: ${Math.round(age / 1000)}s)`);
                item.resolve({
                    ok: false,
                    error: `Queued tool '${item.toolName}' expired after ${Math.round(age / 1000)}s`,
                });
                continue;
            }
            try {
                const result = await this.onReplayTool(item.toolName, item.params);
                item.resolve(result);
            }
            catch (err) {
                const msg = err instanceof Error ? err.message : String(err);
                item.resolve({ ok: false, error: `Replay failed: ${msg.substring(0, 256)}` });
            }
        }
    }
}
//# sourceMappingURL=bridge-reliability.js.map