export const DOMAIN_DEFINITION_OF_DONE = {
    blueprint: {
        domain: "Blueprint",
        main_failure_mode: "Graph structure compiles, but the gameplay behavior is still wrong in PIE.",
        required_proof: [
            "get_editor_focus_state",
            "focus_asset_editor",
            "get_blueprint_editor_context",
            "list_blueprint_graphs",
            "find_blueprint_nodes",
            "get_blueprint_compile_diagnostics",
            "assert_blueprint_compiles",
            "validate_blueprint_health",
            "run_quick_playtest",
        ],
        minimum_done_bar: [
            "Inspect the correct Blueprint and graph before mutation.",
            "Compile diagnostics are clean or intentionally explained.",
            "Runtime behavior was checked in PIE for the actual gameplay path.",
            "Destructive graph edits keep snapshot-style before/after evidence.",
        ],
        not_done_if: [
            "The Blueprint only compiles but the gameplay path was not exercised.",
            "The wrong graph or wrong asset might have been edited.",
        ],
    },
    leveldesign: {
        domain: "LevelDesign",
        main_failure_mode: "The space looks reasonable in screenshots but traversal, sightlines, or cover spacing break once the player moves through it.",
        required_proof: [
            "observe_ue_project",
            "capture_viewport_sync",
            "look_at_and_capture",
            "build_navmesh",
            "get_navmesh_status",
            "run_quick_playtest",
            "save_level",
        ],
        minimum_done_bar: [
            "Review the space from gameplay-relevant angles.",
            "Rebuild navmesh after traversal-affecting changes and check status.",
            "Traverse the space in play, not only from the editor camera.",
        ],
        not_done_if: [
            "Navmesh was not rebuilt after geometry changed.",
            "The layout was judged only from still images.",
        ],
    },
    vfx: {
        domain: "VFX",
        main_failure_mode: "The Niagara system compiles and looks good in isolation, but is unreadable, noisy, or too expensive in gameplay context.",
        required_proof: [
            "get_niagara_editor_context",
            "get_niagara_stack_context",
            "list_niagara_modules",
            "assert_niagara_compiles",
            "preview_niagara",
            "spawn_niagara_at_location",
            "look_at_and_capture",
            "analyze_scene_screenshot",
            "run_quick_playtest",
            "get_performance_report",
        ],
        minimum_done_bar: [
            "Compile cleanly before world verification.",
            "Check the effect at gameplay camera distance in world.",
            "Review gameplay-facing effects during motion or combat timing.",
        ],
        not_done_if: [
            "Compile passed but the effect was never reviewed in-world.",
            "Readability was checked only from close-up asset preview.",
        ],
    },
    gas: {
        domain: "GAS",
        main_failure_mode: "The ability and effect assets exist, but costs, cooldowns, stacking, actor assignment, or replicated gameplay truth are wrong at runtime.",
        required_proof: [
            "get_gas_assets",
            "get_gameplay_tags",
            "create_gameplay_effect",
            "configure_gameplay_effect",
            "create_gameplay_ability",
            "create_attribute_set",
            "assert_blueprint_compiles",
            "validate_blueprint_health",
            "add_ability_to_actor",
            "set_actor_gameplay_tags",
            "run_quick_playtest",
            "audit_net_replication",
            "inspect_actor_replication",
        ],
        minimum_done_bar: [
            "Verify the intended tags, attributes, abilities, and effects.",
            "Compile authored Blueprints cleanly.",
            "Assign to a real actor with an ASC and exercise repeated runtime use.",
        ],
        not_done_if: [
            "The asset set exists but no actor-level runtime proof was run.",
            "Replication-sensitive ability work skipped network truth inspection.",
        ],
    },
    ui: {
        domain: "UI",
        main_failure_mode: "The widget compiles and looks fine in the designer, but is invisible, unfocusable, or broken during actual interaction in PIE.",
        required_proof: [
            "get_widget_editor_context",
            "list_widget_tree",
            "compile_widget_blueprint",
            "verify_widget_blueprint_layout",
            "assert_widget_visible_in_pie",
            "capture_ui_state",
            "simulate_widget_navigation_in_pie",
            "run_ui_flow_test",
        ],
        minimum_done_bar: [
            "Inspect the widget tree and active editor context before restructure.",
            "Compile and layout audit pass after meaningful changes.",
            "Prove visibility and interaction in PIE.",
        ],
        not_done_if: [
            "The screen was only checked in the designer.",
            "Interactive navigation or confirm/cancel flows were not tested.",
        ],
    },
    networking: {
        domain: "Networking",
        main_failure_mode: "Replication cost looks acceptable in static audit, but actor-level authority, relevance, or delivery truth is still wrong for the actual gameplay path.",
        required_proof: [
            "audit_net_replication",
            "inspect_actor_replication",
            "create_replication_group",
            "add_actor_to_replication_group",
            "set_replication_group_filter_status",
            "run_quick_playtest",
        ],
        minimum_done_bar: [
            "Run project-wide audit for the affected classes or feature area.",
            "Inspect at least one concrete actor or class instance.",
            "Pair networking proof with owning-domain runtime proof when gameplay can be exercised.",
        ],
        not_done_if: [
            "Only project-wide audit was run and no actor-level truth was inspected.",
            "A multiplayer gameplay feature was called done without runtime proof.",
        ],
    },
    saveload: {
        domain: "Save/Load",
        main_failure_mode: "The snapshot or checkpoint was captured, but restoration either does not return the intended state or the evidence is too weak to prove what changed.",
        required_proof: [
            "capture_level_snapshot",
            "save_scene_checkpoint",
            "get_world_state_digest",
            "restore_level_snapshot",
            "restore_scene_checkpoint",
            "save_level",
            "save_asset",
            "save_dirty_assets",
            "load_level",
        ],
        minimum_done_bar: [
            "Capture the restore target deliberately.",
            "Keep a digest or richer checkpoint artifact around the mutation.",
            "Actually perform and check restoration before sign-off.",
        ],
        not_done_if: [
            "A checkpoint exists but no restore was attempted.",
            "The task claims full player-save semantics instead of editor rollback semantics.",
        ],
    },
    performance: {
        domain: "Performance",
        main_failure_mode: "The feature is functionally correct, but it pushes frame time, draw calls, streaming pressure, or effect cost outside the acceptable envelope.",
        required_proof: [
            "run_quick_playtest",
            "get_performance_report",
            "observe_ue_project",
            "capture_viewport_sync",
            "save_scene_checkpoint",
            "restore_scene_checkpoint",
            "get_world_state_digest",
        ],
        minimum_done_bar: [
            "Measure performance under a representative runtime or review context.",
            "Attach a captured report to sign-off instead of inferring from intuition.",
            "Keep rollback evidence before risky tuning.",
        ],
        not_done_if: [
            "There is no performance report for a heavy or near-final task.",
            "Measurement happened in a context that does not resemble real usage.",
        ],
    },
};
const DOMAIN_SIGNAL_RULES = {
    blueprint: {
        keywords: ["blueprint", "graph", "bp_"],
        tools: ["create_blueprint", "compile_blueprint", "add_blueprint_", "connect_blueprint_", "set_blueprint_", "validate_blueprint_health", "assert_blueprint_compiles"],
        requiredTools: ["run_quick_playtest"],
        severity: "error",
        guidance: "Blueprint work is not done on compile proof alone. Run runtime proof with run_quick_playtest.",
    },
    leveldesign: {
        keywords: ["arena", "blockout", "corridor", "room", "level", "map", "navmesh", "cover", "traversal"],
        tools: ["build_navmesh", "get_navmesh_status", "create_landscape", "save_level", "load_level", "create_level", "generate_blockout"],
        requiredTools: ["build_navmesh", "get_navmesh_status", "run_quick_playtest"],
        severity: "error",
        guidance: "Level design work needs navmesh proof plus runtime traversal proof before sign-off.",
    },
    vfx: {
        keywords: ["niagara", "vfx", "particle", "effect"],
        tools: ["niagara", "preview_niagara", "spawn_niagara_"],
        requiredTools: ["run_quick_playtest"],
        severity: "warning",
        guidance: "Gameplay-facing VFX should be reviewed in runtime, not only in isolated preview.",
    },
    gas: {
        keywords: ["ability", "gameplay effect", "attribute", "gas", "cooldown", "cost"],
        tools: ["ability", "gameplay_effect", "attribute_set", "set_actor_gameplay_tags", "audit_net_replication", "inspect_actor_replication"],
        requiredTools: ["run_quick_playtest"],
        severity: "error",
        guidance: "GAS authoring is incomplete without actor-level runtime proof of costs, cooldowns, and state changes.",
    },
    ui: {
        keywords: ["widget", "hud", "menu", "overlay", "umg", "ui"],
        tools: ["widget", "ui_flow", "assert_widget_visible_in_pie", "capture_ui_state", "simulate_widget_navigation_in_pie"],
        requiredTools: ["assert_widget_visible_in_pie", "capture_ui_state"],
        severity: "error",
        guidance: "UI work is not done in the designer alone. Prove visibility and live state in PIE.",
    },
    networking: {
        keywords: ["network", "replication", "multiplayer", "iris"],
        tools: ["replication", "network", "iris", "multiplayer"],
        requiredTools: ["audit_net_replication", "inspect_actor_replication"],
        severity: "warning",
        guidance: "Networking-sensitive work should pair project audit with actor-level replication truth.",
    },
    saveload: {
        keywords: ["snapshot", "checkpoint", "restore", "rollback", "save/load"],
        tools: ["snapshot", "checkpoint", "restore_", "save_scene_checkpoint", "capture_level_snapshot"],
        requiredTools: ["restore_level_snapshot"],
        severity: "warning",
        guidance: "Save/load style work is incomplete until a restore path is actually exercised.",
    },
    performance: {
        keywords: ["performance", "fps", "memory", "render cost", "optimization"],
        tools: ["performance", "fps", "memory", "render_cost", "optimize_"],
        requiredTools: ["get_performance_report"],
        severity: "warning",
        guidance: "Performance work should attach a real performance report instead of relying on intuition.",
    },
};
export function normalizeDomainKey(domain) {
    return domain
        .trim()
        .toLowerCase()
        .replace(/[\s/_-]+/g, "");
}
export function getDomainDefinitionOfDoneContract(domain) {
    if (!domain) {
        return {
            source_doc: "docs/AI_School/DOMAIN_DEFINITION_OF_DONE.md",
            domains: Object.values(DOMAIN_DEFINITION_OF_DONE),
        };
    }
    const normalized = normalizeDomainKey(domain);
    if (normalized === "level") {
        return DOMAIN_DEFINITION_OF_DONE.leveldesign;
    }
    if (normalized === "saveload" || normalized === "save") {
        return DOMAIN_DEFINITION_OF_DONE.saveload;
    }
    return DOMAIN_DEFINITION_OF_DONE[normalized] || null;
}
function matchesRuleText(rule, text) {
    return rule.keywords.some((keyword) => text.includes(keyword));
}
function matchesRuleTool(rule, toolName) {
    return rule.tools.some((needle) => toolName.includes(needle));
}
export function inferDomainsFromTask(task) {
    const lower = task.toLowerCase();
    const detected = new Set();
    for (const [domain, rule] of Object.entries(DOMAIN_SIGNAL_RULES)) {
        if (matchesRuleText(rule, lower)) {
            detected.add(domain);
        }
    }
    return [...detected];
}
export function inferDomainsFromHistory(sceneChanges, recentTools = []) {
    const detected = new Set();
    const tools = [
        ...sceneChanges.map((change) => change.tool.toLowerCase()),
        ...recentTools.map((tool) => tool.toLowerCase()),
    ];
    for (const [domain, rule] of Object.entries(DOMAIN_SIGNAL_RULES)) {
        if (tools.some((toolName) => matchesRuleTool(rule, toolName))) {
            detected.add(domain);
        }
    }
    return [...detected];
}
export function findDomainProofGaps(domains, recentTools = []) {
    const normalizedTools = recentTools.map((tool) => tool.toLowerCase());
    const gaps = [];
    for (const domain of domains) {
        const rule = DOMAIN_SIGNAL_RULES[domain];
        if (!rule) {
            continue;
        }
        const missingTools = rule.requiredTools.filter((toolName) => !normalizedTools.includes(toolName.toLowerCase()));
        if (missingTools.length === 0) {
            continue;
        }
        gaps.push({
            domain,
            label: `${DOMAIN_DEFINITION_OF_DONE[domain].domain} proof contract`,
            severity: rule.severity,
            missing_tools: missingTools,
            guidance: rule.guidance,
        });
    }
    return gaps;
}
export function buildDomainContractResourceUris(task) {
    return inferDomainsFromTask(task).map((domain) => `riftborn://copilot/domain-definition-of-done/${domain}`);
}