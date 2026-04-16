import { parseVisionText } from "./vision-intelligence.js";
const QUALITY_RANK = {
    poor: 0,
    fair: 1,
    good: 2,
    excellent: 3,
};
const ANALYSIS_KEYS = [
    "analysis",
    "vision_analysis",
    "temporal_analysis",
    "description",
    "scene_description",
];
const SCREENSHOT_KEYS = [
    "screenshot_path",
    "path",
    "image_path",
    "filename",
];
const TEMPORAL_PATTERNS = [
    { pattern: /\b(?:flicker|flickering|flash(?:ing)?)\b/i, severity: "high", description: "Temporal flicker detected" },
    { pattern: /\b(?:shimmer|shimmering|sparkle|specular aliasing)\b/i, severity: "high", description: "Shimmer or specular aliasing detected" },
    { pattern: /\b(?:pop[\s-]?in|popping|lod pop|streaming pop)\b/i, severity: "high", description: "Visible pop-in or LOD transitions detected" },
    { pattern: /\b(?:ghosting|smear(?:ing)?|trailing)\b/i, severity: "medium", description: "Ghosting or temporal smear detected" },
    { pattern: /\b(?:exposure pumping|auto[- ]exposure pumping|brightness pumping)\b/i, severity: "medium", description: "Exposure instability detected" },
    { pattern: /\b(?:timing issue|mistimed|out of sync|desync(?:hronized)?)\b/i, severity: "medium", description: "VFX or animation timing inconsistency detected" },
    { pattern: /\b(?:continuity issue|continuity break|jump cut|camera jump)\b/i, severity: "medium", description: "Temporal continuity issue detected" },
];
export const SCENE_ENHANCEMENT_TOOLS = [
    {
        name: "semantic_scene_audit",
        description: "Run a grounded scene audit by combining viewport vision with structured scene queries. " +
            "Captures the current scene, audits visible actors, optionally audits materials, optionally captures a depth grid, " +
            "and returns a consolidated quality summary with issues and recommendations.",
        inputSchema: {
            type: "object",
            properties: {
                prompt: {
                    type: "string",
                    description: "Optional custom audit prompt. Defaults to a semantic art-direction and scene-readability review.",
                },
                max_visible_actors: {
                    type: "number",
                    description: "Maximum visible actors to query from the active frustum. Default 40.",
                    default: 40,
                },
                include_materials: {
                    type: "boolean",
                    description: "Whether to include a material audit for visible scene content. Default true.",
                    default: true,
                },
                max_material_actors: {
                    type: "number",
                    description: "Maximum actors to include in the material audit. Default 40.",
                    default: 40,
                },
                include_depth: {
                    type: "boolean",
                    description: "Whether to capture a depth grid for occlusion and spacing cues. Default true.",
                    default: true,
                },
                depth_grid_size: {
                    type: "number",
                    description: "Depth capture grid size. Default 8.",
                    default: 8,
                },
            },
        },
    },
    {
        name: "temporal_quality_audit",
        description: "Capture a frame sequence and audit temporal quality issues such as flicker, shimmer, pop-in, ghosting, " +
            "exposure pumping, and timing continuity. Optionally runs the sequence during PIE.",
        inputSchema: {
            type: "object",
            properties: {
                duration_seconds: {
                    type: "number",
                    description: "Capture duration in seconds. Default 3.",
                    default: 3,
                },
                fps: {
                    type: "number",
                    description: "Capture FPS. Default 5.",
                    default: 5,
                },
                max_keyframes: {
                    type: "number",
                    description: "Maximum keyframes to retain for analysis. Default 4.",
                    default: 4,
                },
                play_mode: {
                    type: "boolean",
                    description: "If true, start PIE before capture and stop PIE after capture. Default false.",
                    default: false,
                },
                prompt: {
                    type: "string",
                    description: "Optional custom temporal analysis prompt.",
                },
            },
        },
    },
    {
        name: "reference_render_compare",
        description: "Capture paired lit and path-tracing reference renders, analyze both, and summarize the quality gap between them. " +
            "Useful for finding lighting, material, and atmosphere compromises in the realtime view.",
        inputSchema: {
            type: "object",
            properties: {
                width: {
                    type: "number",
                    description: "Capture width in pixels. Default 1920.",
                    default: 1920,
                },
                height: {
                    type: "number",
                    description: "Capture height in pixels. Default 1080.",
                    default: 1080,
                },
                restore_mode: {
                    type: "string",
                    description: "Viewport mode to restore when finished. Default lit.",
                    default: "lit",
                },
                prompt: {
                    type: "string",
                    description: "Optional custom comparison goal. Defaults to a realtime-vs-reference fidelity review.",
                },
            },
        },
    },
    {
        name: "photoreal_polish_pass",
        description: "Run a bounded photoreal polish workflow using existing RiftbornAI photorealism tools. " +
            "Can apply instance variation, advanced lighting realism, then verify the result with a semantic scene audit " +
            "and optional reference-render comparison.",
        inputSchema: {
            type: "object",
            properties: {
                material_path: {
                    type: "string",
                    description: "Material path to use for per-instance variation. Optional.",
                },
                foliage_type_filter: {
                    type: "string",
                    description: "Optional foliage type filter for per-instance variation.",
                },
                foliage_material_paths: {
                    type: "string",
                    description: "Comma-separated foliage material paths for advanced lighting realism.",
                },
                enable_instance_variation: {
                    type: "boolean",
                    description: "Whether to run setup_instance_variation when material_path is provided. Default true.",
                    default: true,
                },
                enable_advanced_lighting: {
                    type: "boolean",
                    description: "Whether to run setup_advanced_lighting_realism. Default true.",
                    default: true,
                },
                water_caustics_enabled: {
                    type: "boolean",
                    description: "Enable water caustic decals during advanced lighting realism. Default false.",
                    default: false,
                },
                volumetric_all_lights: {
                    type: "boolean",
                    description: "Enable volumetric scattering on all lights. Default true.",
                    default: true,
                },
                light_function_enabled: {
                    type: "boolean",
                    description: "Enable dappled shadow light function. Default true.",
                    default: true,
                },
                hue_shift_range: {
                    type: "number",
                    description: "Per-instance hue shift range. Default 0.08.",
                    default: 0.08,
                },
                roughness_variation: {
                    type: "number",
                    description: "Per-instance roughness variation. Default 0.12.",
                    default: 0.12,
                },
                age_factor_range: {
                    type: "number",
                    description: "Per-instance age/damage factor range. Default 0.35.",
                    default: 0.35,
                },
                run_reference_compare: {
                    type: "boolean",
                    description: "Run reference_render_compare after the polish pass. Default true.",
                    default: true,
                },
                width: {
                    type: "number",
                    description: "Optional reference compare width in pixels. Default 1600.",
                    default: 1600,
                },
                height: {
                    type: "number",
                    description: "Optional reference compare height in pixels. Default 900.",
                    default: 900,
                },
            },
        },
    },
    {
        name: "perception_budget_pass",
        description: "Analyze scene cost and visible content, optionally configure the adaptive visibility budgeter, " +
            "apply a performance budget, and report the resulting perception-vs-cost tradeoffs.",
        inputSchema: {
            type: "object",
            properties: {
                top_n: {
                    type: "number",
                    description: "How many expensive actors to surface. Default 10.",
                    default: 10,
                },
                max_visible_actors: {
                    type: "number",
                    description: "How many visible actors to sample from the current frustum. Default 25.",
                    default: 25,
                },
                preset: {
                    type: "string",
                    description: "Budget preset to apply: cinematic, balanced, aggressive. Default balanced.",
                    default: "balanced",
                },
                scope: {
                    type: "string",
                    description: "Actor scope for apply_performance_budget. Default all.",
                    default: "all",
                },
                max_actors: {
                    type: "number",
                    description: "Maximum actors touched by apply_performance_budget. Default 250.",
                    default: 250,
                },
                selected_only: {
                    type: "boolean",
                    description: "Only apply the budget to selected actors. Default false.",
                    default: false,
                },
                auto_configure: {
                    type: "boolean",
                    description: "Whether to configure the visibility budgeter before applying the preset. Default true.",
                    default: true,
                },
                enabled: {
                    type: "boolean",
                    description: "Enable the adaptive visibility budgeter. Default true.",
                    default: true,
                },
                evaluation_interval: {
                    type: "number",
                    description: "Visibility budget evaluation interval. Default 0.2.",
                    default: 0.2,
                },
                prediction_time: {
                    type: "number",
                    description: "Camera prediction prewarm time in seconds. Default 0.35.",
                    default: 0.35,
                },
                view_cone_buffer_degrees: {
                    type: "number",
                    description: "Additional warm buffer around the current FOV. Default 18.",
                    default: 18,
                },
                verify_visuals: {
                    type: "boolean",
                    description: "Run a follow-up semantic_scene_audit after the budget pass. Default true.",
                    default: true,
                },
            },
        },
    },
    {
        name: "multi_view_scene_audit",
        description: "Capture an orbit or sweep of 4-8 viewpoints, analyze each view, and return an aggregate scene audit " +
            "that is less sensitive to single-angle blind spots.",
        inputSchema: {
            type: "object",
            properties: {
                target_label: {
                    type: "string",
                    description: "Optional actor label to orbit around. If omitted, the first visible actor is used when available.",
                },
                target_x: {
                    type: "number",
                    description: "Optional target X for orbit captures when no target_label is provided.",
                },
                target_y: {
                    type: "number",
                    description: "Optional target Y for orbit captures when no target_label is provided.",
                },
                target_z: {
                    type: "number",
                    description: "Optional target Z for orbit captures when no target_label is provided.",
                },
                view_count: {
                    type: "number",
                    description: "How many views to capture. Default 4.",
                    default: 4,
                },
                distance: {
                    type: "number",
                    description: "Orbit camera distance in cm. Default 1800.",
                    default: 1800,
                },
                pitch: {
                    type: "number",
                    description: "Orbit camera pitch in degrees. Default -20.",
                    default: -20,
                },
                start_yaw: {
                    type: "number",
                    description: "Starting yaw angle for the first orbit view. Default 45.",
                    default: 45,
                },
                include_materials: {
                    type: "boolean",
                    description: "Whether to include a material audit. Default true.",
                    default: true,
                },
                max_visible_actors: {
                    type: "number",
                    description: "Maximum visible actors to sample when inferring a target. Default 24.",
                    default: 24,
                },
                max_material_actors: {
                    type: "number",
                    description: "Maximum actors to include in the material audit. Default 24.",
                    default: 24,
                },
                prompt: {
                    type: "string",
                    description: "Optional custom multi-view audit prompt.",
                },
            },
        },
    },
    {
        name: "semantic_repair_plan",
        description: "Convert the current scene audit into a bounded repair plan with prioritized tool calls, targets, rationale, " +
            "and rollback notes.",
        inputSchema: {
            type: "object",
            properties: {
                prompt: {
                    type: "string",
                    description: "Optional custom repair-planning prompt.",
                },
                max_steps: {
                    type: "number",
                    description: "Maximum repair steps to return. Default 6.",
                    default: 6,
                },
                include_reference_compare: {
                    type: "boolean",
                    description: "Whether to include a reference render comparison before planning. Default false.",
                    default: false,
                },
                target_label: {
                    type: "string",
                    description: "Optional actor label to prioritize in the audit and plan.",
                },
            },
        },
    },
    {
        name: "grounding_repair_pass",
        description: "Repair visible grounding issues by snapping movable actors to traced support surfaces and optionally re-grounding foliage. " +
            "Returns per-actor operations plus a follow-up placement audit.",
        inputSchema: {
            type: "object",
            properties: {
                target_labels: {
                    type: "string",
                    description: "Optional comma-separated actor labels to repair. Defaults to visible actors from the current frustum.",
                },
                max_visible_actors: {
                    type: "number",
                    description: "How many visible actors to inspect when inferring repair targets. Default 12.",
                    default: 12,
                },
                trace_start_offset: {
                    type: "number",
                    description: "Vertical offset above each actor before tracing downward. Default 500.",
                    default: 500,
                },
                trace_depth: {
                    type: "number",
                    description: "How far downward to trace. Default 5000.",
                    default: 5000,
                },
                surface_offset: {
                    type: "number",
                    description: "Optional vertical offset applied after grounding. Default 0.",
                    default: 0,
                },
                ground_foliage: {
                    type: "boolean",
                    description: "Whether to run ground_foliage_to_landscape. Default true.",
                    default: true,
                },
                support_mode: {
                    type: "string",
                    description: "Support policy for ground_foliage_to_landscape. Default auto.",
                    default: "auto",
                },
                verify: {
                    type: "boolean",
                    description: "Run a follow-up placement audit after the pass. Default true.",
                    default: true,
                },
            },
        },
    },
    {
        name: "temporal_camera_sweep_audit",
        description: "Run a deterministic camera sweep around a target, optionally augment it with a frame-sequence capture, " +
            "and summarize continuity, pop-in, shimmer, and related temporal risks.",
        inputSchema: {
            type: "object",
            properties: {
                target_label: {
                    type: "string",
                    description: "Optional actor label to sweep around. Defaults to the first visible actor when available.",
                },
                view_count: {
                    type: "number",
                    description: "How many sweep views to capture. Default 5.",
                    default: 5,
                },
                sweep_angle_degrees: {
                    type: "number",
                    description: "Total horizontal sweep arc in degrees. Default 120.",
                    default: 120,
                },
                distance: {
                    type: "number",
                    description: "Sweep distance from the target in cm. Default 1600.",
                    default: 1600,
                },
                pitch: {
                    type: "number",
                    description: "Sweep pitch in degrees. Default -18.",
                    default: -18,
                },
                play_mode: {
                    type: "boolean",
                    description: "Whether to start PIE before the sweep and stop PIE after it. Default false.",
                    default: false,
                },
                include_sequence_capture: {
                    type: "boolean",
                    description: "Whether to also run capture_frame_sequence for temporal evidence. Default true.",
                    default: true,
                },
                duration_seconds: {
                    type: "number",
                    description: "Frame-sequence duration when include_sequence_capture is enabled. Default 3.",
                    default: 3,
                },
                fps: {
                    type: "number",
                    description: "Frame-sequence FPS when include_sequence_capture is enabled. Default 5.",
                    default: 5,
                },
                prompt: {
                    type: "string",
                    description: "Optional custom temporal sweep prompt.",
                },
            },
        },
    },
    {
        name: "scene_regression_baseline",
        description: "Capture, compare, list, or delete session-scoped scene regression baselines composed of multi-view quality, " +
            "performance, world-state, and optional checkpoint/reference data.",
        inputSchema: {
            type: "object",
            properties: {
                action: {
                    type: "string",
                    description: "capture, compare, list, or delete. Default capture.",
                    default: "capture",
                },
                name: {
                    type: "string",
                    description: "Baseline name. Required for capture, compare, and delete.",
                },
                notes: {
                    type: "string",
                    description: "Optional note to attach to the captured baseline.",
                },
                view_count: {
                    type: "number",
                    description: "How many views to capture for the baseline. Default 4.",
                    default: 4,
                },
                include_reference_compare: {
                    type: "boolean",
                    description: "Capture a lit-vs-path-traced comparison in the baseline. Default false.",
                    default: false,
                },
                include_checkpoint: {
                    type: "boolean",
                    description: "Save a scene checkpoint during capture. Default true.",
                    default: true,
                },
                include_digest: {
                    type: "boolean",
                    description: "Capture a world-state digest during capture. Default true.",
                    default: true,
                },
            },
            required: ["action"],
        },
    },
    {
        name: "hero_content_protection",
        description: "Protect current hero content before optimization by assigning adaptive significance policies to selected or visible actors " +
            "and optionally reconfiguring the scene-wide visibility budgeter.",
        inputSchema: {
            type: "object",
            properties: {
                actor_labels: {
                    type: "string",
                    description: "Optional comma-separated actor labels to protect. Defaults to visible actors from the current frustum.",
                },
                max_visible_actors: {
                    type: "number",
                    description: "How many visible actors to sample when inferring hero content. Default 6.",
                    default: 6,
                },
                default_policy: {
                    type: "string",
                    description: "Policy for the primary protected actor. Default hero.",
                    default: "hero",
                },
                secondary_policy: {
                    type: "string",
                    description: "Policy for additional protected actors. Default gameplay_critical.",
                    default: "gameplay_critical",
                },
                preset: {
                    type: "string",
                    description: "Budget preset to associate with protected actors. Default balanced.",
                    default: "balanced",
                },
                configure_budget: {
                    type: "boolean",
                    description: "Whether to reconfigure the visibility budgeter after protection. Default true.",
                    default: true,
                },
                optimize_after_protection: {
                    type: "boolean",
                    description: "Whether to run optimize_open_world_scene after applying protection. Default false.",
                    default: false,
                },
            },
        },
    },
    {
        name: "lighting_consistency_audit",
        description: "Audit lighting consistency across multiple viewpoints and optional path-traced reference comparison. " +
            "Focuses on exposure, shadow depth, fog integration, and color-temperature coherence.",
        inputSchema: {
            type: "object",
            properties: {
                target_label: {
                    type: "string",
                    description: "Optional orbit target label. Defaults to the first visible actor when available.",
                },
                view_count: {
                    type: "number",
                    description: "How many lighting audit views to capture. Default 4.",
                    default: 4,
                },
                distance: {
                    type: "number",
                    description: "Orbit distance in cm. Default 1800.",
                    default: 1800,
                },
                pitch: {
                    type: "number",
                    description: "Orbit pitch in degrees. Default -18.",
                    default: -18,
                },
                include_reference_compare: {
                    type: "boolean",
                    description: "Whether to include a path-traced reference comparison. Default true.",
                    default: true,
                },
                prompt: {
                    type: "string",
                    description: "Optional custom lighting audit prompt.",
                },
            },
        },
    },
    {
        name: "material_plausibility_audit",
        description: "Audit visible materials for plausibility by combining material_audit with focused visual captures around sampled hero actors.",
        inputSchema: {
            type: "object",
            properties: {
                actor_labels: {
                    type: "string",
                    description: "Optional comma-separated actor labels to inspect. Defaults to material-audited visible actors.",
                },
                focus_count: {
                    type: "number",
                    description: "How many actors to inspect visually. Default 3.",
                    default: 3,
                },
                distance: {
                    type: "number",
                    description: "Camera distance for focused captures in cm. Default 1200.",
                    default: 1200,
                },
                pitch: {
                    type: "number",
                    description: "Camera pitch for focused captures. Default -15.",
                    default: -15,
                },
                max_material_actors: {
                    type: "number",
                    description: "Maximum actors to include in material_audit. Default 12.",
                    default: 12,
                },
                include_reference_compare: {
                    type: "boolean",
                    description: "Whether to include a path-traced reference comparison. Default false.",
                    default: false,
                },
                prompt: {
                    type: "string",
                    description: "Optional custom material audit prompt.",
                },
            },
        },
    },
    {
        name: "quality_gate",
        description: "Run a non-mutating ship/block gate by combining multi-view semantic checks, temporal sweep checks, optional reference comparison, " +
            "performance signals, and optional baseline regression comparison.",
        inputSchema: {
            type: "object",
            properties: {
                target_label: {
                    type: "string",
                    description: "Optional actor label to center the gate's view captures around.",
                },
                baseline_name: {
                    type: "string",
                    description: "Optional baseline name to compare against during the gate.",
                },
                min_fps: {
                    type: "number",
                    description: "Minimum acceptable FPS before the gate fails. Default 30.",
                    default: 30,
                },
                include_reference_compare: {
                    type: "boolean",
                    description: "Whether to include reference_render_compare. Default true.",
                    default: true,
                },
                include_temporal_sweep: {
                    type: "boolean",
                    description: "Whether to include temporal_camera_sweep_audit. Default true.",
                    default: true,
                },
                play_mode: {
                    type: "boolean",
                    description: "Run temporal sweep in PIE. Default false.",
                    default: false,
                },
            },
        },
    },
    {
        name: "benchmark_pack",
        description: "Run an aggregated benchmark pack that bundles the quality gate, optional quick playtest, performance snapshot, and optional engine benchmark output.",
        inputSchema: {
            type: "object",
            properties: {
                baseline_name: {
                    type: "string",
                    description: "Optional baseline name to feed into quality_gate.",
                },
                run_engine_benchmark: {
                    type: "boolean",
                    description: "Whether to run the engine benchmark suite. Default true.",
                    default: true,
                },
                include_quality_gate: {
                    type: "boolean",
                    description: "Whether to run quality_gate. Default true.",
                    default: true,
                },
                include_quick_playtest: {
                    type: "boolean",
                    description: "Whether to run run_quick_playtest. Default true.",
                    default: true,
                },
                playtest_duration: {
                    type: "number",
                    description: "Quick playtest duration in seconds. Default 3.",
                    default: 3,
                },
                play_mode: {
                    type: "boolean",
                    description: "Pass play_mode through to the quality gate's temporal sweep. Default false.",
                    default: false,
                },
            },
        },
    },
];
function safeJsonParse(value) {
    if (typeof value !== "string") {
        return value;
    }
    try {
        return JSON.parse(value);
    }
    catch {
        return value;
    }
}
function toRecord(value) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
        return {};
    }
    return value;
}
function unwrapResult(response) {
    return safeJsonParse(response.result);
}
function findString(value, keys, depth = 0) {
    if (depth > 4 || value == null) {
        return undefined;
    }
    if (typeof value === "string") {
        return undefined;
    }
    if (Array.isArray(value)) {
        for (const entry of value) {
            const nested = findString(entry, keys, depth + 1);
            if (nested) {
                return nested;
            }
        }
        return undefined;
    }
    if (typeof value === "object") {
        const record = value;
        for (const key of keys) {
            const candidate = record[key];
            if (typeof candidate === "string" && candidate.length > 0) {
                return candidate;
            }
        }
        for (const nestedValue of Object.values(record)) {
            const nested = findString(nestedValue, keys, depth + 1);
            if (nested) {
                return nested;
            }
        }
    }
    return undefined;
}
function extractAnalysisText(data) {
    return findString(data, ANALYSIS_KEYS) ?? "";
}
function extractScreenshotPath(data) {
    return findString(data, SCREENSHOT_KEYS);
}
function uniqueIssues(issues) {
    const seen = new Set();
    const deduped = [];
    for (const issue of issues) {
        const key = `${issue.category}:${issue.severity}:${issue.description}`;
        if (seen.has(key)) {
            continue;
        }
        seen.add(key);
        deduped.push(issue);
    }
    return deduped;
}
function buildIssueRecommendations(issues) {
    const recommendations = [];
    for (const issue of issues) {
        switch (issue.category) {
            case "lighting":
                recommendations.push("Revisit direct light contrast, shadow depth, fog integration, and post exposure.");
                break;
            case "material":
                recommendations.push("Audit material slots, roughness balance, and texture fidelity on visible hero assets.");
                break;
            case "placement":
                recommendations.push("Correct actor grounding, clipping, and support-surface contact in the active camera view.");
                break;
            case "scale":
                recommendations.push("Normalize visible asset scale against the surrounding scene and camera read.");
                break;
            case "composition":
                recommendations.push("Reduce clutter or introduce clearer focal hierarchy in the current composition.");
                break;
            case "missing_content":
                recommendations.push("Fill barren screen regions with support props, foliage, or atmosphere layers that preserve the scene theme.");
                break;
            case "performance":
                recommendations.push("Use the adaptive visibility budget and shadow scheduling before removing player-facing content.");
                break;
        }
    }
    return [...new Set(recommendations)];
}
function parseTemporalIssues(text) {
    const issues = [];
    const seen = new Set();
    for (const pattern of TEMPORAL_PATTERNS) {
        if (pattern.pattern.test(text) && !seen.has(pattern.description)) {
            seen.add(pattern.description);
            issues.push({ severity: pattern.severity, description: pattern.description });
        }
    }
    return issues;
}
function summarizeVisibleActors(data, maxItems = 8) {
    const record = toRecord(data);
    const candidates = record.actors
        ?? record.visible_actors
        ?? record.results
        ?? record.items;
    if (!Array.isArray(candidates)) {
        return [];
    }
    return candidates
        .slice(0, maxItems)
        .map((entry) => toRecord(safeJsonParse(entry)))
        .filter((entry) => Object.keys(entry).length > 0);
}
function parseCsvList(value) {
    if (typeof value !== "string") {
        return [];
    }
    return value
        .split(",")
        .map((entry) => entry.trim())
        .filter((entry) => entry.length > 0);
}
function toFiniteNumber(value) {
    if (typeof value === "number" && Number.isFinite(value)) {
        return value;
    }
    if (typeof value === "string" && value.trim().length > 0) {
        const parsed = Number(value);
        if (Number.isFinite(parsed)) {
            return parsed;
        }
    }
    return undefined;
}
function readVector(value) {
    const record = toRecord(value);
    const x = toFiniteNumber(record.x);
    const y = toFiniteNumber(record.y);
    const z = toFiniteNumber(record.z);
    if (x == null || y == null || z == null) {
        return undefined;
    }
    return { x, y, z };
}
function extractActorCandidates(data, maxItems = 12) {
    return summarizeVisibleActors(data, maxItems)
        .map((entry) => {
        const location = readVector(entry.location)
            ?? readVector(entry.world_location)
            ?? (() => {
                const x = toFiniteNumber(entry.x);
                const y = toFiniteNumber(entry.y);
                const z = toFiniteNumber(entry.z);
                return x != null && y != null && z != null ? { x, y, z } : undefined;
            })();
        const label = typeof entry.label === "string"
            ? entry.label
            : typeof entry.name === "string"
                ? entry.name
                : typeof entry.actor_label === "string"
                    ? entry.actor_label
                    : undefined;
        const class_name = typeof entry.class === "string"
            ? entry.class
            : typeof entry.class_name === "string"
                ? entry.class_name
                : typeof entry.type === "string"
                    ? entry.type
                    : undefined;
        return {
            label,
            class_name,
            location,
            raw: entry,
        };
    })
        .filter((entry) => entry.label || entry.location);
}
function parseQuality(value) {
    return value === "poor" || value === "fair" || value === "good" || value === "excellent"
        ? value
        : "fair";
}
function worstQuality(values) {
    if (values.length === 0) {
        return "fair";
    }
    return values.reduce((worst, current) => QUALITY_RANK[current] < QUALITY_RANK[worst] ? current : worst);
}
function buildOrbitAngles(viewCount, startYaw) {
    const count = Math.max(1, Math.min(8, Math.trunc(viewCount)));
    if (count === 1) {
        return [startYaw];
    }
    const step = 360 / count;
    return Array.from({ length: count }, (_unused, index) => startYaw + step * index);
}
function buildSweepAngles(viewCount, sweepAngle) {
    const count = Math.max(1, Math.min(8, Math.trunc(viewCount)));
    if (count === 1) {
        return [0];
    }
    const half = sweepAngle / 2;
    const step = sweepAngle / (count - 1);
    return Array.from({ length: count }, (_unused, index) => -half + step * index);
}
function buildViewAudit(views) {
    const issues = uniqueIssues(views.flatMap((view) => view.issues));
    return {
        quality: worstQuality(views.map((view) => view.quality)),
        issues,
        recommendations: buildIssueRecommendations(issues),
    };
}
function parseViewportPose(data) {
    const record = toRecord(data);
    const position = readVector(record.location)
        ?? readVector(record.position)
        ?? (() => {
            const x = toFiniteNumber(record.x);
            const y = toFiniteNumber(record.y);
            const z = toFiniteNumber(record.z);
            return x != null && y != null && z != null ? { x, y, z } : undefined;
        })();
    const rotationRecord = toRecord(record.rotation);
    const pitch = toFiniteNumber(rotationRecord.pitch) ?? toFiniteNumber(record.pitch);
    const yaw = toFiniteNumber(rotationRecord.yaw) ?? toFiniteNumber(record.yaw);
    const roll = toFiniteNumber(rotationRecord.roll) ?? toFiniteNumber(record.roll) ?? 0;
    const fov = toFiniteNumber(record.fov);
    if (!position || pitch == null || yaw == null) {
        return null;
    }
    return {
        position,
        rotation: { pitch, yaw, roll },
        fov,
    };
}
function extractHitLocation(data) {
    const record = toRecord(data);
    return readVector(record.impact_point)
        ?? readVector(record.hit_location)
        ?? readVector(record.location)
        ?? readVector(record.hit)
        ?? (() => {
            const x = toFiniteNumber(record.x);
            const y = toFiniteNumber(record.y);
            const z = toFiniteNumber(record.z);
            return x != null && y != null && z != null ? { x, y, z } : undefined;
        })();
}
function shouldSkipGrounding(candidate) {
    const className = (candidate.class_name ?? "").toLowerCase();
    return className.includes("light")
        || className.includes("camera")
        || className.includes("volume")
        || className.includes("fog")
        || className.includes("postprocess");
}
function buildTemporalRecommendations(issues) {
    const recommendations = [];
    for (const issue of issues) {
        if (issue.description.includes("Shimmer")) {
            recommendations.push("Audit temporal AA, specular response, and foliage material stability along the sweep.");
        }
        else if (issue.description.includes("pop-in")) {
            recommendations.push("Review LOD thresholds, streaming residency, and visibility prewarm around the sweep path.");
        }
        else if (issue.description.includes("Ghosting")) {
            recommendations.push("Inspect motion-vector quality, translucent materials, and temporal accumulation settings.");
        }
        else if (issue.description.includes("Exposure")) {
            recommendations.push("Stabilize auto exposure and rebalance key/fill lighting between adjacent views.");
        }
        else if (issue.description.includes("timing")) {
            recommendations.push("Revisit VFX or animation timing relative to camera travel and gameplay beats.");
        }
        else if (issue.description.includes("continuity")) {
            recommendations.push("Preserve silhouettes, composition, and actor visibility across adjacent sweep frames.");
        }
        else if (issue.description.includes("flicker")) {
            recommendations.push("Eliminate flicker sources in lighting, emissives, and shadow cascades before shipping.");
        }
    }
    return [...new Set(recommendations)];
}
function extractPlainText(data) {
    if (typeof data === "string") {
        return data;
    }
    const analysis = extractAnalysisText(data);
    if (analysis) {
        return analysis;
    }
    return JSON.stringify(data);
}
function parsePerformanceSummary(data) {
    const text = extractPlainText(data);
    const concerns = [];
    const fpsMatch = text.match(/\bFPS\b[^0-9]*([0-9]+(?:\.[0-9]+)?)/i) ?? text.match(/\b([0-9]+(?:\.[0-9]+)?)\s*fps\b/i);
    const fps = fpsMatch ? Number(fpsMatch[1]) : undefined;
    if (fps != null && fps < 30) {
        concerns.push("FPS is below 30.");
    }
    else if (fps != null && fps < 45) {
        concerns.push("FPS is below the preferred 45 FPS comfort threshold.");
    }
    if (/\bover[- ]budget\b|\btexture pool over\b|\blow fps\b|\bstutter\b|\blatency spike\b/i.test(text)) {
        concerns.push("Performance snapshot reports budget or latency pressure.");
    }
    return {
        text,
        fps,
        concerns: [...new Set(concerns)],
    };
}
function compareRegressionSnapshots(baseline, current) {
    const baselineAudit = toRecord(baseline.multi_view).audit;
    const currentAudit = toRecord(current.multi_view).audit;
    const baselineQuality = parseQuality(toRecord(baselineAudit).quality);
    const currentQuality = parseQuality(toRecord(currentAudit).quality);
    const baselineIssues = uniqueIssues(toRecord(baselineAudit).issues ?? []);
    const currentIssues = uniqueIssues(toRecord(currentAudit).issues ?? []);
    const baselineKeys = new Set(baselineIssues.map((issue) => `${issue.category}:${issue.description}`));
    const currentKeys = new Set(currentIssues.map((issue) => `${issue.category}:${issue.description}`));
    const newIssues = currentIssues.filter((issue) => !baselineKeys.has(`${issue.category}:${issue.description}`));
    const resolvedIssues = baselineIssues.filter((issue) => !currentKeys.has(`${issue.category}:${issue.description}`));
    const baselineFps = baseline.performance_summary?.fps;
    const currentFps = current.performance_summary?.fps;
    const fpsDelta = baselineFps != null && currentFps != null ? currentFps - baselineFps : undefined;
    const baselineWorldHash = typeof toRecord(baseline.world_state).hash === "string" ? toRecord(baseline.world_state).hash : undefined;
    const currentWorldHash = typeof toRecord(current.world_state).hash === "string" ? toRecord(current.world_state).hash : undefined;
    const worldStateChanged = baselineWorldHash != null && currentWorldHash != null
        ? baselineWorldHash !== currentWorldHash
        : undefined;
    let status = "stable";
    if (QUALITY_RANK[currentQuality] < QUALITY_RANK[baselineQuality] || newIssues.length > resolvedIssues.length || (fpsDelta != null && fpsDelta < -5)) {
        status = "regressed";
    }
    else if (QUALITY_RANK[currentQuality] > QUALITY_RANK[baselineQuality] || resolvedIssues.length > newIssues.length || (fpsDelta != null && fpsDelta > 5)) {
        status = "improved";
    }
    return {
        status,
        quality_delta: QUALITY_RANK[currentQuality] - QUALITY_RANK[baselineQuality],
        baseline_quality: baselineQuality,
        current_quality: currentQuality,
        new_issues: newIssues,
        resolved_issues: resolvedIssues,
        fps_delta: fpsDelta,
        world_state_changed: worldStateChanged,
    };
}
async function requireTool(executeTool, toolName, params) {
    const response = await executeTool(toolName, params);
    if (!response.ok) {
        throw new Error(`${toolName} failed: ${response.error ?? "unknown error"}`);
    }
    return { response, data: unwrapResult(response) };
}
async function optionalTool(executeTool, toolName, params, warnings) {
    const response = await executeTool(toolName, params);
    if (!response.ok) {
        warnings.push(`${toolName}: ${response.error ?? "unknown error"}`);
        return null;
    }
    return { response, data: unwrapResult(response) };
}
async function inferTargetContext(executeTool, args, warnings, maxVisibleActors = 24) {
    const targetLabel = typeof args.target_label === "string" && args.target_label.trim().length > 0
        ? args.target_label.trim()
        : undefined;
    if (targetLabel) {
        return { targetLabel, candidates: [] };
    }
    const targetX = toFiniteNumber(args.target_x);
    const targetY = toFiniteNumber(args.target_y);
    const targetZ = toFiniteNumber(args.target_z);
    if (targetX != null && targetY != null && targetZ != null) {
        return {
            targetPoint: { x: targetX, y: targetY, z: targetZ },
            candidates: [],
        };
    }
    const frustum = await optionalTool(executeTool, "frustum_query", {
        max_results: maxVisibleActors,
    }, warnings);
    const candidates = extractActorCandidates(frustum?.data, maxVisibleActors);
    const firstLabeled = candidates.find((entry) => entry.label);
    return {
        targetLabel: firstLabeled?.label,
        frustum: frustum?.data,
        candidates,
    };
}
async function captureOrbitViews(executeTool, args, warnings) {
    const views = [];
    if (args.targetLabel || args.targetPoint) {
        for (const [index, yaw] of args.yawAngles.entries()) {
            const capture = await requireTool(executeTool, "look_at_and_capture", {
                target_label: args.targetLabel,
                target_x: args.targetPoint?.x,
                target_y: args.targetPoint?.y,
                target_z: args.targetPoint?.z,
                distance: args.distance,
                yaw,
                pitch: args.pitch,
                analyze: true,
                prompt: `${args.prompt} View ${index + 1} of ${args.yawAngles.length}.`,
                filename: `scene_enhancement_view_${Date.now()}_${index + 1}`,
            });
            const analysis = extractAnalysisText(capture.data);
            const parsed = parseVisionText(analysis);
            views.push({
                view_id: `view_${index + 1}`,
                yaw,
                pitch: args.pitch,
                analysis,
                issues: uniqueIssues(parsed.issues),
                quality: parseQuality(parsed.quality),
                screenshot_path: extractScreenshotPath(capture.data),
                capture: capture.data,
            });
        }
        return views;
    }
    const viewport = await optionalTool(executeTool, "get_viewport_info", {}, warnings);
    const pose = parseViewportPose(viewport?.data);
    if (!pose) {
        warnings.push("Could not infer an orbit target or current viewport pose for multi-view capture.");
        return views;
    }
    for (const [index, yawOffset] of args.yawAngles.entries()) {
        const move = await executeTool("set_viewport_location", {
            x: pose.position.x,
            y: pose.position.y,
            z: pose.position.z,
            pitch: pose.rotation.pitch,
            yaw: pose.rotation.yaw + yawOffset,
            roll: pose.rotation.roll,
            fov: pose.fov,
        });
        if (!move.ok) {
            warnings.push(`set_viewport_location(view ${index + 1}): ${move.error ?? "unknown error"}`);
            continue;
        }
        const capture = await requireTool(executeTool, "capture_at_resolution", {
            width: args.width ?? 1600,
            height: args.height ?? 900,
            analyze: true,
            prompt: `${args.prompt} View ${index + 1} of ${args.yawAngles.length}.`,
            filename: `scene_enhancement_viewport_${Date.now()}_${index + 1}`,
        });
        const analysis = extractAnalysisText(capture.data);
        const parsed = parseVisionText(analysis);
        views.push({
            view_id: `view_${index + 1}`,
            yaw: pose.rotation.yaw + yawOffset,
            pitch: pose.rotation.pitch,
            analysis,
            issues: uniqueIssues(parsed.issues),
            quality: parseQuality(parsed.quality),
            screenshot_path: extractScreenshotPath(capture.data),
            capture: capture.data,
        });
    }
    return views;
}
async function runSemanticSceneAudit(executeTool, args) {
    const warnings = [];
    const prompt = typeof args.prompt === "string" && args.prompt.trim().length > 0
        ? args.prompt
        : "Audit this Unreal Engine scene semantically. Evaluate lighting readability, material plausibility, " +
            "spatial grounding, composition, density, and atmosphere. Call out concrete visual problems.";
    const maxVisibleActors = Math.max(1, Number(args.max_visible_actors) || 40);
    const maxMaterialActors = Math.max(1, Number(args.max_material_actors) || 40);
    const includeMaterials = args.include_materials !== false;
    const includeDepth = args.include_depth !== false;
    const depthGridSize = Math.max(2, Number(args.depth_grid_size) || 8);
    const observation = await requireTool(executeTool, "observe_ue_project", {
        capture_screenshot: true,
        include_vision: true,
        prompt,
    });
    const frustum = await requireTool(executeTool, "frustum_query", {
        max_results: maxVisibleActors,
    });
    const materialAudit = includeMaterials
        ? await optionalTool(executeTool, "material_audit", {
            max_actors: maxMaterialActors,
            include_textures: true,
        }, warnings)
        : null;
    const depth = includeDepth
        ? await optionalTool(executeTool, "capture_depth_buffer", {
            grid_size: depthGridSize,
        }, warnings)
        : null;
    const analysisText = extractAnalysisText(observation.data);
    const parsedVision = parseVisionText(analysisText);
    const recommendations = buildIssueRecommendations(parsedVision.issues);
    return {
        ok: true,
        result: {
            prompt,
            screenshot_path: extractScreenshotPath(observation.data),
            audit: {
                quality: parsedVision.quality,
                issues: uniqueIssues(parsedVision.issues),
                recommendations,
            },
            visible_actors: {
                count: summarizeVisibleActors(frustum.data, maxVisibleActors).length,
                sample: summarizeVisibleActors(frustum.data, 8),
            },
            observation: observation.data,
            frustum: frustum.data,
            material_audit: materialAudit?.data,
            depth: depth?.data,
            warnings,
        },
    };
}
async function runTemporalQualityAudit(executeTool, args) {
    const playMode = args.play_mode === true;
    const warnings = [];
    let pieStarted = false;
    try {
        if (playMode) {
            const startPie = await executeTool("start_pie", {
                mode: "viewport",
                num_players: 1,
            });
            if (!startPie.ok) {
                return { ok: false, error: `start_pie failed: ${startPie.error ?? "unknown error"}` };
            }
            pieStarted = true;
        }
        const capture = await requireTool(executeTool, "capture_frame_sequence", {
            duration_seconds: Number(args.duration_seconds) || 3,
            fps: Number(args.fps) || 5,
            max_keyframes: Number(args.max_keyframes) || 4,
            analyze: true,
            prompt: typeof args.prompt === "string" ? args.prompt : undefined,
        });
        const analysisText = extractAnalysisText(capture.data);
        const temporalIssues = parseTemporalIssues(analysisText);
        return {
            ok: true,
            result: {
                play_mode: playMode,
                capture: capture.data,
                temporal_analysis: analysisText,
                issues: temporalIssues,
                warnings,
            },
        };
    }
    finally {
        if (pieStarted) {
            const stopPie = await executeTool("stop_pie", {});
            if (!stopPie.ok) {
                warnings.push(`stop_pie: ${stopPie.error ?? "unknown error"}`);
            }
        }
    }
}
function buildReferenceDelta(litIssues, referenceIssues) {
    const sharedDescriptions = new Set(referenceIssues.map((issue) => `${issue.category}:${issue.description}`));
    const litOnly = litIssues.filter((issue) => !sharedDescriptions.has(`${issue.category}:${issue.description}`));
    const shared = litIssues.filter((issue) => sharedDescriptions.has(`${issue.category}:${issue.description}`));
    const recommendations = buildIssueRecommendations(litOnly.length > 0 ? litOnly : litIssues)
        .map((entry) => `${entry} Use the path-traced capture as the fidelity target.`);
    return {
        lit_only_issues: uniqueIssues(litOnly),
        shared_issues: uniqueIssues(shared),
        recommendations: [...new Set(recommendations)],
    };
}
async function runReferenceRenderCompare(executeTool, args) {
    const width = Math.max(128, Number(args.width) || 1920);
    const height = Math.max(128, Number(args.height) || 1080);
    const restoreMode = typeof args.restore_mode === "string" && args.restore_mode.trim().length > 0
        ? args.restore_mode
        : "lit";
    const goal = typeof args.prompt === "string" && args.prompt.trim().length > 0
        ? args.prompt
        : "Compare the realtime lit view to the path-traced reference and identify the biggest fidelity gaps.";
    const warnings = [];
    let litCapture;
    let referenceCapture;
    try {
        const setLit = await executeTool("set_viewport_mode", { mode: "lit" });
        if (!setLit.ok) {
            return { ok: false, error: `set_viewport_mode(lit) failed: ${setLit.error ?? "unknown error"}` };
        }
        litCapture = (await requireTool(executeTool, "capture_at_resolution", {
            width,
            height,
            analyze: true,
            prompt: `Realtime lit pass. ${goal} Focus on lighting, materials, atmosphere, and any compromises.`,
            filename: `reference_compare_lit_${Date.now()}`,
        })).data;
        const setPathTracing = await executeTool("set_viewport_mode", { mode: "path_tracing" });
        if (!setPathTracing.ok) {
            return { ok: false, error: `set_viewport_mode(path_tracing) failed: ${setPathTracing.error ?? "unknown error"}` };
        }
        referenceCapture = (await requireTool(executeTool, "capture_at_resolution", {
            width,
            height,
            analyze: true,
            prompt: `Path-traced reference pass. ${goal} Treat this as the higher-fidelity target and identify standout strengths.`,
            filename: `reference_compare_path_tracing_${Date.now()}`,
        })).data;
    }
    finally {
        const restore = await executeTool("set_viewport_mode", { mode: restoreMode });
        if (!restore.ok) {
            warnings.push(`restore viewport mode '${restoreMode}': ${restore.error ?? "unknown error"}`);
        }
    }
    if (!litCapture || !referenceCapture) {
        return { ok: false, error: "reference_render_compare did not capture both lit and path-traced renders" };
    }
    const litText = extractAnalysisText(litCapture);
    const referenceText = extractAnalysisText(referenceCapture);
    const litIssues = parseVisionText(litText).issues;
    const referenceIssues = parseVisionText(referenceText).issues;
    const delta = buildReferenceDelta(litIssues, referenceIssues);
    return {
        ok: true,
        result: {
            goal,
            width,
            height,
            restore_mode: restoreMode,
            lit: {
                screenshot_path: extractScreenshotPath(litCapture),
                analysis: litText,
                issues: uniqueIssues(litIssues),
                capture: litCapture,
            },
            reference: {
                screenshot_path: extractScreenshotPath(referenceCapture),
                analysis: referenceText,
                issues: uniqueIssues(referenceIssues),
                capture: referenceCapture,
            },
            delta,
            warnings,
        },
    };
}
async function runPhotorealPolishPass(executeTool, args) {
    const warnings = [];
    const operations = [];
    const materialPath = typeof args.material_path === "string" ? args.material_path : "";
    const enableInstanceVariation = args.enable_instance_variation !== false;
    const enableAdvancedLighting = args.enable_advanced_lighting !== false;
    if (enableInstanceVariation) {
        if (!materialPath) {
            warnings.push("setup_instance_variation skipped because no material_path was provided.");
        }
        else {
            const instanceVariation = await executeTool("setup_instance_variation", {
                material_path: materialPath,
                foliage_type_filter: typeof args.foliage_type_filter === "string" ? args.foliage_type_filter : "",
                hue_shift_range: Number(args.hue_shift_range) || 0.08,
                roughness_variation: Number(args.roughness_variation) || 0.12,
                age_factor_range: Number(args.age_factor_range) || 0.35,
                modify_material: true,
            });
            if (!instanceVariation.ok) {
                warnings.push(`setup_instance_variation: ${instanceVariation.error ?? "unknown error"}`);
            }
            else {
                operations.push({
                    tool: "setup_instance_variation",
                    result: unwrapResult(instanceVariation),
                });
            }
        }
    }
    if (enableAdvancedLighting) {
        const advancedLighting = await executeTool("setup_advanced_lighting_realism", {
            foliage_material_paths: typeof args.foliage_material_paths === "string" ? args.foliage_material_paths : "",
            light_function_enabled: args.light_function_enabled !== false,
            volumetric_all_lights: args.volumetric_all_lights !== false,
            water_caustics_enabled: args.water_caustics_enabled === true,
        });
        if (!advancedLighting.ok) {
            warnings.push(`setup_advanced_lighting_realism: ${advancedLighting.error ?? "unknown error"}`);
        }
        else {
            operations.push({
                tool: "setup_advanced_lighting_realism",
                result: unwrapResult(advancedLighting),
            });
        }
    }
    const audit = await runSemanticSceneAudit(executeTool, {
        prompt: "Audit the scene after the photoreal polish pass. Focus on lighting depth, material plausibility, " +
            "foliage variation, and atmosphere integration.",
        include_materials: true,
        include_depth: false,
        max_visible_actors: 32,
        max_material_actors: 32,
    });
    const referenceCompare = args.run_reference_compare !== false
        ? await runReferenceRenderCompare(executeTool, {
            width: Number(args.width) || 1600,
            height: Number(args.height) || 900,
            restore_mode: "lit",
            prompt: "Use the path-traced render as a photoreal benchmark for this polish pass.",
        })
        : null;
    return {
        ok: true,
        result: {
            operations,
            audit: audit.result,
            reference_compare: referenceCompare?.result,
            warnings: [
                ...warnings,
                ...(referenceCompare && !referenceCompare.ok ? [referenceCompare.error ?? "reference_render_compare failed"] : []),
            ],
        },
    };
}
async function runPerceptionBudgetPass(executeTool, args) {
    const warnings = [];
    const topN = Math.max(1, Number(args.top_n) || 10);
    const maxVisibleActors = Math.max(1, Number(args.max_visible_actors) || 25);
    const sceneCost = await requireTool(executeTool, "analyze_scene_cost", {
        top_n: topN,
    });
    const frustum = await requireTool(executeTool, "frustum_query", {
        max_results: maxVisibleActors,
    });
    const beforeStatus = await optionalTool(executeTool, "get_visibility_budget_status", {}, warnings);
    let configureResult;
    if (args.auto_configure !== false) {
        const configure = await optionalTool(executeTool, "configure_visibility_budget", {
            enabled: args.enabled !== false,
            evaluation_interval: Number(args.evaluation_interval) || 0.2,
            use_view_direction_bias: true,
            prediction_time: Number(args.prediction_time) || 0.35,
            view_cone_buffer_degrees: Number(args.view_cone_buffer_degrees) || 18,
            force_evaluate: true,
        }, warnings);
        configureResult = configure?.data;
    }
    const apply = await optionalTool(executeTool, "apply_performance_budget", {
        preset: typeof args.preset === "string" ? args.preset : "balanced",
        scope: typeof args.scope === "string" ? args.scope : "all",
        max_actors: Number(args.max_actors) || 250,
        selected_only: args.selected_only === true,
    }, warnings);
    const applyResult = apply?.data;
    const after = await optionalTool(executeTool, "get_visibility_budget_status", {}, warnings);
    const afterStatus = after?.data;
    const verifyVisuals = args.verify_visuals !== false
        ? await runSemanticSceneAudit(executeTool, {
            prompt: "Audit the scene after the perception-focused budget pass. " +
                "Focus on preserving player-facing readability, silhouettes, lighting clarity, and hero content.",
            include_materials: false,
            include_depth: false,
            max_visible_actors: 20,
        })
        : null;
    return {
        ok: true,
        result: {
            scene_cost: sceneCost.data,
            visible_actors: {
                count: summarizeVisibleActors(frustum.data, maxVisibleActors).length,
                sample: summarizeVisibleActors(frustum.data, 8),
            },
            budget_status_before: beforeStatus?.data,
            configure_result: configureResult,
            apply_result: applyResult,
            budget_status_after: afterStatus,
            visual_verification: verifyVisuals?.result,
            warnings: [
                ...warnings,
                ...(verifyVisuals && !verifyVisuals.ok ? [verifyVisuals.error ?? "semantic_scene_audit failed"] : []),
            ],
        },
    };
}
async function runMultiViewSceneAudit(executeTool, args) {
    const warnings = [];
    const prompt = typeof args.prompt === "string" && args.prompt.trim().length > 0
        ? args.prompt
        : "Audit the scene from multiple viewpoints. Focus on readability, composition, lighting balance, material plausibility, grounding, and missing content.";
    const viewCount = Math.max(1, Math.min(8, Number(args.view_count) || 4));
    const distance = Number(args.distance) || 1800;
    const pitch = Number(args.pitch) || -20;
    const startYaw = Number(args.start_yaw) || 45;
    const maxVisibleActors = Math.max(1, Number(args.max_visible_actors) || 24);
    const includeMaterials = args.include_materials !== false;
    const maxMaterialActors = Math.max(1, Number(args.max_material_actors) || 24);
    const targetContext = await inferTargetContext(executeTool, args, warnings, maxVisibleActors);
    const views = await captureOrbitViews(executeTool, {
        yawAngles: buildOrbitAngles(viewCount, startYaw),
        prompt,
        distance,
        pitch,
        targetLabel: targetContext.targetLabel,
        targetPoint: targetContext.targetPoint,
    }, warnings);
    const audit = buildViewAudit(views);
    const materialAudit = includeMaterials
        ? await optionalTool(executeTool, "material_audit", {
            max_actors: maxMaterialActors,
            include_textures: true,
        }, warnings)
        : null;
    return {
        ok: true,
        result: {
            target: {
                label: targetContext.targetLabel,
                point: targetContext.targetPoint,
            },
            views,
            audit,
            visible_actors: {
                count: targetContext.candidates.length,
                sample: targetContext.candidates.slice(0, 8).map((entry) => entry.raw),
            },
            frustum: targetContext.frustum,
            material_audit: materialAudit?.data,
            warnings,
        },
    };
}
function buildRepairSteps(issues, primaryActor, maxSteps = 6) {
    const severityRank = { high: 0, medium: 1, low: 2 };
    const primaryLabel = primaryActor?.label;
    const primaryLocation = primaryActor?.location;
    const sorted = [...issues].sort((left, right) => severityRank[left.severity] - severityRank[right.severity]);
    return sorted.slice(0, maxSteps).map((issue, index) => {
        let tool = "semantic_scene_audit";
        let suggestedArgs = {};
        let rollback = "No-op";
        let requiredInputs = [];
        let note;
        switch (issue.category) {
            case "lighting":
                tool = "create_light";
                suggestedArgs = {
                    type: "Point",
                    ...(primaryLocation
                        ? {
                            x: primaryLocation.x,
                            y: primaryLocation.y,
                            z: primaryLocation.z + 300,
                        }
                        : {}),
                    ...(primaryLabel ? { label: `${primaryLabel}_RepairLight` } : {}),
                };
                rollback = "undo_last";
                break;
            case "material":
                if (issue.description.includes("Missing")) {
                    tool = "set_actor_material";
                    suggestedArgs = primaryLabel ? { actor_label: primaryLabel } : {};
                    requiredInputs = ["material_path"];
                    note = "Choose a replacement material asset path for the affected actor before running this step.";
                }
                else {
                    tool = "set_material_parameter";
                    requiredInputs = ["material_path", "parameter_name", "value"];
                    note = primaryLabel
                        ? `Inspect the material instance used by '${primaryLabel}' and choose the parameter that matches the observed issue.`
                        : "Inspect the affected material instance and choose the parameter that matches the observed issue.";
                }
                rollback = "undo_last";
                break;
            case "placement":
                tool = "grounding_repair_pass";
                suggestedArgs = primaryLabel ? { target_labels: primaryLabel } : {};
                rollback = "rollback";
                break;
            case "scale":
                tool = "scale_actor";
                suggestedArgs = primaryLabel ? { label: primaryLabel } : {};
                requiredInputs = ["scale"];
                note = "Provide a uniform scale or explicit x/y/z scale before running this step.";
                rollback = "undo_last";
                break;
            case "composition":
                tool = "move_actor";
                suggestedArgs = primaryLabel ? { label: primaryLabel } : {};
                requiredInputs = ["x", "y", "z"];
                note = primaryLocation
                    ? `Start from the current actor position (${primaryLocation.x}, ${primaryLocation.y}, ${primaryLocation.z}) and nudge to improve framing.`
                    : "Provide a new world-space x/y/z location that improves the camera read.";
                rollback = "undo_last";
                break;
            case "missing_content":
                tool = "spawn_actor";
                suggestedArgs = { class_name: "StaticMeshActor" };
                rollback = "undo_last";
                break;
            case "performance":
                tool = "perception_budget_pass";
                suggestedArgs = { preset: "balanced" };
                rollback = "rollback";
                break;
        }
        return {
            priority: index + 1,
            issue,
            tool,
            suggested_args: suggestedArgs,
            ...(requiredInputs.length > 0 ? { required_inputs: requiredInputs } : {}),
            rationale: `Address the ${issue.category} issue: ${issue.description}`,
            ...(note ? { note } : {}),
            rollback,
        };
    });
}
async function runSemanticRepairPlan(executeTool, args) {
    const maxSteps = Math.max(1, Number(args.max_steps) || 6);
    const multiView = await runMultiViewSceneAudit(executeTool, {
        target_label: typeof args.target_label === "string" ? args.target_label : undefined,
        prompt: typeof args.prompt === "string" ? args.prompt : undefined,
        include_materials: true,
    });
    if (!multiView.ok) {
        return multiView;
    }
    const multiPayload = toRecord(multiView.result);
    const audit = toRecord(multiPayload.audit);
    const issues = uniqueIssues(audit.issues ?? []);
    const visibleActors = extractActorCandidates(multiPayload.frustum, 8);
    const target = toRecord(multiPayload.target);
    const primaryActor = visibleActors[0]
        ?? (() => {
            const label = typeof target.label === "string" ? target.label : undefined;
            const location = readVector(target.point);
            return label || location ? { label, location, raw: target } : undefined;
        })();
    const referenceCompare = args.include_reference_compare === true
        ? await runReferenceRenderCompare(executeTool, {
            restore_mode: "lit",
            prompt: "Use the reference render to sharpen the semantic repair plan.",
        })
        : null;
    return {
        ok: true,
        result: {
            audit: multiPayload,
            steps: buildRepairSteps(issues, primaryActor, maxSteps),
            reference_compare: referenceCompare?.result,
            warnings: referenceCompare && !referenceCompare.ok ? [referenceCompare.error ?? "reference_render_compare failed"] : [],
        },
    };
}
async function runGroundingRepairPass(executeTool, args) {
    const warnings = [];
    const maxVisibleActors = Math.max(1, Number(args.max_visible_actors) || 12);
    const targetLabels = parseCsvList(args.target_labels);
    const traceStartOffset = Number(args.trace_start_offset) || 500;
    const traceDepth = Number(args.trace_depth) || 5000;
    const surfaceOffset = Number(args.surface_offset) || 0;
    const frustum = targetLabels.length === 0
        ? await optionalTool(executeTool, "frustum_query", { max_results: maxVisibleActors }, warnings)
        : null;
    const candidates = targetLabels.length > 0
        ? targetLabels.map((label) => ({ label, raw: { label } }))
        : extractActorCandidates(frustum?.data, maxVisibleActors);
    const movedActors = [];
    const skipped = [];
    for (const candidate of candidates) {
        let location = candidate.location;
        if (!location && candidate.label) {
            const transform = await optionalTool(executeTool, "get_actor_transform", {
                actor_name: candidate.label,
            }, warnings);
            const transformRecord = toRecord(transform?.data);
            location = readVector(transformRecord.location)
                ?? readVector(transformRecord.position)
                ?? (() => {
                    const x = toFiniteNumber(transformRecord.x);
                    const y = toFiniteNumber(transformRecord.y);
                    const z = toFiniteNumber(transformRecord.z);
                    return x != null && y != null && z != null ? { x, y, z } : undefined;
                })();
        }
        if (!candidate.label || !location || shouldSkipGrounding({ ...candidate, location })) {
            if (candidate.label) {
                skipped.push(candidate.label);
            }
            continue;
        }
        const trace = await optionalTool(executeTool, "line_trace", {
            start_x: location.x,
            start_y: location.y,
            start_z: location.z + traceStartOffset,
            end_x: location.x,
            end_y: location.y,
            end_z: location.z - traceDepth,
            channel: "Visibility",
        }, warnings);
        const hit = extractHitLocation(trace?.data);
        if (!hit) {
            skipped.push(candidate.label);
            continue;
        }
        const move = await optionalTool(executeTool, "move_actor", {
            label: candidate.label,
            x: location.x,
            y: location.y,
            z: hit.z + surfaceOffset,
        }, warnings);
        if (move) {
            movedActors.push({
                label: candidate.label,
                from: location,
                to: { x: location.x, y: location.y, z: hit.z + surfaceOffset },
                result: move.data,
            });
        }
    }
    const foliageResult = args.ground_foliage !== false
        ? await optionalTool(executeTool, "ground_foliage_to_landscape", {
            support_mode: typeof args.support_mode === "string" ? args.support_mode : "auto",
            surface_offset: surfaceOffset,
        }, warnings)
        : null;
    const verification = args.verify !== false
        ? await runSemanticSceneAudit(executeTool, {
            prompt: "Audit the current scene for grounding, support contact, floating actors, clipping, and terrain attachment.",
            include_materials: false,
            include_depth: true,
            max_visible_actors: maxVisibleActors,
        })
        : null;
    return {
        ok: true,
        result: {
            moved_actors: movedActors,
            skipped_actors: skipped,
            foliage_result: foliageResult?.data,
            verification: verification?.result,
            warnings: [
                ...warnings,
                ...(verification && !verification.ok ? [verification.error ?? "semantic_scene_audit failed"] : []),
            ],
        },
    };
}
async function runTemporalCameraSweepAudit(executeTool, args) {
    const playMode = args.play_mode === true;
    const warnings = [];
    let pieStarted = false;
    try {
        if (playMode) {
            const startPie = await executeTool("start_pie", {
                mode: "viewport",
                num_players: 1,
            });
            if (!startPie.ok) {
                return { ok: false, error: `start_pie failed: ${startPie.error ?? "unknown error"}` };
            }
            pieStarted = true;
        }
        const prompt = typeof args.prompt === "string" && args.prompt.trim().length > 0
            ? args.prompt
            : "Audit continuity during a deterministic camera sweep. Call out pop-in, shimmer, ghosting, exposure shifts, and readability breaks between adjacent views.";
        const targetContext = await inferTargetContext(executeTool, args, warnings, 12);
        const distance = Number(args.distance) || 1600;
        const pitch = Number(args.pitch) || -18;
        const sweepAngles = buildSweepAngles(Math.max(1, Number(args.view_count) || 5), Number(args.sweep_angle_degrees) || 120);
        const views = await captureOrbitViews(executeTool, {
            yawAngles: sweepAngles,
            prompt,
            distance,
            pitch,
            targetLabel: targetContext.targetLabel,
            targetPoint: targetContext.targetPoint,
        }, warnings);
        const sequenceCapture = args.include_sequence_capture !== false
            ? await optionalTool(executeTool, "capture_frame_sequence", {
                duration_seconds: Number(args.duration_seconds) || 3,
                fps: Number(args.fps) || 5,
                max_keyframes: 4,
                analyze: true,
                prompt,
            }, warnings)
            : null;
        const allTemporalText = [
            ...views.map((view) => view.analysis),
            sequenceCapture ? extractAnalysisText(sequenceCapture.data) : "",
        ].join("\n");
        const issues = parseTemporalIssues(allTemporalText);
        const recommendations = buildTemporalRecommendations(issues);
        return {
            ok: true,
            result: {
                target: {
                    label: targetContext.targetLabel,
                    point: targetContext.targetPoint,
                },
                views,
                sequence_capture: sequenceCapture?.data,
                issues,
                recommendations,
                warnings,
            },
        };
    }
    finally {
        if (pieStarted) {
            const stopPie = await executeTool("stop_pie", {});
            if (!stopPie.ok) {
                warnings.push(`stop_pie: ${stopPie.error ?? "unknown error"}`);
            }
        }
    }
}
async function captureRegressionSnapshot(executeTool, args, baselineName) {
    const multiView = await runMultiViewSceneAudit(executeTool, {
        target_label: typeof args.target_label === "string" ? args.target_label : undefined,
        view_count: Number(args.view_count) || 4,
        include_materials: true,
        prompt: "Capture a stable multi-view baseline of the scene for later regression comparison.",
    });
    if (!multiView.ok) {
        throw new Error(multiView.error ?? "multi_view_scene_audit failed");
    }
    const performance = await optionalTool(executeTool, "get_performance_snapshot", {}, []);
    const checkpoint = args.include_checkpoint !== false
        ? await optionalTool(executeTool, "save_scene_checkpoint", {
            name: baselineName,
            overwrite: true,
            include_screenshot: true,
            include_digest: true,
            include_performance: true,
        }, [])
        : null;
    const worldState = args.include_digest !== false
        ? await optionalTool(executeTool, "get_world_state_digest", {
            save_name: baselineName,
        }, [])
        : null;
    const referenceCompare = args.include_reference_compare === true
        ? await runReferenceRenderCompare(executeTool, {
            restore_mode: "lit",
            prompt: "Capture a stable lit-vs-reference baseline for regression tracking.",
        })
        : null;
    return {
        name: baselineName,
        captured_at: new Date().toISOString(),
        multi_view: toRecord(multiView.result),
        performance: performance?.data,
        performance_summary: performance ? parsePerformanceSummary(performance.data) : undefined,
        checkpoint: checkpoint?.data,
        world_state: worldState?.data,
        reference_compare: referenceCompare?.result,
    };
}
async function runSceneRegressionBaseline(executeTool, args, baselineStore) {
    const action = typeof args.action === "string" ? args.action : "capture";
    if (action === "list") {
        return {
            ok: true,
            result: {
                baselines: Array.from(baselineStore.values()).map((baseline) => ({
                    name: baseline.name,
                    created_at: baseline.created_at,
                    notes: baseline.notes,
                    quality: toRecord(toRecord(baseline.snapshot.multi_view).audit).quality,
                })),
            },
        };
    }
    const baselineName = typeof args.name === "string" && args.name.trim().length > 0
        ? args.name.trim()
        : "";
    if (!baselineName) {
        return { ok: false, error: "scene_regression_baseline requires a non-empty name for capture, compare, and delete actions" };
    }
    if (action === "delete") {
        const removed = baselineStore.delete(baselineName);
        return { ok: true, result: { deleted: removed, name: baselineName } };
    }
    if (action === "capture") {
        const snapshot = await captureRegressionSnapshot(executeTool, args, baselineName);
        baselineStore.set(baselineName, {
            name: baselineName,
            created_at: snapshot.captured_at,
            notes: typeof args.notes === "string" ? args.notes : undefined,
            snapshot,
        });
        return {
            ok: true,
            result: {
                name: baselineName,
                created_at: snapshot.captured_at,
                snapshot,
            },
        };
    }
    if (action === "compare") {
        const baseline = baselineStore.get(baselineName);
        if (!baseline) {
            return { ok: false, error: `No stored scene regression baseline named '${baselineName}'` };
        }
        const current = await captureRegressionSnapshot(executeTool, args, baselineName);
        return {
            ok: true,
            result: {
                name: baselineName,
                baseline: baseline.snapshot,
                current,
                comparison: compareRegressionSnapshots(baseline.snapshot, current),
            },
        };
    }
    return { ok: false, error: `Unsupported scene_regression_baseline action '${action}'` };
}
async function runHeroContentProtection(executeTool, args) {
    const warnings = [];
    const explicitLabels = parseCsvList(args.actor_labels);
    const maxVisibleActors = Math.max(1, Number(args.max_visible_actors) || 6);
    const frustum = explicitLabels.length === 0
        ? await optionalTool(executeTool, "frustum_query", { max_results: maxVisibleActors }, warnings)
        : null;
    const candidates = explicitLabels.length > 0
        ? explicitLabels.map((label) => ({ label, raw: { label } }))
        : extractActorCandidates(frustum?.data, maxVisibleActors);
    const assignments = [];
    for (const [index, candidate] of candidates.entries()) {
        if (!candidate.label) {
            continue;
        }
        const className = (candidate.class_name ?? "").toLowerCase();
        const policy = className.includes("character") || className.includes("pawn")
            ? "gameplay_critical"
            : index === 0
                ? (typeof args.default_policy === "string" ? args.default_policy : "hero")
                : (typeof args.secondary_policy === "string" ? args.secondary_policy : "gameplay_critical");
        const setPolicy = await optionalTool(executeTool, "set_actor_significance_policy", {
            actor_label: candidate.label,
            policy,
            preset: typeof args.preset === "string" ? args.preset : "balanced",
        }, warnings);
        if (setPolicy) {
            assignments.push({
                actor_label: candidate.label,
                policy,
                result: setPolicy.data,
            });
        }
    }
    const configureResult = args.configure_budget !== false
        ? await optionalTool(executeTool, "configure_visibility_budget", {
            enabled: true,
            force_evaluate: true,
        }, warnings)
        : null;
    const perceptualCost = await optionalTool(executeTool, "analyze_perceptual_cost", { top_n: 5 }, warnings);
    const optimization = args.optimize_after_protection === true
        ? await optionalTool(executeTool, "optimize_open_world_scene", {
            preset: typeof args.preset === "string" ? args.preset : "balanced",
            scope: "all",
            selected_only: false,
            max_actors: 250,
            protect_characters: true,
        }, warnings)
        : null;
    return {
        ok: true,
        result: {
            assignments,
            configure_result: configureResult?.data,
            perceptual_cost: perceptualCost?.data,
            optimization_result: optimization?.data,
            warnings,
        },
    };
}
async function runLightingConsistencyAudit(executeTool, args) {
    const multiView = await runMultiViewSceneAudit(executeTool, {
        ...args,
        include_materials: false,
        prompt: typeof args.prompt === "string" && args.prompt.trim().length > 0
            ? args.prompt
            : "Audit lighting consistency across these views. Focus on exposure balance, shadow depth, fog integration, and color-temperature coherence.",
    });
    if (!multiView.ok) {
        return multiView;
    }
    const payload = toRecord(multiView.result);
    const views = Array.isArray(payload.views) ? payload.views.map((entry) => toRecord(entry)) : [];
    const hotspots = views.filter((view) => {
        const issues = Array.isArray(view.issues) ? view.issues : [];
        return issues.some((issue) => issue.category === "lighting") || parseQuality(view.quality) !== "excellent";
    });
    const audit = toRecord(payload.audit);
    const lightingIssues = uniqueIssues((audit.issues ?? []).filter((issue) => issue.category === "lighting"));
    const referenceCompare = args.include_reference_compare !== false
        ? await runReferenceRenderCompare(executeTool, {
            restore_mode: "lit",
            prompt: "Use the path-traced pass to judge whether realtime lighting stays consistent across viewpoints.",
        })
        : null;
    const referencePenalty = referenceCompare?.ok ? toRecord(toRecord(referenceCompare.result).delta).lit_only_issues : [];
    const score = Math.max(0, 100 - lightingIssues.length * 15 - hotspots.length * 10 - (Array.isArray(referencePenalty) ? referencePenalty.length * 5 : 0));
    return {
        ok: true,
        result: {
            score,
            audit: payload,
            lighting_issues: lightingIssues,
            hotspots,
            reference_compare: referenceCompare?.result,
            warnings: referenceCompare && !referenceCompare.ok ? [referenceCompare.error ?? "reference_render_compare failed"] : [],
        },
    };
}
async function runMaterialPlausibilityAudit(executeTool, args) {
    const warnings = [];
    const materialAudit = await requireTool(executeTool, "material_audit", {
        max_actors: Math.max(1, Number(args.max_material_actors) || 12),
        include_textures: true,
    });
    const frustum = await optionalTool(executeTool, "frustum_query", { max_results: 12 }, warnings);
    const explicitLabels = parseCsvList(args.actor_labels);
    const materialActors = extractActorCandidates(materialAudit.data, Math.max(1, Number(args.focus_count) || 3));
    const visibleActors = extractActorCandidates(frustum?.data, Math.max(1, Number(args.focus_count) || 3));
    const focusLabels = explicitLabels.length > 0
        ? explicitLabels
        : [...new Set([...materialActors, ...visibleActors].map((entry) => entry.label).filter((entry) => Boolean(entry)))].slice(0, Math.max(1, Number(args.focus_count) || 3));
    const prompt = typeof args.prompt === "string" && args.prompt.trim().length > 0
        ? args.prompt
        : "Audit material plausibility. Focus on roughness/specular response, metallic plausibility, texture definition, and whether the material read matches the object type.";
    const views = [];
    for (const [index, label] of focusLabels.entries()) {
        const capture = await optionalTool(executeTool, "look_at_and_capture", {
            target_label: label,
            distance: Number(args.distance) || 1200,
            yaw: 45,
            pitch: Number(args.pitch) || -15,
            analyze: true,
            prompt: `${prompt} Focus actor ${index + 1} of ${focusLabels.length}: ${label}.`,
            filename: `material_plausibility_${Date.now()}_${index + 1}`,
        }, warnings);
        if (!capture) {
            continue;
        }
        const analysis = extractAnalysisText(capture.data);
        const parsed = parseVisionText(analysis);
        views.push({
            view_id: `material_${index + 1}`,
            yaw: 45,
            pitch: Number(args.pitch) || -15,
            analysis,
            issues: uniqueIssues(parsed.issues),
            quality: parseQuality(parsed.quality),
            screenshot_path: extractScreenshotPath(capture.data),
            capture: capture.data,
        });
    }
    const audit = buildViewAudit(views);
    const materialIssues = uniqueIssues(audit.issues.filter((issue) => issue.category === "material"));
    const referenceCompare = args.include_reference_compare === true
        ? await runReferenceRenderCompare(executeTool, {
            restore_mode: "lit",
            prompt: "Use the path-traced render to validate whether visible materials read plausibly in realtime.",
        })
        : null;
    const score = Math.max(0, 100 - materialIssues.length * 20 - views.filter((view) => view.quality === "poor").length * 10);
    return {
        ok: true,
        result: {
            score,
            focus_labels: focusLabels,
            material_audit: materialAudit.data,
            views,
            audit,
            material_issues: materialIssues,
            reference_compare: referenceCompare?.result,
            warnings: [
                ...warnings,
                ...(referenceCompare && !referenceCompare.ok ? [referenceCompare.error ?? "reference_render_compare failed"] : []),
            ],
        },
    };
}
async function runQualityGate(executeTool, args, baselineStore) {
    const blockers = [];
    const warnings = [];
    const multiView = await runMultiViewSceneAudit(executeTool, {
        target_label: typeof args.target_label === "string" ? args.target_label : undefined,
        view_count: 4,
        include_materials: true,
        prompt: "Run a ship-readiness audit across multiple viewpoints.",
    });
    if (!multiView.ok) {
        return multiView;
    }
    const multiPayload = toRecord(multiView.result);
    const multiAudit = toRecord(multiPayload.audit);
    const multiQuality = parseQuality(multiAudit.quality);
    const multiIssues = uniqueIssues(multiAudit.issues ?? []);
    const highIssues = multiIssues.filter((issue) => issue.severity === "high");
    if (multiQuality === "poor" || highIssues.length >= 2) {
        blockers.push("Multi-view semantic audit found severe readability or art-direction problems.");
    }
    else if (multiQuality === "fair" || highIssues.length === 1) {
        warnings.push("Multi-view semantic audit found at least one high-severity scene issue.");
    }
    const temporalSweep = args.include_temporal_sweep !== false
        ? await runTemporalCameraSweepAudit(executeTool, {
            target_label: typeof args.target_label === "string" ? args.target_label : undefined,
            play_mode: args.play_mode === true,
            include_sequence_capture: true,
        })
        : null;
    if (temporalSweep?.ok) {
        const temporalIssues = Array.isArray(toRecord(temporalSweep.result).issues)
            ? toRecord(temporalSweep.result).issues
            : [];
        if (temporalIssues.some((issue) => issue.severity === "high")) {
            blockers.push("Temporal sweep found high-severity continuity or stability issues.");
        }
        else if (temporalIssues.length > 0) {
            warnings.push("Temporal sweep found medium-severity temporal instability that should be reviewed.");
        }
    }
    else if (temporalSweep && !temporalSweep.ok) {
        warnings.push(temporalSweep.error ?? "temporal_camera_sweep_audit failed");
    }
    const referenceCompare = args.include_reference_compare !== false
        ? await runReferenceRenderCompare(executeTool, {
            restore_mode: "lit",
            prompt: "Use the path-traced pass as a ship-quality reference gate.",
        })
        : null;
    if (referenceCompare?.ok) {
        const delta = toRecord(toRecord(referenceCompare.result).delta);
        const litOnlyIssues = Array.isArray(delta.lit_only_issues) ? delta.lit_only_issues.length : 0;
        if (litOnlyIssues >= 3) {
            warnings.push("Reference comparison found several realtime-only fidelity gaps.");
        }
    }
    else if (referenceCompare && !referenceCompare.ok) {
        warnings.push(referenceCompare.error ?? "reference_render_compare failed");
    }
    const performance = await optionalTool(executeTool, "get_performance_snapshot", {}, warnings);
    const performanceSummary = performance ? parsePerformanceSummary(performance.data) : undefined;
    const minFps = Number(args.min_fps) || 30;
    if (performanceSummary?.fps != null && performanceSummary.fps < minFps) {
        blockers.push(`Performance snapshot is below the minimum FPS threshold (${minFps}).`);
    }
    else if ((performanceSummary?.concerns.length ?? 0) > 0) {
        warnings.push(...(performanceSummary?.concerns ?? []));
    }
    const perceptualCost = await optionalTool(executeTool, "analyze_perceptual_cost", { top_n: 5 }, warnings);
    const baselineComparison = typeof args.baseline_name === "string" && args.baseline_name.trim().length > 0
        ? await runSceneRegressionBaseline(executeTool, {
            action: "compare",
            name: args.baseline_name,
            target_label: args.target_label,
            view_count: 4,
            include_reference_compare: false,
            include_checkpoint: false,
            include_digest: true,
        }, baselineStore)
        : null;
    if (baselineComparison?.ok) {
        const comparison = toRecord(toRecord(baselineComparison.result).comparison);
        if (comparison.status === "regressed") {
            blockers.push("Scene regression baseline comparison reported a regression.");
        }
        else if (comparison.status === "improved") {
            warnings.push("Scene is improved relative to the stored baseline.");
        }
    }
    else if (baselineComparison && !baselineComparison.ok) {
        warnings.push(baselineComparison.error ?? "scene_regression_baseline compare failed");
    }
    const status = blockers.length > 0 ? "fail" : warnings.length > 0 ? "warn" : "pass";
    return {
        ok: true,
        result: {
            status,
            blockers,
            warnings: [...new Set(warnings)],
            checks: {
                multi_view: multiView.result,
                temporal_sweep: temporalSweep?.result,
                reference_compare: referenceCompare?.result,
                performance: performance?.data,
                performance_summary: performanceSummary,
                perceptual_cost: perceptualCost?.data,
                baseline_comparison: baselineComparison?.result,
            },
        },
    };
}
async function runBenchmarkPack(executeTool, args, baselineStore) {
    const warnings = [];
    const qualityGate = args.include_quality_gate !== false
        ? await runQualityGate(executeTool, {
            baseline_name: typeof args.baseline_name === "string" ? args.baseline_name : undefined,
            include_reference_compare: true,
            include_temporal_sweep: true,
            play_mode: args.play_mode === true,
        }, baselineStore)
        : null;
    const quickPlaytest = args.include_quick_playtest !== false
        ? await optionalTool(executeTool, "run_quick_playtest", {
            duration_seconds: Number(args.playtest_duration) || 3,
        }, warnings)
        : null;
    const benchmark = args.run_engine_benchmark !== false
        ? await optionalTool(executeTool, "run_benchmark", {}, warnings)
        : null;
    const performance = await optionalTool(executeTool, "get_performance_snapshot", {}, warnings);
    const qualityStatus = qualityGate?.ok ? toRecord(qualityGate.result).status : undefined;
    return {
        ok: true,
        result: {
            status: qualityStatus === "fail" ? "fail" : qualityStatus === "warn" || warnings.length > 0 ? "warn" : "pass",
            quality_gate: qualityGate?.result,
            quick_playtest: quickPlaytest?.data,
            engine_benchmark: benchmark?.data,
            performance: performance?.data,
            warnings: [
                ...warnings,
                ...(qualityGate && !qualityGate.ok ? [qualityGate.error ?? "quality_gate failed"] : []),
            ],
        },
    };
}
export function createSceneEnhancementHandlers(executeTool) {
    const baselineStore = new Map();
    return {
        semantic_scene_audit: async (args) => runSemanticSceneAudit(executeTool, args),
        temporal_quality_audit: async (args) => runTemporalQualityAudit(executeTool, args),
        reference_render_compare: async (args) => runReferenceRenderCompare(executeTool, args),
        photoreal_polish_pass: async (args) => runPhotorealPolishPass(executeTool, args),
        perception_budget_pass: async (args) => runPerceptionBudgetPass(executeTool, args),
        multi_view_scene_audit: async (args) => runMultiViewSceneAudit(executeTool, args),
        semantic_repair_plan: async (args) => runSemanticRepairPlan(executeTool, args),
        grounding_repair_pass: async (args) => runGroundingRepairPass(executeTool, args),
        temporal_camera_sweep_audit: async (args) => runTemporalCameraSweepAudit(executeTool, args),
        scene_regression_baseline: async (args) => runSceneRegressionBaseline(executeTool, args, baselineStore),
        hero_content_protection: async (args) => runHeroContentProtection(executeTool, args),
        lighting_consistency_audit: async (args) => runLightingConsistencyAudit(executeTool, args),
        material_plausibility_audit: async (args) => runMaterialPlausibilityAudit(executeTool, args),
        quality_gate: async (args) => runQualityGate(executeTool, args, baselineStore),
        benchmark_pack: async (args) => runBenchmarkPack(executeTool, args, baselineStore),
    };
}
//# sourceMappingURL=scene-enhancement.js.map