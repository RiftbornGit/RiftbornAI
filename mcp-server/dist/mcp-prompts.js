import { GetPromptRequestSchema, ListPromptsRequestSchema } from "@modelcontextprotocol/sdk/types.js";
import { normalizeResourceSearchQuery } from "./mcp-resources.js";
const MAX_PROMPT_ARG_LENGTH = 512;
export function normalizePromptArgument(value, fallback) {
    if (!value)
        return fallback;
    const normalized = value
        .trim()
        .replace(/[\r\n\t\0]+/g, " ")
        .replace(/\s+/g, " ")
        .slice(0, MAX_PROMPT_ARG_LENGTH);
    return normalized || fallback;
}
export const PROMPTS = [
    {
        name: "create-ability",
        description: "Scaffold a Gameplay Ability asset set with effect, optional cost, and cooldown assets",
        arguments: [
            { name: "ability_name", description: "Name for the ability (e.g., Fireball, Dash, HealingWave)", required: true },
            { name: "description", description: "What the ability does (e.g., 'Launches a fireball that deals 50 fire damage and applies a 5s burn DOT')", required: true },
            { name: "cost_type", description: "Resource cost type: mana, stamina, health, cooldown_only", required: false },
        ],
    },
    {
        name: "fix-blueprints",
        description: "Diagnose and repair broken Blueprints in the project. Finds missing references, orphan nodes, broken function calls.",
        arguments: [
            { name: "scope", description: "Folder scope to scan (default: /Game/)", required: false },
            { name: "dry_run", description: "If 'true', only diagnose without making changes", required: false },
        ],
    },
    {
        name: "setup-character",
        description: "Set up a player-character scaffold with mesh, animation, input assets, and pawn wiring steps",
        arguments: [
            { name: "character_name", description: "Name for the character Blueprint (e.g., BP_PlayerCharacter)", required: true },
            { name: "movement_type", description: "Movement style: third_person, first_person, top_down, side_scroller", required: false },
        ],
    },
    {
        name: "debug-issue",
        description: "Diagnose a gameplay or editor issue with console commands, log analysis, and recommendations",
        arguments: [
            { name: "problem", description: "Description of the problem (e.g., 'character not taking damage', 'AI not moving', 'low fps')", required: true },
        ],
    },
    {
        name: "create-game-mode",
        description: "Scaffold a game mode setup with the core Blueprint assets and HUD shell",
        arguments: [
            { name: "mode_name", description: "Name for the game mode (e.g., TeamDeathmatch, CaptureTheFlag, BattleRoyale)", required: true },
            { name: "genre", description: "Game genre: fps, tps, rpg, rts, platformer, racing", required: false },
        ],
    },
    {
        name: "add-vfx",
        description: "Add visual effects to an actor or location using Niagara particle systems",
        arguments: [
            { name: "effect_type", description: "Effect type: fire, smoke, sparks, explosion, rain, snow, magic, blood, dust", required: true },
            { name: "target", description: "Actor name to attach to, or 'location' for world-space", required: false },
        ],
    },
    {
        name: "governed-editor-task",
        description: "Run a task through the shared RiftbornAI editor-control loop used by MCP and the built-in copilot",
        arguments: [
            { name: "task", description: "The Unreal Editor task to accomplish", required: true },
        ],
    },
    {
        name: "observe-scene",
        description: "Take a comprehensive look at the current UE scene: screenshot, actor census, viewport state, and AI vision analysis",
        arguments: [
            { name: "focus", description: "What to focus on: lighting, materials, layout, performance, actors", required: false },
        ],
    },
];
export function buildPromptText(name, args) {
    switch (name) {
        case "create-ability": {
            const abilityName = normalizePromptArgument(args?.ability_name, "MyAbility");
            const desc = normalizePromptArgument(args?.description, "A gameplay ability");
            const costType = normalizePromptArgument(args?.cost_type, "mana");
            return `Create a Gameplay Ability asset set for "${abilityName}".

Ability Description: ${desc}
Cost Type: ${costType}

Please:
1. First call \`get_project_info\` to understand the current project structure
2. Create the Gameplay Ability Blueprint (GA_${abilityName})
3. Create the Gameplay Effect Blueprint (GE_${abilityName}_Effect) with the described behavior
4. If cost_type is not 'cooldown_only', create a cost effect (GE_${abilityName}_Cost)
5. Create a cooldown effect (GE_${abilityName}_Cooldown)
6. Report what was created, what still needs manual gameplay logic or tag wiring, and how to hook the assets up`;
        }
        case "fix-blueprints": {
            const scope = normalizePromptArgument(args?.scope, "/Game/");
            const dryRun = normalizePromptArgument(args?.dry_run, "false") === "true";
            return `${dryRun ? "Diagnose (dry run only — no changes)" : "Diagnose and repair"} Blueprint issues in scope "${scope}".

Steps:
1. Call \`get_project_info\` to understand the project
2. Use \`get_blueprint_errors\` to gather the current Blueprint failures
3. Use \`list_assets\` or \`search_files\` if you need to narrow the scope to specific Blueprint assets under "${scope}"
4. Use \`validate_blueprint_health\` or \`analyze_blueprint_error\` on the highest-signal broken Blueprints
5. ${dryRun ? "Report the root causes, likely fixes, and the exact Blueprints that need repair" : "Apply targeted repairs with the real Blueprint analysis and compile tools you have available, then \`recompile_blueprints\` to confirm the result"}
6. Summarize: initial errors, repairs attempted, final compile status, and any remaining manual follow-up`;
        }
        case "setup-character": {
            const charName = normalizePromptArgument(args?.character_name, "BP_PlayerCharacter");
            const moveType = normalizePromptArgument(args?.movement_type, "third_person");
            return `Set up a ${moveType} player-character scaffold called "${charName}".

Steps:
1. Call \`get_project_info\` to check existing characters and skeleton assets
2. Call \`create_character_from_third_person\` for the new character scaffold
3. If the current surface exposes the beta character helpers, call \`use_manny_mesh\` for the UE mannequin stack; otherwise stay with the created scaffold and report the missing helper explicitly
4. Create concrete input assets with \`create_input_action\`, \`create_input_mapping_context\`, and \`add_input_mapping\`
5. Call \`set_default_pawn_class\` for default-pawn wiring; use \`spawn_third_person_character\` only when the beta character helpers are actually present in the current surface
6. Explicitly report any missing input bindings, GameMode wiring, or template assets instead of assuming a fully finished character
7. Call \`observe_ue_project\` to verify the character is present and wired correctly`;
        }
        case "debug-issue": {
            const problem = normalizePromptArgument(args?.problem, "unknown issue");
            return `Debug this issue: "${problem}"

Steps:
1. Call \`get_output_log\` to collect the current editor/runtime errors
2. If the issue is visual, layout-related, or scene-specific, call \`observe_ue_project\`
3. Use \`execute_console_command\` for targeted follow-up checks
4. If the issue appears to require a capability you cannot find, say which real tool or engine surface is missing instead of inventing one
5. Provide a diagnosis with:
   - Root cause (or top candidates)
   - Specific fix steps
   - Verification steps or console commands to confirm the fix`;
        }
        case "create-game-mode": {
            const modeName = normalizePromptArgument(args?.mode_name, "MyGameMode");
            const genre = normalizePromptArgument(args?.genre, "fps");
            return `Scaffold a "${modeName}" game mode setup for a ${genre} game.

Steps:
1. Call \`get_project_info\` to understand the current setup
2. Call \`create_blueprint\` to create the GameMode, GameState, and PlayerController blueprints you actually need
3. Create concrete input assets with \`create_input_action\` and \`create_input_mapping_context\`
4. Create the HUD shell with \`create_widget\`
5. Report whether GameMode override can be applied automatically in the current build; if not, include the manual level-assignment step
6. Report all created assets and how they connect`;
        }
        case "add-vfx": {
            const effectType = normalizePromptArgument(args?.effect_type, "fire");
            const target = normalizePromptArgument(args?.target, "location");
            return `Add a ${effectType} visual effect ${target === "location" ? "at a world location" : `attached to actor "${target}"`}.

Steps:
1. Create a Niagara system from the "${effectType.charAt(0).toUpperCase() + effectType.slice(1)}" template
2. ${target === "location" ? "Spawn it at the player's current location or world origin" : `Attach it to the "${target}" actor`}
3. Verify the effect is visible
4. Report the asset path and how to customize it`;
        }
        case "governed-editor-task": {
            const task = normalizePromptArgument(args?.task, "accomplish the requested Unreal Editor task");
            const boundedTask = normalizeResourceSearchQuery(task);
            return `Use RiftbornAI's shared governed editor loop to accomplish this task: "${task}".

Workflow:
1. Read \`riftborn://copilot/operating-contract\`
2. If tool selection is unclear, inspect \`riftborn://tools/categories\` and \`riftborn://tools/search/${encodeURIComponent(boundedTask)}\`
3. Inspect the current project or scene state before making changes
4. Check blockers and diagnostics early with \`get_modal_blockers\`, \`assert_no_modal_blockers\`, \`get_notification_center_state\`, \`get_output_log_context\`, \`get_message_log_context\`, \`drain_log_alerts\`, and \`get_compile_diagnostics\`
5. If the task needs exact object-level or World Outliner control, prefer \`get_world_outliner_context\`, \`assert_actor_selection\`, \`list_object_properties\`, \`get_object_property_typed\`, \`assert_object_property_equals\`, \`set_object_property_typed\`, and \`call_reflected_function\` over guessing or screenshot-driven UI actions
6. Use explicit persistence and focus assertions when changes must land on disk or stay grounded in the right editor context: \`assert_editor_focus\`, \`assert_asset_dirty_state\`, \`save_asset\`, \`save_dirty_assets\`, \`checkout_asset\`, and \`revert_asset\`
7. If the task is inside an asset editor, call \`get_editor_focus_state\` first, then use the matching editor-native context tool only when it is actually listed in the current surface. The Blueprint, Material, Sequencer, Control Rig, Niagara, Widget, and PCG context helpers are on the default-visible lane
8. When the task needs deeper editor-native inspection inside asset editors, use the available editor context tools from step 7. If the exact domain helper is not listed, say so and fall back to logs, assertions, screenshots, or reflected control rather than assuming it exists
9. Use \`compile_blueprint\` after Blueprint mutations and \`recompile_material_asset\` after material graph changes to verify correctness
10. If the task is about world-state branching or rollback, prefer \`capture_level_snapshot\`, \`restore_level_snapshot\`, \`create_level_variant_sets_asset\`, and \`switch_variant_by_name\`
11. If the task is about operator-facing controls or authored affordances, prefer \`create_remote_control_preset\`, \`expose_property_to_remote_control\`, \`register_smart_object_actor\`, and related Smart Object slot tools
12. If the task is about authored decision tables or render pipeline setup, prefer \`evaluate_chooser_table\`, \`evaluate_proxy_table\`, \`create_movie_graph_config\`, and \`queue_trailer_render_job\`
13. Execute the smallest valid sequence of real registered tools
14. Verify the result with scene inspection, screenshots, logs, assertions, or PIE as appropriate
15. Report what changed, what was verified, and any remaining manual gaps`;
        }
        case "observe-scene": {
            const focus = normalizePromptArgument(args?.focus, "general");
            return `Take a comprehensive look at the current Unreal Engine scene${focus !== "general" ? `, focusing on ${focus}` : ""}.

Use \`observe_ue_project\` with:
- capture_screenshot: true
- include_vision: true
${focus !== "general" ? `- prompt: "Focus on ${focus}: describe what you see in detail"` : ""}

Then summarize:
- What's in the scene (actor census)
- Visual assessment (from screenshot)
- Any issues or suggestions
- ${focus !== "general" ? `Specific ${focus} analysis` : "General observations"}`;
        }
        default:
            return `Run the "${name}" task in Unreal Engine using the tools available to you.\n\n` +
                Object.entries(args ?? {})
                    .filter(([, v]) => v != null)
                    .map(([k, v]) => `${k}: ${normalizePromptArgument(v, "")}`)
                    .join('\n');
    }
}
export function registerPromptHandlers(server) {
    server.setRequestHandler(ListPromptsRequestSchema, async () => ({
        prompts: PROMPTS,
    }));
    server.setRequestHandler(GetPromptRequestSchema, async (request) => {
        const { name, arguments: args } = request.params;
        const prompt = PROMPTS.find((entry) => entry.name === name);
        if (!prompt) {
            throw new Error(`Unknown prompt: ${name}`);
        }
        return {
            description: prompt.description,
            messages: [
                {
                    role: "user",
                    content: {
                        type: "text",
                        text: buildPromptText(name, args),
                    },
                },
            ],
        };
    });
}
//# sourceMappingURL=mcp-prompts.js.map