/**
 * Schema Intelligence — Round 6 enhancements
 *
 * 1. coerceParams — Auto-fix type mismatches agents make (string→number, string→boolean, etc.)
 * 2. validateParams — Local schema validation before bridge round-trip (saves latency)
 * 3. buildRecoveryAction — Structured recovery: exact tool + params to fix a failure
 *
 * The tool surface has 1326 numeric params and 352 booleans. LLMs routinely send these
 * as strings ("100" instead of 100, "true" instead of true). Coercion catches this
 * transparently; validation catches what coercion can't fix.
 */
import type { Tool } from "@modelcontextprotocol/sdk/types.js";
export interface ValidationError {
    ok: false;
    error: string;
    error_category: "parameter_invalid";
    recovery_hint: string;
    retryable: false;
}
export interface RecoveryAction {
    tool: string;
    params?: Record<string, unknown>;
    reason: string;
}
export declare class SchemaIntelligence {
    private schemas;
    constructor(tools: Tool[]);
    /**
     * Coerce parameter values to match the schema's expected types.
     * Handles: string↔number, string↔boolean, number↔boolean (0/1),
     * stringified JSON → object/array.
     *
     * Returns a new object — does not mutate the input.
     */
    coerceParams(toolName: string, params: Record<string, unknown>): Record<string, unknown>;
    /**
     * Validate params against the tool's schema.
     * Returns null if valid, a ValidationError if not.
     *
     * Checks:
     * - Required fields present (not undefined/null/"")
     * - Enum compliance (after coercion)
     * - Basic type compliance (after coercion)
     */
    validateParams(toolName: string, params: Record<string, unknown>): ValidationError | null;
}
/**
 * Build a structured recovery action from an error category.
 * Returns null if no recovery is known for this error type.
 *
 * The returned action tells the agent exactly what tool to call next
 * (with suggested params) to fix the problem.
 */
export declare function buildRecoveryAction(toolName: string, errorCategory: string, error: string): RecoveryAction | null;
//# sourceMappingURL=schema-intelligence.d.ts.map