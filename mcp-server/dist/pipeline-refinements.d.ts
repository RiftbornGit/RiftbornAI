/**
 * Pipeline Refinements — Round 21
 *
 * Ten targeted improvements across the dispatch pipeline:
 *
 *  1. canonicalizeOutput — Normalize inconsistent field names from C++ responses
 *  2. preflightEntityCheck — Verify referenced entities exist in scene state
 *  3. scopeDigest — Filter digest to match agent's current activity focus
 *  4. buildParamEcho — Echo actual params sent after all pipeline transformations
 *  5. SessionBookmarks — Named save points in the session timeline
 *  6. estimateToolDuration — Attach timing predictions from session history
 *  7. classifyParallelSafety — Classify which planned tools can run concurrently
 *  8. ResponseDeltaTracker — Track what changed between repeated reads
 *  9. buildContextSummary — Summarize what the context propagator knows
 * 10. FailureBudget — Circuit breaker for tools that keep failing
 */
import type { SceneChange } from "./scene-safety.js";
import type { Digest, BuildPhase } from "./session-intelligence.js";
import type { SessionEntry } from "./system-enhancements.js";
/**
 * Add canonical field aliases to a tool result so downstream consumers
 * (context propagator, micro-verify, error recovery) see consistent names.
 * Original fields are preserved; canonical aliases are added only if absent.
 */
export declare function canonicalizeOutput(toolName: string, result: Record<string, unknown>): Record<string, unknown>;
/**
 * Check if a tool references an entity that was deleted in this session.
 * Returns a warning string if the entity was deleted, null otherwise.
 * Does NOT warn about entities never seen — they may predate the session.
 */
export declare function preflightEntityCheck(toolName: string, params: Record<string, unknown>, changes: SceneChange[]): string | null;
/**
 * Detect the agent's current focus phase from recent tool calls.
 * Returns a phase only if ≥60% of the window belongs to one phase.
 */
export declare function detectCurrentFocus(recentTools: string[], windowSize?: number): BuildPhase | null;
export type ScopedDigest = Digest & {
    _focus?: BuildPhase;
};
/**
 * Filter a digest to only show milestones relevant to the agent's current
 * focus. Keeps the focused phase plus its neighbors in build order.
 * Returns the full digest unchanged if no clear focus is detected.
 */
export declare function scopeDigest(digest: Digest, recentTools: string[]): ScopedDigest;
export interface ParamEcho {
    final_params: Record<string, unknown>;
    defaults_applied: string[];
    context_inferred: string[];
}
/**
 * Build a param echo block showing what was actually sent to the bridge
 * after all pipeline transformations. Returns null if nothing was auto-filled.
 */
export declare function buildParamEcho(finalParams: Record<string, unknown>, defaultsApplied: Record<string, unknown>, inferred: Record<string, unknown>): ParamEcho | null;
export interface Bookmark {
    name: string;
    changeId: number;
    timestamp: number;
}
export declare class SessionBookmarks {
    private bookmarks;
    mark(name: string, changeId: number, timestamp?: number): Bookmark;
    get(name: string): Bookmark | undefined;
    list(): Bookmark[];
    /** Return changes that occurred after a named bookmark. */
    changesSince(name: string, allChanges: SceneChange[]): SceneChange[];
    clear(): void;
    toJSON(): Record<string, Bookmark>;
    loadFrom(data: Record<string, Bookmark>): void;
}
export interface DurationPrediction {
    estimate_ms: number;
    confidence: "none" | "low" | "high";
}
/**
 * Predict how long a tool call will take, using historical session data
 * when available, falling back to category baselines.
 */
export declare function estimateToolDuration(toolName: string, recentEntries: SessionEntry[]): DurationPrediction;
export interface ParallelClassification {
    parallel: string[];
    serial: string[];
}
/**
 * Classify a set of planned tools into those safe to run concurrently (reads)
 * and those that must run serially (mutations, vision, builds).
 * Matches the ToolExecutionQueue's read/write classification.
 */
export declare function classifyParallelSafety(tools: string[]): ParallelClassification;
/**
 * Tracks the last response per tool and computes deltas when the same
 * tool is called again. Only meaningful for observation/query tools.
 */
export declare class ResponseDeltaTracker {
    private last;
    /**
     * Record a new result. Returns a delta object if this tool was called
     * before and the result changed, null otherwise.
     */
    track(toolName: string, result: Record<string, unknown>): Record<string, {
        was: unknown;
        now: unknown;
    }> | null;
    clear(): void;
}
export interface ContextSummary {
    known_entities: string[];
    known_paths: string[];
    field_count: number;
}
/**
 * Build a compact summary of what the context propagator currently knows.
 * Useful for debugging why context injection did or didn't work.
 */
export declare function buildContextSummary(snapshot: Record<string, unknown>): ContextSummary;
export interface FailureCheck {
    blocked: boolean;
    reason?: string;
    failures: number;
}
export interface FailureStatus {
    blocked_tools: string[];
    at_risk: string[];
}
/**
 * Circuit breaker that blocks tools after repeated consecutive failures.
 * Prevents agents from wasting turns retrying broken tools.
 * Consecutive count decays on success, resets on explicit reset.
 */
export declare class FailureBudget {
    private state;
    recordFailure(tool: string, error?: string): void;
    recordSuccess(tool: string): void;
    check(tool: string): FailureCheck;
    getStatus(): FailureStatus;
    reset(tool: string): void;
    clear(): void;
}
//# sourceMappingURL=pipeline-refinements.d.ts.map