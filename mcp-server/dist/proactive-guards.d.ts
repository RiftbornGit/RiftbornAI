/**
 * Proactive Guards — Round 8 enhancements
 *
 * Three capabilities that prevent wasted tool calls before they happen:
 *
 * 1. ParameterDefaults — Smart defaults for common optional parameters.
 *    When an agent omits `type` on `create_light`, inject "PointLight".
 *    When `sculpt_landscape` has no `radius`, inject 500. Reduces round-trips
 *    caused by UE returning unhelpful "parameter missing" errors for optional
 *    params the C++ side actually requires.
 *
 * 2. DependencyGuard — Session-history-aware prerequisite checking.
 *    `checkPrerequisite` (Round 2) checks param presence. This checks whether
 *    the agent has actually called the prerequisite tool this session. E.g.,
 *    calling `paint_landscape_layer` without any `create_landscape` in history
 *    → early warning with the exact tool to call first.
 *
 * 3. IdempotencyGuard — Detects and warns about duplicate mutation calls.
 *    Catches "same tool + same params within 5 seconds" for mutation tools.
 *    Wires the existing SessionTracker.isDuplicate into the pipeline.
 */
import type { SessionEntry } from "./system-enhancements.js";
/**
 * Inject smart defaults for missing optional parameters.
 *
 * Only fills in values for keys that are NOT already present in the params.
 * Never overwrites anything the agent explicitly provided.
 *
 * Returns { params, defaults_applied } so the pipeline can stamp
 * which defaults were auto-injected (transparency for the agent).
 */
export declare function applyDefaults(toolName: string, params: Record<string, unknown>): {
    params: Record<string, unknown>;
    defaults_applied: Record<string, unknown>;
};
export interface DependencyWarning {
    tool: string;
    missing_prerequisite: string;
    message: string;
    suggestion: string;
}
/**
 * Check whether all prerequisite tools have been called in this session.
 *
 * Takes the session history (from SessionTracker) and checks against the
 * dependency rules. Returns null if all deps are satisfied, or a warning
 * about the first missing prerequisite.
 *
 * This is a WARNING, not a hard block — the tool may still succeed if
 * the prerequisite was created in a prior session or exists in the level.
 */
export declare function checkDependencies(toolName: string, sessionHistory: SessionEntry[]): DependencyWarning | null;
export interface DuplicateWarning {
    tool: string;
    message: string;
    last_call_ago_ms: number;
}
/**
 * Check if this mutation call duplicates a recent one.
 *
 * Returns null if:
 * - The tool is read-only
 * - The tool is explicitly idempotent
 * - No matching call in the recent session history window
 *
 * Returns a warning if the same tool+params was successfully called within windowMs.
 */
export declare function checkIdempotency(toolName: string, params: Record<string, unknown>, sessionHistory: SessionEntry[], windowMs?: number): DuplicateWarning | null;
//# sourceMappingURL=proactive-guards.d.ts.map