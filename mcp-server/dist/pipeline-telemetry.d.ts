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
export declare const FEATURE_MARKERS: Record<string, string>;
export declare const ALL_FEATURES: string[];
interface FeatureEvent {
    feature: string;
    tool: string;
    timestamp: number;
}
export declare const MAX_EVENTS = 500;
export declare class PipelineTelemetry {
    private events;
    private featureCounts;
    private toolStats;
    private totalCalls;
    private totalMetadataFields;
    /** Record a specific feature activation. */
    record(feature: string, tool: string): void;
    /** Record a tool call outcome (success/fail + duration). */
    recordCall(tool: string, ok: boolean, durationMs: number): void;
    /** Record how many metadata fields were in a response. */
    recordMetadataCount(count: number): void;
    /** Feature utilization sorted by count descending. */
    getFeatureUtilization(): {
        feature: string;
        count: number;
        pct: string;
    }[];
    /** Features that have NEVER fired — candidates for removal. */
    getDeadFeatures(): string[];
    /** Top N tools by call count. */
    getTopTools(n?: number): {
        tool: string;
        calls: number;
        errors: number;
        error_rate: string;
        avg_ms: number;
    }[];
    /** Top N tools by error count — the ones causing the most trouble. */
    getProblematicTools(n?: number): {
        tool: string;
        errors: number;
        calls: number;
        error_rate: string;
    }[];
    /** Average metadata fields per response — measures pipeline overhead. */
    getAvgMetadataFields(): number;
    /** Recent feature events (last N). */
    recentEvents(n?: number): FeatureEvent[];
    /** Full report combining all stats. */
    getReport(): PipelineTelemetryReport;
    /** Serialize for persistence. */
    toJSON(): object;
    /** Restore from serialized data. */
    loadFrom(data: unknown): void;
    /** Reset all state. */
    clear(): void;
}
export interface PipelineTelemetryReport {
    total_calls: number;
    unique_tools_used: number;
    avg_metadata_fields_per_response: number;
    feature_utilization: {
        feature: string;
        count: number;
        pct: string;
    }[];
    dead_features: string[];
    top_tools: {
        tool: string;
        calls: number;
        errors: number;
        error_rate: string;
        avg_ms: number;
    }[];
    problematic_tools: {
        tool: string;
        errors: number;
        calls: number;
        error_rate: string;
    }[];
}
/**
 * Scan a dispatch response for metadata keys that indicate feature activation.
 * Call once per dispatch return to auto-record all feature telemetry.
 */
export declare function recordResponseTelemetry(telemetry: PipelineTelemetry, toolName: string, response: Record<string, unknown>): void;
export {};
//# sourceMappingURL=pipeline-telemetry.d.ts.map