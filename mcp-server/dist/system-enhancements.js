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
import { createSanitizer, createToSafeRecord } from "./sanitize-utils.js";
const sanitizeStructuredValue = createSanitizer({ maxDepth: 20, trackCircular: true, depthSentinel: "[MaxDepth]", circularSentinel: "[Circular]", sortKeys: true });
const toSafeRecord = createToSafeRecord(sanitizeStructuredValue);
function buildSafeResponse(response, extras = {}) {
    const out = toSafeRecord(response);
    for (const [key, value] of Object.entries(toSafeRecord(extras))) {
        out[key] = value;
    }
    return out;
}
// ---------------------------------------------------------------------------
// 1. ReadCache — TTL cache for read-only bridge calls
// ---------------------------------------------------------------------------
const READ_PREFIXES = ["get_", "list_", "find_", "search_", "describe_", "is_"];
function isReadTool(toolName) {
    const lower = toolName.toLowerCase();
    return READ_PREFIXES.some((p) => lower.startsWith(p));
}
function stableValue(value) {
    return sanitizeStructuredValue(value);
}
function cacheKey(toolName, params) {
    // Sort keys to prevent ordering-dependent cache misses:
    // {a:1,b:2} and {b:2,a:1} must produce the same key.
    const sorted = Object.keys(params)
        .sort()
        .reduce((acc, k) => {
        acc[k] = params[k];
        return acc;
    }, {});
    return toolName + ":v1:" + JSON.stringify(stableValue(sorted));
}
export class ReadCache {
    cache = new Map();
    ttlMs;
    maxEntries;
    constructor(ttlMs = 30_000, maxEntries = 200) {
        this.ttlMs = ttlMs;
        this.maxEntries = maxEntries;
    }
    /** Check if a tool name is cacheable (read-only pattern). */
    isCacheable(toolName) {
        return isReadTool(toolName);
    }
    /** Look up a cached response. Returns null on miss or expiry. */
    get(toolName, params) {
        const key = cacheKey(toolName, params);
        const entry = this.cache.get(key);
        if (!entry)
            return null;
        const now = Date.now();
        if (now > entry.expiresAt) {
            this.cache.delete(key);
            return null;
        }
        const ageMs = Math.round(now - (entry.expiresAt - this.ttlMs));
        return buildSafeResponse(entry.response, { _cached: true, _cache_age_ms: ageMs });
    }
    /** Store a successful read response. */
    set(toolName, params, response) {
        if (!response.ok)
            return; // don't cache errors
        // Evict oldest if full
        if (this.cache.size >= this.maxEntries) {
            const oldest = this.cache.keys().next().value;
            if (oldest !== undefined)
                this.cache.delete(oldest);
        }
        const key = cacheKey(toolName, params);
        this.cache.set(key, {
            response: toSafeRecord(response),
            expiresAt: Date.now() + this.ttlMs,
        });
    }
    /** Invalidate all cached entries. Called when any mutation tool runs. */
    invalidate() {
        this.cache.clear();
    }
    /** Current number of cached entries. */
    get size() {
        return this.cache.size;
    }
}
// ---------------------------------------------------------------------------
// 2. normalizeResponse — Cap oversized responses to save agent tokens
// ---------------------------------------------------------------------------
const DEFAULT_MAX_STRING = 8_192; // 8KB per string field
const DEFAULT_MAX_ARRAY = 50; // 50 items per array
/**
 * Normalize a tool response to stay within a token budget.
 *
 * - String `result` fields > 8KB are truncated with `...[truncated]` marker.
 * - Array `result` fields > 50 items are sliced with `_total_count` injected.
 * - Object results are JSON-stringified and size-checked.
 * - Adds `_truncated: true` when any truncation occurs.
 */
export function normalizeResponse(response, options) {
    const safeResponse = toSafeRecord(response);
    if (!safeResponse.ok || safeResponse.result === undefined || safeResponse.result === null) {
        return safeResponse;
    }
    const maxStr = options?.maxStringLength ?? DEFAULT_MAX_STRING;
    const maxArr = options?.maxArrayItems ?? DEFAULT_MAX_ARRAY;
    const result = sanitizeStructuredValue(safeResponse.result);
    // String result — truncate if too long
    if (typeof result === "string") {
        if (result.length > maxStr) {
            return buildSafeResponse(safeResponse, {
                result: result.substring(0, maxStr) + `\n...[truncated, ${result.length} chars total]`,
                _truncated: true,
                _original_length: result.length,
            });
        }
        return buildSafeResponse(safeResponse, { result });
    }
    // Array result — slice if too many items
    if (Array.isArray(result)) {
        if (result.length > maxArr) {
            return buildSafeResponse(safeResponse, {
                result: result.slice(0, maxArr),
                _truncated: true,
                _total_count: result.length,
                _shown: maxArr,
            });
        }
        return buildSafeResponse(safeResponse, { result });
    }
    // Object result — check serialized size
    if (result && typeof result === "object") {
        const objectResult = result;
        const serialized = JSON.stringify(objectResult);
        if (serialized.length > maxStr) {
            // Try to preserve structure: if it has known list fields, truncate those
            const normalized = truncateObjectFields(objectResult, maxStr, maxArr);
            const newSerialized = JSON.stringify(normalized);
            if (newSerialized.length > maxStr) {
                // Still too big — return truncated JSON string
                return {
                    ...buildSafeResponse(safeResponse, {
                        result: serialized.substring(0, maxStr) + `\n...[truncated, ${serialized.length} chars total]`,
                        _truncated: true,
                        _original_length: serialized.length,
                    }),
                };
            }
            return buildSafeResponse(safeResponse, { result: normalized, _truncated: true });
        }
        return buildSafeResponse(safeResponse, { result: objectResult });
    }
    return buildSafeResponse(safeResponse, { result });
}
function truncateObjectFields(obj, maxStr, maxArr) {
    const out = {};
    for (const [key, value] of Object.entries(obj)) {
        if (typeof value === "string" && value.length > maxStr) {
            out[key] = value.substring(0, maxStr / 2) + `...[truncated]`;
        }
        else if (Array.isArray(value) && value.length > maxArr) {
            out[key] = value.slice(0, maxArr);
            out[`_${key}_total`] = value.length;
        }
        else {
            out[key] = value;
        }
    }
    return out;
}
export function sanitizeSessionEntry(value) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
        return null;
    }
    const raw = toSafeRecord(value);
    if (typeof raw.tool !== "string") {
        return null;
    }
    return {
        tool: raw.tool,
        ok: Boolean(raw.ok),
        duration_ms: typeof raw.duration_ms === "number" && Number.isFinite(raw.duration_ms)
            ? Math.max(0, Math.trunc(raw.duration_ms))
            : 0,
        timestamp: typeof raw.timestamp === "number" && Number.isFinite(raw.timestamp)
            ? raw.timestamp
            : 0,
        ...(typeof raw.error === "string" ? { error: raw.error } : {}),
        ...(typeof raw.cached === "boolean" ? { cached: raw.cached } : {}),
    };
}
/**
 * Tracks tool calls across the session in a fixed-size ring buffer.
 *
 * Used for:
 * - Injecting recent context into error responses (_session field)
 * - Exposing session history as a resource (riftborn://session/history)
 * - Detecting rapid duplicate calls
 */
export class SessionTracker {
    buffer = [];
    maxSize;
    totalCalls = 0;
    totalErrors = 0;
    constructor(maxSize = 50) {
        this.maxSize = maxSize;
    }
    /** Record a tool call. */
    record(entry) {
        this.totalCalls++;
        if (!entry.ok)
            this.totalErrors++;
        this.buffer.push(entry);
        if (this.buffer.length > this.maxSize) {
            this.buffer.shift();
        }
    }
    /** Get the N most recent tool calls. */
    recent(n = 5) {
        return this.buffer.slice(-n);
    }
    /** Get session summary for injection into error responses. */
    getErrorContext() {
        return {
            calls_in_session: this.totalCalls,
            errors_in_session: this.totalErrors,
            last_3_tools: this.buffer.slice(-3).map((e) => `${e.tool}(${e.ok ? "ok" : "ERR"})`),
        };
    }
    /** Full session history for resource exposure. */
    getHistory() {
        const freq = {};
        for (const entry of this.buffer) {
            freq[entry.tool] = (freq[entry.tool] || 0) + 1;
        }
        return {
            total_calls: this.totalCalls,
            total_errors: this.totalErrors,
            recent: this.buffer.slice(-20),
            tool_frequency: freq,
        };
    }
    /** Check if the exact same tool+params was called very recently (within windowMs). */
    isDuplicate(tool, paramsKey, windowMs = 5_000) {
        const cutoff = Date.now() - windowMs;
        for (let i = this.buffer.length - 1; i >= 0; i--) {
            const entry = this.buffer[i];
            if (entry.timestamp < cutoff)
                break;
            // Match on BOTH tool name AND params key to avoid blocking
            // legitimate sequential operations on different targets.
            if (entry.tool === tool && entry.ok && (entry.paramsKey ?? "") === paramsKey) {
                return true;
            }
        }
        return false;
    }
    /** Serialize state for persistence. */
    serialize() {
        return {
            buffer: this.buffer
                .map((entry) => sanitizeSessionEntry(entry))
                .filter((entry) => entry !== null),
            totalCalls: this.totalCalls,
            totalErrors: this.totalErrors,
        };
    }
    /** Restore state from a persisted snapshot. */
    restore(data) {
        this.buffer = (data.buffer ?? [])
            .map((entry) => sanitizeSessionEntry(entry))
            .filter((entry) => entry !== null)
            .slice(-this.maxSize);
        this.totalCalls = typeof data.totalCalls === "number" && Number.isFinite(data.totalCalls)
            ? Math.max(0, Math.trunc(data.totalCalls))
            : 0;
        this.totalErrors = typeof data.totalErrors === "number" && Number.isFinite(data.totalErrors)
            ? Math.max(0, Math.trunc(data.totalErrors))
            : 0;
    }
}
//# sourceMappingURL=system-enhancements.js.map