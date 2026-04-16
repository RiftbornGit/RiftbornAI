/**
 * Shared recursive value sanitization utilities.
 *
 * Every module in the MCP server needs to strip prototype-pollution keys
 * from untrusted objects before merging/returning them.  This module provides
 * a configurable factory so each consumer can specify depth limits, circular
 * detection, and key-sorting without duplicating the same 20-line recursion.
 */
/** Prototype-pollution keys stripped from every untrusted object. */
export declare const PROTO_BLOCKED_KEYS: ReadonlySet<string>;
export interface SanitizeOptions {
    /** Maximum recursion depth (default 12). */
    maxDepth?: number;
    /** Keys to strip from every object (default PROTO_BLOCKED_KEYS). */
    blockedKeys?: ReadonlySet<string>;
    /** Value returned when maxDepth is exceeded (default null). */
    depthSentinel?: unknown;
    /** Value returned for circular references (default null). Only used when trackCircular is true. */
    circularSentinel?: unknown;
    /** Track and break circular references with a WeakSet (default false). */
    trackCircular?: boolean;
    /** Sort object keys lexicographically (default false). */
    sortKeys?: boolean;
}
export type SanitizeFn = (value: unknown, depth?: number, seen?: WeakSet<object>) => unknown;
/**
 * Create a recursive sanitizer with the given configuration.
 *
 * The returned function:
 *  - strips keys listed in `blockedKeys`
 *  - caps recursion at `maxDepth`
 *  - optionally detects circular references
 *  - optionally sorts object keys
 */
export declare function createSanitizer(options?: SanitizeOptions): SanitizeFn;
/**
 * Create a `toSafeRecord` function that sanitizes a value, then unwraps it
 * to a plain record.  Non-object / array results become `{}`.
 */
export declare function createToSafeRecord(sanitize: SanitizeFn): (value: unknown) => Record<string, unknown>;
/**
 * Create a `mergeSafeRecords` function that shallow-merges multiple records
 * after passing each through `toSafeRecord`.
 */
export declare function createMergeSafeRecords(toSafeRecord: (value: unknown) => Record<string, unknown>): (...records: Array<Record<string, unknown> | null | undefined>) => Record<string, unknown>;
//# sourceMappingURL=sanitize-utils.d.ts.map