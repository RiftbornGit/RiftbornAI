/**
 * Performance Intelligence — Operational awareness from session timing data
 *
 * Turns raw SessionTracker entries into actionable intelligence:
 *
 * 1. LatencyTracker — Rolling per-category stats (P50, P95, call count, error rate).
 *    Categories: vision (screenshots/observe), build (compile/lighting), spawn
 *    (actor/light creation), query (reads/gets), mutation (set/modify), and default.
 *
 * 2. detectAnomalies — Compares recent calls against rolling baselines.
 *    Catches latency spikes, error rate spikes, and bridge degradation.
 *
 * 3. estimateDuration — Predicts how long a tool call will take based on
 *    category baseline + actual historical data for that specific tool.
 *
 * 4. buildHealthSummary — Compact health block for response injection:
 *    { calls, avg_ms, error_rate_pct, trend, anomalies? }
 */
function sanitizeSessionEntry(value) {
    const entry = {
        tool: typeof value.tool === "string" ? value.tool : "",
        ok: Boolean(value.ok),
        duration_ms: Number.isFinite(value.duration_ms) ? Math.max(0, Math.trunc(value.duration_ms)) : 0,
        timestamp: Number.isFinite(value.timestamp) ? value.timestamp : 0,
    };
    if (typeof value.error === "string") {
        entry.error = value.error;
    }
    if (typeof value.cached === "boolean") {
        entry.cached = value.cached;
    }
    return entry;
}
// ============================================================================
// Tool → Category classification
// ============================================================================
const VISION_PREFIXES = [
    "observe", "analyze_scene", "capture_viewport", "look_at_and_capture",
    "take_screenshot", "vision_",
];
const BUILD_PREFIXES = [
    "compile_blueprint", "build_lighting", "build_navmesh", "hot_reload",
    "trigger_live_coding", "cook_project", "package_project", "recompile",
];
const SPAWN_PREFIXES = [
    "spawn_", "create_light", "create_landscape", "create_post_process",
    "create_static_mesh", "create_niagara", "duplicate_actor", "add_foliage",
    "paint_foliage", "create_character", "spawn_third_person",
];
const QUERY_PREFIXES = [
    "get_", "list_", "find_", "search_", "describe_", "inspect_",
];
export function classifyTool(toolName) {
    const lower = toolName.toLowerCase();
    if (VISION_PREFIXES.some((p) => lower.startsWith(p)))
        return "vision";
    if (BUILD_PREFIXES.some((p) => lower.startsWith(p)))
        return "build";
    if (SPAWN_PREFIXES.some((p) => lower.startsWith(p)))
        return "spawn";
    if (QUERY_PREFIXES.some((p) => lower.startsWith(p)))
        return "query";
    if (lower.startsWith("set_") || lower.startsWith("move_") ||
        lower.startsWith("rotate_") || lower.startsWith("scale_") ||
        lower.startsWith("delete_") || lower.startsWith("paint_landscape") ||
        lower.startsWith("sculpt_"))
        return "mutation";
    return "default";
}
// ============================================================================
// Category baseline durations (ms) — used when no history exists
// ============================================================================
const CATEGORY_BASELINES = {
    vision: 3000,
    build: 10000,
    spawn: 1500,
    query: 500,
    mutation: 1000,
    default: 2000,
};
// ============================================================================
// Stats helpers
// ============================================================================
function percentile(sorted, p) {
    if (sorted.length === 0)
        return 0;
    const idx = Math.ceil((p / 100) * sorted.length) - 1;
    return sorted[Math.max(0, idx)];
}
function computeStats(entries, category) {
    const durations = entries
        .filter((e) => e.duration_ms > 0)
        .map((e) => e.duration_ms)
        .sort((a, b) => a - b);
    const errors = entries.filter((e) => !e.ok).length;
    return {
        category,
        calls: entries.length,
        errors,
        p50_ms: percentile(durations, 50),
        p95_ms: percentile(durations, 95),
        error_rate_pct: entries.length > 0 ? Math.round((errors / entries.length) * 100) : 0,
    };
}
// ============================================================================
// LatencyTracker
// ============================================================================
export class LatencyTracker {
    entries = [];
    maxEntries;
    constructor(maxEntries = 200) {
        this.maxEntries = maxEntries;
    }
    /** Record a completed tool call. */
    record(entry) {
        const safeEntry = sanitizeSessionEntry(entry);
        const category = classifyTool(safeEntry.tool);
        const storedEntry = {
            tool: safeEntry.tool,
            ok: safeEntry.ok,
            duration_ms: safeEntry.duration_ms,
            timestamp: safeEntry.timestamp,
            category,
        };
        if (safeEntry.error) {
            storedEntry.error = safeEntry.error;
        }
        if (typeof safeEntry.cached === "boolean") {
            storedEntry.cached = safeEntry.cached;
        }
        this.entries.push(storedEntry);
        if (this.entries.length > this.maxEntries) {
            this.entries.shift();
        }
    }
    /** Get stats for a specific category. */
    statsFor(category) {
        const filtered = this.entries.filter((e) => e.category === category);
        return computeStats(filtered, category);
    }
    /** Get stats for all categories with data. */
    allStats() {
        const groups = new Map();
        for (const entry of this.entries) {
            const cat = entry.category;
            if (!groups.has(cat))
                groups.set(cat, []);
            groups.get(cat).push(entry);
        }
        return Array.from(groups.entries()).map(([cat, entries]) => computeStats(entries, cat));
    }
    /** Get the last N entries (most recent first). */
    recent(n = 10) {
        return this.entries.slice(-n).reverse().map((entry) => {
            const cloned = {
                tool: entry.tool,
                ok: entry.ok,
                duration_ms: entry.duration_ms,
                timestamp: entry.timestamp,
                category: entry.category,
            };
            if (entry.error) {
                cloned.error = entry.error;
            }
            if (typeof entry.cached === "boolean") {
                cloned.cached = entry.cached;
            }
            return cloned;
        });
    }
    /** Get stats for a specific tool name (not category). */
    toolStats(toolName) {
        const matching = this.entries.filter((e) => e.tool === toolName);
        if (matching.length === 0)
            return { calls: 0, avg_ms: 0, error_rate_pct: 0 };
        const durations = matching.filter((e) => e.duration_ms > 0).map((e) => e.duration_ms);
        const avg = durations.length > 0
            ? Math.round(durations.reduce((a, b) => a + b, 0) / durations.length)
            : 0;
        const errors = matching.filter((e) => !e.ok).length;
        return {
            calls: matching.length,
            avg_ms: avg,
            error_rate_pct: Math.round((errors / matching.length) * 100),
        };
    }
    /** Reset all data. */
    reset() {
        this.entries = [];
    }
}
// ============================================================================
// Anomaly detection
// ============================================================================
const LATENCY_SPIKE_FACTOR = 3; // 3x above P50 = spike
const ERROR_SPIKE_THRESHOLD = 50; // 50%+ error rate = spike
const DEGRADATION_WINDOW = 5; // Look at last 5 calls for trend
export function detectAnomalies(tracker, recentEntries) {
    const anomalies = [];
    if (recentEntries.length < 3)
        return anomalies;
    // Check per-category latency spikes
    const recentByCategory = new Map();
    for (const entry of recentEntries) {
        const cat = classifyTool(entry.tool);
        if (entry.duration_ms > 0) {
            if (!recentByCategory.has(cat))
                recentByCategory.set(cat, []);
            recentByCategory.get(cat).push(entry.duration_ms);
        }
    }
    for (const [cat, durations] of Array.from(recentByCategory.entries())) {
        const baseline = tracker.statsFor(cat);
        if (baseline.calls < 3 || baseline.p50_ms === 0)
            continue;
        const recentAvg = durations.reduce((a, b) => a + b, 0) / durations.length;
        if (recentAvg > baseline.p50_ms * LATENCY_SPIKE_FACTOR) {
            anomalies.push({
                type: "latency_spike",
                message: `${cat} tools averaging ${Math.round(recentAvg)}ms (baseline P50: ${baseline.p50_ms}ms)`,
                category: cat,
            });
        }
    }
    // Check overall error rate in recent window
    const recentErrors = recentEntries.filter((e) => !e.ok).length;
    const recentErrorRate = Math.round((recentErrors / recentEntries.length) * 100);
    if (recentErrorRate >= ERROR_SPIKE_THRESHOLD) {
        anomalies.push({
            type: "error_spike",
            message: `${recentErrorRate}% error rate in last ${recentEntries.length} calls`,
        });
    }
    // Check for bridge degradation: consecutive slow calls in the last N
    const last5 = recentEntries.slice(-DEGRADATION_WINDOW);
    const slowCount = last5.filter((e) => e.duration_ms > 5000).length;
    const errorCount = last5.filter((e) => !e.ok).length;
    if (slowCount + errorCount >= 4) {
        anomalies.push({
            type: "bridge_degradation",
            message: `${slowCount} slow + ${errorCount} failed in last ${DEGRADATION_WINDOW} calls — bridge may be overloaded`,
        });
    }
    return anomalies;
}
// ============================================================================
// Duration estimation
// ============================================================================
export function estimateDuration(toolName, tracker) {
    // First: check tool-specific history
    const toolData = tracker.toolStats(toolName);
    if (toolData.calls >= 3) {
        return {
            estimate_ms: toolData.avg_ms,
            confidence: toolData.calls >= 10 ? "high" : "medium",
            basis: `${toolData.calls} prior calls of ${toolName}`,
        };
    }
    // Second: check category history
    const category = classifyTool(toolName);
    const catStats = tracker.statsFor(category);
    if (catStats.calls >= 5) {
        return {
            estimate_ms: catStats.p50_ms,
            confidence: "low",
            basis: `${category} category P50 (${catStats.calls} calls)`,
        };
    }
    // Third: fall back to category baseline
    return {
        estimate_ms: CATEGORY_BASELINES[category],
        confidence: "none",
        basis: `${category} category default baseline`,
    };
}
// ============================================================================
// Bridge trend detection
// ============================================================================
export function detectTrend(recentEntries) {
    if (recentEntries.length < 4)
        return "stable";
    const mid = Math.floor(recentEntries.length / 2);
    const older = recentEntries.slice(0, mid);
    const newer = recentEntries.slice(mid);
    const avgDuration = (entries) => {
        const valid = entries.filter((e) => e.duration_ms > 0);
        return valid.length > 0
            ? valid.reduce((a, b) => a + b.duration_ms, 0) / valid.length
            : 0;
    };
    const olderAvg = avgDuration(older);
    const newerAvg = avgDuration(newer);
    if (olderAvg === 0 || newerAvg === 0)
        return "stable";
    const ratio = newerAvg / olderAvg;
    if (ratio > 2.0)
        return "degrading";
    if (ratio < 0.5 && olderAvg > 3000)
        return "recovering";
    return "stable";
}
// ============================================================================
// Health summary
// ============================================================================
export function buildHealthSummary(tracker, recentEntries) {
    const allStats = tracker.allStats();
    const totalCalls = allStats.reduce((s, c) => s + c.calls, 0);
    const allDurations = recentEntries
        .filter((e) => e.duration_ms > 0)
        .map((e) => e.duration_ms);
    const avgLatency = allDurations.length > 0
        ? Math.round(allDurations.reduce((a, b) => a + b, 0) / allDurations.length)
        : 0;
    const totalErrors = allStats.reduce((s, c) => s + c.errors, 0);
    const errorRate = totalCalls > 0 ? Math.round((totalErrors / totalCalls) * 100) : 0;
    const trend = detectTrend(recentEntries);
    const anomalies = detectAnomalies(tracker, recentEntries);
    return {
        total_calls: totalCalls,
        avg_latency_ms: avgLatency,
        error_rate_pct: errorRate,
        trend,
        anomalies,
    };
}
// ============================================================================
// shouldAttachHealth — when to inject health block
// ============================================================================
const HEALTH_INTERVAL = 15;
export function shouldAttachHealth(callNumber, interval = HEALTH_INTERVAL, recentEntries = []) {
    // Every N calls
    if (callNumber > 0 && callNumber % interval === 0)
        return true;
    // On any anomaly trigger (3+ errors in recent window)
    if (recentEntries.length >= 5) {
        const errors = recentEntries.slice(-5).filter((e) => !e.ok).length;
        if (errors >= 3)
            return true;
    }
    return false;
}
//# sourceMappingURL=performance-intelligence.js.map