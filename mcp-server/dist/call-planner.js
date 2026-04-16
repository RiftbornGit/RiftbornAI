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
import { getToolDependencyGraph } from "./tool-dependency-graph.js";
const MAX_GOAL_QUERY_LENGTH = 256;
const BLOCKED_PLANNER_KEYS = new Set(["__proto__", "constructor", "prototype"]);
function toSafeRecord(value) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
        return {};
    }
    const out = {};
    for (const [key, entry] of Object.entries(value)) {
        if (!BLOCKED_PLANNER_KEYS.has(key)) {
            out[key] = entry;
        }
    }
    return out;
}
export function normalizeGoalQuery(goal) {
    return goal
        .trim()
        .replace(/\s+/g, " ")
        .slice(0, MAX_GOAL_QUERY_LENGTH);
}
// ---------------------------------------------------------------------------
// Goal resolution: keyword → target tools
// ---------------------------------------------------------------------------
/**
 * Known goal keywords → target tool(s).
 * These map natural-language goals to the "leaf" tools the agent probably
 * wants to reach. The planner then works backwards through the graph.
 */
const GOAL_MAP = {
    // Terrain
    landscape: ["create_landscape"],
    terrain: ["create_landscape", "sculpt_landscape"],
    sculpt: ["sculpt_landscape"],
    "paint terrain": ["paint_landscape_layer"],
    "paint landscape": ["paint_landscape_layer"],
    // Grass / foliage
    grass: ["add_grass_variety"],
    foliage: ["paint_foliage"],
    "foliage instance": ["add_foliage_instance"],
    // Materials
    material: ["create_material"],
    "pbr material": ["create_pbr_material"],
    "material instance": ["create_material_instance"],
    "landscape material": ["apply_landscape_material"],
    // Blueprint
    blueprint: ["compile_blueprint"],
    "blueprint component": ["add_blueprint_component"],
    "blueprint variable": ["add_blueprint_variable"],
    "blueprint event": ["add_blueprint_event"],
    // Character
    character: ["create_character_from_third_person"],
    "playable character": ["make_character_playable"],
    // Lighting
    light: ["create_light"],
    "post process": ["set_post_process_settings"],
    "post-process": ["set_post_process_settings"],
    // VFX
    niagara: ["spawn_niagara_at_location"],
    particles: ["spawn_niagara_at_location"],
    vfx: ["spawn_niagara_at_location"],
    // Audio
    metasound: ["add_metasound_node"],
    audio: ["create_metasound_source"],
    // Animation
    animation: ["create_anim_blueprint"],
    "anim blueprint": ["add_anim_state"],
    // Sequencer
    cinematic: ["add_sequence_track"],
    sequence: ["create_level_sequence"],
    // PIE
    playtest: ["start_pie"],
    play: ["start_pie"],
    // AI
    "behavior tree": ["add_bt_task"],
    ai: ["add_bt_task"],
    "state tree": ["add_state_tree_state"],
    eqs: ["add_eqs_generator"],
    navigation: ["build_navmesh"],
    // PCG
    pcg: ["execute_pcg"],
    procedural: ["execute_pcg"],
    // Data
    datatable: ["add_datatable_row"],
    // Complete pipelines (multiple goals)
    "forest environment": [
        "create_landscape", "sculpt_landscape", "apply_landscape_material",
        "paint_landscape_layer", "add_grass_variety", "paint_foliage",
    ],
    arena: [
        "create_landscape", "sculpt_landscape", "apply_landscape_material",
        "create_light", "build_navmesh",
    ],
};
/**
 * Resolve a goal string to target tools.
 *
 * First checks exact match in GOAL_MAP, then tries substring matching,
 * then checks if it's a direct tool name.
 */
export function resolveGoal(goal) {
    const lower = normalizeGoalQuery(goal).toLowerCase();
    // Exact match
    if (GOAL_MAP[lower])
        return GOAL_MAP[lower];
    // Substring match — pick the longest matching key
    let bestMatch = "";
    for (const key of Object.keys(GOAL_MAP)) {
        if (lower.includes(key) && key.length > bestMatch.length) {
            bestMatch = key;
        }
    }
    if (bestMatch)
        return GOAL_MAP[bestMatch];
    // Direct tool name
    if (lower.includes("_"))
        return [lower];
    return [];
}
// ---------------------------------------------------------------------------
// Plan builder
// ---------------------------------------------------------------------------
/**
 * Parameter hints: known param names that can be pre-filled from context.
 * Maps context propagator keys to tool param names.
 */
const CONTEXT_TO_PARAM = {
    last_landscape_label: {
        sculpt_landscape: "landscape_label",
        paint_landscape_layer: "landscape_label",
        add_landscape_layer: "landscape_label",
        apply_landscape_material: "landscape_name",
    },
    last_material_path: {
        set_material_parameter: "material_path",
        apply_landscape_material: "material_path",
    },
    last_landscape_material_path: {
        apply_landscape_material: "material_path",
    },
    last_blueprint_path: {
        add_blueprint_component: "blueprint_path",
        add_blueprint_variable: "blueprint_path",
        add_blueprint_event: "blueprint_path",
        compile_blueprint: "blueprint_path",
        open_blueprint: "path",
    },
    last_grass_type_path: {
        add_grass_variety: "grass_type",
    },
    last_actor_label: {
        move_actor: "label",
        rotate_actor: "label",
        scale_actor: "label",
        delete_actor: "label",
        set_actor_material: "label",
        set_actor_color: "label",
    },
};
/**
 * Gather known parameters from a context propagator snapshot for a given tool.
 */
function gatherKnownParams(tool, contextSnapshot) {
    const params = {};
    const safeContext = toSafeRecord(contextSnapshot);
    for (const [contextKey, toolMap] of Object.entries(CONTEXT_TO_PARAM)) {
        const paramName = toolMap[tool];
        if (paramName && safeContext[contextKey] !== undefined) {
            params[paramName] = safeContext[contextKey];
        }
    }
    return params;
}
/**
 * Get a human-readable note for a plan step.
 */
function stepNote(tool, deps) {
    if (deps.length === 0)
        return "No prerequisites — can call immediately.";
    const reasons = deps.map((d) => d.reason);
    return `Requires: ${reasons.join("; ")}`;
}
/**
 * Build an execution plan for a set of target tools.
 *
 * The plan is a topologically-sorted sequence that includes all
 * prerequisites from the dependency graph, with steps already
 * completed in this session marked accordingly.
 */
export function buildPlan(targets, sessionHistory, contextSnapshot = {}) {
    const graph = getToolDependencyGraph();
    const safeContextSnapshot = toSafeRecord(contextSnapshot);
    // Build set of completed tool calls
    const completed = new Set();
    for (const entry of sessionHistory) {
        if (entry.ok)
            completed.add(entry.tool);
    }
    // Collect all tools needed: targets + their transitive prereqs
    const allNeeded = new Set();
    for (const target of targets) {
        allNeeded.add(target);
        for (const prereq of graph.getAllPrereqs(target)) {
            allNeeded.add(prereq);
        }
    }
    // Topological sort: process targets one by one, collecting their chains
    const ordered = [];
    const visited = new Set();
    function topoVisit(tool) {
        if (visited.has(tool))
            return;
        visited.add(tool);
        const deps = graph.getDirectDeps(tool);
        for (const dep of deps) {
            // Visit the first (canonical) alternative
            topoVisit(dep.requires[0]);
        }
        if (allNeeded.has(tool)) {
            ordered.push(tool);
        }
    }
    for (const target of targets) {
        topoVisit(target);
    }
    // Build plan steps
    const steps = ordered.map((tool) => {
        const deps = graph.getDirectDeps(tool);
        const alternatives = deps.flatMap((d) => d.requires.slice(1));
        const isCompleted = completed.has(tool);
        const knownParams = gatherKnownParams(tool, safeContextSnapshot);
        return {
            tool,
            completed: isCompleted,
            alternatives,
            known_params: knownParams,
            note: isCompleted ? "Already completed this session." : stepNote(tool, deps),
        };
    });
    const completedCount = steps.filter((s) => s.completed).length;
    const remainingCount = steps.length - completedCount;
    return {
        goals: targets,
        steps,
        completed_count: completedCount,
        remaining_count: remainingCount,
        estimated_calls: remainingCount,
    };
}
/**
 * Build a plan from a natural-language goal string.
 * Resolves the goal to target tools, then builds the plan.
 */
export function planFromGoal(goal, sessionHistory, contextSnapshot = {}) {
    const targets = resolveGoal(goal);
    if (targets.length === 0)
        return null;
    return buildPlan(targets, sessionHistory, contextSnapshot);
}
/**
 * Convert a plan to a batch_execute-compatible step array.
 * Only includes non-completed steps, with known_params merged in.
 */
export function planToBatchSteps(plan) {
    return plan.steps
        .filter((s) => !s.completed)
        .map((s) => ({
        tool: s.tool,
        args: toSafeRecord(s.known_params),
    }));
}
/**
 * List all available goal keywords.
 */
export function listGoals() {
    return Object.keys(GOAL_MAP).sort();
}
//# sourceMappingURL=call-planner.js.map