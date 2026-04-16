/**
 * Bridge Reliability Layer for RiftbornAI MCP Server
 *
 * Production-grade retry logic, concurrency control, parameter validation,
 * health monitoring, and timeout classification for the UE5 HTTP bridge.
 */
export interface RiftbornResponse {
    ok: boolean;
    result?: any;
    error?: string;
    [key: string]: any;
}
export interface BridgeConfig {
    host: string;
    httpPort: number;
    authToken: string;
}
export type BridgeStatus = "connected" | "disconnected" | "degraded";
type HttpMethod = "GET" | "POST";
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
export declare function classifyTimeout(toolName: string): number;
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
export declare function sanitizeParams(toolName: string, params: Record<string, unknown>): Record<string, unknown>;
export declare function isDisconnectedBridgeError(msg: string | undefined): boolean;
/**
 * Strip prototype-pollution keys from a parsed JSON response (recursive).
 * Lighter than sanitizeParams — no numeric clamping.
 */
export declare function stripProtoKeys(obj: Record<string, unknown>): Record<string, unknown>;
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
export declare function retryableHttpRequest(config: BridgeConfig, method: HttpMethod, urlPath: string, body?: object, timeoutMs?: number): Promise<RiftbornResponse>;
/**
 * Concurrency controller for tool execution against the UE5 game thread.
 *
 * **Reads** (get_, list_, describe_, find_, search_, resolve_) run in parallel up to 3 concurrent.
 * **Mutations** (everything else) run serially to avoid game-thread contention.
 * Each item has a per-tool timeout based on {@link classifyTimeout}.
 * Queue depth is capped at 50 to prevent unbounded backlog.
 */
export declare class ToolExecutionQueue {
    private mutationQueue;
    private mutationRunning;
    private readSemaphore;
    private readQueue;
    get queueDepth(): number;
    /**
     * Enqueue a tool execution. Reads run in parallel (up to MAX_CONCURRENT_READS),
     * mutations run serially to avoid game-thread contention.
     */
    enqueue(toolName: string, execute: () => Promise<RiftbornResponse>, timeoutMs?: number): Promise<RiftbornResponse>;
    private drainReads;
    private drainMutations;
    private runWithTimeout;
}
/**
 * Monitors the UE5 HTTP bridge with periodic health checks and offline replay.
 *
 * Polls the bridge's `/riftborn/health` endpoint every 30 seconds.
 * Tracks three states: "connected", "degraded" (bridge responds with errors), "disconnected".
 * When disconnected, tool calls can be queued (up to 10) for automatic replay on reconnect.
 * Stale queued items (>2 minutes old) are discarded on replay.
 */
export declare class BridgeHealthMonitor {
    private status;
    private pollTimer;
    private offlineQueue;
    private config;
    private schemaVersionChecked;
    private onReplayTool;
    constructor(config: BridgeConfig, onReplayTool: (toolName: string, params: Record<string, unknown>) => Promise<RiftbornResponse>);
    /** Current bridge status. */
    bridgeStatus(): BridgeStatus;
    /** Start periodic health polling. */
    start(): void;
    /** Stop health polling. */
    stop(): void;
    /**
     * Queue a tool call for replay when the bridge reconnects.
     * Returns a promise that resolves when the tool is eventually executed,
     * or rejects if the queue is full.
     */
    queueForReplay(toolName: string, params: Record<string, unknown>): Promise<RiftbornResponse> | null;
    private check;
    private replayQueue;
}
export {};
//# sourceMappingURL=bridge-reliability.d.ts.map