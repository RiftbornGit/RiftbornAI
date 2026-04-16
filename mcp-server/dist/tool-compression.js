/**
 * Tool Compression & Workflow Graph — Round 5 enhancements
 *
 * 1. compressToolListing — Shrinks the ListTools payload by ~40%:
 *    - Shortens long descriptions to first sentence
 *    - Strips parameter-level descriptions from inputSchema
 *    - Preserves property names, types, required, enum, default
 *    Full schemas remain available via find_tools and describe_tool.
 *
 * 2. WorkflowGraph — Encodes common multi-step sequences that agents
 *    struggle to discover independently. Powers the `get_workflow` tool.
 */
const MAX_WORKFLOW_QUERY_LENGTH = 64;
export function normalizeWorkflowQuery(name) {
    return name
        .trim()
        .replace(/\s+/g, " ")
        .slice(0, MAX_WORKFLOW_QUERY_LENGTH)
        .toLowerCase();
}
// ---------------------------------------------------------------------------
// 1. Tool Listing Compression
// ---------------------------------------------------------------------------
/**
 * Shorten a description to its first meaningful sentence.
 * Keeps up to maxLength chars, breaking at sentence boundary when possible.
 */
function shortenDescription(desc, maxLength = 150) {
    if (desc.length <= maxLength)
        return desc;
    // Try to break at first sentence ending within budget
    const sentenceEnd = desc.search(/[.!]\s/);
    if (sentenceEnd > 0 && sentenceEnd < maxLength) {
        return desc.substring(0, sentenceEnd + 1);
    }
    // Fall back to word boundary
    const truncated = desc.substring(0, maxLength);
    const lastSpace = truncated.lastIndexOf(" ");
    if (lastSpace > maxLength * 0.6) {
        return truncated.substring(0, lastSpace) + "...";
    }
    return truncated + "...";
}
/**
 * Strip description fields from inputSchema properties while preserving
 * type information, required fields, enum, and default values.
 */
function compressSchema(schema) {
    if (!schema || typeof schema !== "object")
        return schema;
    const out = { type: schema.type };
    if (schema.required)
        out.required = schema.required;
    if (schema.properties) {
        out.properties = {};
        for (const [key, prop] of Object.entries(schema.properties)) {
            const compressed = { type: prop.type };
            // Keep enum — agents need it for valid values
            if (prop.enum)
                compressed.enum = prop.enum;
            // Keep default — agents use it to decide what to provide
            if (prop.default !== undefined)
                compressed.default = prop.default;
            // Keep items type for arrays
            if (prop.items) {
                compressed.items = prop.items.type ? { type: prop.items.type } : prop.items;
            }
            out.properties[key] = compressed;
        }
    }
    return out;
}
/**
 * Compress a tool listing for the ListTools response.
 * Returns a new array — does not mutate the original tools.
 *
 * - Descriptions shortened to first sentence (max 150 chars)
 * - Parameter descriptions removed from inputSchema
 * - Type info, required fields, enums, defaults preserved
 */
export function compressToolListing(tools) {
    return tools.map((tool) => ({
        name: tool.name,
        description: shortenDescription(tool.description || ""),
        inputSchema: compressSchema(tool.inputSchema),
    }));
}
const WORKFLOWS = {
    landscape: {
        name: "Landscape Pipeline",
        description: "Build a complete terrain with material, layers, painting, and automatic grass. " +
            "This is the bottom-up pipeline from AI School — follow the order exactly.",
        steps: [
            {
                tool: "create_landscape",
                description: "Create the terrain heightmap.",
                key_params: ["size", "sections"],
            },
            {
                tool: "create_landscape_material",
                description: "Create a layered material with weight-blended layers (Grass, Dirt, Rock, etc.).",
                key_params: ["name", "layers"],
            },
            {
                tool: "apply_landscape_material",
                description: "Assign the material to the landscape.",
                key_params: ["landscape_name", "material_path"],
            },
            {
                tool: "add_landscape_layer",
                description: "Register each paint layer on the landscape (repeat per layer).",
                key_params: ["landscape_label", "layer_name"],
                notes: "Call once per layer defined in the material.",
            },
            {
                tool: "paint_landscape_layer",
                description: "Paint layers onto the terrain.",
                key_params: ["landscape_label", "layer_name", "center_x", "center_y", "radius", "strength"],
                notes: "Paint Grass as the base first, then Dirt/Rock for paths and features.",
            },
            {
                tool: "sculpt_landscape",
                description: "Shape the terrain (raise, lower, smooth, flatten).",
                key_params: ["landscape_label", "mode", "position"],
                notes: "Optional. Can sculpt before or after painting.",
            },
            {
                tool: "create_landscape_grass_type",
                description: "Create automatic grass that follows the Grass paint layer.",
                key_params: ["name"],
            },
            {
                tool: "add_grass_variety",
                description: "Add grass/fern meshes to the grass type (repeat per variety).",
                key_params: ["grass_type", "mesh", "density"],
            },
        ],
    },
    character: {
        name: "Character Pipeline",
        description: "Create a playable third-person character with mesh, animation, input, and camera.",
        steps: [
            {
                tool: "create_character_from_third_person",
                description: "Create a character Blueprint from the UE5 Third Person template.",
                key_params: ["name"],
            },
            {
                tool: "make_character_playable",
                description: "Create GameMode, set as default pawn, apply to world settings.",
                key_params: [],
                notes: "One-shot setup. Creates GameMode if needed.",
            },
        ],
    },
    blueprint: {
        name: "Blueprint Pipeline",
        description: "Create a Blueprint with components, variables, events, and compile it.",
        steps: [
            {
                tool: "open_blueprint",
                description: "Open an existing Blueprint, or use create_blueprint to make a new one.",
                key_params: ["path"],
            },
            {
                tool: "add_blueprint_component",
                description: "Add components (StaticMesh, Collision, etc.).",
                key_params: ["path", "component_class", "component_name"],
                notes: "Repeat for each component needed.",
            },
            {
                tool: "add_blueprint_variable",
                description: "Add variables (Health, Speed, etc.).",
                key_params: ["path", "variable_name", "variable_type"],
            },
            {
                tool: "add_blueprint_event",
                description: "Add event graph nodes (BeginPlay, Tick, custom events).",
                key_params: ["path", "event_name"],
            },
            {
                tool: "compile_blueprint",
                description: "Compile and validate. Check for errors.",
                key_params: ["blueprint_path"],
            },
        ],
    },
    material: {
        name: "Material Pipeline",
        description: "Create a PBR material, tune parameters, and apply to actors.",
        steps: [
            {
                tool: "create_pbr_material",
                description: "Create a material with base color, roughness, metallic.",
                key_params: ["name", "base_color", "roughness", "metallic"],
                notes: "Or use create_material for a blank material.",
            },
            {
                tool: "set_material_parameter",
                description: "Adjust scalar/vector/texture parameters.",
                key_params: ["material_path", "parameter_name", "value"],
                notes: "Repeat for each parameter to tune.",
            },
            {
                tool: "create_material_instance",
                description: "Create an instance for per-actor variation.",
                key_params: ["parent_path", "name"],
                notes: "Optional. Use instances to share base material with different parameters.",
            },
            {
                tool: "set_actor_material",
                description: "Apply the material to an actor.",
                key_params: ["label", "material_path"],
            },
        ],
    },
    lighting: {
        name: "Lighting Pipeline",
        description: "Light a scene with directional sun, sky light, fog, and post-process.",
        steps: [
            {
                tool: "create_light",
                description: "Create the main directional light (sun).",
                key_params: ["type", "label", "rotation"],
                notes: "type: 'Directional'. Set rotation for sun angle.",
            },
            {
                tool: "create_light",
                description: "Create a sky light for ambient fill.",
                key_params: ["type", "label"],
                notes: "type: 'SkyLight'. Set after directional for proper sky capture.",
            },
            {
                tool: "create_post_process_volume",
                description: "Add post-process volume for color grading, bloom, exposure.",
                key_params: ["label"],
            },
            {
                tool: "set_post_process_settings",
                description: "Configure bloom, exposure, color grading, vignette.",
                key_params: ["actor_name"],
                notes: "Set infinite_extent=true for global effect.",
            },
        ],
    },
    forest: {
        name: "Forest Pipeline",
        description: "Build a complete forest environment from terrain through atmosphere. " +
            "Follows AI School bottom-up order: terrain → material → grass → trees → fog → post-process.",
        steps: [
            { tool: "create_landscape", description: "Create terrain.", key_params: ["size"] },
            { tool: "create_landscape_material", description: "Layered terrain material (Grass, Dirt, Rock, ForestFloor).", key_params: ["layers"] },
            { tool: "apply_landscape_material", description: "Assign material to landscape.", key_params: ["landscape_name", "material_path"] },
            { tool: "add_landscape_layer", description: "Register each layer.", key_params: ["layer_name"], notes: "Repeat per layer." },
            { tool: "paint_landscape_layer", description: "Paint base grass, then dirt paths, rock outcrops.", key_params: ["layer_name", "center_x", "center_y"] },
            { tool: "create_landscape_grass_type", description: "Automatic grass ground cover.", key_params: ["name"] },
            { tool: "add_grass_variety", description: "Add grass/fern meshes.", key_params: ["mesh", "density"] },
            { tool: "paint_foliage", description: "Place trees and large shrubs.", key_params: ["mesh", "center_x", "center_y", "radius", "count"], notes: "Use for trees/shrubs, NOT grass." },
            { tool: "create_light", description: "Directional sun.", key_params: ["type"], notes: "type: 'Directional'" },
            { tool: "create_light", description: "Sky light.", key_params: ["type"], notes: "type: 'SkyLight'" },
            { tool: "create_post_process_volume", description: "Global post-process.", key_params: ["label"] },
            { tool: "set_post_process_settings", description: "Warm color grading, subtle bloom.", key_params: ["actor_name"] },
        ],
    },
    arena: {
        name: "Arena Pipeline",
        description: "Build a playable arena with floor, walls, cover, lights, and navigation.",
        steps: [
            { tool: "create_landscape", description: "Arena floor terrain.", key_params: ["size"] },
            { tool: "spawn_actor", description: "Place wall segments around the perimeter.", key_params: ["actor_class", "label"], notes: "Repeat for each wall piece." },
            { tool: "spawn_actor", description: "Place cover objects (boxes, barriers).", key_params: ["actor_class", "label"], notes: "Repeat for each cover piece." },
            { tool: "create_light", description: "Arena lighting.", key_params: ["type", "label"] },
            { tool: "build_navmesh", description: "Generate navigation mesh for AI.", key_params: [] },
            { tool: "create_character_from_third_person", description: "Create playable character.", key_params: ["name"] },
            { tool: "make_character_playable", description: "Set as default pawn.", key_params: [] },
            { tool: "start_pie", description: "Playtest the arena.", key_params: [] },
        ],
    },
};
/**
 * Get a named workflow (multi-step build sequence).
 * Returns null if workflow name is not found.
 */
export function getWorkflow(name) {
    return WORKFLOWS[normalizeWorkflowQuery(name)] ?? null;
}
/**
 * List all available workflow names with descriptions.
 */
export function listWorkflows() {
    return Object.entries(WORKFLOWS).map(([key, w]) => ({
        name: key,
        description: w.description,
        step_count: w.steps.length,
    }));
}
//# sourceMappingURL=tool-compression.js.map