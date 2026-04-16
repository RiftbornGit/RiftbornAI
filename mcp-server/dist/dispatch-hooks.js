/**
 * Round 20 — Dispatch Lifecycle Hooks
 *
 * Two capabilities that give agents fail-fast + auto-recovery:
 *
 * 1. Post-dispatch micro-verification — After any creation tool succeeds,
 *    immediately run a fast assert to confirm the thing actually exists.
 *    If it doesn't, annotate the response with _verify_failed so the agent
 *    knows not to build on a broken foundation.
 *
 * 2. Rollback execution — Read SceneChangeLog's undo plans and actually
 *    execute the reverse operations via dispatch. Two tools:
 *      undo_last  — undo the last N operations (default 1)
 *      rollback   — undo all changes matching a scope/label filter
 */
const MICRO_VERIFY_RULES = {
    spawn_actor: {
        assertTool: "assert_actor_exists",
        getParams: (p) => p.label ? { label: p.label } : null,
        getLabel: (p) => `Actor "${p.label}" exists after spawn`,
    },
    create_static_mesh_actor: {
        assertTool: "assert_actor_exists",
        getParams: (p) => p.label ? { label: p.label } : null,
        getLabel: (p) => `Static mesh "${p.label}" exists after create`,
    },
    create_light: {
        assertTool: "assert_actor_exists",
        getParams: (p) => p.label ? { label: p.label } : null,
        getLabel: (p) => `Light "${p.label}" exists after create`,
    },
    create_landscape: {
        assertTool: "assert_actor_exists",
        getParams: (p) => ({ label: p.label || "Landscape" }),
        getLabel: (p) => `Landscape "${p.label || "Landscape"}" exists`,
    },
    duplicate_actor: {
        assertTool: "assert_actor_exists",
        getParams: (p) => p.label ? { label: p.label } : null,
        getLabel: (p) => `Duplicate "${p.label}" exists`,
    },
    spawn_third_person_character: {
        assertTool: "assert_actor_exists",
        getParams: (p) => p.label ? { label: p.label } : null,
        getLabel: (p) => `Character "${p.label}" exists`,
    },
    create_character_from_third_person: {
        assertTool: "assert_actor_exists",
        getParams: (p) => p.label ? { label: p.label } : null,
        getLabel: (p) => `Character "${p.label}" exists`,
    },
    compile_blueprint: {
        assertTool: "assert_blueprint_compiles",
        getParams: (p) => p.blueprint_path ? { asset_path: p.blueprint_path } : {},
        getLabel: (p) => p.blueprint_path ? `Blueprint "${p.blueprint_path}" compiles` : "Blueprint compiles",
    },
};
/**
 * Run a fast micro-verification after a successful tool dispatch.
 * Returns null if the tool doesn't have a verify rule.
 */
export async function microVerify(toolName, toolParams, dispatch) {
    const rule = MICRO_VERIFY_RULES[toolName];
    if (!rule)
        return null;
    const assertParams = rule.getParams(toolParams);
    if (!assertParams)
        return null;
    const label = rule.getLabel(toolParams);
    const t0 = performance.now();
    try {
        const result = await dispatch(rule.assertTool, assertParams);
        return {
            verified: result.ok,
            tool: rule.assertTool,
            label,
            error: result.ok ? undefined : (result.error ?? "assertion failed"),
            duration_ms: Math.round(performance.now() - t0),
        };
    }
    catch (err) {
        return {
            verified: false,
            tool: rule.assertTool,
            label,
            error: err instanceof Error ? err.message : String(err),
            duration_ms: Math.round(performance.now() - t0),
        };
    }
}
/**
 * Check if a tool has a micro-verify rule. Used by index.ts to decide
 * whether to run post-dispatch verification.
 */
export function hasMicroVerifyRule(toolName) {
    return toolName in MICRO_VERIFY_RULES;
}
// ────────────────────────────────────────────────────────────────────────────
// Rollback execution
//
// Takes UndoStep[] from SceneChangeLog.getUndoPlan() and executes them
// via dispatch. Returns a structured report.
// ────────────────────────────────────────────────────────────────────────────
const MAX_ROLLBACK_STEPS = 20;
export async function executeRollback(undoSteps, dispatch) {
    const startTime = performance.now();
    const clamped = undoSteps.slice(0, MAX_ROLLBACK_STEPS);
    const results = [];
    let succeeded = 0;
    let failed = 0;
    for (const step of clamped) {
        const t0 = performance.now();
        try {
            const response = await dispatch(step.tool, step.params);
            const ms = Math.round(performance.now() - t0);
            if (response.ok) {
                succeeded++;
                results.push({ description: step.description, tool: step.tool, ok: true, duration_ms: ms });
            }
            else {
                failed++;
                results.push({ description: step.description, tool: step.tool, ok: false, error: response.error, duration_ms: ms });
            }
        }
        catch (err) {
            failed++;
            results.push({
                description: step.description,
                tool: step.tool,
                ok: false,
                error: err instanceof Error ? err.message : String(err),
                duration_ms: Math.round(performance.now() - t0),
            });
        }
    }
    return {
        steps_attempted: clamped.length,
        steps_succeeded: succeeded,
        steps_failed: failed,
        results,
        duration_ms: Math.round(performance.now() - startTime),
    };
}
/**
 * Filter undo steps by category or label for targeted rollback.
 */
export function filterUndoSteps(steps, filter, changes) {
    if (!filter.category && !filter.label)
        return steps;
    // Build a map from undo step description to the original change
    // UndoStep descriptions contain the label, so we can match back
    return steps.filter((step, idx) => {
        const change = changes[idx]; // steps are in reverse order from changes
        if (!change)
            return false;
        if (filter.category && change.category !== filter.category)
            return false;
        if (filter.label && change.label !== filter.label)
            return false;
        return true;
    });
}
//# sourceMappingURL=dispatch-hooks.js.map