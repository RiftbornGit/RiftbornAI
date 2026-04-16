/**
 * Agent Assist Layer — Enriches tool responses for better AI agent performance.
 *
 * Three capabilities:
 * 1. Error enrichment: categorizes failures + adds recovery hints
 * 2. Parameter normalization: fixes common agent parameter mistakes
 * 3. Workflow hints: suggests next steps after key tool successes
 */
const BLOCKED_RECORD_KEYS = new Set(["__proto__", "constructor", "prototype"]);
function toSafeRecord(record) {
    const out = {};
    for (const [key, value] of Object.entries(record)) {
        if (!BLOCKED_RECORD_KEYS.has(key)) {
            out[key] = value;
        }
    }
    return out;
}
const ERROR_PATTERNS = [
    {
        pattern: /bridge.*(disconnected|not responding|not running)|ECONNREFUSED/i,
        category: "bridge_disconnected",
        hint: "Unreal Editor is not running or the RiftbornAI plugin is not loaded. Start the editor and ensure the plugin is enabled.",
        retryable: true,
    },
    {
        pattern: /timed?\s*out|timeout|ETIMEDOUT/i,
        category: "bridge_timeout",
        hint: "The operation took too long. For build/compile tools this may be normal — try increasing timeout or check if UE is busy with a long operation.",
        retryable: true,
    },
    {
        pattern: /unknown tool|not found.*tool|tool.*not.*(?:found|registered|available)/i,
        category: "tool_not_found",
        hint: "This tool name is not registered. Use list_all_tools or describe_tool to find the correct name.",
        retryable: false,
    },
    {
        pattern: /no actor.*(?:found|label|named)|actor.*not found|could not find actor/i,
        category: "asset_not_found",
        hint: "The specified actor does not exist in the level. Use find_actor_by_label or get_selected_actors to discover valid actor names.",
        retryable: false,
    },
    {
        pattern: /asset.*not found|could not (?:find|load|locate) asset|no asset at/i,
        category: "asset_not_found",
        hint: "The asset path does not exist. Use list_assets to browse available assets, or check for typos in the path.",
        retryable: false,
    },
    {
        pattern: /blueprint.*not open|must open.*blueprint|no.*blueprint.*editor/i,
        category: "prerequisite_missing",
        hint: "A Blueprint must be open in the editor. Call open_blueprint first.",
        retryable: false,
    },
    {
        pattern: /PIE.*not running|not in PIE|requires PIE|play.*not.*active/i,
        category: "prerequisite_missing",
        hint: "Play In Editor (PIE) must be running. Call start_pie first.",
        retryable: false,
    },
    {
        pattern: /no landscape|landscape not found|no.*landscape.*actor/i,
        category: "prerequisite_missing",
        hint: "No landscape exists in the level. Call create_landscape first.",
        retryable: false,
    },
    {
        pattern: /parameter|argument|missing required|invalid.*(?:param|arg|value|type)/i,
        category: "parameter_invalid",
        hint: "Check the tool's inputSchema for correct parameter names and types. Use describe_tool to see the full spec.",
        retryable: false,
    },
    {
        pattern: /permission|access denied|unauthorized|forbidden/i,
        category: "permission_denied",
        hint: "This operation requires higher permissions or governance approval.",
        retryable: false,
    },
    {
        pattern: /assert|check|ensure|crash|exception|unhandled/i,
        category: "ue_runtime_error",
        hint: "Unreal Engine hit a runtime error. Call diagnose_crash to get structured diagnosis, then fix the root cause before retrying.",
        retryable: false,
    },
];
/** Enrich a failed tool response with category, recovery hint, and retryable flag. */
export function enrichError(toolName, response) {
    const errorMsg = response.error ?? "Unknown error";
    for (const { pattern, category, hint, retryable } of ERROR_PATTERNS) {
        if (pattern.test(errorMsg)) {
            return {
                ok: false,
                error: errorMsg,
                error_category: category,
                recovery_hint: hint,
                retryable,
            };
        }
    }
    return {
        ok: false,
        error: errorMsg,
        error_category: "unknown",
        recovery_hint: `Tool '${toolName}' failed. Check the error message and verify parameters match the tool's inputSchema.`,
        retryable: false,
    };
}
// ─── Parameter Normalization ──────────────────────────────────────────────────
/**
 * Common parameter aliases that agents confuse.
 * Maps (toolName or "*") → alias → canonical parameter name.
 */
const PARAM_ALIASES = {
    // Actor tools: agents mix up "actor_name", "name", "actor_label" — canonical is "label"
    move_actor: { actor_name: "label", name: "label", actor_label: "label", actor: "label" },
    rotate_actor: { actor_name: "label", name: "label", actor_label: "label", actor: "label" },
    scale_actor: { actor_name: "label", name: "label", actor_label: "label", actor: "label" },
    delete_actor: { actor_name: "label", name: "label", actor_label: "label", actor: "label" },
    get_actor_info: { actor_name: "label", name: "label", actor: "label" },
    find_actor_by_label: { actor_name: "label", name: "label", actor: "label" },
    focus_actor: { actor_name: "label", name: "label", actor_label: "label", actor: "label" },
    duplicate_actor: { actor_name: "label", name: "label", actor_label: "label", actor: "label" },
    set_actor_transform: { actor_name: "label", name: "label", actor_label: "label" },
    // Blueprint tools: agents mix up "path" vs "blueprint_path" vs "blueprint"
    open_blueprint: { blueprint_path: "path", blueprint: "path" },
    compile_blueprint: { path: "blueprint_path", blueprint: "blueprint_path" },
    // Material tools: agents mix "material" and "material_path"
    set_actor_material: { material: "material_path", path: "material_path" },
    // Landscape tools: agents forget "landscape_label" and use "landscape" or "name"
    sculpt_landscape: { landscape: "landscape_label", name: "landscape_label", label: "landscape_label" },
    paint_landscape_layer: { landscape: "landscape_label", name: "landscape_label", label: "landscape_label" },
    add_landscape_layer: { landscape: "landscape_label", name: "landscape_label", label: "landscape_label" },
    apply_landscape_material: { landscape: "landscape_name", label: "landscape_name", landscape_label: "landscape_name" },
    // Post-process: agents use "label" instead of "actor_name"
    set_post_process_settings: { label: "actor_name", actor_label: "actor_name" },
    // Component property: agents use "label" instead of "actor_name"
    set_component_property: { label: "actor_name", actor: "actor_name", actor_label: "actor_name" },
};
/**
 * Fix common parameter name mistakes.
 * Only copies an alias if the canonical param is missing AND the alias is present.
 * Never overwrites a canonical parameter that was already provided.
 */
export function normalizeParams(toolName, params) {
    const aliases = PARAM_ALIASES[toolName];
    if (!aliases)
        return params;
    const out = toSafeRecord(params);
    for (const [alias, canonical] of Object.entries(aliases)) {
        if (out[canonical] === undefined && out[alias] !== undefined) {
            out[canonical] = out[alias];
            delete out[alias];
        }
    }
    return out;
}
// ─── Workflow Hints ───────────────────────────────────────────────────────────
/**
 * Mapping from tool name → brief "what to do next" hint.
 * Only for high-frequency PRODUCTION tools where the next step isn't obvious.
 */
const WORKFLOW_HINTS = {
    // Landscape pipeline
    create_landscape: "Next: create_landscape_material to add layers, then apply_landscape_material to assign it.",
    create_landscape_material: "Next: apply_landscape_material to assign this material to the landscape, then add_landscape_layer + paint_landscape_layer.",
    apply_landscape_material: "Next: add_landscape_layer for each paintable layer, then paint_landscape_layer to apply them.",
    add_landscape_layer: "Next: paint_landscape_layer to paint this layer onto the terrain.",
    paint_landscape_layer: "Next: create_landscape_grass_type and add_grass_variety for automatic ground cover, or continue painting more layers.",
    // Grass pipeline
    create_landscape_grass_type: "Next: add_grass_variety to add mesh varieties (grass, ferns, litter). For LandscapeGrassOutput wiring, prefer a typed material-editing tool; use execute_python only as an explicitly approved fallback when no typed route exists.",
    add_grass_variety: "Grass type updated. If LandscapeGrassOutput still needs wiring, prefer a typed material-editing tool and treat execute_python as an explicitly approved fallback only when no typed route exists.",
    // Foliage
    paint_foliage: "Verify placement with look_at_and_capture. For automatic ground cover, use create_landscape_grass_type instead.",
    add_foliage_instance: "Verify placement with look_at_and_capture. Use ground_foliage_to_landscape to audit grounding.",
    // Blueprint pipeline
    create_blueprint: "Next: open_blueprint to enter the editor, then add_blueprint_component, add_blueprint_variable, add_blueprint_event. Finish with compile_blueprint.",
    open_blueprint: "Blueprint is now open. Add components, variables, and events, then compile_blueprint when done.",
    add_blueprint_component: "Component added. Continue adding components/variables/events, then compile_blueprint.",
    add_blueprint_event: "Event added. Continue adding events, then compile_blueprint to validate.",
    compile_blueprint: "Blueprint compiled. Spawn it with spawn_actor or verify with validate_blueprint_health.",
    // Material pipeline
    create_material: "Next: add_material_expression to build the graph, connect_material_nodes to wire them, then set_actor_material to apply.",
    create_pbr_material: "Material ready. Apply with set_actor_material or create_material_instance for variants.",
    create_material_instance: "Instance created. Adjust with set_material_parameter, apply with set_actor_material.",
    // Character pipeline
    create_character_from_third_person: "Character created. Next: make_character_playable or spawn_third_person_character to place it in the level.",
    // Post-process
    create_post_process_volume: "Volume created. Configure with set_post_process_settings (bloom, exposure, color grading, vignette, DoF).",
    // Lighting
    create_light: "Light placed. Adjust properties with set_component_property. Verify with look_at_and_capture.",
    // Level management
    create_level: "Level created. Next: load_level to open it, or create_landscape to start building terrain.",
    load_level: "Level loaded. Use observe_ue_project to see what's in it.",
    save_level: "Level saved. Use save_scene_checkpoint for a restorable snapshot.",
    // Spawning
    spawn_actor: "Actor spawned. Verify with look_at_and_capture or get_actor_info.",
    // Vision
    observe_ue_project: "Scene observed. Use the actor list and vision analysis to plan your next action. Check _vision.issues for detected problems.",
    analyze_scene_screenshot: "Analysis complete. Check _vision.issues for actionable problems. Act on findings, then verify again.",
    look_at_and_capture: "Actor inspected. Check _vision.issues for problems. The screenshot path is stored for follow-up vision_compare.",
    capture_viewport_sync: "Screenshot captured. Use analyze_scene_screenshot or vision_compare for AI analysis.",
    capture_viewport_safe: "Screenshot captured. Use analyze_scene_screenshot or vision_compare for AI analysis.",
    vision_observe: "Full observation complete. Check _vision.issues and _vision.quality. Use vision_build_and_verify to fix detected issues.",
    vision_compare: "Comparison complete. Check _vision.issues to see if problems were resolved or new ones introduced.",
    vision_inspect_actor: "Actor inspection complete. Check _vision.issues for material, scale, or placement problems.",
    vision_playtest: "Playtest complete. Check _vision.issues for gameplay problems. PIE has been stopped.",
    vision_sweep: "Sweep complete. Review _vision.issues from each angle. Focus on issues that appear from multiple views.",
    vision_build_and_verify: "Build-and-verify complete. Check _vision.issues to see if the change was successful.",
    // PIE
    start_pie: "PIE running. After testing, call stop_pie. If it crashes, call diagnose_crash.",
    stop_pie: "PIE stopped. Use observe_ue_project to check the editor state.",
};
/**
 * Append a workflow hint to a successful tool response.
 * Returns the response unchanged if no hint exists.
 */
export function addWorkflowHint(toolName, response) {
    const hint = WORKFLOW_HINTS[toolName];
    if (!hint)
        return response;
    const safeResponse = toSafeRecord(response);
    return Object.assign({ ok: response.ok }, safeResponse, { _hint: hint });
}
const PREREQUISITE_GUARDS = {
    // Blueprint editing requires a blueprint to be specified or open
    add_blueprint_component: {
        hint: "A Blueprint must be open in the editor. Call open_blueprint with the Blueprint path first.",
    },
    add_blueprint_variable: {
        hint: "A Blueprint must be open in the editor. Call open_blueprint with the Blueprint path first.",
    },
    add_blueprint_event: {
        requiredParam: "blueprint",
        hint: "Parameter 'blueprint' is required. Provide the Blueprint path, and ensure it's open with open_blueprint.",
    },
    add_blueprint_node: {
        hint: "A Blueprint must be open in the editor. Call open_blueprint with the Blueprint path first.",
    },
    connect_blueprint_nodes: {
        hint: "A Blueprint must be open in the editor. Call open_blueprint with the Blueprint path first.",
    },
    // Landscape tools require a landscape to exist
    sculpt_landscape: {
        requiredParam: "landscape_label",
        hint: "Parameter 'landscape_label' is required. Create a landscape first with create_landscape, then use its label.",
    },
    paint_landscape_layer: {
        requiredParam: "landscape_label",
        hint: "Parameter 'landscape_label' is required. Create a landscape with create_landscape, add layers with add_landscape_layer.",
    },
    add_landscape_layer: {
        requiredParam: "landscape_label",
        hint: "Parameter 'landscape_label' is required. Create a landscape first with create_landscape.",
    },
    // Material graph tools require expressions to exist
    connect_material_nodes: {
        hint: "Material nodes must exist before wiring. Use add_material_expression to create nodes first.",
    },
};
/**
 * Check if a tool call will definitely fail due to missing prerequisites.
 * Returns an EnrichedError if the call is guaranteed to fail, or null if it should proceed.
 */
export function checkPrerequisite(toolName, params) {
    const rule = PREREQUISITE_GUARDS[toolName];
    if (!rule)
        return null;
    // If there's a specific required param check, only fail if it's missing
    if (rule.requiredParam) {
        const val = params[rule.requiredParam];
        if (val === undefined || val === null || val === "") {
            return {
                ok: false,
                error: `Missing required parameter '${rule.requiredParam}' for tool '${toolName}'.`,
                error_category: "prerequisite_missing",
                recovery_hint: rule.hint,
                retryable: false,
            };
        }
        return null; // Param is present, let it through
    }
    // No specific param check — this is a general prerequisite warning.
    // We can't block these without state tracking (we don't know if a blueprint is open).
    // Return null to let them through; the UE5 error will be enriched by enrichError.
    return null;
}
// ─── Execution Timing ─────────────────────────────────────────────────────────
/**
 * Add execution duration to a tool response.
 */
export function addTiming(response, startTime) {
    const safeResponse = toSafeRecord(response);
    return Object.assign({ ok: response.ok }, safeResponse, { _duration_ms: Math.round(performance.now() - startTime) });
}
//# sourceMappingURL=agent-assist.js.map