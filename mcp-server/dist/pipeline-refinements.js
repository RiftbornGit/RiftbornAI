/**
 * Pipeline Refinements — Round 21
 *
 * Ten targeted improvements across the dispatch pipeline:
 *
 *  1. canonicalizeOutput — Normalize inconsistent field names from C++ responses
 *  2. preflightEntityCheck — Verify referenced entities exist in scene state
 *  3. scopeDigest — Filter digest to match agent's current activity focus
 *  4. buildParamEcho — Echo actual params sent after all pipeline transformations
 *  5. SessionBookmarks — Named save points in the session timeline
 *  6. estimateToolDuration — Attach timing predictions from session history
 *  7. classifyParallelSafety — Classify which planned tools can run concurrently
 *  8. ResponseDeltaTracker — Track what changed between repeated reads
 *  9. buildContextSummary — Summarize what the context propagator knows
 * 10. FailureBudget — Circuit breaker for tools that keep failing
 */
import { createSanitizer, createToSafeRecord, PROTO_BLOCKED_KEYS } from "./sanitize-utils.js";
const sanitizePipelineValue = createSanitizer({ trackCircular: true, depthSentinel: "[MaxDepth]", circularSentinel: "[Circular]" });
const toSafeRecord = createToSafeRecord(sanitizePipelineValue);
// ============================================================================
// 1. Output Canonicalization
// ============================================================================
/**
 * Field name mappings: non-canonical → canonical.
 * The canonical form is what the context propagator and downstream
 * consumers expect (e.g., `label` for entities, `path` for assets).
 */
const FIELD_CANON = {
    actor_name: "label",
    actor_label: "label",
    material_path: "path",
    blueprint_path: "path",
    grass_type_path: "path",
};
/** Tools where a `name` field means an entity label (not an asset name). */
const NAME_IS_LABEL_TOOLS = new Set([
    "spawn_actor",
    "create_light",
    "create_static_mesh_actor",
    "duplicate_actor",
    "create_post_process_volume",
    "spawn_third_person_character",
    "create_character_from_third_person",
]);
/**
 * Add canonical field aliases to a tool result so downstream consumers
 * (context propagator, micro-verify, error recovery) see consistent names.
 * Original fields are preserved; canonical aliases are added only if absent.
 */
export function canonicalizeOutput(toolName, result) {
    const out = toSafeRecord(result);
    for (const [source, canonical] of Object.entries(FIELD_CANON)) {
        if (typeof out[source] === "string" && out[source] && !(canonical in out)) {
            out[canonical] = out[source];
        }
    }
    // "name" → "label" only for tools where name means entity label
    if (NAME_IS_LABEL_TOOLS.has(toolName) &&
        typeof out["name"] === "string" &&
        out["name"] &&
        !("label" in out)) {
        out["label"] = out["name"];
    }
    return out;
}
// ============================================================================
// 2. Preflight Entity Check
// ============================================================================
/** Tools that reference an entity expected to already exist. */
const ENTITY_REF_TOOLS = {
    move_actor: { paramKey: "label", kind: "actor" },
    rotate_actor: { paramKey: "label", kind: "actor" },
    scale_actor: { paramKey: "label", kind: "actor" },
    delete_actor: { paramKey: "label", kind: "actor" },
    set_actor_material: { paramKey: "label", kind: "actor" },
    set_actor_color: { paramKey: "label", kind: "actor" },
    set_actor_property: { paramKey: "label", kind: "actor" },
    set_component_property: { paramKey: "label", kind: "actor" },
    set_material_parameter: { paramKey: "material_name", kind: "material" },
};
/**
 * Check if a tool references an entity that was deleted in this session.
 * Returns a warning string if the entity was deleted, null otherwise.
 * Does NOT warn about entities never seen — they may predate the session.
 */
export function preflightEntityCheck(toolName, params, changes) {
    const ref = ENTITY_REF_TOOLS[toolName];
    if (!ref)
        return null;
    const target = params[ref.paramKey];
    if (typeof target !== "string" || !target)
        return null;
    // Walk changes chronologically; track create/delete state
    let lastAction = null;
    for (const change of changes) {
        if (change.label !== target)
            continue;
        if (change.kind === "create")
            lastAction = "create";
        if (change.kind === "delete")
            lastAction = "delete";
    }
    if (lastAction === "delete") {
        return `${ref.kind} "${target}" was deleted earlier this session. Create it again before calling ${toolName}.`;
    }
    return null;
}
// ============================================================================
// 3. Scope-Aware Digest
// ============================================================================
const FOCUS_PHASE_MAP = {
    create_landscape: "terrain", sculpt_landscape: "terrain",
    paint_landscape_layer: "terrain", create_landscape_material: "terrain",
    apply_landscape_material: "terrain",
    create_light: "lighting", create_post_process_volume: "lighting",
    set_post_process_settings: "lighting",
    create_material: "materials", create_pbr_material: "materials",
    create_material_instance: "materials", set_material_parameter: "materials",
    set_actor_material: "materials",
    paint_foliage: "foliage", add_foliage_instance: "foliage",
    create_landscape_grass_type: "foliage", add_grass_variety: "foliage",
    spawn_actor: "actors", move_actor: "actors", delete_actor: "actors",
    duplicate_actor: "actors", rotate_actor: "actors", scale_actor: "actors",
    create_blueprint: "gameplay", compile_blueprint: "gameplay",
    build_navmesh: "gameplay", create_character_from_third_person: "gameplay",
    create_metasound_source: "audio", spawn_audio_component: "audio",
    create_niagara_system: "vfx", spawn_niagara_at_location: "vfx",
    start_pie: "testing", stop_pie: "testing",
};
const BUILD_ORDER = [
    "terrain", "lighting", "materials", "foliage",
    "actors", "gameplay", "audio", "vfx", "testing",
];
/**
 * Detect the agent's current focus phase from recent tool calls.
 * Returns a phase only if ≥60% of the window belongs to one phase.
 */
export function detectCurrentFocus(recentTools, windowSize = 5) {
    const window = recentTools.slice(-windowSize);
    const counts = new Map();
    for (const tool of window) {
        const phase = FOCUS_PHASE_MAP[tool];
        if (phase)
            counts.set(phase, (counts.get(phase) ?? 0) + 1);
    }
    let best = null;
    let bestCount = 0;
    for (const [phase, count] of counts) {
        if (count > bestCount) {
            bestCount = count;
            best = phase;
        }
    }
    return best && bestCount >= Math.ceil(windowSize * 0.6) ? best : null;
}
/**
 * Filter a digest to only show milestones relevant to the agent's current
 * focus. Keeps the focused phase plus its neighbors in build order.
 * Returns the full digest unchanged if no clear focus is detected.
 */
export function scopeDigest(digest, recentTools) {
    const focus = detectCurrentFocus(recentTools);
    if (!focus)
        return digest;
    const relevant = new Set([focus, "testing"]);
    const idx = BUILD_ORDER.indexOf(focus);
    if (idx > 0)
        relevant.add(BUILD_ORDER[idx - 1]);
    if (idx < BUILD_ORDER.length - 1)
        relevant.add(BUILD_ORDER[idx + 1]);
    return {
        call_number: digest.call_number,
        milestones: digest.milestones.filter(m => relevant.has(m.phase)),
        phases_completed: [...digest.phases_completed],
        phases_missing: [...digest.phases_missing],
        error_patterns: [...digest.error_patterns],
        ...(digest.suggestion ? { suggestion: digest.suggestion } : {}),
        _focus: focus,
    };
}
/**
 * Build a param echo block showing what was actually sent to the bridge
 * after all pipeline transformations. Returns null if nothing was auto-filled.
 */
export function buildParamEcho(finalParams, defaultsApplied, inferred) {
    const dKeys = Object.keys(defaultsApplied);
    const iKeys = Object.keys(inferred);
    if (dKeys.length === 0 && iKeys.length === 0)
        return null;
    return {
        final_params: toSafeRecord(finalParams),
        defaults_applied: dKeys,
        context_inferred: iKeys,
    };
}
const MAX_BOOKMARKS = 50;
const MAX_BOOKMARK_NAME = 64;
export class SessionBookmarks {
    bookmarks = new Map();
    mark(name, changeId, timestamp = Date.now()) {
        const key = name.trim().slice(0, MAX_BOOKMARK_NAME).toLowerCase();
        if (!key)
            throw new Error("Bookmark name cannot be empty");
        // Evict oldest if at capacity
        if (this.bookmarks.size >= MAX_BOOKMARKS && !this.bookmarks.has(key)) {
            let oldestKey = null;
            let oldestTs = Infinity;
            for (const [k, bm] of this.bookmarks) {
                if (bm.timestamp < oldestTs) {
                    oldestTs = bm.timestamp;
                    oldestKey = k;
                }
            }
            if (oldestKey)
                this.bookmarks.delete(oldestKey);
        }
        const bookmark = { name: key, changeId, timestamp };
        this.bookmarks.set(key, bookmark);
        return bookmark;
    }
    get(name) {
        return this.bookmarks.get(name.trim().toLowerCase());
    }
    list() {
        return Array.from(this.bookmarks.values()).sort((a, b) => a.timestamp - b.timestamp);
    }
    /** Return changes that occurred after a named bookmark. */
    changesSince(name, allChanges) {
        const bm = this.get(name);
        if (!bm)
            return [];
        return allChanges.filter(c => c.id > bm.changeId);
    }
    clear() {
        this.bookmarks.clear();
    }
    toJSON() {
        const out = {};
        for (const [key, bookmark] of this.bookmarks) {
            if (!PROTO_BLOCKED_KEYS.has(key)) {
                out[key] = {
                    name: bookmark.name,
                    changeId: bookmark.changeId,
                    timestamp: bookmark.timestamp,
                };
            }
        }
        return out;
    }
    loadFrom(data) {
        this.bookmarks.clear();
        for (const [key, bm] of Object.entries(toSafeRecord(data))) {
            if (key
                && bm
                && typeof bm === "object"
                && !Array.isArray(bm)
                && typeof bm.changeId === "number") {
                this.bookmarks.set(key, {
                    name: typeof bm.name === "string" ? bm.name : key,
                    changeId: bm.changeId,
                    timestamp: typeof bm.timestamp === "number" ? bm.timestamp : 0,
                });
            }
        }
    }
}
const DURATION_BASELINES = {
    vision: 3000, build: 10000, spawn: 1500,
    query: 500, mutation: 1000, default: 2000,
};
function classifyToolDurationCategory(toolName) {
    const lower = toolName.toLowerCase();
    if (lower.startsWith("observe") || lower.startsWith("capture") || lower.startsWith("vision_") || lower.startsWith("analyze_scene"))
        return "vision";
    if (lower.startsWith("compile") || lower.startsWith("build"))
        return "build";
    if (lower.startsWith("spawn_") || lower.startsWith("create_"))
        return "spawn";
    if (lower.startsWith("get_") || lower.startsWith("list_") || lower.startsWith("find_"))
        return "query";
    if (lower.startsWith("set_") || lower.startsWith("move_") || lower.startsWith("delete_"))
        return "mutation";
    return "default";
}
/**
 * Predict how long a tool call will take, using historical session data
 * when available, falling back to category baselines.
 */
export function estimateToolDuration(toolName, recentEntries) {
    const history = recentEntries.filter(e => e.tool === toolName && e.ok && e.duration_ms > 0);
    if (history.length >= 3) {
        const avg = Math.round(history.reduce((s, e) => s + e.duration_ms, 0) / history.length);
        return { estimate_ms: avg, confidence: "high" };
    }
    if (history.length > 0) {
        const avg = Math.round(history.reduce((s, e) => s + e.duration_ms, 0) / history.length);
        return { estimate_ms: avg, confidence: "low" };
    }
    const cat = classifyToolDurationCategory(toolName);
    return { estimate_ms: DURATION_BASELINES[cat] ?? 2000, confidence: "none" };
}
// ============================================================================
// 7. Parallel Safety Classifier
// ============================================================================
const PARALLEL_SAFE_PREFIXES = ["get_", "list_", "find_", "search_", "describe_", "is_"];
/**
 * Classify a set of planned tools into those safe to run concurrently (reads)
 * and those that must run serially (mutations, vision, builds).
 * Matches the ToolExecutionQueue's read/write classification.
 */
export function classifyParallelSafety(tools) {
    const parallel = [];
    const serial = [];
    for (const tool of tools) {
        const lower = tool.toLowerCase();
        if (PARALLEL_SAFE_PREFIXES.some(p => lower.startsWith(p))) {
            parallel.push(tool);
        }
        else {
            serial.push(tool);
        }
    }
    return { parallel, serial };
}
// ============================================================================
// 8. Response Delta Tracker
// ============================================================================
/**
 * Tracks the last response per tool and computes deltas when the same
 * tool is called again. Only meaningful for observation/query tools.
 */
export class ResponseDeltaTracker {
    last = new Map();
    /**
     * Record a new result. Returns a delta object if this tool was called
     * before and the result changed, null otherwise.
     */
    track(toolName, result) {
        const prev = this.last.get(toolName);
        const safeResult = toSafeRecord(result);
        this.last.set(toolName, safeResult);
        if (!prev)
            return null;
        const delta = {};
        const allKeys = new Set([...Object.keys(prev), ...Object.keys(safeResult)]);
        for (const key of allKeys) {
            if (key.startsWith("_"))
                continue; // skip metadata
            const was = prev[key];
            const now = safeResult[key];
            if (JSON.stringify(was) !== JSON.stringify(now)) {
                delta[key] = { was, now };
            }
        }
        return Object.keys(delta).length > 0 ? delta : null;
    }
    clear() {
        this.last.clear();
    }
}
const ENTITY_KEYS = ["label", "actor_label", "landscape_label", "ppv_label", "character_label"];
const PATH_KEYS = ["path", "material_path", "blueprint_path", "grass_type_name", "landscape_material_path"];
/**
 * Build a compact summary of what the context propagator currently knows.
 * Useful for debugging why context injection did or didn't work.
 */
export function buildContextSummary(snapshot) {
    const safeSnapshot = toSafeRecord(snapshot);
    const entities = [];
    const paths = [];
    for (const [key, value] of Object.entries(safeSnapshot)) {
        if (typeof value !== "string" || !value)
            continue;
        if (ENTITY_KEYS.includes(key)) {
            entities.push(value);
        }
        else if (PATH_KEYS.includes(key) || value.startsWith("/Game/")) {
            paths.push(value);
        }
    }
    return {
        known_entities: [...new Set(entities)],
        known_paths: [...new Set(paths)],
        field_count: Object.keys(safeSnapshot).length,
    };
}
// ============================================================================
// 10. Failure Budget (Circuit Breaker)
// ============================================================================
const FAILURE_THRESHOLD = 3;
const DECAY_ON_SUCCESS = 2;
/**
 * Circuit breaker that blocks tools after repeated consecutive failures.
 * Prevents agents from wasting turns retrying broken tools.
 * Consecutive count decays on success, resets on explicit reset.
 */
export class FailureBudget {
    state = new Map();
    recordFailure(tool, error) {
        const cur = this.state.get(tool) ?? { consecutive: 0, total: 0 };
        cur.consecutive++;
        cur.total++;
        cur.lastError = error;
        this.state.set(tool, cur);
    }
    recordSuccess(tool) {
        const cur = this.state.get(tool);
        if (cur) {
            cur.consecutive = Math.max(0, cur.consecutive - DECAY_ON_SUCCESS);
        }
    }
    check(tool) {
        const s = this.state.get(tool);
        if (!s || s.consecutive < FAILURE_THRESHOLD) {
            return { blocked: false, failures: s?.consecutive ?? 0 };
        }
        return {
            blocked: true,
            reason: `${tool} has failed ${s.consecutive} consecutive times. Last error: ${s.lastError ?? "unknown"}. Try a different approach.`,
            failures: s.consecutive,
        };
    }
    getStatus() {
        const blocked = [];
        const atRisk = [];
        for (const [tool, s] of this.state) {
            if (s.consecutive >= FAILURE_THRESHOLD)
                blocked.push(tool);
            else if (s.consecutive >= 2)
                atRisk.push(tool);
        }
        return { blocked_tools: blocked, at_risk: atRisk };
    }
    reset(tool) {
        this.state.delete(tool);
    }
    clear() {
        this.state.clear();
    }
}
//# sourceMappingURL=pipeline-refinements.js.map