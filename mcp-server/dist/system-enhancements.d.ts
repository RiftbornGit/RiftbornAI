/**
 * System-Level Enhancements for the RiftbornAI MCP Server
 *
 * Three capabilities that improve the core pipeline:
 *
 * 1. ReadCache — TTL-based cache for read-only bridge calls (get_*, list_*, find_*).
 *    Invalidated when any mutation executes. Saves bridge round-trips.
 *
 * 2. normalizeResponse — Caps oversized responses to save agent tokens.
 *    Truncates large strings, slices long arrays, adds _truncated markers.
 *
 * 3. SessionTracker — Ring buffer of recent tool calls with timing.
 *    Injected into error responses so agents know what they've already done.
 *    Exposed as a resource for session introspection.
 */
import type { RiftbornResponse } from "./riftborn-types.js";
export declare class ReadCache {
    private cache;
    private ttlMs;
    private maxEntries;
    constructor(ttlMs?: number, maxEntries?: number);
    /** Check if a tool name is cacheable (read-only pattern). */
    isCacheable(toolName: string): boolean;
    /** Look up a cached response. Returns null on miss or expiry. */
    get(toolName: string, params: Record<string, unknown>): (RiftbornResponse & {
        _cached: true;
        _cache_age_ms: number;
    }) | null;
    /** Store a successful read response. */
    set(toolName: string, params: Record<string, unknown>, response: RiftbornResponse): void;
    /** Invalidate all cached entries. Called when any mutation tool runs. */
    invalidate(): void;
    /** Current number of cached entries. */
    get size(): number;
}
export interface NormalizationOptions {
    maxStringLength?: number;
    maxArrayItems?: number;
}
/**
 * Normalize a tool response to stay within a token budget.
 *
 * - String `result` fields > 8KB are truncated with `...[truncated]` marker.
 * - Array `result` fields > 50 items are sliced with `_total_count` injected.
 * - Object results are JSON-stringified and size-checked.
 * - Adds `_truncated: true` when any truncation occurs.
 */
export declare function normalizeResponse(response: RiftbornResponse, options?: NormalizationOptions): RiftbornResponse;
export interface SessionEntry {
    tool: string;
    ok: boolean;
    duration_ms: number;
    timestamp: number;
    error?: string;
    cached?: boolean;
    paramsKey?: string;
}
export declare function sanitizeSessionEntry(value: unknown): SessionEntry | null;
/**
 * Tracks tool calls across the session in a fixed-size ring buffer.
 *
 * Used for:
 * - Injecting recent context into error responses (_session field)
 * - Exposing session history as a resource (riftborn://session/history)
 * - Detecting rapid duplicate calls
 */
export declare class SessionTracker {
    private buffer;
    private maxSize;
    private totalCalls;
    private totalErrors;
    constructor(maxSize?: number);
    /** Record a tool call. */
    record(entry: SessionEntry): void;
    /** Get the N most recent tool calls. */
    recent(n?: number): SessionEntry[];
    /** Get session summary for injection into error responses. */
    getErrorContext(): {
        calls_in_session: number;
        errors_in_session: number;
        last_3_tools: string[];
    };
    /** Full session history for resource exposure. */
    getHistory(): {
        total_calls: number;
        total_errors: number;
        recent: SessionEntry[];
        tool_frequency: Record<string, number>;
    };
    /** Check if the exact same tool+params was called very recently (within windowMs). */
    isDuplicate(tool: string, paramsKey: string, windowMs?: number): boolean;
    /** Serialize state for persistence. */
    serialize(): {
        buffer: SessionEntry[];
        totalCalls: number;
        totalErrors: number;
    };
    /** Restore state from a persisted snapshot. */
    restore(data: {
        buffer: SessionEntry[];
        totalCalls: number;
        totalErrors: number;
    }): void;
}
//# sourceMappingURL=system-enhancements.d.ts.map