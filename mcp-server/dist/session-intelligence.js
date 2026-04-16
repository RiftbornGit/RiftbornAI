/**
 * Session Intelligence — Ambient awareness for long build sessions
 *
 * Three capabilities that keep agents oriented:
 *
 * 1. ProgressTracker — Maps tool calls to build milestones. Tracks what
 *    phase the project is in (terrain → lighting → materials → actors → gameplay).
 *
 * 2. ErrorPatternDetector — Spots consecutive failures, bridge instability,
 *    and repeated identical errors. Surfaces warnings before agents waste turns.
 *
 * 3. SessionDigest — Compact summary attached to every Nth response.
 *    Shows progress, open issues, and what to consider next.
 */
// ============================================================================
// Tool → Phase mapping
// ============================================================================
const TOOL_PHASE_MAP = {
    // Terrain
    create_landscape: { phase: "terrain", label: "Landscape created" },
    sculpt_landscape: { phase: "terrain", label: "Terrain sculpted" },
    create_landscape_material: { phase: "terrain", label: "Terrain material created" },
    apply_landscape_material: { phase: "terrain", label: "Terrain material applied" },
    paint_landscape_layer: { phase: "terrain", label: "Terrain layer painted" },
    // Lighting
    create_light: { phase: "lighting", label: "Light placed" },
    create_post_process_volume: { phase: "lighting", label: "Post-process volume created" },
    set_post_process_settings: { phase: "lighting", label: "Post-process configured" },
    // Materials
    create_material: { phase: "materials", label: "Material created" },
    create_pbr_material: { phase: "materials", label: "PBR material created" },
    create_material_instance: { phase: "materials", label: "Material instance created" },
    set_actor_material: { phase: "materials", label: "Material applied to actor" },
    set_material_parameter: { phase: "materials", label: "Material parameter set" },
    // Foliage
    paint_foliage: { phase: "foliage", label: "Foliage painted" },
    add_foliage_instance: { phase: "foliage", label: "Foliage instance added" },
    create_landscape_grass_type: { phase: "foliage", label: "Grass type created" },
    add_grass_variety: { phase: "foliage", label: "Grass variety added" },
    // Actors
    spawn_actor: { phase: "actors", label: "Actor spawned" },
    duplicate_actor: { phase: "actors", label: "Actor duplicated" },
    create_static_mesh_actor: { phase: "actors", label: "Static mesh placed" },
    // Gameplay
    create_blueprint: { phase: "gameplay", label: "Blueprint created" },
    compile_blueprint: { phase: "gameplay", label: "Blueprint compiled" },
    build_navmesh: { phase: "gameplay", label: "NavMesh built" },
    create_character_from_third_person: { phase: "gameplay", label: "Character created" },
    start_pie: { phase: "testing", label: "PIE playtest started" },
    // Audio
    create_metasound_source: { phase: "audio", label: "MetaSound created" },
    spawn_audio_component: { phase: "audio", label: "Audio emitter placed" },
    // VFX
    create_niagara_system: { phase: "vfx", label: "Niagara system created" },
    spawn_niagara_at_location: { phase: "vfx", label: "VFX placed" },
};
const BUILD_ORDER = [
    "terrain", "lighting", "materials", "foliage", "actors", "gameplay", "audio", "vfx", "testing",
];
// ============================================================================
// ProgressTracker
// ============================================================================
export class ProgressTracker {
    milestones = [];
    completedPhases = new Set();
    /** Record a successful tool call. Returns a milestone if the tool maps to a build phase. */
    record(toolName, timestamp) {
        const mapping = TOOL_PHASE_MAP[toolName];
        if (!mapping)
            return null;
        const milestone = {
            phase: mapping.phase,
            label: mapping.label,
            toolName,
            timestamp,
        };
        this.milestones.push(milestone);
        this.completedPhases.add(mapping.phase);
        return milestone;
    }
    /** Get all milestones in order. */
    getMilestones() {
        return [...this.milestones];
    }
    /** Get phases that have been touched. */
    getCompletedPhases() {
        return BUILD_ORDER.filter(p => this.completedPhases.has(p));
    }
    /** Get phases that haven't been touched, in build order. */
    getMissingPhases() {
        return BUILD_ORDER.filter(p => !this.completedPhases.has(p));
    }
    /** Get the most recent N milestones. */
    recent(n = 5) {
        return this.milestones.slice(-n);
    }
    get totalMilestones() {
        return this.milestones.length;
    }
    clear() {
        this.milestones = [];
        this.completedPhases.clear();
    }
    /** Serialize milestones for persistence. */
    serialize() {
        return [...this.milestones];
    }
    /** Restore from persisted milestones. Rebuilds completedPhases automatically. */
    restoreFromMilestones(data) {
        this.milestones = data ?? [];
        this.completedPhases.clear();
        for (const m of this.milestones)
            this.completedPhases.add(m.phase);
    }
}
// ============================================================================
// ErrorPatternDetector
// ============================================================================
const BRIDGE_ERROR_PATTERNS = /bridge|timeout|ECONNREFUSED|ECONNRESET|EPIPE|socket hang up/i;
export function detectErrorPatterns(entries) {
    const patterns = [];
    if (entries.length === 0)
        return patterns;
    // 1. Consecutive failure streak (same tool failing 3+ times)
    const recentFails = [];
    for (let i = entries.length - 1; i >= 0; i--) {
        if (!entries[i].ok)
            recentFails.unshift(entries[i]);
        else
            break;
    }
    if (recentFails.length >= 3) {
        const toolName = recentFails[recentFails.length - 1].tool;
        const allSameTool = recentFails.every(e => e.tool === toolName);
        patterns.push({
            type: "streak",
            message: allSameTool
                ? `${toolName} has failed ${recentFails.length} consecutive times. Try different parameters or check bridge health.`
                : `${recentFails.length} consecutive tool failures. The bridge may be unhealthy.`,
            tool: allSameTool ? toolName : undefined,
            count: recentFails.length,
        });
    }
    // 2. Bridge instability — multiple bridge-related errors in recent history
    const bridgeErrors = entries.filter(e => !e.ok && e.error && BRIDGE_ERROR_PATTERNS.test(e.error));
    if (bridgeErrors.length >= 3) {
        patterns.push({
            type: "bridge_instability",
            message: `${bridgeErrors.length} bridge-related errors in session. Connection may be unstable.`,
            count: bridgeErrors.length,
        });
    }
    // 3. Repeated identical error message (same error text appearing 3+ times)
    const errorCounts = new Map();
    for (const e of entries) {
        if (!e.ok && e.error) {
            const key = e.error.substring(0, 100);
            const existing = errorCounts.get(key);
            if (existing) {
                existing.count++;
            }
            else {
                errorCounts.set(key, { count: 1, tool: e.tool });
            }
        }
    }
    for (const [msg, { count, tool }] of errorCounts) {
        if (count >= 3) {
            patterns.push({
                type: "repeated_error",
                message: `Same error repeated ${count} times: "${msg.substring(0, 80)}"`,
                tool,
                count,
            });
        }
    }
    return patterns;
}
// ============================================================================
// Suggestion generator
// ============================================================================
const PHASE_SUGGESTIONS = {
    terrain: "Consider creating terrain with create_landscape and sculpt_landscape.",
    lighting: "Scene needs lighting. Add a directional light with create_light for a sun, and a sky light for ambient.",
    materials: "Apply materials to your actors. Use create_material or create_pbr_material.",
    foliage: "Consider adding foliage with create_landscape_grass_type for ground cover and paint_foliage for trees.",
    actors: "Place gameplay-relevant actors with spawn_actor.",
    gameplay: "Add gameplay elements: Blueprints, NavMesh, characters.",
    audio: "Consider adding audio with create_metasound_source and spawn_audio_component.",
    vfx: "Consider adding VFX with create_niagara_system.",
    testing: "Test your level with start_pie to verify it plays correctly.",
};
export function generateSuggestion(completedPhases, missingPhases, errorPatterns) {
    // Priority 1: Address error patterns
    if (errorPatterns.some(p => p.type === "streak" && p.count >= 5)) {
        return "Multiple consecutive failures detected. Consider using a different approach or checking bridge connectivity.";
    }
    if (errorPatterns.some(p => p.type === "bridge_instability")) {
        return "Bridge appears unstable. Verify UE5 is running and the HTTP bridge is responsive.";
    }
    // Priority 2: Suggest next build phase
    if (missingPhases.length === 0)
        return undefined;
    // Find the first missing phase in build order
    const nextPhase = missingPhases[0];
    // Special case: don't suggest terrain if we already have actors (might be working on existing level)
    if (nextPhase === "terrain" && completedPhases.includes("actors")) {
        return missingPhases.length > 1 ? PHASE_SUGGESTIONS[missingPhases[1]] : undefined;
    }
    return PHASE_SUGGESTIONS[nextPhase];
}
// ============================================================================
// SessionDigest — Compact summary for periodic injection
// ============================================================================
/**
 * Build a session digest from the progress tracker and session entries.
 * This is the main entry point for assembling the digest.
 */
export function buildDigest(callNumber, progress, recentEntries) {
    const milestones = progress.recent(5);
    const completedPhases = progress.getCompletedPhases();
    const missingPhases = progress.getMissingPhases();
    const errorPatterns = detectErrorPatterns(recentEntries);
    const suggestion = generateSuggestion(completedPhases, missingPhases, errorPatterns);
    const digest = {
        call_number: callNumber,
        milestones,
        phases_completed: completedPhases,
        phases_missing: missingPhases,
        error_patterns: errorPatterns,
    };
    if (suggestion) {
        digest.suggestion = suggestion;
    }
    return digest;
}
/**
 * Decide whether to attach a digest to this response.
 * Attaches every `interval` calls, or when an error pattern is detected.
 */
export function shouldAttachDigest(callNumber, interval, recentEntries) {
    // Always attach on interval boundary
    if (callNumber > 0 && callNumber % interval === 0)
        return true;
    // Attach early if there's an error streak of 3+
    const trailingFails = countTrailingFailures(recentEntries);
    if (trailingFails >= 3)
        return true;
    return false;
}
function countTrailingFailures(entries) {
    let count = 0;
    for (let i = entries.length - 1; i >= 0; i--) {
        if (entries[i].ok)
            break;
        count++;
    }
    return count;
}
//# sourceMappingURL=session-intelligence.js.map