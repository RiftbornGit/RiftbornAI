/**
 * Pipeline Telemetry — Lightweight instrumentation that records which pipeline
 * features activate, aggregates over time, and surfaces via pipeline_stats.
 *
 * Answers the question: "Which of our 21 rounds of features actually matters?"
 *
 * Design:
 *   - `recordResponseTelemetry()` scans each dispatch response for metadata keys
 *     that indicate feature activation (e.g. `_retried` → "smart_retry").
 *   - `PipelineTelemetry` class aggregates counts + per-tool stats.
 *   - `getReport()` produces a structured report: utilization, dead features,
 *     top tools, problematic tools, overhead metrics.
 *   - Persistent via toJSON/loadFrom for SessionPersistence integration.
 */
// ── Feature markers: response keys → human-readable feature names ──────────
export const FEATURE_MARKERS = {
    _retried: "smart_retry",
    _inferred: "context_inject",
    _defaults_applied: "defaults_applied",
    _dependency_warning: "dependency_warning",
    _dependency_graph: "dependency_graph_miss",
    _duplicate_warning: "idempotency_check",
    _conflict_warning: "conflict_detect",
    _scene_safety: "scene_safety",
    _throttle: "adaptive_throttle",
    _param_echo: "param_echo",
    _preflight_warning: "preflight_check",
    _duration_estimate: "duration_estimate",
    _unlocked_tools: "unlocked_tools",
    _digest: "digest_attach",
    _context: "context_summary",
    _health: "health_summary",
    _changed_since_last: "delta_track",
    _verified: "micro_verify_pass",
    _verify_failed: "micro_verify_fail",
    _recovery: "error_recovery",
    _diagnostics: "error_diagnostics",
    _failure_budget: "circuit_breaker",
    _resolved_from: "fuzzy_resolution",
};
// All known features — includes those tracked via explicit calls (not response markers)
export const ALL_FEATURES = [
    ...new Set([
        ...Object.values(FEATURE_MARKERS),
        "schema_coercion", // tracked when coerceParams mutates values
        "prereq_block", // tracked when prerequisite check blocks execution
        "validation_fail", // tracked when schema validation rejects params
    ]),
];
export const MAX_EVENTS = 500;
export class PipelineTelemetry {
    events = [];
    featureCounts = new Map();
    toolStats = new Map();
    totalCalls = 0;
    totalMetadataFields = 0;
    /** Record a specific feature activation. */
    record(feature, tool) {
        if (this.events.length >= MAX_EVENTS)
            this.events.shift();
        this.events.push({ feature, tool, timestamp: Date.now() });
        this.featureCounts.set(feature, (this.featureCounts.get(feature) || 0) + 1);
    }
    /** Record a tool call outcome (success/fail + duration). */
    recordCall(tool, ok, durationMs) {
        this.totalCalls++;
        const existing = this.toolStats.get(tool) || { calls: 0, errors: 0, total_ms: 0 };
        existing.calls++;
        if (!ok)
            existing.errors++;
        existing.total_ms += durationMs;
        this.toolStats.set(tool, existing);
    }
    /** Record how many metadata fields were in a response. */
    recordMetadataCount(count) {
        this.totalMetadataFields += count;
    }
    /** Feature utilization sorted by count descending. */
    getFeatureUtilization() {
        if (this.totalCalls === 0)
            return [];
        return [...this.featureCounts.entries()]
            .sort((a, b) => b[1] - a[1])
            .map(([feature, count]) => ({
            feature,
            count,
            pct: ((count / this.totalCalls) * 100).toFixed(1) + "%",
        }));
    }
    /** Features that have NEVER fired — candidates for removal. */
    getDeadFeatures() {
        return ALL_FEATURES.filter(f => !this.featureCounts.has(f));
    }
    /** Top N tools by call count. */
    getTopTools(n = 10) {
        return [...this.toolStats.entries()]
            .sort((a, b) => b[1].calls - a[1].calls)
            .slice(0, Math.max(1, Math.min(n, 50)))
            .map(([tool, s]) => ({
            tool,
            calls: s.calls,
            errors: s.errors,
            error_rate: s.calls > 0 ? ((s.errors / s.calls) * 100).toFixed(1) + "%" : "0%",
            avg_ms: s.calls > 0 ? Math.round(s.total_ms / s.calls) : 0,
        }));
    }
    /** Top N tools by error count — the ones causing the most trouble. */
    getProblematicTools(n = 5) {
        return [...this.toolStats.entries()]
            .filter(([, s]) => s.errors > 0)
            .sort((a, b) => b[1].errors - a[1].errors)
            .slice(0, Math.max(1, Math.min(n, 20)))
            .map(([tool, s]) => ({
            tool,
            errors: s.errors,
            calls: s.calls,
            error_rate: ((s.errors / s.calls) * 100).toFixed(1) + "%",
        }));
    }
    /** Average metadata fields per response — measures pipeline overhead. */
    getAvgMetadataFields() {
        return this.totalCalls > 0
            ? Math.round((this.totalMetadataFields / this.totalCalls) * 10) / 10
            : 0;
    }
    /** Recent feature events (last N). */
    recentEvents(n = 20) {
        return this.events.slice(-Math.max(1, Math.min(n, MAX_EVENTS)));
    }
    /** Full report combining all stats. */
    getReport() {
        return {
            total_calls: this.totalCalls,
            unique_tools_used: this.toolStats.size,
            avg_metadata_fields_per_response: this.getAvgMetadataFields(),
            feature_utilization: this.getFeatureUtilization(),
            dead_features: this.getDeadFeatures(),
            top_tools: this.getTopTools(10),
            problematic_tools: this.getProblematicTools(5),
        };
    }
    /** Serialize for persistence. */
    toJSON() {
        return {
            featureCounts: Object.fromEntries(this.featureCounts),
            toolStats: Object.fromEntries(this.toolStats),
            totalCalls: this.totalCalls,
            totalMetadataFields: this.totalMetadataFields,
        };
    }
    /** Restore from serialized data. */
    loadFrom(data) {
        if (!data || typeof data !== "object")
            return;
        const d = data;
        if (d.featureCounts && typeof d.featureCounts === "object") {
            for (const [k, v] of Object.entries(d.featureCounts)) {
                if (typeof v === "number")
                    this.featureCounts.set(k, v);
            }
        }
        if (d.toolStats && typeof d.toolStats === "object") {
            for (const [k, v] of Object.entries(d.toolStats)) {
                if (v && typeof v.calls === "number") {
                    this.toolStats.set(k, { calls: v.calls, errors: v.errors || 0, total_ms: v.total_ms || 0 });
                }
            }
        }
        if (typeof d.totalCalls === "number")
            this.totalCalls = d.totalCalls;
        if (typeof d.totalMetadataFields === "number")
            this.totalMetadataFields = d.totalMetadataFields;
    }
    /** Reset all state. */
    clear() {
        this.events = [];
        this.featureCounts.clear();
        this.toolStats.clear();
        this.totalCalls = 0;
        this.totalMetadataFields = 0;
    }
}
/**
 * Scan a dispatch response for metadata keys that indicate feature activation.
 * Call once per dispatch return to auto-record all feature telemetry.
 */
export function recordResponseTelemetry(telemetry, toolName, response) {
    let metadataCount = 0;
    for (const [key, feature] of Object.entries(FEATURE_MARKERS)) {
        if (key in response && response[key] != null) {
            telemetry.record(feature, toolName);
            metadataCount++;
        }
    }
    telemetry.recordMetadataCount(metadataCount);
}
//# sourceMappingURL=pipeline-telemetry.js.map