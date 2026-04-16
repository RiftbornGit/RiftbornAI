/**
 * Tool Resolution & Response Shaping — Round 7 enhancements
 *
 * 1. ToolResolver — Fuzzy tool name resolution. When agents call a wrong name
 *    (e.g. "spawn_light" → "create_light"), auto-resolves and executes the
 *    correct tool instead of returning "Unknown tool". Uses Levenshtein distance
 *    + prefix/suffix matching for high confidence, otherwise returns suggestions.
 *
 * 2. ResponseShaper — Per-tool extraction of the fields agents actually need.
 *    e.g. spawn_actor returns 15+ fields from UE; agents only need label, class, location.
 *    Reduces token consumption without losing actionable information.
 *
 * 3. CostTagger — Tags each response with execution cost class so agents can
 *    plan: "cheap" (<1s reads), "moderate" (1-5s mutations), "expensive" (>5s compiles/builds).
 */
// ---------------------------------------------------------------------------
// 1. ToolResolver — Fuzzy tool name resolution
// ---------------------------------------------------------------------------
/**
 * Bounded Levenshtein distance (bails out if distance exceeds maxDist).
 * Returns maxDist+1 if the distance is too large.
 */
function levenshtein(a, b, maxDist) {
    if (Math.abs(a.length - b.length) > maxDist)
        return maxDist + 1;
    const m = a.length;
    const n = b.length;
    // Single row optimization
    let prev = new Uint16Array(n + 1);
    let curr = new Uint16Array(n + 1);
    for (let j = 0; j <= n; j++)
        prev[j] = j;
    for (let i = 1; i <= m; i++) {
        curr[0] = i;
        let rowMin = i;
        for (let j = 1; j <= n; j++) {
            const cost = a[i - 1] === b[j - 1] ? 0 : 1;
            curr[j] = Math.min(prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost);
            if (curr[j] < rowMin)
                rowMin = curr[j];
        }
        if (rowMin > maxDist)
            return maxDist + 1;
        [prev, curr] = [curr, prev];
    }
    return prev[n];
}
/**
 * Static alias map for the most common agent mistakes.
 * Maps wrong_name → correct_name.
 */
const TOOL_ALIASES = {
    // Light
    spawn_light: "create_light",
    add_light: "create_light",
    // Mesh / mesh assignment
    set_mesh: "set_skeletal_mesh",
    set_static_mesh: "set_actor_material", // common confusion
    // Navigation
    create_navmesh: "build_navmesh",
    generate_navmesh: "build_navmesh",
    navmesh: "build_navmesh",
    bake_navmesh: "build_navmesh",
    // Actors
    place_actor: "spawn_actor",
    create_actor: "spawn_actor",
    add_actor: "spawn_actor",
    remove_actor: "delete_actor",
    destroy_actor: "delete_actor",
    // Blueprint
    create_bp: "create_blueprint",
    open_bp: "open_blueprint",
    compile_bp: "compile_blueprint",
    // Landscape
    create_terrain: "create_landscape",
    make_landscape: "create_landscape",
    paint_terrain: "paint_landscape_layer",
    sculpt_terrain: "sculpt_landscape",
    // Character
    create_player: "create_character_from_third_person",
    spawn_character: "spawn_third_person_character",
    spawn_player: "spawn_third_person_character",
    // Material
    make_material: "create_material",
    apply_material: "set_actor_material",
    assign_material: "set_actor_material",
    // Camera
    move_camera: "set_viewport_location",
    set_camera_position: "set_viewport_location",
    set_camera: "set_viewport_location",
    screenshot: "capture_viewport_sync",
    take_screenshot: "capture_viewport_sync",
    capture_screenshot: "capture_viewport_sync",
    // PIE
    play: "start_pie",
    stop: "stop_pie",
    start_play: "start_pie",
    stop_play: "stop_pie",
    // Observation
    observe: "observe_ue_project",
    look_at: "look_at_and_capture",
    // Save / Level
    save: "save_level",
    load_map: "load_level",
    open_level: "load_level",
    open_map: "load_level",
    // Info / Discovery
    describe: "describe_tool",
    help: "describe_tool",
    search_tools: "find_tools",
    tool_search: "find_tools",
    // Post-process
    create_ppv: "create_post_process_volume",
    post_process: "create_post_process_volume",
    set_post_process: "set_post_process_settings",
};
export class ToolResolver {
    names;
    nameSet;
    constructor(tools) {
        this.names = tools.map((t) => t.name);
        this.nameSet = new Set(this.names);
    }
    /**
     * Resolve a tool name that wasn't found in the registry.
     *
     * Resolution order:
     * 1. Exact match (already handled upstream — this is a fallback)
     * 2. Static alias lookup
     * 3. Levenshtein distance ≤ 2 (high confidence auto-resolve)
     * 4. Prefix/suffix overlap (medium confidence — return suggestions)
     */
    resolve(name) {
        const lower = name.toLowerCase();
        // 1. Exact
        if (this.nameSet.has(name)) {
            return { resolved: name, confidence: "exact", suggestions: [] };
        }
        // 2. Static alias
        const alias = TOOL_ALIASES[lower];
        if (alias && this.nameSet.has(alias)) {
            return { resolved: alias, confidence: "alias", suggestions: [alias] };
        }
        // 3. Levenshtein ≤ 2 (auto-resolve)
        let bestDist = 3;
        let bestMatch = null;
        const candidates = [];
        for (const candidate of this.names) {
            const dist = levenshtein(lower, candidate, 3);
            if (dist < bestDist) {
                bestDist = dist;
                bestMatch = candidate;
            }
            if (dist <= 3) {
                candidates.push({ name: candidate, dist });
            }
        }
        if (bestMatch && bestDist <= 2) {
            return {
                resolved: bestMatch,
                confidence: "high",
                suggestions: candidates
                    .sort((a, b) => a.dist - b.dist)
                    .slice(0, 5)
                    .map((c) => c.name),
            };
        }
        // 4. Prefix/suffix matching
        const parts = lower.split("_").filter(Boolean);
        const matches = [];
        for (const candidate of this.names) {
            const cParts = candidate.split("_");
            // At least 2 shared segments
            const shared = parts.filter((p) => cParts.includes(p));
            if (shared.length >= 2) {
                matches.push(candidate);
            }
        }
        if (matches.length > 0) {
            return {
                resolved: null,
                confidence: "medium",
                suggestions: matches.slice(0, 5),
            };
        }
        // No match
        return {
            resolved: null,
            confidence: "none",
            suggestions: candidates
                .sort((a, b) => a.dist - b.dist)
                .slice(0, 3)
                .map((c) => c.name),
        };
    }
}
// ---------------------------------------------------------------------------
// 2. ResponseShaper — Extract the fields agents actually need
// ---------------------------------------------------------------------------
/**
 * Tool-specific field extractors. Each maps tool name → list of keys to keep
 * from the result object. If a tool isn't listed, the full response is returned.
 */
import { VISION_KEEP_FIELDS } from "./vision-intelligence.js";
const RESULT_KEEP_FIELDS = {
    // Vision tools — screenshot path, analysis text, actor census
    ...VISION_KEEP_FIELDS,
    // Actor tools — agents need label, class, location
    spawn_actor: ["label", "class", "location", "actor_label", "class_name", "path"],
    move_actor: ["label", "location", "x", "y", "z"],
    rotate_actor: ["label", "rotation", "pitch", "yaw", "roll"],
    scale_actor: ["label", "scale"],
    get_actor_info: ["label", "class", "location", "rotation", "scale", "components", "tags", "mobility", "actor_label", "class_name"],
    get_actor_transform: ["label", "location", "rotation", "scale"],
    find_actor_by_label: ["label", "class", "location", "found", "actor_label", "class_name"],
    delete_actor: ["label", "deleted", "success"],
    duplicate_actor: ["label", "new_label", "new_actor_label"],
    // Blueprint tools — agents need path and compile status
    compile_blueprint: ["success", "compiled", "errors", "warnings", "path", "blueprint_path"],
    open_blueprint: ["success", "path", "opened"],
    // Material tools
    create_material: ["path", "name", "material_path", "success"],
    create_pbr_material: ["path", "name", "material_path", "success"],
    create_material_instance: ["path", "name", "material_path", "parent", "success"],
    // Landscape
    create_landscape: ["label", "size", "material", "landscape_label", "success"],
    sculpt_landscape: ["success", "mode", "affected_vertices"],
    create_landscape_material: ["path", "name", "material_path", "layers", "success"],
    // Lighting
    create_light: ["label", "type", "location", "intensity", "success"],
    // Level
    create_level: ["path", "name", "success"],
    save_level: ["path", "success"],
    load_level: ["path", "name", "success"],
    // Character
    create_character_from_third_person: ["path", "label", "name", "blueprint_path", "success"],
    // Post-process
    create_post_process_volume: ["label", "location", "success"],
    // Foliage
    paint_foliage: ["count", "mesh", "success", "instances_placed"],
    add_foliage_instance: ["location", "mesh", "success"],
    create_landscape_grass_type: ["path", "name", "success"],
    // Navigation
    build_navmesh: ["success", "bounds", "area"],
    // PIE
    start_pie: ["success", "session_id"],
    stop_pie: ["success"],
};
/**
 * Shape a tool response to only include the fields agents need.
 * Falls back to the full response if:
 * - Tool has no shaping rule
 * - Result is not an object
 * - Result is a string or array (handled by normalizeResponse)
 */
export function shapeResponse(toolName, result) {
    if (result === null || result === undefined)
        return result;
    if (typeof result !== "object" || Array.isArray(result))
        return result;
    const keepFields = RESULT_KEEP_FIELDS[toolName];
    if (!keepFields)
        return result;
    const obj = result;
    const shaped = {};
    let kept = 0;
    for (const field of keepFields) {
        if (field in obj) {
            shaped[field] = obj[field];
            kept++;
        }
    }
    // If we didn't match any fields, the response structure doesn't match
    // our expectations — return the full result to avoid data loss
    if (kept === 0)
        return result;
    return shaped;
}
const EXPENSIVE_PREFIXES = [
    "compile_", "build_", "render_", "generate_dungeon",
    "simulate_ecosystem", "hydraulic_erosion", "fracture_",
    "bake_", "import_",
];
const CHEAP_PREFIXES = [
    "get_", "list_", "find_", "describe_", "is_", "search_", "resolve_",
    "inspect_", "assert_", "validate_", "check_",
];
/**
 * Classify a tool's expected execution cost.
 * Based on prefix patterns: reads are cheap, compiles/builds are expensive,
 * everything else is moderate.
 */
export function classifyCost(toolName) {
    const lower = toolName.toLowerCase();
    if (EXPENSIVE_PREFIXES.some((p) => lower.startsWith(p)))
        return "expensive";
    if (CHEAP_PREFIXES.some((p) => lower.startsWith(p)))
        return "cheap";
    return "moderate";
}
/**
 * Classify cost from actual execution duration.
 * Overrides the prefix heuristic with real timing data.
 */
export function classifyCostFromDuration(durationMs) {
    if (durationMs < 500)
        return "cheap";
    if (durationMs < 5_000)
        return "moderate";
    return "expensive";
}
//# sourceMappingURL=tool-resolution.js.map