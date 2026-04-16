/**
 * Agent Assist Layer — Enriches tool responses for better AI agent performance.
 *
 * Three capabilities:
 * 1. Error enrichment: categorizes failures + adds recovery hints
 * 2. Parameter normalization: fixes common agent parameter mistakes
 * 3. Workflow hints: suggests next steps after key tool successes
 */
import { RiftbornResponse } from "./riftborn-types.js";
export type ErrorCategory = "bridge_disconnected" | "bridge_timeout" | "tool_not_found" | "parameter_invalid" | "prerequisite_missing" | "ue_runtime_error" | "asset_not_found" | "permission_denied" | "unknown";
export interface EnrichedError {
    ok: false;
    error: string;
    error_category: ErrorCategory;
    recovery_hint: string;
    retryable: boolean;
}
/** Enrich a failed tool response with category, recovery hint, and retryable flag. */
export declare function enrichError(toolName: string, response: RiftbornResponse): EnrichedError;
/**
 * Fix common parameter name mistakes.
 * Only copies an alias if the canonical param is missing AND the alias is present.
 * Never overwrites a canonical parameter that was already provided.
 */
export declare function normalizeParams(toolName: string, params: Record<string, unknown>): Record<string, unknown>;
/**
 * Append a workflow hint to a successful tool response.
 * Returns the response unchanged if no hint exists.
 */
export declare function addWorkflowHint(toolName: string, response: RiftbornResponse): RiftbornResponse;
/**
 * Check if a tool call will definitely fail due to missing prerequisites.
 * Returns an EnrichedError if the call is guaranteed to fail, or null if it should proceed.
 */
export declare function checkPrerequisite(toolName: string, params: Record<string, unknown>): EnrichedError | null;
/**
 * Add execution duration to a tool response.
 */
export declare function addTiming(response: RiftbornResponse, startTime: number): RiftbornResponse;
//# sourceMappingURL=agent-assist.d.ts.map