/**
 * Vision Intelligence — Structured analysis of vision AI responses
 *
 * Turns free-text vision observations into actionable structured data:
 * - Issue detection with severity, category, and suggested fix
 * - Scene diffing via actor snapshot comparison
 * - Verification tracking across the build loop
 */
import { createSanitizer, createToSafeRecord } from "./sanitize-utils.js";
const sanitizeVisionValue = createSanitizer({ trackCircular: true, depthSentinel: "[MaxDepth]", circularSentinel: "[Circular]" });
const toSafeRecord = createToSafeRecord(sanitizeVisionValue);
function getRecordField(record, key) {
    return record[key];
}
function getNestedRecord(record, key) {
    return toSafeRecord(record[key]);
}
const ISSUE_PATTERNS = [
    // Lighting — high severity when scene is unlit, medium for quality issues
    { pattern: /\b(?:too\s+dark|underlit|no\s+light(?:ing)?|pitch\s*black|very\s+dark|completely\s+dark)\b/i,
        category: "lighting", severity: "high", description: "Scene is too dark",
        suggested_tool: "create_light" },
    { pattern: /\b(?:too\s+bright|overexposed|blown\s+out|washed\s+out)\b/i,
        category: "lighting", severity: "medium", description: "Scene is overexposed" },
    { pattern: /\b(?:flat\s+light(?:ing)?|no\s+shadows?|lacks?\s+shadow|uniform\s+light(?:ing)?)\b/i,
        category: "lighting", severity: "medium", description: "Lighting lacks depth",
        suggested_tool: "create_light" },
    { pattern: /\b(?:uneven\s+exposure|inconsistent\s+exposure|exposure\s+mismatch|brightness\s+mismatch)\b/i,
        category: "lighting", severity: "medium", description: "Exposure feels inconsistent" },
    { pattern: /\b(?:color\s+temperature\s+mismatch|mixed\s+color\s+temperature|cool\s+and\s+warm\s+lights?\s+clash)\b/i,
        category: "lighting", severity: "medium", description: "Lighting color temperature feels inconsistent" },
    { pattern: /\b(?:weak\s+atmosphere|fog\s+disconnect|fog\s+feels?\s+detached|atmosphere\s+feels?\s+thin)\b/i,
        category: "lighting", severity: "medium", description: "Atmosphere integration feels weak" },
    // Materials — checkerboard/purple = missing texture = high
    { pattern: /\b(?:checker\s*board|default\s+material|missing\s+(?:material|texture)|no\s+material|purple\s+(?:and\s+black|texture|material))\b/i,
        category: "material", severity: "high", description: "Missing or default material",
        suggested_tool: "set_actor_material" },
    { pattern: /\b(?:texture\s+(?:stretch|tiling|repeat)|uv\s+(?:issue|stretch|problem)|stretched\s+texture)\b/i,
        category: "material", severity: "medium", description: "Material UV/tiling issue",
        suggested_tool: "set_material_parameter" },
    { pattern: /\b(?:too\s+shiny|overly\s+shiny|plastic|waxy|wet\s+look|specular\s+too\s+strong)\b/i,
        category: "material", severity: "medium", description: "Specular or roughness response looks implausible",
        suggested_tool: "set_material_parameter" },
    { pattern: /\b(?:too\s+rough|chalky|powdery|matte\s+everywhere|flat\s+material)\b/i,
        category: "material", severity: "medium", description: "Material roughness response looks implausible",
        suggested_tool: "set_material_parameter" },
    { pattern: /\b(?:metallic\s+mismatch|non[- ]metal(?:lic)?\s+looks?\s+metallic|metal\s+looks?\s+non[- ]metal(?:lic)?)\b/i,
        category: "material", severity: "high", description: "Metallic response looks incorrect",
        suggested_tool: "set_material_parameter" },
    { pattern: /\b(?:weak\s+material\s+definition|lacks?\s+material\s+definition|muddy\s+material)\b/i,
        category: "material", severity: "medium", description: "Material definition is weak",
        suggested_tool: "set_material_parameter" },
    // Placement — floating or clipping actors
    { pattern: /\b(?:floating|hovering|not\s+grounded|above\s+(?:the\s+)?ground|suspended\s+in\s+(?:the\s+)?air)\b/i,
        category: "placement", severity: "high", description: "Actor appears floating",
        suggested_tool: "move_actor" },
    { pattern: /\b(?:clipping|z-fight(?:ing)?|intersect(?:ing)?|inside\s+(?:the|each|another))\b/i,
        category: "placement", severity: "medium", description: "Actor clipping detected",
        suggested_tool: "move_actor" },
    { pattern: /\b(?:sunken|buried|bad\s+support\s+contact|not\s+touching\s+the\s+ground)\b/i,
        category: "placement", severity: "medium", description: "Actor grounding looks incorrect",
        suggested_tool: "move_actor" },
    // Scale
    { pattern: /\b(?:too\s+(?:big|large|huge)|oversized|enormous|disproportionate(?:ly)?\s+large)\b/i,
        category: "scale", severity: "medium", description: "Object appears too large",
        suggested_tool: "scale_actor" },
    { pattern: /\b(?:too\s+small|tiny|minuscule|barely\s+visible|disproportionate(?:ly)?\s+small)\b/i,
        category: "scale", severity: "medium", description: "Object appears too small",
        suggested_tool: "scale_actor" },
    // Missing content
    { pattern: /\b(?:empty\s+(?:area|space|level|scene)|sparse|barren|nothing\s+(?:here|there|visible)|bare\s+(?:ground|terrain|landscape))\b/i,
        category: "missing_content", severity: "medium", description: "Area appears empty",
        suggested_tool: "spawn_actor" },
    // Composition
    { pattern: /\b(?:cluttered|overcrowded|too\s+many\s+(?:actors?|objects?|meshes?))\b/i,
        category: "composition", severity: "low", description: "Scene feels overcrowded" },
    // Performance
    { pattern: /\b(?:frame\s*rate|low\s+fps|(?:fps|performance)\s+(?:issue|drop|problem)|lag(?:ging)?|stutter(?:ing)?)\b/i,
        category: "performance", severity: "high", description: "Performance concern" },
];
const POSITIVE_WORDS = /\b(?:good|great|excellent|nice|well|correct|proper|balanced|solid|beautiful|polished|clean|plausible|coherent|defined)\b/gi;
const NEGATIVE_WORDS = /\b(?:bad|poor|wrong|incorrect|missing|broken|ugly|issue|problem|needs?\s+(?:work|improvement|fix)|should\s+(?:be|have)|implausible|inconsistent)\b/gi;
// ============================================================================
// VisionResultParser — Extract structured issues from vision AI text
// ============================================================================
export function parseVisionText(text) {
    if (!text || typeof text !== "string") {
        return { issues: [], quality: "fair", actor_mentions: [] };
    }
    // Detect issues via pattern matching
    const issues = [];
    const seenCategories = new Set();
    for (const p of ISSUE_PATTERNS) {
        if (p.pattern.test(text) && !seenCategories.has(`${p.category}:${p.description}`)) {
            seenCategories.add(`${p.category}:${p.description}`);
            const issue = {
                category: p.category,
                severity: p.severity,
                description: p.description,
            };
            if (p.suggested_tool)
                issue.suggested_tool = p.suggested_tool;
            issues.push(issue);
        }
    }
    // Overall quality heuristic from positive vs negative language
    const positives = (text.match(POSITIVE_WORDS) || []).length;
    const negatives = (text.match(NEGATIVE_WORDS) || []).length;
    const highIssues = issues.filter(i => i.severity === "high").length;
    let quality;
    if (highIssues >= 2 || negatives > positives * 3)
        quality = "poor";
    else if (highIssues >= 1 || negatives > positives)
        quality = "fair";
    else if (positives > negatives * 2 && issues.length === 0)
        quality = "excellent";
    else
        quality = "good";
    // Extract actor names mentioned in the analysis (quoted strings after actor-related words)
    const actorPattern = /\b(?:actor|object|mesh|light|volume)\s+['"]([^'"]+)['"]/gi;
    const actor_mentions = [];
    let match;
    while ((match = actorPattern.exec(text)) !== null) {
        if (!actor_mentions.includes(match[1]))
            actor_mentions.push(match[1]);
    }
    return { issues, quality, actor_mentions };
}
// ============================================================================
// SceneDiffTracker — Track actor snapshots and produce diffs
// ============================================================================
const MOVE_THRESHOLD = 10; // UU — below this, consider "same position"
export class SceneDiffTracker {
    lastSnapshot = new Map();
    lastTimestamp = 0;
    /**
     * Record an actor list from an observation result.
     * Returns a diff if there was a previous snapshot, null otherwise.
     */
    update(actors) {
        const current = new Map();
        for (const a of actors) {
            if (a.label)
                current.set(a.label, a);
        }
        if (this.lastSnapshot.size === 0) {
            this.lastSnapshot = current;
            this.lastTimestamp = Date.now();
            return null; // First observation — no diff possible
        }
        const diff = {
            actors_added: [],
            actors_removed: [],
            actors_moved: [],
            timestamp: Date.now(),
        };
        // Find added and moved actors
        for (const [label, actor] of current) {
            const prev = this.lastSnapshot.get(label);
            if (!prev) {
                diff.actors_added.push(label);
            }
            else if (actor.location && prev.location) {
                const d = distance3d(prev.location, actor.location);
                if (d > MOVE_THRESHOLD) {
                    diff.actors_moved.push({ label, from: prev.location, to: actor.location });
                }
            }
        }
        // Find removed actors
        for (const label of this.lastSnapshot.keys()) {
            if (!current.has(label)) {
                diff.actors_removed.push(label);
            }
        }
        this.lastSnapshot = current;
        this.lastTimestamp = Date.now();
        return diff;
    }
    /** Get labels of all actors in the last snapshot. */
    get actorLabels() {
        return Array.from(this.lastSnapshot.keys());
    }
    /** Check if a specific actor was seen in the last observation. */
    has(label) {
        return this.lastSnapshot.has(label);
    }
    get snapshotSize() {
        return this.lastSnapshot.size;
    }
    clear() {
        this.lastSnapshot.clear();
        this.lastTimestamp = 0;
    }
}
function distance3d(a, b) {
    return Math.sqrt((a.x - b.x) ** 2 + (a.y - b.y) ** 2 + (a.z - b.z) ** 2);
}
// ============================================================================
// Vision context extraction — pull structured data from vision tool results
// ============================================================================
/**
 * Extract vision-related context values from a tool result.
 * Returns key-value pairs suitable for ContextPropagator storage.
 */
export function extractVisionContext(toolName, result) {
    if (!result || typeof result !== "object")
        return {};
    const obj = toSafeRecord(result);
    const ctx = {};
    // Screenshot path extraction
    const screenshotFields = ["screenshot_path", "screenshot", "path", "image_path", "filename"];
    const resultRecord = getNestedRecord(obj, "result");
    const observationRecord = getNestedRecord(obj, "observation");
    const observationResultRecord = getNestedRecord(observationRecord, "result");
    const analysisRecord = getNestedRecord(obj, "analysis");
    const analysisResultRecord = getNestedRecord(analysisRecord, "result");
    for (const field of screenshotFields) {
        const v = getRecordField(obj, field)
            ?? getRecordField(resultRecord, field)
            ?? getRecordField(observationRecord, field)
            ?? getRecordField(observationResultRecord, field);
        if (typeof v === "string" && v.length > 0 && (v.includes("/") || v.includes("\\"))) {
            ctx.last_screenshot_path = v;
            break;
        }
    }
    // Vision analysis text extraction
    const analysisFields = ["analysis", "vision_analysis", "description", "scene_description"];
    for (const field of analysisFields) {
        const v = getRecordField(obj, field)
            ?? getRecordField(resultRecord, field)
            ?? getRecordField(observationRecord, field)
            ?? getRecordField(observationResultRecord, field)
            ?? getRecordField(analysisRecord, field)
            ?? getRecordField(analysisResultRecord, field);
        if (typeof v === "string" && v.length > 10) {
            ctx.last_vision_summary = v.substring(0, 500);
            break;
        }
    }
    // Level name extraction from observe_ue_project
    if (toolName === "observe_ue_project" || toolName === "vision_observe") {
        const level = getRecordField(obj, "level_name")
            ?? getRecordField(resultRecord, "level_name")
            ?? getRecordField(observationResultRecord, "level_name");
        if (typeof level === "string" && level.length > 0) {
            ctx.last_observed_level = level;
        }
    }
    return ctx;
}
// ============================================================================
// enhanceVisionResponse — Post-process vision tool results
// ============================================================================
/**
 * Enhance a vision tool result with structured analysis.
 * Called after a vision tool returns but before the response is sent to the agent.
 *
 * Adds `_vision` block with parsed issues, quality score, and scene diff.
 */
export function enhanceVisionResponse(toolName, result, diffTracker) {
    if (!result || typeof result !== "object" || Array.isArray(result))
        return result;
    const obj = toSafeRecord(result);
    // Only enhance vision-related tools
    const visionTools = new Set([
        "observe_ue_project", "analyze_scene_screenshot", "look_at_and_capture",
        "capture_viewport_sync", "capture_viewport_safe",
        "vision_observe", "vision_compare", "vision_inspect_actor",
        "vision_playtest", "vision_sweep", "vision_build_and_verify",
    ]);
    if (!visionTools.has(toolName))
        return result;
    // Find the analysis text in the result tree
    const analysisText = findAnalysisText(obj);
    if (!analysisText)
        return result;
    const parsed = parseVisionText(analysisText);
    // Build vision enhancement block
    const vision = {};
    if (parsed.issues.length > 0)
        vision.issues = parsed.issues;
    vision.quality = parsed.quality;
    if (parsed.actor_mentions.length > 0)
        vision.actor_mentions = parsed.actor_mentions;
    // Scene diff from actor list (if tracker provided)
    if (diffTracker) {
        const actors = findActorList(obj);
        if (actors.length > 0) {
            const diff = diffTracker.update(actors);
            if (diff && (diff.actors_added.length > 0 || diff.actors_removed.length > 0 || diff.actors_moved.length > 0)) {
                vision.scene_diff = diff;
            }
        }
    }
    // Suggest next action based on highest-severity issue
    const highIssue = parsed.issues.find(i => i.severity === "high");
    if (highIssue?.suggested_tool) {
        vision.suggested_next = `Fix: ${highIssue.description} → use ${highIssue.suggested_tool}`;
    }
    return Object.assign({}, obj, { _vision: sanitizeVisionValue(vision) });
}
// ============================================================================
// Response shaping rules for vision tools
// ============================================================================
/** Fields to keep for each vision-related tool in shapeResponse */
export const VISION_KEEP_FIELDS = {
    observe_ue_project: [
        "ok", "screenshot_path", "analysis", "vision_analysis", "description",
        "scene_description", "actor_count", "actors", "level_name", "game_mode",
    ],
    analyze_scene_screenshot: [
        "ok", "screenshot_path", "analysis", "description", "prompt",
    ],
    look_at_and_capture: [
        "ok", "screenshot_path", "analysis", "description", "target",
    ],
    capture_viewport_sync: [
        "ok", "screenshot_path", "path", "width", "height",
    ],
    capture_viewport_safe: [
        "ok", "screenshot_path", "path", "width", "height",
    ],
};
// ============================================================================
// Helpers — dig into nested result structures to find key data
// ============================================================================
function findAnalysisText(obj) {
    // Direct fields
    for (const key of ["analysis", "vision_analysis", "description", "scene_description"]) {
        if (typeof obj[key] === "string" && obj[key].length > 10)
            return obj[key];
    }
    // Nested under .result
    if (obj.result && typeof obj.result === "object" && !Array.isArray(obj.result)) {
        for (const key of ["analysis", "vision_analysis", "description", "scene_description"]) {
            const result = obj.result;
            if (typeof result[key] === "string" && result[key].length > 10)
                return result[key];
        }
    }
    // Nested under .observation (vision_observe wraps observe_ue_project)
    if (obj.observation && typeof obj.observation === "object" && !Array.isArray(obj.observation)) {
        return findAnalysisText(obj.observation);
    }
    // Nested under .inspection (vision_inspect_actor)
    if (obj.inspection && typeof obj.inspection === "object" && !Array.isArray(obj.inspection)) {
        return findAnalysisText(obj.inspection);
    }
    // visual_verification from vision_build_and_verify
    if (obj.visual_verification && typeof obj.visual_verification === "object" && !Array.isArray(obj.visual_verification)) {
        return findAnalysisText(obj.visual_verification);
    }
    // views array from vision_sweep — concatenate analyses
    if (Array.isArray(obj.views)) {
        const texts = [];
        for (const v of obj.views) {
            if (v?.observation && typeof v.observation === "object" && !Array.isArray(v.observation)) {
                const t = findAnalysisText(v.observation);
                if (t)
                    texts.push(t);
            }
        }
        if (texts.length > 0)
            return texts.join(" | ");
    }
    return null;
}
function findActorList(obj) {
    // Direct .actors array
    if (Array.isArray(obj.actors))
        return normalizeActors(obj.actors);
    // Nested in .result
    if (obj.result && typeof obj.result === "object" && !Array.isArray(obj.result)) {
        const result = obj.result;
        if (Array.isArray(result.actors))
            return normalizeActors(result.actors);
    }
    // Nested in .observation
    if (obj.observation && typeof obj.observation === "object" && !Array.isArray(obj.observation)) {
        return findActorList(obj.observation);
    }
    return [];
}
function normalizeActors(raw) {
    return raw
        .filter((a) => Boolean(a) && typeof a === "object" && !Array.isArray(a) && typeof a.label === "string")
        .map((a) => ({
        label: a.label,
        class: typeof a.class === "string" ? a.class : typeof a.class_name === "string" ? a.class_name : undefined,
        location: a.location && typeof a.location === "object" && !Array.isArray(a.location) && typeof a.location.x === "number"
            ? a.location
            : undefined,
    }));
}
//# sourceMappingURL=vision-intelligence.js.map