/**
 * Pipeline Intelligence — Round 4 enhancements for the RiftbornAI MCP Server
 *
 * 1. ToolRouter — Keyword search over the tool registry.
 *    Powers the `find_tools` meta-tool so agents can discover tools
 *    without scanning the full 672+ tool listing (~145K tokens).
 *
 * 2. shouldSmartRetry — Decides whether a failed tool call should auto-retry.
 *    Only retries on transient bridge failures (disconnected/timeout), max once.
 *
 * 3. ContextPropagator — Tracks outputs from successful tool calls and
 *    auto-fills missing parameters in subsequent calls.
 *    e.g. spawn_actor returns label → move_actor auto-fills label.
 */
const MAX_TOOL_SEARCH_QUERY_LENGTH = 256;
const MAX_TOOL_SEARCH_RESULTS = 50;
const MAX_TOOL_PREFIX_RESULTS = 100;
const BLOCKED_CONTEXT_KEYS = new Set(["__proto__", "constructor", "prototype"]);
export function normalizeToolSearchQuery(query) {
    return query
        .trim()
        .replace(/\s+/g, " ")
        .slice(0, MAX_TOOL_SEARCH_QUERY_LENGTH);
}
export function clampToolSearchResults(maxResults, fallback = 20) {
    const requested = Number.isFinite(maxResults) ? Math.trunc(maxResults) : fallback;
    const safeFallback = Math.max(1, Math.min(MAX_TOOL_SEARCH_RESULTS, fallback));
    return Math.max(1, Math.min(MAX_TOOL_SEARCH_RESULTS, requested || safeFallback));
}
function extractParamNames(schema) {
    if (!schema?.properties)
        return [];
    return Object.keys(schema.properties);
}
function extractRequiredParams(schema) {
    if (!schema?.required)
        return [];
    return schema.required;
}
function tokenize(query) {
    return normalizeToolSearchQuery(query)
        .toLowerCase()
        .split(/[\s_,]+/)
        .filter((t) => t.length > 1);
}
export class ToolRouter {
    index;
    constructor(tools) {
        this.index = tools.map((t) => ({
            name: t.name,
            nameLower: t.name.toLowerCase(),
            descLower: (t.description || "").toLowerCase(),
            paramNamesLower: extractParamNames(t.inputSchema).join(" ").toLowerCase(),
            tool: t,
        }));
    }
    /**
     * Search tools by keyword. Scores by name match (highest), parameter names
     * (medium), and description (lowest). Returns up to maxResults sorted by relevance.
     */
    search(query, maxResults = 20) {
        const normalizedQuery = normalizeToolSearchQuery(query);
        const terms = tokenize(normalizedQuery);
        if (terms.length === 0)
            return [];
        const queryLower = normalizedQuery.toLowerCase();
        const scored = [];
        for (const entry of this.index) {
            let score = 0;
            // Full query matches exact name (e.g. "spawn_actor" → spawn_actor)
            if (entry.nameLower === queryLower)
                score += 100;
            for (const term of terms) {
                // Exact full-name match (single-word tool name)
                if (entry.nameLower === term) {
                    score += 100;
                }
                else if (entry.nameLower.includes(term)) {
                    // Partial name match ("landscape" in "create_landscape")
                    score += 15;
                }
                if (entry.descLower.includes(term))
                    score += 3;
                if (entry.paramNamesLower.includes(term))
                    score += 5;
            }
            // Bonus: all query terms present somewhere → stronger signal
            if (score > 0 && terms.length > 1) {
                const allMatch = terms.every((t) => entry.nameLower.includes(t) ||
                    entry.descLower.includes(t) ||
                    entry.paramNamesLower.includes(t));
                if (allMatch)
                    score = Math.round(score * 1.5);
            }
            if (score > 0) {
                const desc = entry.tool.description || "";
                scored.push({
                    name: entry.name,
                    description: desc.length > 200 ? desc.substring(0, 200) + "..." : desc,
                    required_params: extractRequiredParams(entry.tool.inputSchema),
                    relevance: score,
                    inputSchema: entry.tool.inputSchema,
                });
            }
        }
        scored.sort((a, b) => b.relevance - a.relevance);
        return scored.slice(0, clampToolSearchResults(maxResults));
    }
    /** Get tools by exact name prefix (e.g., "create_", "get_"). */
    byPrefix(prefix, maxResults = 30) {
        const lower = normalizeToolSearchQuery(prefix).toLowerCase();
        if (!lower)
            return [];
        return this.index
            .filter((e) => e.nameLower.startsWith(lower))
            .slice(0, Math.max(1, Math.min(MAX_TOOL_PREFIX_RESULTS, Math.trunc(maxResults) || 30)))
            .map((e) => e.name);
    }
    get size() {
        return this.index.length;
    }
}
/**
 * Decide whether a failed tool call should be auto-retried.
 * Only bridge_disconnected and bridge_timeout qualify.
 * Maximum 1 retry per call (attemptNumber must be 0).
 */
export function shouldSmartRetry(errorCategory, retryable, attemptNumber) {
    const NO = { shouldRetry: false, delayMs: 0, reason: "" };
    if (attemptNumber > 0)
        return NO;
    if (!retryable)
        return NO;
    if (errorCategory === "bridge_disconnected") {
        return {
            shouldRetry: true,
            delayMs: 2_000,
            reason: "bridge_disconnected — retrying after reconnect delay",
        };
    }
    if (errorCategory === "bridge_timeout") {
        return {
            shouldRetry: true,
            delayMs: 1_000,
            reason: "bridge_timeout — retrying",
        };
    }
    return NO;
}
// ---------------------------------------------------------------------------
// 3. ContextPropagator — track outputs, auto-fill missing params
// ---------------------------------------------------------------------------
// Per-tool extractors: which fields to pull from a successful result
const RESULT_EXTRACTORS = {
    spawn_actor: [
        { field: "label", contextKey: "last_actor_label" },
        { field: "actor_label", contextKey: "last_actor_label" },
        { field: /Actor\s+'([^']+)'/, contextKey: "last_actor_label" },
    ],
    create_static_mesh_actor: [
        { field: "label", contextKey: "last_actor_label" },
        { field: /label[:\s]+'?([^'"\s,}]+)/, contextKey: "last_actor_label" },
    ],
    create_light: [
        { field: "label", contextKey: "last_actor_label" },
    ],
    create_post_process_volume: [
        { field: "label", contextKey: "last_ppv_label" },
    ],
    create_landscape: [
        { field: "label", contextKey: "last_landscape_label" },
        { field: "landscape_label", contextKey: "last_landscape_label" },
    ],
    create_material: [
        { field: "path", contextKey: "last_material_path" },
        { field: "material_path", contextKey: "last_material_path" },
        { field: /\/Game\/[^\s'"]+/, contextKey: "last_material_path" },
    ],
    create_pbr_material: [
        { field: "path", contextKey: "last_material_path" },
        { field: /\/Game\/[^\s'"]+/, contextKey: "last_material_path" },
    ],
    create_material_instance: [
        { field: "path", contextKey: "last_material_instance_path" },
        { field: /\/Game\/[^\s'"]+/, contextKey: "last_material_instance_path" },
    ],
    create_landscape_material: [
        { field: "path", contextKey: "last_landscape_material_path" },
        { field: /\/Game\/[^\s'"]+/, contextKey: "last_landscape_material_path" },
    ],
    open_blueprint: [
        { field: "path", contextKey: "last_blueprint_path" },
        { field: "blueprint_path", contextKey: "last_blueprint_path" },
    ],
    create_blueprint: [
        { field: "path", contextKey: "last_blueprint_path" },
    ],
    create_level: [
        { field: "path", contextKey: "last_level_path" },
    ],
    create_landscape_grass_type: [
        { field: "path", contextKey: "last_grass_type_path" },
        { field: "name", contextKey: "last_grass_type_name" },
    ],
    // Vision tools — screenshot path for follow-up compare/analyze
    capture_viewport_sync: [
        { field: "screenshot_path", contextKey: "last_screenshot_path" },
        { field: "path", contextKey: "last_screenshot_path" },
    ],
    capture_viewport_safe: [
        { field: "screenshot_path", contextKey: "last_screenshot_path" },
        { field: "path", contextKey: "last_screenshot_path" },
    ],
    look_at_and_capture: [
        { field: "screenshot_path", contextKey: "last_screenshot_path" },
    ],
};
// Map context key → which tool params it can fill
const CONTEXT_TO_PARAMS = {
    last_actor_label: [
        {
            tools: [
                "move_actor",
                "rotate_actor",
                "scale_actor",
                "delete_actor",
                "get_actor_info",
                "focus_actor",
                "duplicate_actor",
                "set_actor_transform",
                "set_actor_property",
            ],
            param: "label",
        },
        {
            tools: ["set_actor_material", "set_actor_color"],
            param: "actor_label",
        },
        {
            tools: ["set_component_property"],
            param: "actor_name",
        },
        {
            tools: ["look_at_and_capture"],
            param: "target_label",
        },
    ],
    last_ppv_label: [
        {
            tools: ["set_post_process_settings"],
            param: "actor_name",
        },
    ],
    last_landscape_label: [
        {
            tools: [
                "sculpt_landscape",
                "paint_landscape_layer",
                "add_landscape_layer",
            ],
            param: "landscape_label",
        },
        {
            tools: ["apply_landscape_material"],
            param: "landscape_name",
        },
    ],
    last_material_path: [
        {
            tools: ["set_actor_material", "apply_landscape_material"],
            param: "material_path",
        },
        {
            tools: ["set_material_parameter"],
            param: "material_path",
        },
        {
            tools: ["create_material_instance"],
            param: "parent_path",
        },
    ],
    last_landscape_material_path: [
        {
            tools: ["apply_landscape_material"],
            param: "material_path",
        },
    ],
    last_blueprint_path: [
        {
            tools: ["compile_blueprint"],
            param: "blueprint_path",
        },
        {
            tools: ["open_blueprint"],
            param: "path",
        },
        {
            tools: ["add_blueprint_event"],
            param: "blueprint",
        },
    ],
    last_grass_type_path: [
        {
            tools: ["add_grass_variety"],
            param: "grass_type",
        },
    ],
    last_screenshot_path: [
        {
            tools: ["vision_compare", "analyze_scene_screenshot"],
            param: "before_path",
        },
    ],
};
function extractFromResult(result, extractors) {
    const extracted = {};
    for (const { field, contextKey } of extractors) {
        if (extracted[contextKey])
            continue; // first match wins
        if (typeof field === "string") {
            // Object field extraction
            if (typeof result === "object" && result !== null) {
                const val = result[field];
                if (typeof val === "string" && val.length > 0) {
                    extracted[contextKey] = val;
                }
            }
        }
        else {
            // Regex extraction from string or serialized result
            const text = typeof result === "string" ? result : JSON.stringify(result);
            const match = text.match(field);
            if (match) {
                extracted[contextKey] = match[1] || match[0];
            }
        }
    }
    return extracted;
}
function toSafeStringRecord(value) {
    const out = {};
    for (const [key, entry] of Object.entries(value)) {
        if (BLOCKED_CONTEXT_KEYS.has(key) || typeof entry !== "string") {
            continue;
        }
        out[key] = entry;
    }
    return out;
}
function toSafeUnknownRecord(value) {
    const out = {};
    for (const [key, entry] of Object.entries(value)) {
        if (BLOCKED_CONTEXT_KEYS.has(key)) {
            continue;
        }
        out[key] = entry;
    }
    return out;
}
export class ContextPropagator {
    context = {};
    /** Extract and store context values from a successful tool response. */
    extract(toolName, result) {
        const extractors = RESULT_EXTRACTORS[toolName];
        if (!extractors)
            return {};
        const extracted = toSafeStringRecord(extractFromResult(result, extractors));
        for (const [key, value] of Object.entries(extracted)) {
            this.context[key] = value;
        }
        return extracted;
    }
    /**
     * Inject stored context into missing tool parameters.
     * Returns augmented params and a map of inferred fields → source context key.
     * Never overwrites a param the agent already provided.
     */
    inject(toolName, params) {
        const inferred = {};
        const out = toSafeUnknownRecord(params);
        // Validation pattern for context values before injection.
        // Prevents second-order command injection from malicious tool output.
        const SAFE_LABEL = /^[A-Za-z0-9_\-. ]{1,256}$/;
        const SAFE_PATH = /^\/Game\/[A-Za-z0-9_\-./]{1,512}$/;
        for (const [contextKey, injections] of Object.entries(CONTEXT_TO_PARAMS)) {
            const value = this.context[contextKey];
            if (!value)
                continue;
            // Validate context value based on the type of data it represents
            const isPath = contextKey.includes("path") || contextKey.includes("asset");
            const pattern = isPath ? SAFE_PATH : SAFE_LABEL;
            if (!pattern.test(value))
                continue; // Reject invalid context values silently
            for (const { tools, param } of injections) {
                if (tools.includes(toolName) && out[param] === undefined) {
                    out[param] = value;
                    inferred[param] = contextKey;
                }
            }
        }
        return { params: out, inferred };
    }
    /** Get current context snapshot (for debugging/resources). */
    snapshot() {
        return toSafeStringRecord(this.context);
    }
    /** Restore context from a serialized snapshot. */
    restore(data) {
        this.context = toSafeStringRecord(data);
    }
    /** Clear all stored context. */
    clear() {
        this.context = {};
    }
    get size() {
        return Object.keys(this.context).length;
    }
}
//# sourceMappingURL=pipeline-intelligence.js.map