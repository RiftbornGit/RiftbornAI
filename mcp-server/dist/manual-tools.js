import { GENERATED_TOOLS } from "./generated-tools.js";
import { VISION_LOOP_TOOLS } from "./vision-loop.js";
import { SCENE_ENHANCEMENT_TOOLS } from "./scene-enhancement.js";
import { POLYHAVEN_TOOLS } from "./polyhaven.js";
export const MANUAL_TOOLS = [
    // ============= Manual-Only Tools (not in generated-tools.ts) =============
    {
        name: "get_actor_transform",
        description: "Get the world position, rotation, and scale of an actor. Returns location (x,y,z), rotation (pitch,yaw,roll), and scale (x,y,z).",
        inputSchema: {
            type: "object",
            properties: {
                actor_name: {
                    type: "string",
                    description: "Actor name or label",
                },
            },
            required: ["actor_name"],
        },
    },
    {
        name: "get_actors_in_radius",
        description: "Find all actors within a radius of a world point. Returns actor names, classes, and distances. Useful for spatial queries and proximity checks.",
        inputSchema: {
            type: "object",
            properties: {
                x: { type: "number", description: "Center X coordinate" },
                y: { type: "number", description: "Center Y coordinate" },
                z: { type: "number", description: "Center Z coordinate" },
                radius: { type: "number", description: "Search radius in Unreal units (cm)" },
                class_filter: { type: "string", description: "Optional: only return actors of this class" },
            },
            required: ["x", "y", "z", "radius"],
        },
    },
    // ============= Static Meshes & Components =============
    {
        name: "create_static_mesh_actor",
        description: "Spawn a StaticMeshActor with a mesh already assigned. Use snap_to_ground=true to auto-place on terrain. Engine shapes: /Engine/BasicShapes/Cube, Sphere, Cylinder, Cone, Plane (center-pivoted, 100cm — snap_to_ground auto-offsets so the bottom sits on the surface).",
        inputSchema: {
            type: "object",
            properties: {
                mesh: { type: "string", description: "Static mesh asset path (e.g., '/Engine/BasicShapes/Cube', '/Game/Meshes/SM_Wall')" },
                label: { type: "string", description: "Actor label in World Outliner", default: "StaticMeshActor" },
                folder: { type: "string", description: "World Outliner folder path (auto-assigned if empty)" },
                x: { type: "number", description: "World X position", default: 0 },
                y: { type: "number", description: "World Y position", default: 0 },
                z: { type: "number", description: "World Z position (ignored if snap_to_ground=true)", default: 0 },
                snap_to_ground: {
                    type: "boolean",
                    description: "If true, auto-detect ground via line trace and offset by mesh bounds so the bottom touches the surface.",
                    default: false,
                },
            },
            required: ["mesh"],
        },
    },
    {
        name: "generate_and_import_3d_model",
        description: "Generate a 3D model from a text prompt via Meshy, then import the resulting GLB into Unreal. Also accepts a direct-download url to preserve the existing importer flow. Prompt mode requires MESHY_API_KEY in the MCP server environment.",
        inputSchema: {
            type: "object",
            properties: {
                prompt: {
                    type: "string",
                    description: "Optional Meshy text prompt. Provide either prompt or url.",
                },
                reference_images: {
                    type: "array",
                    description: "Optional image references for Meshy image-to-3D. Provide 1-4 public URLs, data URIs, or readable local image paths. Mutually exclusive with prompt and url.",
                    items: { type: "string" },
                },
                url: {
                    type: "string",
                    description: "Optional direct-download model URL. If provided, bypasses Meshy generation and imports directly.",
                },
                destination_path: {
                    type: "string",
                    description: "UE content path for import (e.g. /Game/Generated/Models)",
                    default: "/Game/Generated/Models",
                },
                asset_name: {
                    type: "string",
                    description: "Name for the imported asset. If omitted in prompt mode, a name is derived from the prompt.",
                },
                model_type: {
                    type: "string",
                    description: "Meshy model type. standard = high detail, lowpoly = cleaner low-poly mesh.",
                    enum: ["standard", "lowpoly"],
                },
                ai_model: {
                    type: "string",
                    description: "Meshy backend model id.",
                    enum: ["latest", "meshy-5", "meshy-6"],
                    default: "latest",
                },
                should_texture: {
                    type: "boolean",
                    description: "If true, run Meshy's refine stage and import the textured result.",
                    default: true,
                },
                texture_prompt: {
                    type: "string",
                    description: "Optional extra texturing guidance for Meshy's refine stage.",
                },
                texture_image: {
                    type: "string",
                    description: "Optional texture-guide image for Meshy texturing. Accepts a public URL, data URI, or readable local image path.",
                },
                enable_pbr: {
                    type: "boolean",
                    description: "Request PBR texture maps during Meshy refine.",
                    default: false,
                },
                should_remesh: {
                    type: "boolean",
                    description: "Enable Meshy remesh so topology and target_polycount can be honored.",
                    default: false,
                },
                topology: {
                    type: "string",
                    description: "Requested topology when should_remesh=true.",
                    enum: ["triangle", "quad"],
                },
                target_polycount: {
                    type: "number",
                    description: "Requested target polycount for remeshed output. Clamped to Meshy's documented 100-300000 range.",
                },
                pose_mode: {
                    type: "string",
                    description: "Optional pose mode for Meshy image generation.",
                    enum: ["", "a-pose", "t-pose"],
                },
                image_enhancement: {
                    type: "boolean",
                    description: "Enable Meshy image enhancement when using reference images.",
                    default: true,
                },
                remove_lighting: {
                    type: "boolean",
                    description: "Ask Meshy to remove lighting from generated textures when supported.",
                    default: true,
                },
                moderation: {
                    type: "boolean",
                    description: "Enable Meshy moderation screening for reference images and texture guidance.",
                    default: false,
                },
                auto_size: {
                    type: "boolean",
                    description: "Ask Meshy to estimate real-world object scale from the reference image(s).",
                    default: false,
                },
                origin_at: {
                    type: "string",
                    description: "Origin placement when auto_size=true.",
                    enum: ["bottom", "center"],
                },
                place_in_scene: {
                    type: "boolean",
                    description: "Spawn the imported mesh in the scene after import.",
                    default: false,
                },
                place_x: {
                    type: "number",
                    description: "X location if place_in_scene=true.",
                    default: 0,
                },
                place_y: {
                    type: "number",
                    description: "Y location if place_in_scene=true.",
                    default: 0,
                },
                place_z: {
                    type: "number",
                    description: "Z location if place_in_scene=true.",
                    default: 0,
                },
                enable_nanite: {
                    type: "boolean",
                    description: "Enable Nanite on the imported mesh.",
                    default: false,
                },
                generate_collision: {
                    type: "boolean",
                    description: "Generate simple collision for the imported mesh.",
                    default: true,
                },
            },
        },
    },
    {
        name: "generate_texture",
        description: "Generate a texture from a text prompt via OpenAI Images, then import it into Unreal. Also accepts a direct-download url to preserve the existing texture importer flow. Prompt mode requires OPENAI_API_KEY in the MCP server environment.",
        inputSchema: {
            type: "object",
            properties: {
                prompt: {
                    type: "string",
                    description: "Optional image-generation prompt. Provide either prompt or url.",
                },
                reference_images: {
                    type: "array",
                    description: "Optional source images for OpenAI image editing/reference generation. Provide URLs, data URIs, or readable local image paths. When present, generate_texture uses the image-edit endpoint instead of fresh generation.",
                    items: { type: "string" },
                },
                mask_image: {
                    type: "string",
                    description: "Optional mask image for in-painting. Accepts a URL, data URI, or readable local image path.",
                },
                url: {
                    type: "string",
                    description: "Optional direct-download texture URL. If provided, bypasses OpenAI generation and imports directly.",
                },
                destination_path: {
                    type: "string",
                    description: "UE content path for import.",
                    default: "/Game/Generated/Textures",
                },
                asset_name: {
                    type: "string",
                    description: "Name for the imported texture. If omitted in prompt mode, a name is derived from the prompt.",
                },
                model: {
                    type: "string",
                    description: "OpenAI image model.",
                    enum: ["gpt-image-1.5", "gpt-image-1", "gpt-image-1-mini"],
                    default: "gpt-image-1.5",
                },
                size: {
                    type: "string",
                    description: "Output image size.",
                    enum: ["auto", "1024x1024", "1536x1024", "1024x1536"],
                    default: "1024x1024",
                },
                quality: {
                    type: "string",
                    description: "OpenAI render quality.",
                    enum: ["auto", "low", "medium", "high"],
                    default: "auto",
                },
                background: {
                    type: "string",
                    description: "Background mode for GPT image models.",
                    enum: ["auto", "opaque", "transparent"],
                    default: "auto",
                },
                output_format: {
                    type: "string",
                    description: "Output image format.",
                    enum: ["png", "jpeg", "webp"],
                    default: "png",
                },
                output_compression: {
                    type: "number",
                    description: "Compression percentage for jpeg/webp output.",
                },
                moderation: {
                    type: "string",
                    description: "Moderation strictness for GPT image models.",
                    enum: ["auto", "low"],
                    default: "auto",
                },
                input_fidelity: {
                    type: "string",
                    description: "How closely edited images should preserve the provided reference images.",
                    enum: ["low", "high"],
                    default: "low",
                },
                compression: {
                    type: "string",
                    description: "UE texture compression setting after import.",
                    default: "Default",
                },
            },
        },
    },
    {
        name: "generate_audio",
        description: "Generate a sound effect from a text prompt via ElevenLabs, import it into Unreal as a SoundWave, and optionally play or attach it in the scene. Requires ELEVENLABS_API_KEY in the MCP server environment.",
        inputSchema: {
            type: "object",
            properties: {
                prompt: {
                    type: "string",
                    description: "Text prompt describing the desired sound effect.",
                },
                asset_name: {
                    type: "string",
                    description: "Optional name for the imported SoundWave asset.",
                },
                destination_path: {
                    type: "string",
                    description: "UE content path for imported audio.",
                    default: "/Game/Generated/Audio",
                },
                duration_seconds: {
                    type: "number",
                    description: "Requested sound length in seconds. ElevenLabs supports 0.5-30 seconds.",
                },
                prompt_influence: {
                    type: "number",
                    description: "How closely the result should follow the prompt, from 0 to 1.",
                    default: 0.3,
                },
                loop: {
                    type: "boolean",
                    description: "Request a smoothly looping sound effect when supported.",
                    default: false,
                },
                model_id: {
                    type: "string",
                    description: "ElevenLabs sound-generation model id.",
                    default: "eleven_text_to_sound_v2",
                },
                actor_label: {
                    type: "string",
                    description: "Optional actor label to attach the generated sound to via spawn_audio_component.",
                },
                auto_play: {
                    type: "boolean",
                    description: "If true and actor_label is supplied, auto-play the attached audio component.",
                    default: false,
                },
                play_2d: {
                    type: "boolean",
                    description: "If true, play the imported sound immediately as 2D audio instead of attaching it to an actor.",
                    default: false,
                },
            },
            required: ["prompt"],
        },
    },
    {
        name: "generate_blockout",
        description: "Generate a playable Geometry Script blockout from a short description. Supports room, hallway, arena, tower, courtyard, and bridge-style layouts, plus openings, arches, ramps, stairs, and support columns.",
        inputSchema: {
            type: "object",
            properties: {
                description: {
                    type: "string",
                    description: "Short text description used to infer the blockout template and optional features.",
                },
                template: {
                    type: "string",
                    description: "Explicit blockout template override.",
                    enum: ["room", "hallway", "arena", "tower", "courtyard", "bridge"],
                },
                actor_prefix: {
                    type: "string",
                    description: "Optional prefix for generated blockout actors.",
                },
                origin_x: {
                    type: "number",
                    description: "Center X location for the blockout.",
                    default: 0,
                },
                origin_y: {
                    type: "number",
                    description: "Center Y location for the blockout.",
                    default: 0,
                },
                origin_z: {
                    type: "number",
                    description: "Base Z location for the blockout floor.",
                    default: 0,
                },
                width: {
                    type: "number",
                    description: "Overall blockout width in Unreal units (cm).",
                },
                depth: {
                    type: "number",
                    description: "Overall blockout depth in Unreal units (cm).",
                },
                height: {
                    type: "number",
                    description: "Wall height per level in Unreal units (cm).",
                },
                wall_thickness: {
                    type: "number",
                    description: "Wall thickness in Unreal units (cm).",
                    default: 40,
                },
                floor_thickness: {
                    type: "number",
                    description: "Floor slab thickness in Unreal units (cm).",
                    default: 20,
                },
                levels: {
                    type: "number",
                    description: "Number of levels for tower-style blockouts.",
                    default: 1,
                },
                open_sides: {
                    type: "array",
                    description: "Optional sides to leave open or turn into entry openings. Allowed values: north, south, east, west.",
                    items: { type: "string" },
                },
                opening_width: {
                    type: "number",
                    description: "Width of entry openings created on open sides, in Unreal units (cm).",
                },
                opening_height: {
                    type: "number",
                    description: "Height of entry openings created on open sides, in Unreal units (cm).",
                },
                add_stairs: {
                    type: "boolean",
                    description: "Add staircase geometry inside the blockout when supported by the template.",
                    default: false,
                },
                stair_style: {
                    type: "string",
                    description: "Choose between linear and curved stairs when stairs are enabled.",
                    enum: ["linear", "curved"],
                    default: "linear",
                },
                add_ramp: {
                    type: "boolean",
                    description: "Add a sloped ramp between levels or across the layout when appropriate.",
                    default: false,
                },
                add_arches: {
                    type: "boolean",
                    description: "Build entry openings as arch-like gateway compositions with split wall pieces and a top lintel.",
                    default: false,
                },
                add_columns: {
                    type: "boolean",
                    description: "Add support columns or corner pillars, useful for courtyards and arenas.",
                    default: false,
                },
                create_ceiling: {
                    type: "boolean",
                    description: "Force a ceiling slab for closed templates like rooms and hallways.",
                },
            },
        },
    },
    {
        name: "get_verification_status",
        description: "Get current verification status. May be VALID, DEGRADED (warning), or INVALIDATED (results meaningless due to GAMEPLAY_MUTATING commands).",
        inputSchema: {
            type: "object",
            properties: {},
            required: [],
        },
    },
    // ============= Composite Diagnostic Tools =============
    {
        name: "run_diagnostic",
        description: "Run a composite diagnostic tool. These are pre-packaged, verified workflows that capture structured data without gameplay-mutating side effects. Available: profile_performance, gas_debug, collision_debug, ai_debug, network_debug",
        inputSchema: {
            type: "object",
            properties: {
                name: {
                    type: "string",
                    description: "Diagnostic to run",
                    enum: ["profile_performance", "gas_debug", "collision_debug", "ai_debug", "network_debug"],
                },
            },
            required: ["name"],
        },
    },
    {
        name: "list_diagnostics",
        description: "List all available composite diagnostic tools with descriptions.",
        inputSchema: {
            type: "object",
            properties: {},
            required: [],
        },
    },
    // ======== Build Monitoring (Phase 1) ========
    {
        name: "get_build_events",
        description: "Get recent build events (started/succeeded/failed) with timestamps. Tracks live coding, lighting builds, and blueprint compilation.",
        inputSchema: {
            type: "object",
            properties: {
                max_count: { type: "number", description: "Max events to return", default: 20 },
                clear: { type: "boolean", description: "Clear events after reading", default: false },
            },
        },
    },
    {
        name: "get_build_errors",
        description: "Get structured compile errors from output log and HotReload. Returns file paths, line numbers, and messages.",
        inputSchema: {
            type: "object",
            properties: {
                max_count: { type: "number", description: "Max errors to return", default: 50 },
                source: { type: "string", description: "Error source filter: all, log, compile", default: "all" },
            },
        },
    },
    {
        name: "get_error_summary",
        description: "Compact error/warning count snapshot. Shows total errors, warnings, new errors since last check, build status, and compile status.",
        inputSchema: { type: "object", properties: {} },
    },
    {
        name: "get_errors_since",
        description: "Get errors/warnings that occurred after a given timestamp or since the last call. Delta-based error checking.",
        inputSchema: {
            type: "object",
            properties: {
                since_timestamp: { type: "number", description: "Only return errors after this timestamp (seconds). 0 = since last call.", default: 0 },
                severity: { type: "string", description: "Filter: all, errors, warnings", default: "all" },
                max_count: { type: "number", description: "Max entries to return", default: 50 },
            },
        },
    },
    {
        name: "wait_for_build",
        description: "Wait for an active build/compile to complete. Returns immediately if no build active, otherwise creates a latent polling job.",
        inputSchema: {
            type: "object",
            properties: {
                timeout_seconds: { type: "number", description: "Max wait time in seconds", default: 120 },
            },
        },
    },
    // ======== Modal Dismissal (Phase 2) ========
    {
        name: "dismiss_modal_dialog",
        description: "Dismiss a blocking modal dialog (Restore Packages, Crash Reporter, etc.) by clicking its dismiss button via Slate. Safe — uses normal event routing.",
        inputSchema: {
            type: "object",
            properties: {
                classification: { type: "string", description: "Dialog type: auto, restore_packages, crash_reporter, save_packages, blueprint_compile_errors, memory_pressure, generic", default: "auto" },
                force: { type: "boolean", description: "Also dismiss unclassified/generic modals", default: false },
            },
        },
    },
    // ======== Editor Control (Phase 3) ========
    {
        name: "set_viewport_mode",
        description: "Change viewport visualization mode: lit, unlit, wireframe, detail_lighting, lighting_only, path_tracing, collision, reflection.",
        inputSchema: {
            type: "object",
            properties: {
                mode: { type: "string", description: "Viewport mode" },
            },
            required: ["mode"],
        },
    },
    {
        name: "navigate_content_browser",
        description: "Open Content Browser to a specific folder or sync to a specific asset.",
        inputSchema: {
            type: "object",
            properties: {
                path: { type: "string", description: "Content folder path (e.g., '/Game/Blueprints')" },
                sync_to_asset: { type: "string", description: "Asset path to select and scroll to" },
            },
        },
    },
    {
        name: "set_editor_pref",
        description: "Set an editor preference via the config system. Restricted to allowed config sections for safety.",
        inputSchema: {
            type: "object",
            properties: {
                section: { type: "string", description: "Config section (e.g., '/Script/UnrealEd.LevelEditorPlaySettings')" },
                key: { type: "string", description: "Config key name" },
                value: { type: "string", description: "Value to set" },
            },
            required: ["section", "key", "value"],
        },
    },
    {
        name: "click_editor_button",
        description: "Find and click a button in the editor UI by its text label. Searches all non-modal top-level windows.",
        inputSchema: {
            type: "object",
            properties: {
                button_text: { type: "string", description: "Button label text to find and click" },
            },
            required: ["button_text"],
        },
    },
    // ======== Crash Diagnosis (bridge-independent) ========
    {
        name: "diagnose_crash",
        description: "Read UE crash logs from disk and return structured diagnosis. Works even when the editor has crashed (does NOT require the C++ bridge). Parses Saved/Crashes/ and Saved/Logs/ for exception type, stack trace, faulting frame, and repair guidance. Call this after any PIE crash, compile crash, or when the editor becomes unresponsive.",
        inputSchema: {
            type: "object",
            properties: {
                project_dir: { type: "string", description: "UE project root directory. Auto-detected if omitted." },
                max_age_seconds: { type: "number", description: "Only report crashes newer than this many seconds. 0 = any age.", default: 0 },
            },
        },
    },
    // ======== Tool Discovery (reduces token overhead) ========
    {
        name: "find_tools",
        description: "Search the tool registry by keyword. Returns matching tools with descriptions and parameter schemas sorted by relevance. " +
            "Use this to discover tools for a specific task instead of scanning the full tool list. " +
            "Searches tool names, descriptions, and parameter names.",
        inputSchema: {
            type: "object",
            properties: {
                query: {
                    type: "string",
                    description: "Search keywords (e.g., 'landscape sculpt', 'actor material', 'blueprint compile', 'foliage grass')",
                },
                max_results: {
                    type: "number",
                    description: "Maximum results to return (default 20)",
                    default: 20,
                },
            },
            required: ["query"],
        },
    },
    // ======== Workflow Discovery (common multi-step sequences) ========
    {
        name: "get_workflow",
        description: "Get an ordered multi-step build sequence for a common task. " +
            "Returns the exact tools to call, in order, with key parameters. " +
            "Available workflows: landscape, character, blueprint, material, lighting, forest, arena. " +
            "Use query='list' to see all available workflows.",
        inputSchema: {
            type: "object",
            properties: {
                query: {
                    type: "string",
                    description: "Workflow name (landscape, character, blueprint, material, lighting, forest, arena) or 'list' to see all.",
                },
            },
            required: ["query"],
        },
    },
    // ======== Batch Execution (reduces agent round-trips) ========
    {
        name: "batch_execute",
        description: `Execute multiple tools in sequence in a single round-trip. Each step runs only if the previous step succeeded (unless continue_on_error is true). Returns all results together. Use this to chain related operations like: create_landscape → create_landscape_material → apply_landscape_material.

Each step is an object with "tool" (tool name) and "args" (arguments object). Steps run in order. If a step fails and continue_on_error is false (default), execution stops and all completed results plus the error are returned.

Maximum 10 steps per batch to prevent runaway sequences.`,
        inputSchema: {
            type: "object",
            properties: {
                steps: {
                    type: "array",
                    description: "Ordered list of tool calls to execute sequentially.",
                    items: {
                        type: "object",
                        properties: {
                            tool: { type: "string", description: "Tool name to call" },
                            args: { type: "object", description: "Arguments for the tool" },
                        },
                        required: ["tool"],
                    },
                    minItems: 1,
                    maxItems: 10,
                },
                continue_on_error: {
                    type: "boolean",
                    description: "If true, continue executing remaining steps even if one fails. Default: false (stop on first error).",
                    default: false,
                },
            },
            required: ["steps"],
        },
    },
    // ======== Call Planner (dependency-aware workflow planning) ========
    {
        name: "plan_workflow",
        description: "Given a goal (keyword or tool name), compute the ordered prerequisite chain needed to reach it. " +
            "Checks session history to skip already-completed steps and pre-fills known parameters from context. " +
            "Returns executable steps compatible with batch_execute. " +
            "Use goal='list' to see all available goal keywords. " +
            "Examples: goal='grass' → create_landscape_grass_type → add_grass_variety; " +
            "goal='paint landscape' → create_landscape → add_landscape_layer → paint_landscape_layer.",
        inputSchema: {
            type: "object",
            properties: {
                goal: {
                    type: "string",
                    description: "Goal keyword (e.g., 'grass', 'blueprint', 'forest environment', 'playable character') " +
                        "or exact tool name (e.g., 'paint_landscape_layer'). Use 'list' to see all goals.",
                },
            },
            required: ["goal"],
        },
    },
    // ======== Autonomous Verification (Round 19) ========
    {
        name: "verify_session",
        description: "Auto-generate and run verification checks from what the agent built this session. " +
            "Reads the scene change log, context, and progress milestones to produce assert_* " +
            "calls (actor exists, blueprint compiles, no modal blockers, clean output log). " +
            "Returns structured PASS/FAIL/WARN per check. Use after building to confirm everything worked. " +
            "Scope: 'all' (default) runs every check; 'actors'/'materials'/'blueprints' filters by category.",
        inputSchema: {
            type: "object",
            properties: {
                scope: {
                    type: "string",
                    description: "Category filter: 'all' (default), 'actors', 'materials', 'blueprints', 'lighting', 'landscape'.",
                    default: "all",
                },
            },
        },
    },
    {
        name: "smoke_test_pie",
        description: "Structural Play-In-Editor smoke test. Pre-checks → start PIE → wait → runtime checks " +
            "(output log errors) → stop PIE → post-checks. Returns PASS/FAIL/CRASH with per-phase results. " +
            "Faster and more deterministic than vision_playtest — use this for quick structural validation.",
        inputSchema: {
            type: "object",
            properties: {
                duration_seconds: {
                    type: "number",
                    description: "How many seconds to leave PIE running (2–30). Default 3.",
                    default: 3,
                },
                max_errors: {
                    type: "number",
                    description: "Max errors allowed in output log. Default 0.",
                    default: 0,
                },
            },
        },
    },
    // ======== Session Bookmarks (Round 21) ========
    {
        name: "bookmark_session",
        description: "Create a named checkpoint in the session timeline. Use to mark milestones like 'terrain done' or 'before trees'. " +
            "Bookmarks let you scope undo/verify to 'changes since <bookmark>'. Overwrites if the name already exists.",
        inputSchema: {
            type: "object",
            properties: {
                name: {
                    type: "string",
                    description: "Short name for the bookmark (e.g., 'terrain done', 'lighting pass 1').",
                },
            },
            required: ["name"],
        },
    },
    {
        name: "list_bookmarks",
        description: "List all session bookmarks (named checkpoints) in chronological order. " +
            "Each bookmark records a change ID and timestamp. Use with undo/verify to scope by checkpoint.",
        inputSchema: { type: "object", properties: {} },
    },
    // ======== Pipeline Telemetry (Round 22) ========
    {
        name: "pipeline_stats",
        description: "Show pipeline telemetry: which features fire, which tools are called most, error rates, dead features. " +
            "Answers 'is the pipeline over-engineered?' with data. No arguments required.",
        inputSchema: { type: "object", properties: {} },
    },
    // ======== Pipeline Trace (Round 24) ========
    {
        name: "pipeline_trace",
        description: "Show per-call dispatch traces: which stages ran, decisions made, timing per stage. " +
            "Use tool filter for a specific tool, slow=true for slowest calls, or stage_stats=true for aggregate stage timing.",
        inputSchema: {
            type: "object",
            properties: {
                count: {
                    type: "number",
                    description: "Number of traces to return (1–200). Default 20.",
                },
                tool: {
                    type: "string",
                    description: "Filter traces to a specific tool name.",
                },
                slow: {
                    type: "boolean",
                    description: "If true, return the slowest traces instead of most recent.",
                },
                stage_stats: {
                    type: "boolean",
                    description: "If true, return per-stage aggregate timing instead of individual traces.",
                },
                hotspots: {
                    type: "boolean",
                    description: "If true, return bottleneck analytics (hottest tools/stages by latency and error profile).",
                },
                top: {
                    type: "number",
                    description: "Maximum hotspot rows to return (1-50). Default 10.",
                },
                min_calls: {
                    type: "number",
                    description: "Only include tools/stages observed at least this many times. Default 1.",
                },
                slow_threshold_ms: {
                    type: "number",
                    description: "Threshold used to classify traces as slow in hotspot reporting. Default 1500.",
                },
            },
        },
    },
    // ======== Dispatch Lifecycle / Rollback (Round 20) ========
    {
        name: "undo_last",
        description: "Undo the last N operations by executing their reverse (delete spawned actors, etc.). " +
            "Reads the scene change log and runs the undo plan. Returns per-step pass/fail. " +
            "Only reversible operations (creates → deletes) are undone; moves/modifies note that original values may be lost.",
        inputSchema: {
            type: "object",
            properties: {
                count: {
                    type: "number",
                    description: "How many recent operations to undo (1–20). Default 1.",
                    default: 1,
                },
            },
        },
    },
    {
        name: "rollback",
        description: "Undo all recent operations matching a filter. Use category to undo all actors, lights, etc. " +
            "Use label to undo operations on a specific actor. Returns per-step pass/fail. " +
            "Max 20 steps per rollback.",
        inputSchema: {
            type: "object",
            properties: {
                category: {
                    type: "string",
                    description: "Filter by scene category: actor, light, material, foliage, vfx, audio, landscape, postprocess, blueprint.",
                },
                label: {
                    type: "string",
                    description: "Filter by actor/asset label.",
                },
                count: {
                    type: "number",
                    description: "How many recent changes to search (1–200). Default 20.",
                    default: 20,
                },
            },
        },
    },
    ...SCENE_ENHANCEMENT_TOOLS,
    // ============= Teaching Mode & Reports =============
    {
        name: "explain_tool_execution",
        description: "Get a detailed explanation of what a tool does, its parameters, required inputs, and usage tips. Use this in teaching mode to help users understand the tool surface.",
        inputSchema: {
            type: "object",
            properties: {
                tool_name: { type: "string", description: "Name of the tool to explain (e.g., 'create_landscape', 'compile_blueprint')" },
            },
            required: ["tool_name"],
        },
    },
    {
        name: "generate_scene_report",
        description: "Generate a comprehensive scene quality report: observation + vision quality scores + performance metrics. Combines observe_ue_project, score_scene_quality, and get_performance_snapshot into one call.",
        inputSchema: {
            type: "object",
            properties: {
                categories: { type: "string", description: "Comma-separated scoring categories (default: lighting,composition,realism,contrast,color_grading)" },
            },
        },
    },
    // ============= Semantic Dependency Graph =============
    {
        name: "query_dependencies",
        description: "Query the asset dependency graph for a package. Returns what the package depends on and what depends on it. Uses the SemanticGraphDB to trace dependency chains.",
        inputSchema: {
            type: "object",
            properties: {
                package_name: { type: "string", description: "UE package path (e.g., /Game/Blueprints/BP_Enemy)" },
                max_depth: { type: "number", description: "Maximum traversal depth (default: 3, max: 20)" },
            },
            required: ["package_name"],
        },
    },
    {
        name: "blast_radius",
        description: "Calculate the blast radius of a breaking change to a package. Shows all packages that would be affected, at each depth level. Use before refactoring or deleting assets.",
        inputSchema: {
            type: "object",
            properties: {
                package_name: { type: "string", description: "Package to simulate breaking (e.g., /Game/Materials/M_Base)" },
                max_depth: { type: "number", description: "Maximum traversal depth (default: 5, max: 20)" },
            },
            required: ["package_name"],
        },
    },
];
export function buildAllTools() {
    const suppressStartupLogs = process.argv.includes("--self-test")
        || process.env.RIFTBORN_SUPPRESS_STARTUP_LOGS === "true";
    const manualToolNames = new Set(MANUAL_TOOLS.map((tool) => tool.name));
    const visionToolNames = new Set(VISION_LOOP_TOOLS.map((tool) => tool.name));
    const generatedOnlyTools = GENERATED_TOOLS.filter((tool) => !manualToolNames.has(tool.name) && !visionToolNames.has(tool.name));
    const mergedTools = [...MANUAL_TOOLS, ...VISION_LOOP_TOOLS, ...POLYHAVEN_TOOLS, ...generatedOnlyTools];
    const seenNames = new Set();
    let duplicateCount = 0;
    const uniqueTools = mergedTools.filter((tool) => {
        if (seenNames.has(tool.name)) {
            duplicateCount++;
            return false;
        }
        seenNames.add(tool.name);
        return true;
    });
    const visionCount = VISION_LOOP_TOOLS.length;
    if (!suppressStartupLogs) {
        console.error(`[RiftbornAI] Merged ${MANUAL_TOOLS.length} manual + ${visionCount} vision + ${generatedOnlyTools.length} generated = ${uniqueTools.length} unique tools` +
            (duplicateCount > 0 ? ` (${duplicateCount} duplicate definitions skipped)` : ""));
    }
    return uniqueTools;
}
export const MANUAL_TOOL_NAMES = new Set(MANUAL_TOOLS.map((tool) => tool.name));
export const CATEGORY_MAP = {
    investigate: "Project Investigation",
    get: "Inspection / Query",
    list: "Inspection / Query",
    create: "Asset Creation",
    delete: "Destructive",
    spawn: "Actor Spawning",
    edit: "File Editing",
    read: "File Reading",
    log: "Debugging",
    execute: "Execution",
    search: "Search",
    design: "Design System",
    diagnose: "Debugging",
    run: "Diagnostics",
    set: "Property Mutation",
    bookmark: "Session Management",
    pipeline: "Diagnostics",
    add: "Blueprint / Component",
    play: "Playback",
    stop: "Playback",
    build: "Build",
    open: "Editor",
    take: "Vision",
    analyze: "Vision",
    observe: "Vision",
    send: "Agent Communication",
    use: "Character",
    verify: "Verification",
    setup: "Orchestration",
    generate: "Code Generation",
    classify: "Classification",
    activate: "VFX",
    semantic: "Diagnostics",
    temporal: "Vision",
    reference: "Vision",
    photoreal: "Orchestration",
    perception: "Performance",
    multi: "Vision",
    grounding: "Orchestration",
    scene: "Diagnostics",
    hero: "Performance",
    lighting: "Vision",
    material: "Vision",
    quality: "Diagnostics",
    benchmark: "Diagnostics",
    self: "Self-Improvement",
};
//# sourceMappingURL=manual-tools.js.map