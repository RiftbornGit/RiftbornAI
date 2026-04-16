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
import type { SessionEntry } from "./system-enhancements.js";
export type ToolCategory = "vision" | "build" | "spawn" | "query" | "mutation" | "default";
export type BridgeTrend = "stable" | "degrading" | "recovering";
export interface CategoryStats {
    category: ToolCategory;
    calls: number;
    errors: number;
    p50_ms: number;
    p95_ms: number;
    error_rate_pct: number;
}
export interface Anomaly {
    type: "latency_spike" | "error_spike" | "bridge_degradation";
    message: string;
    category?: ToolCategory;
}
export interface DurationEstimate {
    estimate_ms: number;
    confidence: "none" | "low" | "medium" | "high";
    basis: string;
}
export interface HealthSummary {
    total_calls: number;
    avg_latency_ms: number;
    error_rate_pct: number;
    trend: BridgeTrend;
    anomalies: Anomaly[];
}
export declare function classifyTool(toolName: string): ToolCategory;
export declare class LatencyTracker {
    private entries;
    private maxEntries;
    constructor(maxEntries?: number);
    /** Record a completed tool call. */
    record(entry: SessionEntry): void;
    /** Get stats for a specific category. */
    statsFor(category: ToolCategory): CategoryStats;
    /** Get stats for all categories with data. */
    allStats(): CategoryStats[];
    /** Get the last N entries (most recent first). */
    recent(n?: number): Array<SessionEntry & {
        category: ToolCategory;
    }>;
    /** Get stats for a specific tool name (not category). */
    toolStats(toolName: string): {
        calls: number;
        avg_ms: number;
        error_rate_pct: number;
    };
    /** Reset all data. */
    reset(): void;
}
export declare function detectAnomalies(tracker: LatencyTracker, recentEntries: SessionEntry[]): Anomaly[];
export declare function estimateDuration(toolName: string, tracker: LatencyTracker): DurationEstimate;
export declare function detectTrend(recentEntries: SessionEntry[]): BridgeTrend;
export declare function buildHealthSummary(tracker: LatencyTracker, recentEntries: SessionEntry[]): HealthSummary;
export declare function shouldAttachHealth(callNumber: number, interval?: number, recentEntries?: SessionEntry[]): boolean;
//# sourceMappingURL=performance-intelligence.d.ts.map