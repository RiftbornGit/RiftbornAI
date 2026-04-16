/**
 * Safe response builders for the MCP dispatch error path and the
 * plan_workflow override. Extracted from index.ts so the entry-point
 * stays focused on wiring and bootstrap.
 *
 * The sanitizer drops prototype-pollution keys, handles cyclic refs,
 * and caps nesting depth — keep this file small and dependency-free.
 */
import type { RiftbornResponse } from "./riftborn-types.js";
export declare const sanitizeIndexValue: import("./sanitize-utils.js").SanitizeFn;
export declare const toSafeRecord: (value: unknown) => Record<string, unknown>;
export declare const mergeSafeRecords: (...records: Array<Record<string, unknown> | null | undefined>) => Record<string, unknown>;
export declare function buildSafeCatchResponse(enriched: RiftbornResponse, options: {
    diagnostics?: unknown;
    session: unknown;
    resolutionMeta?: Record<string, unknown> | null;
}): RiftbornResponse;
export declare function buildSafePlanWorkflowResponse(plan: Record<string, unknown>, batchSteps: unknown, parallelSafety: unknown): RiftbornResponse;
//# sourceMappingURL=response-builders.d.ts.map