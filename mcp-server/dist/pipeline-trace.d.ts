/**
 * Pipeline Tracing — Per-call observability for the dispatch pipeline.
 *
 * Records which stages ran, what decisions they made, and how long each took.
 * Stored in a ring buffer; queryable via the pipeline_trace tool.
 *
 * Round 24.
 */
import type { DispatchContext, DispatchStage } from "./dispatch-pipeline.js";
export interface TraceEntry {
    stage: string;
    duration_ms: number;
    /** Key decision or annotation from the stage, e.g. "earlyReturn", "retry", "coerced:2" */
    decision?: string;
}
export interface PipelineTrace {
    tool: string;
    timestamp: number;
    total_ms: number;
    stages: TraceEntry[];
    outcome: "success" | "earlyReturn" | "error";
    /** Set when outcome is earlyReturn — which stage short-circuited */
    stopped_at?: string;
    /** Number of metadata fields attached to the final response */
    metadata_fields?: number;
}
export interface PipelineTraceReport {
    total_traces: number;
    buffer_size: number;
    traces: PipelineTrace[];
}
export interface StageHotspot {
    stage: string;
    calls: number;
    total_ms: number;
    avg_ms: number;
    p95_ms: number;
    max_ms: number;
    decision_rate: number;
}
export interface ToolHotspot {
    tool: string;
    calls: number;
    avg_total_ms: number;
    p95_total_ms: number;
    max_total_ms: number;
    error_rate: number;
    early_return_rate: number;
    slow_rate: number;
    top_stage?: string;
}
export interface PipelineHotspotReport {
    trace_count: number;
    slow_threshold_ms: number;
    tool_hotspots: ToolHotspot[];
    stage_hotspots: StageHotspot[];
}
/**
 * Lightweight per-dispatch trace collector. Create one per call,
 * attach to ctx, and the traced runner populates it.
 */
export declare class TraceRecorder {
    readonly entries: TraceEntry[];
    private _annotations;
    /** Stage runner calls this to add annotation the current stage can reference. */
    annotate(decision: string): void;
    /** Called by the traced runner after each stage completes. */
    record(stageName: string, durationMs: number): void;
}
/**
 * Like runPipeline, but wraps each stage with timing and records a trace.
 * Stages annotate decisions via ctx.traceRecorder.annotate().
 */
export declare function runTracedPipeline(stages: DispatchStage[], ctx: DispatchContext & {
    traceRecorder: TraceRecorder;
}): Promise<import("./riftborn-types.js").RiftbornResponse>;
export declare class PipelineTraceStore {
    private buffer;
    private readonly maxSize;
    constructor(maxSize?: number);
    /** Record a completed pipeline trace. */
    record(trace: PipelineTrace): void;
    /** Get recent traces, optionally filtered by tool name. */
    recent(count?: number, toolFilter?: string): PipelineTrace[];
    /** Get traces where a specific stage made a decision (non-empty annotation). */
    byStageDecision(stageName: string, count?: number): PipelineTrace[];
    /** Get the slowest traces. */
    slowest(count?: number): PipelineTrace[];
    /** Build a report for the pipeline_trace tool. */
    getReport(opts?: {
        count?: number;
        tool?: string;
        slow?: boolean;
    }): PipelineTraceReport;
    /** Per-stage aggregate timing stats across all recorded traces. */
    stageStats(): Record<string, {
        calls: number;
        total_ms: number;
        avg_ms: number;
        max_ms: number;
        decisions: number;
    }>;
    /**
     * Actionable performance report: hottest tools and stages by latency/error profile.
     */
    hotspots(opts?: {
        top?: number;
        min_calls?: number;
        slow_threshold_ms?: number;
    }): PipelineHotspotReport;
    get size(): number;
    clear(): void;
}
/**
 * After runTracedPipeline completes, call this to build the PipelineTrace
 * from the recorder and context.
 */
export declare function buildTrace(ctx: DispatchContext & {
    traceRecorder: TraceRecorder;
}, outcome: "success" | "earlyReturn" | "error", totalMs: number): PipelineTrace;
//# sourceMappingURL=pipeline-trace.d.ts.map