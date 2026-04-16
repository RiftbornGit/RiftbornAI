/**
 * Round 19 — Autonomous Verification Engine
 *
 * After an AI agent builds things via RiftbornAI, this module generates and
 * runs structured verification checks based on what was actually done.
 *
 * Core idea: SceneChangeLog knows every create/modify/delete. ContextPropagator
 * knows the last landscape/actor/material/blueprint. This module reads that
 * tracked state and converts it into assert_* tool calls that run in UE,
 * returning structured PASS/FAIL — no expensive vision AI needed.
 *
 * Two user-facing tools:
 *   verify_session  — auto-generate + run checks from session history
 *   smoke_test_pie  — start PIE → structural checks → stop PIE → report
 */
const CHANGE_VERIFIERS = {
    // Actor creates → assert exists
    spawn_actor: (c) => c.label ? [{
            tool: "assert_actor_exists",
            params: { label: c.label },
            label: `Actor "${c.label}" exists`,
            severity: "error",
        }] : [],
    create_static_mesh_actor: (c) => c.label ? [{
            tool: "assert_actor_exists",
            params: { label: c.label },
            label: `Static mesh "${c.label}" exists`,
            severity: "error",
        }] : [],
    duplicate_actor: (c) => c.label ? [{
            tool: "assert_actor_exists",
            params: { label: c.label },
            label: `Duplicate "${c.label}" exists`,
            severity: "error",
        }] : [],
    create_light: (c) => c.label ? [{
            tool: "assert_actor_exists",
            params: { label: c.label },
            label: `Light "${c.label}" exists`,
            severity: "error",
        }] : [],
    create_landscape: (c) => [{
            tool: "assert_actor_exists",
            params: { label: c.label || "Landscape" },
            label: "Landscape exists",
            severity: "error",
        }],
    create_post_process_volume: (c) => c.label ? [{
            tool: "assert_actor_exists",
            params: { label: c.label },
            label: `PPV "${c.label}" exists`,
            severity: "error",
        }] : [{
            tool: "assert_actor_exists",
            params: { label: "PostProcessVolume" },
            label: "Post-process volume exists",
            severity: "warning",
        }],
    spawn_third_person_character: (c) => c.label ? [{
            tool: "assert_actor_exists",
            params: { label: c.label },
            label: `Character "${c.label}" exists`,
            severity: "error",
        }] : [],
    create_character_from_third_person: (c) => c.label ? [{
            tool: "assert_actor_exists",
            params: { label: c.label },
            label: `Character "${c.label}" exists`,
            severity: "error",
        }] : [],
    // Blueprint compile → assert compiles
    compile_blueprint: (c) => {
        const path = c.params?.blueprint_path;
        return path ? [{
                tool: "assert_blueprint_compiles",
                params: { asset_path: path },
                label: `Blueprint "${path}" compiles`,
                severity: "error",
            }] : [{
                tool: "assert_blueprint_compiles",
                params: {},
                label: "Active blueprint compiles",
                severity: "error",
            }];
    },
    // Delete → assert DOESN'T exist (verify cleanup)
    delete_actor: (c) => c.label ? [{
            tool: "assert_actor_exists",
            params: { label: c.label },
            label: `Deleted "${c.label}" is gone`,
            severity: "warning",
            // Note: this check EXPECTS failure — handled in runPlan
        }] : [],
};
// We tag delete checks so runPlan knows to invert the result
const DELETE_TOOLS = new Set(["delete_actor"]);
// ────────────────────────────────────────────────────────────────────────────
// Structural checks (always appended)
// ────────────────────────────────────────────────────────────────────────────
const STRUCTURAL_CHECKS = [
    {
        tool: "assert_no_modal_blockers",
        params: {},
        label: "No modal dialogs blocking editor",
        severity: "error",
    },
    {
        tool: "assert_output_log_clean",
        params: { max_errors: 0 },
        label: "No errors in output log",
        severity: "warning",
    },
];
// ────────────────────────────────────────────────────────────────────────────
// Phase-based checks (from ProgressTracker milestones)
// ────────────────────────────────────────────────────────────────────────────
const PHASE_VERIFIERS = {
    terrain: [{
            tool: "assert_actor_exists",
            params: { label: "Landscape" },
            label: "Terrain: landscape actor exists",
            severity: "warning",
        }],
    lighting: [{
            tool: "assert_output_log_clean",
            params: { max_errors: 0 },
            label: "Lighting: no errors after light setup",
            severity: "warning",
        }],
};
export function generatePlan(input) {
    const checks = [];
    const seen = new Set();
    // 1. Generate checks from scene changes
    for (const change of input.sceneChanges) {
        const verifier = CHANGE_VERIFIERS[change.tool];
        if (!verifier)
            continue;
        for (const check of verifier(change)) {
            // Deduplicate by tool+label
            const key = `${check.tool}:${check.label}`;
            if (seen.has(key))
                continue;
            seen.add(key);
            checks.push(check);
        }
    }
    // 2. Add phase-based checks from milestones
    if (input.milestones) {
        const phases = new Set(input.milestones.map(m => m.phase));
        for (const phase of phases) {
            const phaseChecks = PHASE_VERIFIERS[phase];
            if (!phaseChecks)
                continue;
            for (const check of phaseChecks) {
                const key = `${check.tool}:${check.label}`;
                if (seen.has(key))
                    continue;
                seen.add(key);
                checks.push(check);
            }
        }
    }
    // 3. Add context-based checks
    if (input.context?.last_blueprint_path) {
        const key = "assert_blueprint_compiles:ctx_bp";
        if (!seen.has(key)) {
            seen.add(key);
            checks.push({
                tool: "assert_blueprint_compiles",
                params: { asset_path: input.context.last_blueprint_path },
                label: `Blueprint "${input.context.last_blueprint_path}" compiles`,
                severity: "error",
            });
        }
    }
    // 4. Always add structural checks
    for (const check of STRUCTURAL_CHECKS) {
        const key = `${check.tool}:${check.label}`;
        if (!seen.has(key)) {
            seen.add(key);
            checks.push(check);
        }
    }
    return {
        checks,
        generated_from: "session_history",
        change_count: input.sceneChanges.length,
    };
}
// ────────────────────────────────────────────────────────────────────────────
// Plan execution
// ────────────────────────────────────────────────────────────────────────────
/**
 * Identify checks that test deletion (expect failure from assert_actor_exists).
 */
function isDeleteVerification(check, sceneChanges) {
    if (check.tool !== "assert_actor_exists")
        return false;
    const label = check.params.label;
    return sceneChanges.some(c => DELETE_TOOLS.has(c.tool) && c.label === label);
}
export async function runPlan(plan, dispatch, sceneChanges = []) {
    const startTime = performance.now();
    const results = [];
    let failed = 0;
    let warned = 0;
    for (const check of plan.checks) {
        const t0 = performance.now();
        try {
            const response = await dispatch(check.tool, check.params);
            const durationMs = Math.round(performance.now() - t0);
            // Delete verifications invert: if assert_actor_exists FAILS, that's good
            const expectFailure = isDeleteVerification(check, sceneChanges);
            const passed = expectFailure ? !response.ok : response.ok;
            results.push({
                label: check.label + (expectFailure ? " (expect absent)" : ""),
                tool: check.tool,
                passed,
                error: passed ? undefined : (response.error ?? "assertion failed"),
                duration_ms: durationMs,
            });
            if (!passed) {
                if (check.severity === "error")
                    failed++;
                else
                    warned++;
            }
        }
        catch (err) {
            const durationMs = Math.round(performance.now() - t0);
            results.push({
                label: check.label,
                tool: check.tool,
                passed: false,
                error: err instanceof Error ? err.message : String(err),
                duration_ms: durationMs,
            });
            if (check.severity === "error")
                failed++;
            else
                warned++;
        }
    }
    const totalMs = Math.round(performance.now() - startTime);
    const overall = failed > 0 ? "FAIL" : warned > 0 ? "WARN" : "PASS";
    const report = {
        overall,
        total: results.length,
        passed: results.filter(r => r.passed).length,
        failed,
        warned,
        checks: results,
        duration_ms: totalMs,
    };
    if (failed > 0) {
        const failedChecks = results.filter(r => !r.passed).map(r => r.label);
        report.suggestion = `Fix: ${failedChecks.slice(0, 3).join(", ")}${failedChecks.length > 3 ? ` (+${failedChecks.length - 3} more)` : ""}`;
    }
    return report;
}
const PIE_CRASH_PATTERNS = /crash|ECONNREFUSED|ETIMEDOUT|socket hang up|bridge disconnected|fatal/i;
export async function pieSmokeTest(dispatch, opts = {}) {
    const startTime = performance.now();
    const duration = Math.min(Math.max(2, opts.duration_seconds ?? 3), 30);
    const maxErrors = opts.max_errors ?? 0;
    const preChecks = [];
    const runtimeChecks = [];
    const postChecks = [];
    let pieStarted = false;
    // ── Pre-checks ──
    const preAssertions = [
        { tool: "assert_no_modal_blockers", params: {}, label: "Pre-PIE: no modal blockers", severity: "error" },
        { tool: "assert_output_log_clean", params: { max_errors: 0 }, label: "Pre-PIE: output log clean", severity: "warning" },
    ];
    for (const check of preAssertions) {
        const t0 = performance.now();
        try {
            const r = await dispatch(check.tool, check.params);
            preChecks.push({ label: check.label, tool: check.tool, passed: r.ok, error: r.ok ? undefined : r.error, duration_ms: Math.round(performance.now() - t0) });
        }
        catch (err) {
            preChecks.push({ label: check.label, tool: check.tool, passed: false, error: err instanceof Error ? err.message : String(err), duration_ms: Math.round(performance.now() - t0) });
        }
    }
    // Abort if hard pre-check failure
    const preHardFail = preChecks.some(c => !c.passed && preAssertions.find(a => a.label === c.label)?.severity === "error");
    if (preHardFail) {
        return {
            overall: "FAIL",
            pre_checks: preChecks,
            pie_started: false,
            runtime_checks: [],
            post_checks: [],
            duration_ms: Math.round(performance.now() - startTime),
            suggestion: "Fix modal blockers before starting PIE.",
        };
    }
    // ── Start PIE ──
    const pieT0 = performance.now();
    try {
        const pieResult = await dispatch("start_pie", { mode: "viewport", num_players: 1 });
        const pieDuration = Math.round(performance.now() - pieT0);
        if (!pieResult.ok) {
            const isCrash = PIE_CRASH_PATTERNS.test(pieResult.error ?? "");
            return {
                overall: isCrash ? "CRASH" : "FAIL",
                pre_checks: preChecks,
                pie_started: false,
                runtime_checks: [{
                        label: "Start PIE", tool: "start_pie", passed: false,
                        error: pieResult.error, duration_ms: pieDuration,
                    }],
                post_checks: [],
                duration_ms: Math.round(performance.now() - startTime),
                suggestion: isCrash ? "PIE likely crashed. Run diagnose_crash() to investigate." : "PIE failed to start. Check for compile errors.",
            };
        }
        pieStarted = true;
    }
    catch (err) {
        const errMsg = err instanceof Error ? err.message : String(err);
        return {
            overall: PIE_CRASH_PATTERNS.test(errMsg) ? "CRASH" : "FAIL",
            pre_checks: preChecks,
            pie_started: false,
            runtime_checks: [{
                    label: "Start PIE", tool: "start_pie", passed: false,
                    error: errMsg, duration_ms: Math.round(performance.now() - pieT0),
                }],
            post_checks: [],
            duration_ms: Math.round(performance.now() - startTime),
            suggestion: "PIE threw an exception. Check bridge connectivity.",
        };
    }
    // ── Runtime wait ──
    await new Promise(r => setTimeout(r, duration * 1000));
    // ── Runtime checks (during PIE) ──
    const runtimeAssertions = [
        { tool: "assert_output_log_clean", params: { max_errors: maxErrors }, label: "Runtime: no new errors during PIE", severity: "warning" },
    ];
    for (const check of runtimeAssertions) {
        const t0 = performance.now();
        try {
            const r = await dispatch(check.tool, check.params);
            runtimeChecks.push({ label: check.label, tool: check.tool, passed: r.ok, error: r.ok ? undefined : r.error, duration_ms: Math.round(performance.now() - t0) });
        }
        catch (err) {
            runtimeChecks.push({ label: check.label, tool: check.tool, passed: false, error: err instanceof Error ? err.message : String(err), duration_ms: Math.round(performance.now() - t0) });
        }
    }
    // ── Stop PIE ──
    try {
        await dispatch("stop_pie", {});
    }
    catch (err) {
        console.error("[RiftbornAI] stop_pie failed during verification:", err instanceof Error ? err.message : String(err));
    }
    // ── Post-checks ──
    const postAssertions = [
        { tool: "assert_no_modal_blockers", params: {}, label: "Post-PIE: no modal blockers", severity: "warning" },
        { tool: "assert_output_log_clean", params: { max_errors: maxErrors }, label: "Post-PIE: output log clean", severity: "warning" },
    ];
    for (const check of postAssertions) {
        const t0 = performance.now();
        try {
            const r = await dispatch(check.tool, check.params);
            postChecks.push({ label: check.label, tool: check.tool, passed: r.ok, error: r.ok ? undefined : r.error, duration_ms: Math.round(performance.now() - t0) });
        }
        catch (err) {
            postChecks.push({ label: check.label, tool: check.tool, passed: false, error: err instanceof Error ? err.message : String(err), duration_ms: Math.round(performance.now() - t0) });
        }
    }
    // ── Compile result ──
    const allChecks = [...preChecks, ...runtimeChecks, ...postChecks];
    const anyFail = allChecks.some(c => !c.passed);
    const isCrash = allChecks.some(c => c.error && PIE_CRASH_PATTERNS.test(c.error));
    return {
        overall: isCrash ? "CRASH" : anyFail ? "FAIL" : "PASS",
        pre_checks: preChecks,
        pie_started: pieStarted,
        runtime_checks: runtimeChecks,
        post_checks: postChecks,
        duration_ms: Math.round(performance.now() - startTime),
        ...(isCrash ? { suggestion: "PIE crash detected. Run diagnose_crash()." } : {}),
    };
}
//# sourceMappingURL=autonomous-verify.js.map