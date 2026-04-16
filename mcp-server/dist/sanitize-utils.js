/**
 * Shared recursive value sanitization utilities.
 *
 * Every module in the MCP server needs to strip prototype-pollution keys
 * from untrusted objects before merging/returning them.  This module provides
 * a configurable factory so each consumer can specify depth limits, circular
 * detection, and key-sorting without duplicating the same 20-line recursion.
 */
/** Prototype-pollution keys stripped from every untrusted object. */
export const PROTO_BLOCKED_KEYS = new Set([
    "__proto__",
    "constructor",
    "prototype",
]);
/**
 * Create a recursive sanitizer with the given configuration.
 *
 * The returned function:
 *  - strips keys listed in `blockedKeys`
 *  - caps recursion at `maxDepth`
 *  - optionally detects circular references
 *  - optionally sorts object keys
 */
export function createSanitizer(options = {}) {
    const { maxDepth = 12, blockedKeys = PROTO_BLOCKED_KEYS, depthSentinel = null, circularSentinel = null, trackCircular = false, sortKeys = false, } = options;
    return function sanitize(value, depth = 0, seen) {
        if (value === null || value === undefined)
            return value;
        if (depth >= maxDepth)
            return depthSentinel;
        if (Array.isArray(value)) {
            return value.map((item) => sanitize(item, depth + 1, seen));
        }
        if (typeof value === "object") {
            if (trackCircular) {
                if (!seen)
                    seen = new WeakSet();
                if (seen.has(value))
                    return circularSentinel;
                seen.add(value);
            }
            const entries = Object.entries(value);
            if (sortKeys)
                entries.sort(([a], [b]) => a.localeCompare(b));
            const out = {};
            for (const [key, entry] of entries) {
                if (!blockedKeys.has(key)) {
                    out[key] = sanitize(entry, depth + 1, seen);
                }
            }
            if (trackCircular && seen)
                seen.delete(value);
            return out;
        }
        return value;
    };
}
/**
 * Create a `toSafeRecord` function that sanitizes a value, then unwraps it
 * to a plain record.  Non-object / array results become `{}`.
 */
export function createToSafeRecord(sanitize) {
    return function toSafeRecord(value) {
        const sanitized = sanitize(value);
        if (!sanitized || typeof sanitized !== "object" || Array.isArray(sanitized)) {
            return {};
        }
        return sanitized;
    };
}
/**
 * Create a `mergeSafeRecords` function that shallow-merges multiple records
 * after passing each through `toSafeRecord`.
 */
export function createMergeSafeRecords(toSafeRecord) {
    return function mergeSafeRecords(...records) {
        const out = {};
        for (const record of records) {
            if (!record)
                continue;
            for (const [key, value] of Object.entries(toSafeRecord(record))) {
                out[key] = value;
            }
        }
        return out;
    };
}
//# sourceMappingURL=sanitize-utils.js.map