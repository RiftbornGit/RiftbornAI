/**
 * Tool Dependency Graph — Round 16
 *
 * Static DAG of UE5 tool prerequisites. Replaces the flat DEPENDENCY_RULES
 * table in proactive-guards.ts with a proper graph that supports:
 *
 * 1. Transitive resolution — walks ancestors to find ALL prerequisites
 * 2. Ordering — returns the minimal call sequence to reach a target tool
 * 3. Multi-path validation — OR-branches (any of N tools satisfies a dep)
 * 4. Session-aware checking — validates against actual session history
 *
 * The graph is declared once and queried at runtime. It is intentionally
 * not dynamic — tool relationships in UE5 are structural, not discovered.
 */
// ---------------------------------------------------------------------------
// Graph definition
// ---------------------------------------------------------------------------
/**
 * The master dependency graph.
 *
 * Key = tool name, Value = array of dependency edges.
 * Each edge says "before calling Key, at least one tool in edge.requires
 * must have been called successfully."
 *
 * This encodes the UE5 workflow pipelines:
 * - Landscape: create → layer → material → paint → sculpt → grass
 * - Blueprint: create/open → component/variable/event → compile
 * - Material: create → set parameters
 * - Foliage: create grass type → add variety
 * - PIE: start → stop
 * - Post-process: create volume → set settings
 * - Character: create → make playable
 * - Lighting: create light → build lighting
 * - Level sequence: create → add track
 * - Navmesh: spawn actors → build navmesh (soft)
 */
const GRAPH = {
    // ── Landscape pipeline ──────────────────────────────────────────────
    sculpt_landscape: [
        { requires: ["create_landscape"], reason: "No landscape to sculpt." },
    ],
    add_landscape_layer: [
        { requires: ["create_landscape"], reason: "No landscape to add layers to." },
    ],
    apply_landscape_material: [
        { requires: ["create_landscape"], reason: "No landscape to apply material to." },
        {
            requires: ["create_landscape_material", "create_material", "create_pbr_material"],
            reason: "No material created to apply.",
        },
    ],
    paint_landscape_layer: [
        { requires: ["create_landscape"], reason: "No landscape to paint." },
        { requires: ["add_landscape_layer"], reason: "No layer registered to paint with." },
    ],
    // ── Grass pipeline ──────────────────────────────────────────────────
    add_grass_variety: [
        { requires: ["create_landscape_grass_type"], reason: "No grass type to add variety to." },
    ],
    // ── Blueprint pipeline ──────────────────────────────────────────────
    add_blueprint_component: [
        { requires: ["open_blueprint", "create_blueprint"], reason: "No blueprint open." },
    ],
    add_blueprint_variable: [
        { requires: ["open_blueprint", "create_blueprint"], reason: "No blueprint open." },
    ],
    add_blueprint_event: [
        { requires: ["open_blueprint", "create_blueprint"], reason: "No blueprint open." },
    ],
    add_blueprint_node: [
        { requires: ["open_blueprint", "create_blueprint"], reason: "No blueprint open." },
    ],
    add_blueprint_function: [
        { requires: ["open_blueprint", "create_blueprint"], reason: "No blueprint open." },
    ],
    connect_blueprint_nodes: [
        { requires: ["open_blueprint", "create_blueprint"], reason: "No blueprint open." },
    ],
    compile_blueprint: [
        { requires: ["open_blueprint", "create_blueprint"], reason: "No blueprint to compile." },
    ],
    // ── Material pipeline ───────────────────────────────────────────────
    set_material_parameter: [
        {
            requires: ["create_material", "create_pbr_material", "create_material_instance"],
            reason: "No material created to modify.",
        },
    ],
    create_material_instance: [
        {
            requires: ["create_material", "create_pbr_material"],
            reason: "No parent material created.",
        },
    ],
    // ── Post-process ────────────────────────────────────────────────────
    set_post_process_settings: [
        { requires: ["create_post_process_volume"], reason: "No post-process volume exists." },
    ],
    // ── PIE ─────────────────────────────────────────────────────────────
    stop_pie: [
        { requires: ["start_pie"], reason: "PIE is not running." },
    ],
    // ── Character ───────────────────────────────────────────────────────
    make_character_playable: [
        {
            requires: ["create_character_from_third_person", "spawn_third_person_character"],
            reason: "No character created to make playable.",
        },
    ],
    set_character_mesh: [
        {
            requires: ["create_character_from_third_person", "spawn_third_person_character"],
            reason: "No character to set mesh on.",
        },
    ],
    // ── Foliage ─────────────────────────────────────────────────────────
    paint_foliage: [
        { requires: ["create_landscape"], reason: "No landscape surface for foliage." },
    ],
    add_foliage_instance: [
        { requires: ["create_landscape"], reason: "No landscape surface for foliage." },
    ],
    // ── Sequencer ───────────────────────────────────────────────────────
    add_sequence_track: [
        { requires: ["create_level_sequence"], reason: "No sequence to add tracks to." },
    ],
    render_sequence: [
        { requires: ["create_level_sequence"], reason: "No sequence to render." },
    ],
    // ── Niagara ─────────────────────────────────────────────────────────
    set_niagara_parameter: [
        {
            requires: ["create_niagara_system", "spawn_niagara_at_location", "spawn_niagara_attached"],
            reason: "No Niagara system to modify.",
        },
    ],
    activate_niagara: [
        {
            requires: ["spawn_niagara_at_location", "spawn_niagara_attached"],
            reason: "No spawned Niagara actor to activate.",
        },
    ],
    // ── MetaSound ───────────────────────────────────────────────────────
    add_metasound_node: [
        {
            requires: ["create_metasound_source", "create_metasound_preset"],
            reason: "No MetaSound asset to edit.",
        },
    ],
    connect_metasound_nodes: [
        {
            requires: ["create_metasound_source", "create_metasound_preset"],
            reason: "No MetaSound asset to wire.",
        },
    ],
    // ── Animation ───────────────────────────────────────────────────────
    add_anim_state: [
        { requires: ["create_anim_blueprint"], reason: "No AnimBP to add states to." },
    ],
    add_anim_transition: [
        { requires: ["create_anim_blueprint"], reason: "No AnimBP to add transitions to." },
    ],
    // ── Data Tables ─────────────────────────────────────────────────────
    add_datatable_row: [
        { requires: ["create_datatable"], reason: "No DataTable to add rows to." },
    ],
    // ── PCG ─────────────────────────────────────────────────────────────
    add_pcg_node: [
        { requires: ["create_pcg_graph"], reason: "No PCG graph to add nodes to." },
    ],
    connect_pcg_nodes: [
        { requires: ["create_pcg_graph"], reason: "No PCG graph to wire." },
    ],
    execute_pcg: [
        { requires: ["create_pcg_graph"], reason: "No PCG graph to execute." },
    ],
    // ── Behavior Tree ───────────────────────────────────────────────────
    add_bt_task: [
        { requires: ["create_behavior_tree"], reason: "No behavior tree to add tasks to." },
    ],
    add_bt_decorator: [
        { requires: ["create_behavior_tree"], reason: "No behavior tree to add decorators to." },
    ],
    add_bt_service: [
        { requires: ["create_behavior_tree"], reason: "No behavior tree to add services to." },
    ],
    // ── State Tree ──────────────────────────────────────────────────────
    add_state_tree_state: [
        { requires: ["create_state_tree"], reason: "No StateTree to add states to." },
    ],
    // ── EQS ─────────────────────────────────────────────────────────────
    add_eqs_generator: [
        { requires: ["create_env_query"], reason: "No EQS query to add generators to." },
    ],
    add_eqs_test: [
        { requires: ["create_env_query"], reason: "No EQS query to add tests to." },
    ],
};
// ---------------------------------------------------------------------------
// Graph class
// ---------------------------------------------------------------------------
export class ToolDependencyGraph {
    edges;
    /** Reverse index: tool → tools that depend on it. */
    reverseMap;
    constructor(graph) {
        this.edges = graph ?? GRAPH;
        this.reverseMap = new Map();
        this.buildReverseMap();
    }
    buildReverseMap() {
        for (const [tool, deps] of Object.entries(this.edges)) {
            for (const dep of deps) {
                for (const req of dep.requires) {
                    let set = this.reverseMap.get(req);
                    if (!set) {
                        set = new Set();
                        this.reverseMap.set(req, set);
                    }
                    set.add(tool);
                }
            }
        }
    }
    /** Get direct prerequisites for a tool. */
    getDirectDeps(tool) {
        return this.edges[tool] ?? [];
    }
    /** Get tools that directly depend on a given tool. */
    getDependents(tool) {
        const set = this.reverseMap.get(tool);
        return set ? [...set] : [];
    }
    /** Check whether a tool has any registered dependencies. */
    hasDeps(tool) {
        return tool in this.edges;
    }
    /**
     * Collect ALL transitive prerequisites for a tool.
     *
     * Returns a flat set of tool names. For OR-groups, includes all
     * alternatives (any one satisfies the dep at runtime).
     *
     * Cycle-safe via visited tracking.
     */
    getAllPrereqs(tool) {
        const visited = new Set();
        const prereqs = new Set();
        this.collectPrereqs(tool, visited, prereqs);
        return prereqs;
    }
    collectPrereqs(tool, visited, result) {
        if (visited.has(tool))
            return;
        visited.add(tool);
        const deps = this.edges[tool];
        if (!deps)
            return;
        for (const dep of deps) {
            for (const req of dep.requires) {
                result.add(req);
                this.collectPrereqs(req, visited, result);
            }
        }
    }
    /**
     * Build the recommended call sequence to reach a target tool.
     *
     * Returns tools in dependency order (earliest prerequisite first,
     * target tool last). For OR-groups, picks the first alternative.
     *
     * This is a topological sort of the reachable subgraph.
     */
    getCallSequence(tool) {
        const order = [];
        const visited = new Set();
        this.topoVisit(tool, visited, order);
        return {
            target: tool,
            sequence: order.map((s) => s.tool),
            steps: order,
        };
    }
    topoVisit(tool, visited, order) {
        if (visited.has(tool))
            return;
        visited.add(tool);
        const deps = this.edges[tool];
        if (deps) {
            for (const dep of deps) {
                // Pick the first alternative as the canonical path
                const canonical = dep.requires[0];
                this.topoVisit(canonical, visited, order);
            }
        }
        order.push({
            tool,
            alternatives: this.edges[tool]
                ? this.edges[tool].flatMap((d) => d.requires.slice(1))
                : [],
        });
    }
    /**
     * Validate a tool against session history.
     *
     * Returns which direct dependencies are satisfied vs missing.
     * This replaces the flat DEPENDENCY_RULES lookup in proactive-guards.ts.
     */
    validate(tool, sessionHistory) {
        const deps = this.edges[tool];
        if (!deps) {
            return { tool, satisfied: true, missing: [] };
        }
        const calledTools = new Set();
        for (const entry of sessionHistory) {
            if (entry.ok)
                calledTools.add(entry.tool);
        }
        const missing = [];
        for (const dep of deps) {
            const satisfied = dep.requires.some((r) => calledTools.has(r));
            if (!satisfied) {
                const primary = dep.requires[0];
                const altText = dep.requires.length > 1
                    ? ` (or ${dep.requires.slice(1).join(", ")})`
                    : "";
                missing.push({
                    requires: dep.requires,
                    reason: dep.reason,
                    suggestion: `Call ${primary}${altText} first.`,
                });
            }
        }
        return { tool, satisfied: missing.length === 0, missing };
    }
    /**
     * Given a set of successfully called tools, find which tools are now
     * "unlocked" (all their dependencies are in the called set).
     *
     * Useful for suggesting next possible actions.
     */
    getUnlockedTools(calledTools) {
        const unlocked = [];
        for (const [tool, deps] of Object.entries(this.edges)) {
            if (calledTools.has(tool))
                continue;
            const allSatisfied = deps.every((dep) => dep.requires.some((r) => calledTools.has(r)));
            if (allSatisfied)
                unlocked.push(tool);
        }
        return unlocked;
    }
    /** How many tools are tracked in the graph. */
    get size() {
        return Object.keys(this.edges).length;
    }
    /** All tool names that have dependency entries. */
    get tools() {
        return Object.keys(this.edges);
    }
}
// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
let _instance = null;
export function getToolDependencyGraph() {
    if (!_instance) {
        _instance = new ToolDependencyGraph();
    }
    return _instance;
}
/** Reset the singleton (for testing). */
export function resetToolDependencyGraph() {
    _instance = null;
}
//# sourceMappingURL=tool-dependency-graph.js.map