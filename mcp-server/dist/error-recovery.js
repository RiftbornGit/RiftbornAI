/**
 * Error Recovery — Local-state diagnostics for failed tool calls
 *
 * When a tool call fails, mines existing session state (scene change log,
 * context propagator, session tracker) for diagnostic context. Converts
 * 2-3 agent round-trips into 1 by providing:
 *
 * 1. "Did you mean?" suggestions for actor labels and asset paths
 * 2. Repeated-failure pattern detection and escalation
 * 3. Severity classification and ordered recovery steps
 *
 * All diagnostics are purely local — no additional bridge calls.
 */
// ─── Constants ────────────────────────────────────────────────────────────────
const ACTOR_PARAM_KEYS = ["label", "actor_label", "actor_name", "target", "name"];
const ASSET_PARAM_KEYS = ["path", "asset_path", "material_path", "blueprint_path", "mesh", "material"];
// ─── String Similarity ────────────────────────────────────────────────────────
/**
 * Levenshtein-based similarity for short strings.
 * Returns 0.0 (no match) to 1.0 (exact match). Case-insensitive.
 */
export function similarity(a, b) {
    const la = a.toLowerCase();
    const lb = b.toLowerCase();
    if (la === lb)
        return 1.0;
    const lenA = la.length;
    const lenB = lb.length;
    const maxLen = Math.max(lenA, lenB);
    if (maxLen === 0)
        return 1.0;
    // Single-row Levenshtein to save memory
    let prev = new Array(lenB + 1);
    let curr = new Array(lenB + 1);
    for (let j = 0; j <= lenB; j++)
        prev[j] = j;
    for (let i = 1; i <= lenA; i++) {
        curr[0] = i;
        for (let j = 1; j <= lenB; j++) {
            const cost = la[i - 1] === lb[j - 1] ? 0 : 1;
            curr[j] = Math.min(prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost);
        }
        [prev, curr] = [curr, prev];
    }
    return 1.0 - prev[lenB] / maxLen;
}
// ─── Actor Suggestions ────────────────────────────────────────────────────────
/** Extract known actor labels from scene mutation records. */
export function extractKnownActors(sceneRecords) {
    const actors = new Set();
    for (const rec of sceneRecords) {
        for (const key of ACTOR_PARAM_KEYS) {
            const val = rec.params[key];
            if (typeof val === "string" && val.length > 0) {
                actors.add(val);
            }
        }
    }
    return Array.from(actors);
}
/** Find similar actor labels. Up to 3, threshold ≥ 0.4, excludes exact matches. */
export function suggestActors(target, knownActors) {
    if (!target || knownActors.length === 0)
        return [];
    return knownActors
        .map(name => ({ name, score: similarity(target, name) }))
        .filter(x => x.score >= 0.4 && x.score < 1.0)
        .sort((a, b) => b.score - a.score)
        .slice(0, 3)
        .map(x => x.name);
}
// ─── Asset Suggestions ────────────────────────────────────────────────────────
/** Extract known asset paths from scene records and context state. */
export function extractKnownAssets(sceneRecords, contextState) {
    const assets = new Set();
    for (const rec of sceneRecords) {
        for (const key of ASSET_PARAM_KEYS) {
            const val = rec.params[key];
            if (typeof val === "string" && val.startsWith("/")) {
                assets.add(val);
            }
        }
    }
    if (contextState) {
        for (const val of Object.values(contextState)) {
            if (typeof val === "string" && val.startsWith("/Game/")) {
                assets.add(val);
            }
        }
    }
    return Array.from(assets);
}
/** Find similar asset paths by filename segment. Up to 3. */
export function suggestAssets(target, knownAssets) {
    if (!target || knownAssets.length === 0)
        return [];
    const targetFile = target.split("/").pop() ?? target;
    return knownAssets
        .map(p => ({
        path: p,
        score: similarity(targetFile, p.split("/").pop() ?? p),
    }))
        .filter(x => x.score >= 0.4 && x.score < 1.0)
        .sort((a, b) => b.score - a.score)
        .slice(0, 3)
        .map(x => x.path);
}
// ─── Repeated Failure Detection ───────────────────────────────────────────────
/**
 * Detect patterns in recent failures: same tool + same error, same tool +
 * different errors, or same error across different tools.
 */
export function detectRepeatedFailures(toolName, error, recent) {
    const failures = recent.filter(e => !e.ok);
    if (failures.length < 2)
        return null;
    // Same tool, same error (similarity > 0.8)
    const sameToolSameError = failures.filter(e => e.tool === toolName && e.error != null && similarity(e.error, error) > 0.8);
    if (sameToolSameError.length >= 2) {
        return {
            tool: toolName,
            count: sameToolSameError.length,
            lastError: error,
            pattern: "same_tool_same_error",
        };
    }
    // Same tool, different errors (3+ failures)
    const sameToolFails = failures.filter(e => e.tool === toolName);
    if (sameToolFails.length >= 3) {
        return {
            tool: toolName,
            count: sameToolFails.length,
            lastError: error,
            pattern: "same_tool_different_errors",
        };
    }
    // Same error across different tools (2+ tools)
    const sameError = failures.filter(e => e.error != null && similarity(e.error, error) > 0.8);
    if (sameError.length >= 2) {
        const tools = new Set(sameError.map(e => e.tool));
        if (tools.size >= 2) {
            return {
                tool: toolName,
                count: sameError.length,
                lastError: error,
                pattern: "same_error_different_tools",
            };
        }
    }
    return null;
}
// ─── Diagnostic Context ───────────────────────────────────────────────────────
/**
 * Build diagnostic context by mining local state.
 * No bridge calls — uses only data already collected by other pipeline modules.
 */
export function gatherDiagnosticContext(input) {
    const { category, toolName, params, error, sceneRecords, sessionRecords, contextState } = input;
    const result = {
        recovery_steps: [],
        severity: "medium",
    };
    if (category === "actor_not_found") {
        const target = findParamValue(params, ACTOR_PARAM_KEYS);
        if (target) {
            const known = extractKnownActors(sceneRecords);
            const similar = suggestActors(target, known);
            if (similar.length > 0) {
                result.actor_suggestions = similar;
                result.recovery_steps.push(`Actor '${target}' not found. Similar: ${similar.join(", ")}`);
            }
            else if (known.length > 0) {
                result.actor_suggestions = known.slice(0, 5);
                result.recovery_steps.push(`Actor '${target}' not found. Known actors: ${known.slice(0, 5).join(", ")}`);
            }
        }
        result.recovery_steps.push("Call find_actor_by_label to search the level.");
        result.severity = "low";
    }
    if (category === "asset_not_found") {
        const target = findParamValue(params, ASSET_PARAM_KEYS);
        if (target) {
            const known = extractKnownAssets(sceneRecords, contextState);
            const similar = suggestAssets(target, known);
            if (similar.length > 0) {
                result.asset_suggestions = similar;
                result.recovery_steps.push(`Asset '${target}' not found. Similar: ${similar.join(", ")}`);
            }
        }
        result.recovery_steps.push("Verify the asset path. Use list_assets to browse.");
        result.severity = "low";
    }
    if (category === "prerequisite_missing") {
        result.recovery_steps.push(`Prerequisite not met for '${toolName}'. Check error message for what must run first.`);
        result.severity = "medium";
    }
    if (category === "bridge_unreachable" || category === "bridge_timeout") {
        const bridgeFails = sessionRecords.filter(e => !e.ok && e.error != null && /timeout|unreachable|ECONNREFUSED/i.test(e.error)).length;
        if (bridgeFails >= 3) {
            result.recovery_steps.push(`Bridge failed ${bridgeFails} times recently. UE may have crashed — call diagnose_crash.`);
            result.severity = "high";
        }
        else {
            result.recovery_steps.push("Bridge may be temporarily overloaded. Wait and retry.");
        }
    }
    if (category === "ue_runtime_error") {
        result.recovery_steps.push("UE runtime error. Call diagnose_crash for structured diagnosis.");
        result.severity = "high";
    }
    // Repeated failure detection (can escalate severity)
    const repeated = detectRepeatedFailures(toolName, error, sessionRecords);
    if (repeated) {
        result.repeated_failure = repeated;
        if (repeated.pattern === "same_tool_same_error") {
            result.recovery_steps.push(`'${toolName}' failed ${repeated.count}× with same error. Try a different approach.`);
        }
        else if (repeated.pattern === "same_tool_different_errors") {
            result.recovery_steps.push(`'${toolName}' failed ${repeated.count}× with different errors. Tool may be misconfigured.`);
        }
        else {
            result.recovery_steps.push("Multiple tools failing with similar errors — possible systemic issue (bridge or UE crash).");
        }
        result.severity = "high";
    }
    // Fallback
    if (result.recovery_steps.length === 0) {
        result.recovery_steps.push(`Check error message and verify parameters with describe_tool('${toolName}').`);
    }
    return result;
}
// ─── Gate ─────────────────────────────────────────────────────────────────────
/** Whether diagnostics should be attached for this error category. */
export function shouldAttachDiagnostics(category, callCount) {
    if (category === "actor_not_found" || category === "asset_not_found")
        return true;
    if (category === "prerequisite_missing")
        return true;
    if (category === "ue_runtime_error")
        return true;
    if ((category === "bridge_unreachable" || category === "bridge_timeout") && callCount > 2)
        return true;
    return false;
}
// ─── Helpers ──────────────────────────────────────────────────────────────────
function findParamValue(params, keys) {
    for (const key of keys) {
        const val = params[key];
        if (typeof val === "string" && val.length > 0)
            return val;
    }
    return null;
}
//# sourceMappingURL=error-recovery.js.map