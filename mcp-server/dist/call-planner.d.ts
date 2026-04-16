/**
 * Call Planner — Round 17
 *
 * Turns the Tool Dependency Graph (Round 16) into actionable execution plans.
 * Given one or more target tools, the planner:
 *
 * 1. Resolves all transitive prerequisites from the dependency graph
 * 2. Filters out steps already completed in this session
 * 3. Topologically sorts the remaining steps
 * 4. Pre-fills known parameter values from the context propagator snapshot
 * 5. Returns an ordered plan the agent can execute (or feed to batch_execute)
 *
 * Also supports keyword-based goal resolution: "paint terrain with grass" →
 * identifies target tools → builds plan.
 */
import type { SessionEntry } from "./system-enhancements.js";
export declare function normalizeGoalQuery(goal: string): string;
/** A single step in an execution plan. */
export interface PlanStep {
    /** Tool to call. */
    tool: string;
    /** Whether this step was already completed in the session. */
    completed: boolean;
    /** Alternative tools that also satisfy this position (OR-group choices). */
    alternatives: string[];
    /** Pre-filled parameters from context propagator (if available). */
    known_params: Record<string, unknown>;
    /** Human-readable note about this step. */
    note: string;
}
/** A complete execution plan. */
export interface ExecutionPlan {
    /** The goal/target tools this plan achieves. */
    goals: string[];
    /** Ordered steps (earliest prerequisite first, goals last). */
    steps: PlanStep[];
    /** Steps that are already done (for transparency). */
    completed_count: number;
    /** Steps still to execute. */
    remaining_count: number;
    /** Estimated total based on step classification. */
    estimated_calls: number;
}
/**
 * Resolve a goal string to target tools.
 *
 * First checks exact match in GOAL_MAP, then tries substring matching,
 * then checks if it's a direct tool name.
 */
export declare function resolveGoal(goal: string): string[];
/**
 * Build an execution plan for a set of target tools.
 *
 * The plan is a topologically-sorted sequence that includes all
 * prerequisites from the dependency graph, with steps already
 * completed in this session marked accordingly.
 */
export declare function buildPlan(targets: string[], sessionHistory: SessionEntry[], contextSnapshot?: Record<string, unknown>): ExecutionPlan;
/**
 * Build a plan from a natural-language goal string.
 * Resolves the goal to target tools, then builds the plan.
 */
export declare function planFromGoal(goal: string, sessionHistory: SessionEntry[], contextSnapshot?: Record<string, unknown>): ExecutionPlan | null;
/**
 * Convert a plan to a batch_execute-compatible step array.
 * Only includes non-completed steps, with known_params merged in.
 */
export declare function planToBatchSteps(plan: ExecutionPlan): Array<{
    tool: string;
    args: Record<string, unknown>;
}>;
/**
 * List all available goal keywords.
 */
export declare function listGoals(): string[];
//# sourceMappingURL=call-planner.d.ts.map