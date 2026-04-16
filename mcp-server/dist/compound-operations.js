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
import { getWorkflow, listWorkflows } from "./tool-compression.js";
import { createSanitizer, PROTO_BLOCKED_KEYS } from "./sanitize-utils.js";
const MAX_JSON_PARSE_LENGTH = 64 * 1024;
const sanitizeParsedJson = createSanitizer();
// ---------------------------------------------------------------------------
// Output Chaining — captures tool outputs and injects into later steps
// ---------------------------------------------------------------------------
/** Extract known output fields from a tool result into the chaining context. */
// Validation patterns for extracted context values.
// Prevents output injection: a poisoned tool result (e.g. label="/Game/../Evil")
// would propagate unchecked into downstream workflow steps.
const SAFE_LABEL_RE = /^[A-Za-z0-9_\-. ]{1,256}$/;
const SAFE_PATH_RE = /^\/Game\/[A-Za-z0-9_\-./]{1,512}$/;
function validateExtracted(key, value) {
    if (typeof value !== "string" || value.length === 0)
        return undefined;
    const isPath = key.includes("path");
    const pattern = isPath ? SAFE_PATH_RE : SAFE_LABEL_RE;
    return pattern.test(value) ? value : undefined;
}
function extractOutputs(toolName, result) {
    if (!result || typeof result !== "object")
        return {};
    const ctx = {};
    // Generic extraction with validation
    for (const key of ["path", "label", "name", "material_path", "blueprint_path"]) {
        const validated = validateExtracted(key, result[key]);
        if (validated !== undefined) {
            ctx[key] = validated;
        }
    }
    // Tool-specific extractions
    switch (toolName) {
        case "create_landscape":
            ctx.landscape_label = result.label || result.name || "Landscape";
            break;
        case "create_landscape_material":
            ctx.material_path = result.path || result.material_path;
            break;
        case "create_landscape_grass_type":
            ctx.grass_type_name = result.name || result.grass_type;
            break;
        case "create_post_process_volume":
            ctx.ppv_label = result.label || "PostProcessVolume";
            break;
        case "open_blueprint":
            ctx.blueprint_path = result.path;
            break;
        case "create_pbr_material":
        case "create_material":
            ctx.material_path = result.path || result.material_path;
            break;
        case "create_character_from_third_person":
            ctx.character_name = result.name;
            ctx.blueprint_path = result.path;
            break;
    }
    return ctx;
}
function toSafeRecord(value) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
        return {};
    }
    const out = {};
    for (const [key, entry] of Object.entries(value)) {
        if (PROTO_BLOCKED_KEYS.has(key)) {
            continue;
        }
        out[key] = entry;
    }
    return out;
}
function mergeSafeRecords(...records) {
    const out = {};
    for (const record of records) {
        for (const [key, value] of Object.entries(record)) {
            if (PROTO_BLOCKED_KEYS.has(key)) {
                continue;
            }
            out[key] = value;
        }
    }
    return out;
}
/**
 * Chain rules: maps context keys to tool input param names.
 * When a step doesn't have a param value, inject it from the context if a rule matches.
 */
const CHAIN_RULES = {
    apply_landscape_material: {
        landscape_label: "landscape_name",
        material_path: "material_path",
    },
    add_landscape_layer: { landscape_label: "landscape_label" },
    paint_landscape_layer: { landscape_label: "landscape_label" },
    sculpt_landscape: { landscape_label: "landscape_label" },
    add_grass_variety: { grass_type_name: "grass_type" },
    compile_blueprint: { blueprint_path: "blueprint_path" },
    add_blueprint_component: { blueprint_path: "path" },
    add_blueprint_variable: { blueprint_path: "path" },
    add_blueprint_event: { blueprint_path: "blueprint" },
    set_actor_material: { material_path: "material_path" },
    set_material_parameter: { material_path: "material_path" },
    create_material_instance: { material_path: "parent_path" },
    set_post_process_settings: { ppv_label: "actor_name" },
    make_character_playable: { character_name: "character_name" },
};
/** Inject chained params from accumulated context where the step lacks them. */
function injectChainedParams(toolName, params, ctx) {
    const rules = CHAIN_RULES[toolName];
    if (!rules)
        return toSafeRecord(params);
    const injected = toSafeRecord(params);
    for (const [ctxKey, paramName] of Object.entries(rules)) {
        if (injected[paramName] === undefined && ctx[ctxKey] !== undefined) {
            injected[paramName] = ctx[ctxKey];
        }
    }
    return injected;
}
// ---------------------------------------------------------------------------
// Iteration — some steps run once per item (e.g., per landscape layer)
// ---------------------------------------------------------------------------
function getIterationItems(toolName, userParams) {
    if (toolName === "add_landscape_layer") {
        const layers = userParams.layers;
        if (Array.isArray(layers) && layers.length > 0) {
            return layers.map((l) => ({ layer_name: String(l) }));
        }
    }
    if (toolName === "paint_landscape_layer") {
        const layers = userParams.layers;
        if (Array.isArray(layers) && layers.length > 0) {
            return layers.map((l, i) => ({
                layer_name: String(l),
                center_x: 0,
                center_y: 0,
                radius: Number(userParams.paint_radius) || 20000,
                strength: i === 0 ? 1.0 : 0.0,
            }));
        }
    }
    if (toolName === "add_grass_variety") {
        const meshes = userParams.grass_meshes;
        if (Array.isArray(meshes) && meshes.length > 0) {
            return meshes.map((m) => {
                if (typeof m === "string")
                    return { mesh: m, density: 200 };
                return { mesh: m.mesh, density: m.density || 200 };
            });
        }
    }
    return null;
}
// Steps that are skipped unless the user provided relevant params
const OPTIONAL_STEPS = new Set(["sculpt_landscape", "create_material_instance"]);
function hasRelevantParams(step, userParams, stepIndex) {
    if (userParams[`step_${stepIndex}`])
        return true;
    if (step.key_params) {
        return step.key_params.some((p) => userParams[p] !== undefined);
    }
    return false;
}
// ---------------------------------------------------------------------------
// 1. Workflow Executor
// ---------------------------------------------------------------------------
const MAX_WORKFLOW_STEPS = 20;
export async function executeWorkflow(workflowName, userParams, executeTool) {
    const workflow = getWorkflow(workflowName);
    if (!workflow) {
        const available = listWorkflows().map((w) => w.name);
        return {
            ok: false,
            workflow: workflowName,
            steps_completed: 0,
            steps_total: 0,
            results: [],
            outputs: {},
            error: `Unknown workflow: '${workflowName}'. Available: ${available.join(", ")}`,
        };
    }
    if (workflow.steps.length > MAX_WORKFLOW_STEPS) {
        return {
            ok: false,
            workflow: workflowName,
            steps_completed: 0,
            steps_total: workflow.steps.length,
            results: [],
            outputs: {},
            error: `Workflow has ${workflow.steps.length} steps (max ${MAX_WORKFLOW_STEPS})`,
        };
    }
    const skipIndices = new Set(Array.isArray(userParams.skip_steps)
        ? userParams.skip_steps.map(Number).filter((n) => !isNaN(n))
        : []);
    const results = [];
    const ctx = {};
    let stepsCompleted = 0;
    for (let i = 0; i < workflow.steps.length; i++) {
        const step = workflow.steps[i];
        // User-requested skip
        if (skipIndices.has(i)) {
            stepsCompleted++;
            continue;
        }
        // Skip optional steps unless user provided relevant params
        if (OPTIONAL_STEPS.has(step.tool) && !hasRelevantParams(step, userParams, i)) {
            stepsCompleted++;
            continue;
        }
        // Build params: per-step overrides merged over user params + chained context
        const stepOverrides = toSafeRecord(userParams[`step_${i}`]);
        const baseParams = injectChainedParams(step.tool, toSafeRecord(userParams), ctx);
        const finalParams = mergeSafeRecords(baseParams, stepOverrides);
        // Check for iteration (e.g., add_landscape_layer × N layers)
        const iterations = getIterationItems(step.tool, userParams);
        if (iterations && iterations.length > 0) {
            for (const iterItem of iterations) {
                const iterParams = injectChainedParams(step.tool, mergeSafeRecords(finalParams, iterItem), ctx);
                const stepResult = await runStep(step, i, iterParams, executeTool, true);
                results.push(stepResult);
                if (!stepResult.ok) {
                    return failResult(workflowName, stepsCompleted, workflow.steps.length, results, ctx, stepResult);
                }
                Object.assign(ctx, mergeSafeRecords(extractOutputs(step.tool, stepResult.result)));
            }
        }
        else {
            const stepResult = await runStep(step, i, finalParams, executeTool, false);
            results.push(stepResult);
            if (!stepResult.ok) {
                return failResult(workflowName, stepsCompleted, workflow.steps.length, results, ctx, stepResult);
            }
            Object.assign(ctx, mergeSafeRecords(extractOutputs(step.tool, stepResult.result)));
        }
        stepsCompleted++;
    }
    return {
        ok: true,
        workflow: workflowName,
        steps_completed: stepsCompleted,
        steps_total: workflow.steps.length,
        results,
        outputs: ctx,
    };
}
async function runStep(step, index, params, executeTool, iterated) {
    const start = performance.now();
    try {
        const result = await executeTool(step.tool, params);
        return {
            step: index,
            tool: step.tool,
            ok: result.ok,
            result: result.ok ? result.result : undefined,
            error: result.ok ? undefined : result.error,
            duration_ms: Math.round(performance.now() - start),
            ...(iterated ? { iterated: true } : {}),
        };
    }
    catch (e) {
        return {
            step: index,
            tool: step.tool,
            ok: false,
            error: String(e),
            duration_ms: Math.round(performance.now() - start),
            ...(iterated ? { iterated: true } : {}),
        };
    }
}
function failResult(workflowName, stepsCompleted, stepsTotal, results, outputs, failedStep) {
    return {
        ok: false,
        workflow: workflowName,
        steps_completed: stepsCompleted,
        steps_total: stepsTotal,
        results,
        outputs,
        error: `Step ${failedStep.step} (${failedStep.tool}) failed: ${failedStep.error}`,
    };
}
// ---------------------------------------------------------------------------
// 2. Ensure Actor — find-or-create pattern
// ---------------------------------------------------------------------------
export async function ensureActor(label, createParams, executeTool) {
    // Try to find existing actor
    try {
        const found = await executeTool("find_actor_by_label", { label });
        if (found.ok && found.result) {
            return { ok: true, existed: true, label, result: found.result };
        }
    }
    catch {
        // Not found or error — proceed to create
    }
    // Create it
    try {
        const created = await executeTool("spawn_actor", mergeSafeRecords(toSafeRecord(createParams), { label }));
        if (created.ok) {
            return { ok: true, existed: false, label, result: created.result };
        }
        return { ok: false, existed: false, label, error: created.error };
    }
    catch (e) {
        return { ok: false, existed: false, label, error: String(e) };
    }
}
// ---------------------------------------------------------------------------
// 3. Scene Query — structured introspection
// ---------------------------------------------------------------------------
const LANDSCAPE_MARKERS = ["Landscape", "LandscapeProxy", "LandscapeStreamingProxy"];
const DIRECTIONAL_LIGHT_MARKERS = ["DirectionalLight"];
const SKY_LIGHT_MARKERS = ["SkyLight"];
const PPV_MARKERS = ["PostProcessVolume"];
const FOG_MARKERS = ["ExponentialHeightFog"];
const SKY_MARKERS = ["SkyAtmosphere", "BP_Sky_Sphere"];
export async function queryScene(executeTool, options = {}) {
    const radius = Math.min(options.radius || 100000, 500000);
    const maxActors = Math.min(options.max_actors || 200, 1000);
    // Run parallel queries
    const [levelResult, actorsResult] = await Promise.all([
        executeTool("get_current_level", {}).catch(() => ({
            ok: false,
            error: "get_current_level failed",
        })),
        executeTool("get_actors_in_radius", {
            x: 0,
            y: 0,
            z: 0,
            radius,
        }).catch(() => ({
            ok: false,
            error: "get_actors_in_radius failed",
        })),
    ]);
    // Parse level info
    let levelName = "unknown";
    let levelPath = "";
    if (levelResult.ok && levelResult.result) {
        const lr = typeof levelResult.result === "string"
            ? safeJsonParse(levelResult.result)
            : levelResult.result;
        levelName = lr?.name || lr?.level_name || "unknown";
        levelPath = lr?.path || lr?.level_path || "";
    }
    // Parse actors
    const actors = [];
    if (actorsResult.ok && actorsResult.result) {
        const ar = typeof actorsResult.result === "string"
            ? safeJsonParse(actorsResult.result)
            : actorsResult.result;
        if (Array.isArray(ar))
            actors.push(...ar);
        else if (ar?.actors && Array.isArray(ar.actors))
            actors.push(...ar.actors);
    }
    // Categorize by class
    const byClass = {};
    for (const actor of actors) {
        const cls = actor.class || actor.class_name || actor.type || "Unknown";
        byClass[cls] = (byClass[cls] || 0) + 1;
    }
    const classNames = Object.keys(byClass);
    const hasClass = (markers) => markers.some((m) => classNames.some((c) => c.includes(m)));
    return {
        ok: true,
        state: {
            level_name: levelName,
            level_path: levelPath,
            actor_count: actors.length,
            actors_by_class: byClass,
            has_landscape: hasClass(LANDSCAPE_MARKERS),
            has_directional_light: hasClass(DIRECTIONAL_LIGHT_MARKERS),
            has_sky_light: hasClass(SKY_LIGHT_MARKERS),
            has_post_process: hasClass(PPV_MARKERS),
            has_fog: hasClass(FOG_MARKERS),
            has_sky_atmosphere: hasClass(SKY_MARKERS),
            actors_sample: actors.slice(0, Math.min(maxActors, 20)),
        },
    };
}
function safeJsonParse(s) {
    if (typeof s !== "string" || s.length > MAX_JSON_PARSE_LENGTH) {
        return null;
    }
    try {
        return sanitizeParsedJson(JSON.parse(s));
    }
    catch {
        return null;
    }
}
//# sourceMappingURL=compound-operations.js.map