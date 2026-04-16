/**
 * Scene Safety — Change tracking, budget awareness, and undo planning
 *
 * Three capabilities that keep agents from overloading the scene:
 *
 * 1. SceneChangeLog — Records every mutation (create, delete, move, modify).
 *    Provides structured undo plans that reverse recent operations.
 *
 * 2. SceneBudget — Tracks running counts of actors, lights, materials, etc.
 *    Warns when the scene approaches UE performance limits.
 *
 * 3. ConflictDetector — Catches contradictory calls (e.g., deleting an actor
 *    that was just created, moving something that was already deleted).
 */
import { createSanitizer, createToSafeRecord, PROTO_BLOCKED_KEYS } from "./sanitize-utils.js";
const sanitizeSnapshotValue = createSanitizer({ maxDepth: 20, trackCircular: true, depthSentinel: "[MaxDepth]", circularSentinel: "[Circular]" });
const toSafeParamsRecord = createToSafeRecord(sanitizeSnapshotValue);
function cloneSceneChange(change) {
    return {
        ...change,
        params: toSafeParamsRecord(change.params),
    };
}
function sanitizeCountsRecord(counts) {
    const out = {};
    for (const [key, value] of Object.entries(counts ?? {})) {
        if (!PROTO_BLOCKED_KEYS.has(key) && Number.isFinite(value) && value >= 0) {
            out[key] = Math.trunc(value);
        }
    }
    return out;
}
function sanitizeSceneChange(value) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
        return null;
    }
    const raw = value;
    const id = typeof raw.id === "number" && Number.isFinite(raw.id) ? Math.trunc(raw.id) : null;
    const kind = raw.kind;
    const tool = raw.tool;
    const category = raw.category;
    const timestamp = typeof raw.timestamp === "number" && Number.isFinite(raw.timestamp)
        ? raw.timestamp
        : null;
    if (id === null
        || typeof kind !== "string"
        || typeof tool !== "string"
        || typeof category !== "string"
        || timestamp === null) {
        return null;
    }
    return {
        id,
        kind: kind,
        tool,
        label: typeof raw.label === "string" ? raw.label : undefined,
        category: category,
        params: toSafeParamsRecord(raw.params),
        timestamp,
    };
}
const TOOL_CLASSIFICATIONS = {
    // Creates
    spawn_actor: { kind: "create", category: "actor", labelParam: "label" },
    create_static_mesh_actor: { kind: "create", category: "actor", labelParam: "label" },
    duplicate_actor: { kind: "create", category: "actor", labelParam: "label" },
    spawn_third_person_character: { kind: "create", category: "actor", labelParam: "label" },
    create_character_from_third_person: { kind: "create", category: "actor", labelParam: "label" },
    create_light: { kind: "create", category: "light", labelParam: "label" },
    create_landscape: { kind: "create", category: "landscape" },
    create_material: { kind: "create", category: "material", labelParam: "name" },
    create_pbr_material: { kind: "create", category: "material", labelParam: "name" },
    create_material_instance: { kind: "create", category: "material", labelParam: "name" },
    create_post_process_volume: { kind: "create", category: "postprocess" },
    create_niagara_system: { kind: "create", category: "vfx", labelParam: "name" },
    spawn_niagara_at_location: { kind: "create", category: "vfx" },
    spawn_niagara_attached: { kind: "create", category: "vfx" },
    add_foliage_instance: { kind: "create", category: "foliage" },
    paint_foliage: { kind: "create", category: "foliage" },
    spawn_audio_component: { kind: "create", category: "audio" },
    create_blueprint: { kind: "create", category: "blueprint", labelParam: "name" },
    // Deletes
    delete_actor: { kind: "delete", category: "actor", labelParam: "label" },
    // Moves
    move_actor: { kind: "move", category: "actor", labelParam: "label" },
    set_actor_transform: { kind: "move", category: "actor", labelParam: "label" },
    rotate_actor: { kind: "move", category: "actor", labelParam: "label" },
    scale_actor: { kind: "move", category: "actor", labelParam: "label" },
    // Modifications
    set_actor_material: { kind: "modify", category: "material", labelParam: "label" },
    set_actor_color: { kind: "modify", category: "actor", labelParam: "label" },
    set_actor_property: { kind: "modify", category: "actor", labelParam: "label" },
    set_component_property: { kind: "modify", category: "actor", labelParam: "label" },
    set_material_parameter: { kind: "modify", category: "material", labelParam: "material_name" },
    set_post_process_settings: { kind: "modify", category: "postprocess" },
    compile_blueprint: { kind: "modify", category: "blueprint", labelParam: "blueprint_name" },
    sculpt_landscape: { kind: "modify", category: "landscape" },
    paint_landscape_layer: { kind: "modify", category: "landscape" },
};
// ============================================================================
// Budget thresholds — tuned for typical UE5 scenes
// ============================================================================
const BUDGET_THRESHOLDS = {
    actor: { warn: 200, error: 500 },
    light: { warn: 8, error: 20 },
    material: { warn: 50, error: 150 },
    foliage: { warn: 100, error: 300 },
    vfx: { warn: 10, error: 30 },
    audio: { warn: 15, error: 40 },
    landscape: { warn: 2, error: 5 },
    postprocess: { warn: 3, error: 8 },
    blueprint: { warn: 30, error: 80 },
    other: { warn: 100, error: 500 },
};
// ============================================================================
// Undo templates — how to reverse each kind of change
// ============================================================================
const UNDO_TEMPLATES = {
    spawn_actor: (c) => c.label ? {
        description: `Delete actor "${c.label}"`,
        tool: "delete_actor",
        params: { label: c.label },
    } : null,
    create_static_mesh_actor: (c) => c.label ? {
        description: `Delete static mesh "${c.label}"`,
        tool: "delete_actor",
        params: { label: c.label },
    } : null,
    duplicate_actor: (c) => c.label ? {
        description: `Delete duplicated actor "${c.label}"`,
        tool: "delete_actor",
        params: { label: c.label },
    } : null,
    create_light: (c) => c.label ? {
        description: `Delete light "${c.label}"`,
        tool: "delete_actor",
        params: { label: c.label },
    } : null,
    create_post_process_volume: (c) => c.label ? {
        description: `Delete post-process volume "${c.label}"`,
        tool: "delete_actor",
        params: { label: c.label },
    } : null,
    delete_actor: (c) => ({
        description: `Re-create deleted actor "${c.label ?? "unknown"}" (manual — original params may be lost)`,
        tool: "spawn_actor",
        params: (() => {
            const params = toSafeParamsRecord(c.params);
            params.label = c.label ?? "unknown";
            return params;
        })(),
    }),
    move_actor: (c) => {
        // Can only undo if we stored the original position (from params)
        const label = c.label;
        return label ? {
            description: `Move "${label}" back (check original position)`,
            tool: "move_actor",
            params: { label },
        } : null;
    },
    set_actor_transform: (c) => c.label ? {
        description: `Restore transform for "${c.label}" (check original)`,
        tool: "set_actor_transform",
        params: { label: c.label },
    } : null,
    rotate_actor: (c) => c.label ? {
        description: `Restore rotation for "${c.label}" (check original)`,
        tool: "rotate_actor",
        params: { label: c.label },
    } : null,
    scale_actor: (c) => c.label ? {
        description: `Restore scale for "${c.label}" (check original)`,
        tool: "scale_actor",
        params: { label: c.label },
    } : null,
};
// ============================================================================
// SceneChangeLog
// ============================================================================
const MAX_CHANGES = 200;
export class SceneChangeLog {
    changes = [];
    nextId = 1;
    counts = new Map();
    /** Record a tool call as a scene change. Returns the change if tracked. */
    record(toolName, params, timestamp) {
        const cls = TOOL_CLASSIFICATIONS[toolName];
        if (!cls)
            return null;
        const label = cls.labelParam
            ? params[cls.labelParam]
            : undefined;
        const change = {
            id: this.nextId++,
            kind: cls.kind,
            tool: toolName,
            label,
            category: cls.category,
            params: toSafeParamsRecord(params),
            timestamp,
        };
        this.changes.push(change);
        if (this.changes.length > MAX_CHANGES) {
            this.changes.shift();
        }
        // Update running counts
        if (cls.kind === "create") {
            this.counts.set(cls.category, (this.counts.get(cls.category) ?? 0) + 1);
        }
        else if (cls.kind === "delete") {
            const cur = this.counts.get(cls.category) ?? 0;
            if (cur > 0)
                this.counts.set(cls.category, cur - 1);
        }
        return change;
    }
    /** Get the last N changes (most recent first). */
    recent(n = 10) {
        return this.changes.slice(-n).reverse();
    }
    /** Get running budget report with warnings. */
    getBudget() {
        const entries = [];
        const warnings = [];
        for (const [cat, thresholds] of Object.entries(BUDGET_THRESHOLDS)) {
            const category = cat;
            const count = this.counts.get(category) ?? 0;
            if (count === 0)
                continue;
            let status = "ok";
            if (count >= thresholds.error) {
                status = "over_budget";
                warnings.push(`${category}: ${count} exceeds limit of ${thresholds.error}. Scene may have performance issues.`);
            }
            else if (count >= thresholds.warn) {
                status = "warning";
                warnings.push(`${category}: ${count} approaching limit of ${thresholds.error}. Consider consolidating.`);
            }
            entries.push({
                category,
                count,
                warn_at: thresholds.warn,
                error_at: thresholds.error,
                status,
            });
        }
        return { entries, warnings };
    }
    /** Generate an undo plan for the last N changes. */
    getUndoPlan(n = 5) {
        const recent = this.changes.slice(-n).reverse();
        const steps = [];
        for (const change of recent) {
            const templateFn = UNDO_TEMPLATES[change.tool];
            if (templateFn) {
                const step = templateFn(change);
                if (step)
                    steps.push(step);
            }
        }
        return steps;
    }
    /** Check if a proposed tool call conflicts with recent changes. */
    detectConflict(toolName, params) {
        const cls = TOOL_CLASSIFICATIONS[toolName];
        if (!cls?.labelParam)
            return null;
        const label = params[cls.labelParam];
        if (!label)
            return null;
        // Look for contradictions in recent changes
        for (let i = this.changes.length - 1; i >= 0; i--) {
            const prev = this.changes[i];
            if (prev.label !== label)
                continue;
            // Deleting something that was just deleted
            if (cls.kind === "delete" && prev.kind === "delete") {
                return {
                    description: `Actor "${label}" was already deleted (change #${prev.id})`,
                    recent_change_id: prev.id,
                };
            }
            // Modifying/moving something that was deleted
            if ((cls.kind === "modify" || cls.kind === "move") && prev.kind === "delete") {
                return {
                    description: `Actor "${label}" was deleted (change #${prev.id}) — cannot ${cls.kind}`,
                    recent_change_id: prev.id,
                };
            }
            // Creating something with the same label as something that already exists
            if (cls.kind === "create" && prev.kind === "create") {
                return {
                    description: `"${label}" was already created (change #${prev.id}) — will produce a duplicate`,
                    recent_change_id: prev.id,
                };
            }
            // Found the most recent relevant change — stop looking
            break;
        }
        return null;
    }
    /** Build a compact summary for injection into tool responses. */
    buildSafetyBlock() {
        const budget = this.getBudget();
        if (budget.warnings.length === 0 && this.changes.length === 0)
            return null;
        const block = {};
        if (budget.warnings.length > 0) {
            block.budget_warnings = budget.warnings;
        }
        if (this.changes.length > 0) {
            block.recent_changes = this.changes.length;
            block.undo_available = this.getUndoPlan(3).length > 0;
        }
        return block;
    }
    /** Reset state (for testing or level changes). */
    reset() {
        this.changes = [];
        this.nextId = 1;
        this.counts.clear();
    }
    /** Serialize state for persistence. */
    serialize() {
        const counts = {};
        for (const [k, v] of this.counts)
            counts[k] = v;
        return {
            changes: this.changes.map((change) => cloneSceneChange(change)),
            nextId: this.nextId,
            counts: sanitizeCountsRecord(counts),
        };
    }
    /** Restore state from a persisted snapshot. */
    restore(data) {
        this.changes = (data.changes ?? [])
            .map((change) => sanitizeSceneChange(change))
            .filter((change) => change !== null)
            .slice(-MAX_CHANGES);
        this.nextId = typeof data.nextId === "number" && Number.isFinite(data.nextId)
            ? Math.max(1, Math.trunc(data.nextId))
            : 1;
        this.counts.clear();
        for (const [k, v] of Object.entries(sanitizeCountsRecord(data.counts))) {
            if (k in BUDGET_THRESHOLDS) {
                this.counts.set(k, v);
            }
        }
    }
}
//# sourceMappingURL=scene-safety.js.map