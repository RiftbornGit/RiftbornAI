/**
 * Workflow-level tool handler overrides — these are tools implemented
 * directly in the entry-point because they need live state from
 * singletons (sessionTracker, sceneChangeLog, etc.) plus access to the
 * managed dispatcher. Extracted from index.ts as a single install()
 * function so index.ts stays focused on wiring.
 */
import { getWorkflow, listWorkflows, normalizeWorkflowQuery } from "./tool-compression.js";
import { normalizeGoalQuery } from "./call-planner.js";
import { classifyParallelSafety, } from "./pipeline-refinements.js";
import { buildSafePlanWorkflowResponse, toSafeRecord } from "./response-builders.js";
export function installWorkflowHandlers(deps) {
    const { toolHandlers: TOOL_HANDLERS, dispatchManagedTool, sessionTracker, contextPropagator, sceneChangeLog, progressTracker, sessionBookmarks, pipelineTelemetry, pipelineTraceStore, } = deps;
    // Workflow discovery: powers get_workflow for multi-step sequences
    TOOL_HANDLERS["get_workflow"] = async (args) => {
        const query = normalizeWorkflowQuery(String(args.query || ""));
        if (query === "list" || query === "") {
            return { ok: true, result: { workflows: listWorkflows() } };
        }
        const workflow = getWorkflow(query);
        if (!workflow) {
            return { ok: false, error: `Unknown workflow '${query}'. Use query='list' to see available workflows.` };
        }
        return { ok: true, result: workflow };
    };
    // Call Planner: override with live session state + context propagator
    TOOL_HANDLERS["plan_workflow"] = async (args) => {
        const { planFromGoal: planGoal, planToBatchSteps: toBatch, listGoals: goals } = await import("./call-planner.js");
        const goal = normalizeGoalQuery(String(args.goal || ""));
        if (!goal)
            return { ok: false, error: "plan_workflow requires a 'goal' string." };
        if (goal.toLowerCase() === "list")
            return { ok: true, result: { goals: goals() } };
        const plan = planGoal(goal, sessionTracker.recent(200), contextPropagator.snapshot());
        if (!plan) {
            return { ok: false, error: `Unrecognized goal '${goal}'. Use goal='list' to see available goals, or pass an exact tool name.` };
        }
        const parallelSafety = classifyParallelSafety(plan.steps.map((s) => s.tool));
        return buildSafePlanWorkflowResponse(toSafeRecord(plan), toBatch(plan), parallelSafety);
    };
    // Autonomous Verification: override with live session state
    TOOL_HANDLERS["verify_session"] = async (args) => {
        const { generatePlan, runPlan } = await import("./autonomous-verify.js");
        const scope = String(args.scope || "all").toLowerCase();
        const changes = sceneChangeLog.recent(200);
        const SCOPE_CATEGORIES = {
            actors: new Set(["actor"]),
            materials: new Set(["material"]),
            blueprints: new Set(["blueprint"]),
            lighting: new Set(["light"]),
            landscape: new Set(["landscape"]),
        };
        const filtered = scope === "all"
            ? changes
            : changes.filter(c => SCOPE_CATEGORIES[scope]?.has(c.category));
        const plan = generatePlan({
            sceneChanges: filtered,
            milestones: progressTracker.getMilestones(),
            context: contextPropagator.snapshot(),
        });
        if (plan.checks.length === 0) {
            return { ok: true, result: { overall: "PASS", total: 0, passed: 0, failed: 0, warned: 0, checks: [], duration_ms: 0, note: "No verifiable changes found in session history." } };
        }
        const report = await runPlan(plan, dispatchManagedTool, filtered);
        return { ok: report.overall !== "FAIL", result: report };
    };
    TOOL_HANDLERS["smoke_test_pie"] = async (args) => {
        const { pieSmokeTest } = await import("./autonomous-verify.js");
        const duration = typeof args.duration_seconds === "number"
            ? Math.min(Math.max(2, args.duration_seconds), 30)
            : 3;
        const maxErrors = typeof args.max_errors === "number" ? args.max_errors : 0;
        const result = await pieSmokeTest(dispatchManagedTool, { duration_seconds: duration, max_errors: maxErrors });
        return { ok: result.overall === "PASS", result };
    };
    // Round 21: Session bookmarks — named checkpoints for scoped undo/verify
    TOOL_HANDLERS["bookmark_session"] = async (args) => {
        const name = String(args.name || "").trim();
        if (!name)
            return { ok: false, error: "bookmark_session requires a 'name' string." };
        const recent = sceneChangeLog.recent(1);
        const changeId = recent.length > 0 ? recent[0].id ?? recent[0].tool + "_" + Date.now() : "genesis";
        const bm = sessionBookmarks.mark(name, changeId);
        return { ok: true, result: bm };
    };
    TOOL_HANDLERS["list_bookmarks"] = async () => {
        return { ok: true, result: { bookmarks: sessionBookmarks.list() } };
    };
    // Round 22: Pipeline telemetry — which features actually fire
    TOOL_HANDLERS["pipeline_stats"] = async () => {
        return { ok: true, result: pipelineTelemetry.getReport() };
    };
    // Round 24: Pipeline trace — per-call stage detail
    TOOL_HANDLERS["pipeline_trace"] = async (args) => {
        if (args.stage_stats) {
            return { ok: true, result: { stage_stats: pipelineTraceStore.stageStats() } };
        }
        if (args.hotspots) {
            const top = Math.min(Math.max(1, Number(args.top) || 10), 50);
            const min_calls = Math.max(1, Number(args.min_calls) || 1);
            const slow_threshold_ms = Math.max(1, Number(args.slow_threshold_ms) || 1500);
            return {
                ok: true,
                result: {
                    hotspots: pipelineTraceStore.hotspots({ top, min_calls, slow_threshold_ms }),
                },
            };
        }
        const count = Math.min(Math.max(1, Number(args.count) || 20), 200);
        const report = pipelineTraceStore.getReport({
            count,
            tool: args.tool,
            slow: !!args.slow,
        });
        return { ok: true, result: report };
    };
    // Rollback: undo the last N operations
    TOOL_HANDLERS["undo_last"] = async (args) => {
        const { executeRollback } = await import("./dispatch-hooks.js");
        const count = Math.min(Math.max(1, Number(args.count) || 1), 20);
        const steps = sceneChangeLog.getUndoPlan(count);
        if (steps.length === 0) {
            return { ok: true, result: { steps_attempted: 0, steps_succeeded: 0, steps_failed: 0, results: [], duration_ms: 0, note: "Nothing to undo." } };
        }
        const report = await executeRollback(steps, dispatchManagedTool);
        return { ok: report.steps_failed === 0, result: report };
    };
    // Rollback: undo operations matching a filter
    TOOL_HANDLERS["rollback"] = async (args) => {
        const { executeRollback, filterUndoSteps } = await import("./dispatch-hooks.js");
        const count = Math.min(Math.max(1, Number(args.count) || 20), 200);
        const allSteps = sceneChangeLog.getUndoPlan(count);
        const recentChanges = sceneChangeLog.recent(count);
        const filtered = filterUndoSteps(allSteps, { category: args.category, label: args.label }, recentChanges);
        if (filtered.length === 0) {
            return { ok: true, result: { steps_attempted: 0, steps_succeeded: 0, steps_failed: 0, results: [], duration_ms: 0, note: "No matching operations to undo." } };
        }
        const report = await executeRollback(filtered, dispatchManagedTool);
        return { ok: report.steps_failed === 0, result: report };
    };
}
//# sourceMappingURL=workflow-handlers.js.map