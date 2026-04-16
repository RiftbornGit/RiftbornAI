/**
 * Safe response builders for the MCP dispatch error path and the
 * plan_workflow override. Extracted from index.ts so the entry-point
 * stays focused on wiring and bootstrap.
 *
 * The sanitizer drops prototype-pollution keys, handles cyclic refs,
 * and caps nesting depth — keep this file small and dependency-free.
 */
import { createSanitizer, createToSafeRecord, createMergeSafeRecords, } from "./sanitize-utils.js";
export const sanitizeIndexValue = createSanitizer({
    trackCircular: true,
    depthSentinel: "[MaxDepth]",
    circularSentinel: "[Circular]",
});
export const toSafeRecord = createToSafeRecord(sanitizeIndexValue);
export const mergeSafeRecords = createMergeSafeRecords(toSafeRecord);
export function buildSafeCatchResponse(enriched, options) {
    return mergeSafeRecords(toSafeRecord(enriched), options.diagnostics !== undefined
        ? { _diagnostics: sanitizeIndexValue(options.diagnostics) }
        : undefined, { _session: sanitizeIndexValue(options.session) }, options.resolutionMeta ?? undefined);
}
export function buildSafePlanWorkflowResponse(plan, batchSteps, parallelSafety) {
    return {
        ok: true,
        result: mergeSafeRecords(plan, {
            batch_steps: sanitizeIndexValue(batchSteps),
            parallel_safety: sanitizeIndexValue(parallelSafety),
        }),
    };
}
//# sourceMappingURL=response-builders.js.map