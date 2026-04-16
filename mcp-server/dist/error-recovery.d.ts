/**
 * Error Recovery — Local-state diagnostics for failed tool calls
 *
 * When a tool call fails, mines existing session state (scene change log,
 * context propagator, session tracker) for diagnostic context. Converts
 * 2-3 agent round-trips into 1 by providing:
 *
 * 1. "Did you mean?" suggestions for actor labels and asset paths
 * 2. Repeated-failure pattern detection and escalation
 * 3. Severity classification and ordered recovery steps
 *
 * All diagnostics are purely local — no additional bridge calls.
 */
/** Minimal subset of SceneChange we need — compatible without import coupling. */
export interface SceneRecord {
    tool: string;
    params: Record<string, unknown>;
    timestamp: number;
}
/** Minimal subset of SessionEntry we need. */
export interface SessionRecord {
    tool: string;
    ok: boolean;
    duration_ms: number;
    timestamp: number;
    error?: string;
}
export interface RepeatedFailure {
    tool: string;
    count: number;
    lastError: string;
    pattern: "same_tool_same_error" | "same_tool_different_errors" | "same_error_different_tools";
}
export interface DiagnosticContext {
    actor_suggestions?: string[];
    asset_suggestions?: string[];
    repeated_failure?: RepeatedFailure;
    recovery_steps: string[];
    severity: "low" | "medium" | "high";
}
export interface DiagnosticInput {
    category: string;
    toolName: string;
    params: Record<string, unknown>;
    error: string;
    sceneRecords: SceneRecord[];
    sessionRecords: SessionRecord[];
    contextState?: Record<string, unknown>;
}
/**
 * Levenshtein-based similarity for short strings.
 * Returns 0.0 (no match) to 1.0 (exact match). Case-insensitive.
 */
export declare function similarity(a: string, b: string): number;
/** Extract known actor labels from scene mutation records. */
export declare function extractKnownActors(sceneRecords: SceneRecord[]): string[];
/** Find similar actor labels. Up to 3, threshold ≥ 0.4, excludes exact matches. */
export declare function suggestActors(target: string, knownActors: string[]): string[];
/** Extract known asset paths from scene records and context state. */
export declare function extractKnownAssets(sceneRecords: SceneRecord[], contextState?: Record<string, unknown>): string[];
/** Find similar asset paths by filename segment. Up to 3. */
export declare function suggestAssets(target: string, knownAssets: string[]): string[];
/**
 * Detect patterns in recent failures: same tool + same error, same tool +
 * different errors, or same error across different tools.
 */
export declare function detectRepeatedFailures(toolName: string, error: string, recent: SessionRecord[]): RepeatedFailure | null;
/**
 * Build diagnostic context by mining local state.
 * No bridge calls — uses only data already collected by other pipeline modules.
 */
export declare function gatherDiagnosticContext(input: DiagnosticInput): DiagnosticContext;
/** Whether diagnostics should be attached for this error category. */
export declare function shouldAttachDiagnostics(category: string, callCount: number): boolean;
//# sourceMappingURL=error-recovery.d.ts.map