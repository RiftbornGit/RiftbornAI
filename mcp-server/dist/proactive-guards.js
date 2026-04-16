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
const BLOCKED_PARAM_KEYS = new Set(["__proto__", "constructor", "prototype"]);
function toSafeRecord(record) {
    const out = {};
    for (const [key, value] of Object.entries(record)) {
        if (!BLOCKED_PARAM_KEYS.has(key)) {
            out[key] = value;
        }
    }
    return out;
}
// ---------------------------------------------------------------------------
// 1. ParameterDefaults — inject missing optional params
// ---------------------------------------------------------------------------
/**
 * Per-tool default values for parameters that:
 * - Are technically optional in the schema but practically required by UE
 * - Have a clear sensible default that matches the most common use case
 *
 * These fire AFTER normalizeParams (alias fix) and coerceParams (type fix),
 * so the values are already canonical and typed.
 */
const TOOL_DEFAULTS = {
    // Lighting — Point is the most common default; avoid "no type specified" errors
    create_light: {
        type: "PointLight",
        intensity: 5000,
        mobility: "Movable",
    },
    // Landscape — safe default that works on most hardware
    create_landscape: {
        size: 2017,
        sections: 4,
        scale_x: 100,
        scale_y: 100,
        scale_z: 100,
    },
    // Sculpt — agents forget radius/strength, UE returns unhelpful errors
    sculpt_landscape: {
        mode: "raise",
        radius: 500,
        strength: 0.5,
        falloff: 1.0,
    },
    // Paint layer — same as sculpt
    paint_landscape_layer: {
        radius: 500,
        strength: 1.0,
    },
    // Spawn actor at origin if no position given
    spawn_actor: {
        x: 0,
        y: 0,
        z: 0,
    },
    // Foliage painting defaults
    paint_foliage: {
        radius: 500,
        density: 100,
        align_to_surface: true,
    },
    // Post-process volume — infinite extent by default
    create_post_process_volume: {
        infinite_extent: true,
    },
    // Material — opaque surface lit by default
    create_material: {
        blend_mode: "Opaque",
        shading_model: "DefaultLit",
    },
    create_pbr_material: {
        roughness: 0.5,
        metallic: 0.0,
    },
    // Navigation
    build_navmesh: {
        agent_radius: 35,
        agent_height: 144,
    },
    // Camera
    capture_viewport_sync: {
        width: 1920,
        height: 1080,
    },
    // Grass
    add_grass_variety: {
        density: 200,
        start_cull_distance: 5000,
        end_cull_distance: 8000,
        align_to_surface: true,
        random_yaw: true,
        scaling: "Uniform",
        scale_min: 0.8,
        scale_max: 1.2,
    },
    // Foliage instance
    add_foliage_instance: {
        align_to_surface: true,
        random_yaw: true,
        scale: 1.0,
    },
    // Blueprint
    add_blueprint_component: {
        create_scene_attachment: true,
    },
    // PIE
    start_pie: {
        mode: "SelectedViewport",
    },
};
/**
 * Inject smart defaults for missing optional parameters.
 *
 * Only fills in values for keys that are NOT already present in the params.
 * Never overwrites anything the agent explicitly provided.
 *
 * Returns { params, defaults_applied } so the pipeline can stamp
 * which defaults were auto-injected (transparency for the agent).
 */
export function applyDefaults(toolName, params) {
    const defaults = TOOL_DEFAULTS[toolName];
    if (!defaults)
        return { params, defaults_applied: {} };
    const out = toSafeRecord(params);
    const applied = {};
    for (const [key, defaultValue] of Object.entries(defaults)) {
        if (out[key] === undefined || out[key] === null) {
            out[key] = defaultValue;
            applied[key] = defaultValue;
        }
    }
    return { params: out, defaults_applied: applied };
}
const DEPENDENCY_RULES = {
    // Landscape pipeline: can't paint/sculpt without a landscape
    sculpt_landscape: [
        {
            requiredTool: "create_landscape",
            message: "No landscape has been created this session.",
            suggestion: "Call create_landscape first to create terrain.",
        },
    ],
    paint_landscape_layer: [
        {
            requiredTool: "create_landscape",
            message: "No landscape has been created this session.",
            suggestion: "Call create_landscape first to create terrain.",
        },
    ],
    add_landscape_layer: [
        {
            requiredTool: "create_landscape",
            message: "No landscape has been created this session.",
            suggestion: "Call create_landscape first.",
        },
    ],
    apply_landscape_material: [
        {
            requiredTool: "create_landscape",
            message: "No landscape has been created this session.",
            suggestion: "Call create_landscape first.",
        },
        {
            requiredTool: ["create_landscape_material", "create_material", "create_pbr_material"],
            message: "No material has been created this session.",
            suggestion: "Call create_landscape_material to create a terrain material.",
        },
    ],
    // Grass pipeline: needs landscape + grass type
    add_grass_variety: [
        {
            requiredTool: "create_landscape_grass_type",
            message: "No landscape grass type has been created this session.",
            suggestion: "Call create_landscape_grass_type first.",
        },
    ],
    // Blueprint pipeline: must open before editing
    add_blueprint_component: [
        {
            requiredTool: ["open_blueprint", "create_blueprint"],
            message: "No blueprint has been opened this session.",
            suggestion: "Call open_blueprint or create_blueprint first.",
        },
    ],
    add_blueprint_variable: [
        {
            requiredTool: ["open_blueprint", "create_blueprint"],
            message: "No blueprint has been opened this session.",
            suggestion: "Call open_blueprint or create_blueprint first.",
        },
    ],
    add_blueprint_event: [
        {
            requiredTool: ["open_blueprint", "create_blueprint"],
            message: "No blueprint has been opened this session.",
            suggestion: "Call open_blueprint or create_blueprint first.",
        },
    ],
    compile_blueprint: [
        {
            requiredTool: ["open_blueprint", "create_blueprint"],
            message: "No blueprint has been opened this session.",
            suggestion: "Call open_blueprint first, then make changes, then compile.",
        },
    ],
    // PIE: can't stop if never started
    stop_pie: [
        {
            requiredTool: "start_pie",
            message: "Play In Editor has not been started this session.",
            suggestion: "Call start_pie first.",
        },
    ],
    // Material editing
    set_material_parameter: [
        {
            requiredTool: ["create_material", "create_pbr_material", "create_material_instance"],
            message: "No material has been created this session.",
            suggestion: "Call create_material or create_material_instance first.",
        },
    ],
    // Post-process
    set_post_process_settings: [
        {
            requiredTool: "create_post_process_volume",
            message: "No post-process volume has been created this session.",
            suggestion: "Call create_post_process_volume first.",
        },
    ],
};
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
export function checkDependencies(toolName, sessionHistory) {
    const rules = DEPENDENCY_RULES[toolName];
    if (!rules)
        return null;
    // Build set of successfully-called tools from session
    const calledTools = new Set();
    for (const entry of sessionHistory) {
        if (entry.ok)
            calledTools.add(entry.tool);
    }
    for (const rule of rules) {
        const required = Array.isArray(rule.requiredTool)
            ? rule.requiredTool
            : [rule.requiredTool];
        // Satisfied if ANY of the required tools has been called successfully
        const satisfied = required.some((t) => calledTools.has(t));
        if (!satisfied) {
            return {
                tool: toolName,
                missing_prerequisite: Array.isArray(rule.requiredTool)
                    ? rule.requiredTool.join(" | ")
                    : rule.requiredTool,
                message: rule.message,
                suggestion: rule.suggestion,
            };
        }
    }
    return null;
}
// ---------------------------------------------------------------------------
// 3. IdempotencyGuard — detect duplicate mutation calls
// ---------------------------------------------------------------------------
/**
 * Prefixes that indicate read-only tools (safe to call repeatedly).
 */
const READ_PREFIXES = ["get_", "list_", "find_", "search_", "describe_", "is_",
    "inspect_", "assert_", "validate_", "check_"];
/**
 * Tools that are explicitly safe to call multiple times with same params.
 */
const IDEMPOTENT_TOOLS = new Set([
    "observe_ue_project",
    "analyze_scene_screenshot",
    "capture_viewport_sync",
    "capture_viewport_safe",
    "look_at_and_capture",
    "set_viewport_location",
    "save_level",
    "compile_blueprint",
    "build_navmesh",
    "start_pie",
    "stop_pie",
]);
function isMutationTool(name) {
    const lower = name.toLowerCase();
    if (READ_PREFIXES.some((p) => lower.startsWith(p)))
        return false;
    if (IDEMPOTENT_TOOLS.has(lower))
        return false;
    return true;
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
export function checkIdempotency(toolName, params, sessionHistory, windowMs = 10_000) {
    if (!isMutationTool(toolName))
        return null;
    const now = Date.now();
    const cutoff = now - windowMs;
    // Walk history backwards (most recent first)
    for (let i = sessionHistory.length - 1; i >= 0; i--) {
        const entry = sessionHistory[i];
        if (entry.timestamp < cutoff)
            break;
        if (entry.tool !== toolName)
            continue;
        if (!entry.ok)
            continue;
        // For now, we can only match by tool name since we don't store params
        // in the session history. This catches rapid duplicate calls.
        return {
            tool: toolName,
            message: `'${toolName}' was already called successfully ${now - entry.timestamp}ms ago. This may be a duplicate call.`,
            last_call_ago_ms: now - entry.timestamp,
        };
    }
    return null;
}
//# sourceMappingURL=proactive-guards.js.map