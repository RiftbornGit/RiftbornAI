/**
 * Compound Operations — Round 9 enhancements
 *
 * High-level operations that compose multiple primitive tool calls into
 * single atomic MCP calls, giving agents new capabilities:
 *
 * 1. executeWorkflow — Run a complete multi-step workflow (landscape, lighting,
 *    character, etc.) in one call. Handles output→input chaining between steps,
 *    iteration (e.g., per-layer operations), and stops on first failure.
 *
 * 2. ensureActor — Atomic find-or-create pattern. Finds actor by label, creates
 *    only if missing. Eliminates the check-then-act round-trip.
 *
 * 3. queryScene — Structured scene introspection in one call. Returns actor
 *    census, categorized by class, with flags for landscape/lighting/etc.
 */
export type ExecuteToolFn = (name: string, params: Record<string, unknown>) => Promise<{
    ok: boolean;
    result?: any;
    error?: string;
}>;
export interface StepResult {
    step: number;
    tool: string;
    ok: boolean;
    result?: unknown;
    error?: string;
    duration_ms: number;
    iterated?: boolean;
}
export interface WorkflowResult {
    ok: boolean;
    workflow: string;
    steps_completed: number;
    steps_total: number;
    results: StepResult[];
    outputs: Record<string, unknown>;
    error?: string;
}
export interface EnsureResult {
    ok: boolean;
    existed: boolean;
    label: string;
    result?: unknown;
    error?: string;
}
export interface SceneState {
    level_name: string;
    level_path: string;
    actor_count: number;
    actors_by_class: Record<string, number>;
    has_landscape: boolean;
    has_directional_light: boolean;
    has_sky_light: boolean;
    has_post_process: boolean;
    has_fog: boolean;
    has_sky_atmosphere: boolean;
    actors_sample: unknown[];
}
export declare function executeWorkflow(workflowName: string, userParams: Record<string, unknown>, executeTool: ExecuteToolFn): Promise<WorkflowResult>;
export declare function ensureActor(label: string, createParams: Record<string, unknown>, executeTool: ExecuteToolFn): Promise<EnsureResult>;
export declare function queryScene(executeTool: ExecuteToolFn, options?: {
    radius?: number;
    max_actors?: number;
}): Promise<{
    ok: boolean;
    state?: SceneState;
    error?: string;
}>;
//# sourceMappingURL=compound-operations.d.ts.map