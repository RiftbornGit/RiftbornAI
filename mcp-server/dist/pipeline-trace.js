/**
 * Pipeline Tracing — Per-call observability for the dispatch pipeline.
 *
 * Records which stages ran, what decisions they made, and how long each took.
 * Stored in a ring buffer; queryable via the pipeline_trace tool.
 *
 * Round 24.
 */
import { createSanitizer } from "./sanitize-utils.js";
// ── Trace Recorder (attached to DispatchContext) ───────────────────────────
/**
 * Lightweight per-dispatch trace collector. Create one per call,
 * attach to ctx, and the traced runner populates it.
 */
export class TraceRecorder {
    entries = [];
    _annotations = [];
    /** Stage runner calls this to add annotation the current stage can reference. */
    annotate(decision) {
        this._annotations.push(decision);
    }
    /** Called by the traced runner after each stage completes. */
    record(stageName, durationMs) {
        const decision = this._annotations.length > 0
            ? this._annotations.join("; ")
            : undefined;
        this.entries.push({ stage: stageName, duration_ms: durationMs, decision });
        this._annotations = [];
    }
}
// ── Traced Pipeline Runner ─────────────────────────────────────────────────
/**
 * Like runPipeline, but wraps each stage with timing and records a trace.
 * Stages annotate decisions via ctx.traceRecorder.annotate().
 */
export async function runTracedPipeline(stages, ctx) {
    for (const stage of stages) {
        const start = performance.now();
        await stage(ctx);
        const ms = Math.round((performance.now() - start) * 100) / 100;
        ctx.traceRecorder.record(stage.name || "anonymous", ms);
        if (ctx.earlyReturn)
            return ctx.earlyReturn;
    }
    return ctx.finalResult;
}
// ── Trace Store ────────────────────────────────────────────────────────────
const MAX_TRACES = 200;
function round2(value) {
    return Math.round(value * 100) / 100;
}
function percentile(values, p) {
    if (values.length === 0)
        return 0;
    const sorted = [...values].sort((a, b) => a - b);
    const rank = Math.ceil((p / 100) * sorted.length) - 1;
    const idx = Math.min(Math.max(rank, 0), sorted.length - 1);
    return sorted[idx];
}
function sanitizeTraceEntry(value) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
        return null;
    }
    const raw = value;
    if (typeof raw.stage !== "string" || !raw.stage) {
        return null;
    }
    const entry = {
        stage: raw.stage,
        duration_ms: typeof raw.duration_ms === "number" && Number.isFinite(raw.duration_ms)
            ? round2(Math.max(0, raw.duration_ms))
            : 0,
    };
    if (typeof raw.decision === "string" && raw.decision) {
        entry.decision = raw.decision;
    }
    return entry;
}
const sanitizeTraceValue = createSanitizer({ maxDepth: 8, trackCircular: true, depthSentinel: "[MaxDepth]", circularSentinel: "[Circular]" });
function sanitizePipelineTrace(trace) {
    const raw = sanitizeTraceValue(trace);
    const stages = Array.isArray(raw.stages)
        ? raw.stages
            .map((entry) => sanitizeTraceEntry(entry))
            .filter((entry) => entry !== null)
        : [];
    const safeTrace = {
        tool: typeof raw.tool === "string" ? raw.tool : "",
        timestamp: typeof raw.timestamp === "number" && Number.isFinite(raw.timestamp) ? raw.timestamp : 0,
        total_ms: typeof raw.total_ms === "number" && Number.isFinite(raw.total_ms) ? round2(Math.max(0, raw.total_ms)) : 0,
        stages,
        outcome: raw.outcome === "earlyReturn" || raw.outcome === "error" ? raw.outcome : "success",
    };
    if (typeof raw.stopped_at === "string") {
        safeTrace.stopped_at = raw.stopped_at;
    }
    if (typeof raw.metadata_fields === "number" && Number.isFinite(raw.metadata_fields)) {
        safeTrace.metadata_fields = Math.max(0, Math.trunc(raw.metadata_fields));
    }
    return safeTrace;
}
export class PipelineTraceStore {
    buffer = [];
    maxSize;
    constructor(maxSize = MAX_TRACES) {
        this.maxSize = maxSize;
    }
    /** Record a completed pipeline trace. */
    record(trace) {
        this.buffer.push(sanitizePipelineTrace(trace));
        if (this.buffer.length > this.maxSize) {
            this.buffer = this.buffer.slice(-this.maxSize);
        }
    }
    /** Get recent traces, optionally filtered by tool name. */
    recent(count = 20, toolFilter) {
        const clamped = Math.min(Math.max(1, count), this.maxSize);
        const source = toolFilter
            ? this.buffer.filter(t => t.tool === toolFilter)
            : this.buffer;
        return source.slice(-clamped).map((trace) => sanitizePipelineTrace(trace));
    }
    /** Get traces where a specific stage made a decision (non-empty annotation). */
    byStageDecision(stageName, count = 20) {
        const clamped = Math.min(Math.max(1, count), this.maxSize);
        return this.buffer
            .filter(t => t.stages.some(s => s.stage === stageName && s.decision))
            .slice(-clamped)
            .map((trace) => sanitizePipelineTrace(trace));
    }
    /** Get the slowest traces. */
    slowest(count = 10) {
        const clamped = Math.min(Math.max(1, count), this.maxSize);
        return [...this.buffer]
            .sort((a, b) => b.total_ms - a.total_ms)
            .slice(0, clamped)
            .map((trace) => sanitizePipelineTrace(trace));
    }
    /** Build a report for the pipeline_trace tool. */
    getReport(opts = {}) {
        const count = opts.count ?? 20;
        const traces = opts.slow
            ? this.slowest(count)
            : this.recent(count, opts.tool);
        return {
            total_traces: this.buffer.length,
            buffer_size: this.maxSize,
            traces,
        };
    }
    /** Per-stage aggregate timing stats across all recorded traces. */
    stageStats() {
        const stats = {};
        for (const trace of this.buffer) {
            for (const entry of trace.stages) {
                const s = stats[entry.stage] ??= { calls: 0, total_ms: 0, max_ms: 0, decisions: 0 };
                s.calls++;
                s.total_ms += entry.duration_ms;
                if (entry.duration_ms > s.max_ms)
                    s.max_ms = entry.duration_ms;
                if (entry.decision)
                    s.decisions++;
            }
        }
        const result = {};
        for (const [name, s] of Object.entries(stats)) {
            result[name] = {
                calls: s.calls,
                total_ms: s.total_ms,
                avg_ms: Math.round((s.total_ms / s.calls) * 100) / 100,
                max_ms: s.max_ms,
                decisions: s.decisions,
            };
        }
        return result;
    }
    /**
     * Actionable performance report: hottest tools and stages by latency/error profile.
     */
    hotspots(opts = {}) {
        const top = Math.min(Math.max(1, opts.top ?? 10), 50);
        const minCalls = Math.max(1, opts.min_calls ?? 1);
        const slowThresholdMs = Math.max(1, opts.slow_threshold_ms ?? 1500);
        const toolAgg = {};
        const stageAgg = {};
        for (const trace of this.buffer) {
            const t = toolAgg[trace.tool] ??= {
                calls: 0,
                totals: [],
                error: 0,
                early: 0,
                slow: 0,
                stageTotals: {},
            };
            t.calls++;
            t.totals.push(trace.total_ms);
            if (trace.outcome === "error")
                t.error++;
            if (trace.outcome === "earlyReturn")
                t.early++;
            if (trace.total_ms >= slowThresholdMs)
                t.slow++;
            for (const entry of trace.stages) {
                const s = stageAgg[entry.stage] ??= {
                    calls: 0,
                    durations: [],
                    decisions: 0,
                    total_ms: 0,
                    max_ms: 0,
                };
                s.calls++;
                s.durations.push(entry.duration_ms);
                s.total_ms += entry.duration_ms;
                if (entry.duration_ms > s.max_ms)
                    s.max_ms = entry.duration_ms;
                if (entry.decision)
                    s.decisions++;
                t.stageTotals[entry.stage] = (t.stageTotals[entry.stage] ?? 0) + entry.duration_ms;
            }
        }
        const toolHotspots = Object.entries(toolAgg)
            .filter(([, v]) => v.calls >= minCalls)
            .map(([tool, v]) => {
            const topStage = Object.entries(v.stageTotals)
                .sort((a, b) => b[1] - a[1])[0]?.[0];
            return {
                tool,
                calls: v.calls,
                avg_total_ms: round2(v.totals.reduce((a, b) => a + b, 0) / v.calls),
                p95_total_ms: round2(percentile(v.totals, 95)),
                max_total_ms: round2(Math.max(...v.totals)),
                error_rate: round2(v.error / v.calls),
                early_return_rate: round2(v.early / v.calls),
                slow_rate: round2(v.slow / v.calls),
                ...(topStage ? { top_stage: topStage } : {}),
            };
        })
            .sort((a, b) => b.avg_total_ms - a.avg_total_ms || b.calls - a.calls)
            .slice(0, top);
        const stageHotspots = Object.entries(stageAgg)
            .filter(([, v]) => v.calls >= minCalls)
            .map(([stage, v]) => ({
            stage,
            calls: v.calls,
            total_ms: round2(v.total_ms),
            avg_ms: round2(v.total_ms / v.calls),
            p95_ms: round2(percentile(v.durations, 95)),
            max_ms: round2(v.max_ms),
            decision_rate: round2(v.decisions / v.calls),
        }))
            .sort((a, b) => b.avg_ms - a.avg_ms || b.calls - a.calls)
            .slice(0, top);
        return {
            trace_count: this.buffer.length,
            slow_threshold_ms: slowThresholdMs,
            tool_hotspots: toolHotspots,
            stage_hotspots: stageHotspots,
        };
    }
    get size() {
        return this.buffer.length;
    }
    clear() {
        this.buffer = [];
    }
}
// ── Build trace from completed pipeline ────────────────────────────────────
/**
 * After runTracedPipeline completes, call this to build the PipelineTrace
 * from the recorder and context.
 */
export function buildTrace(ctx, outcome, totalMs) {
    const stages = ctx.traceRecorder.entries;
    const stoppedAt = outcome === "earlyReturn"
        ? stages[stages.length - 1]?.stage
        : undefined;
    const metadataFields = ctx.finalResult && typeof ctx.finalResult === "object"
        ? Object.keys(ctx.finalResult).filter(k => k.startsWith("_")).length
        : 0;
    const trace = {
        tool: ctx.resolvedName,
        timestamp: Date.now(),
        total_ms: round2(totalMs),
        stages,
        outcome,
    };
    if (stoppedAt) {
        trace.stopped_at = stoppedAt;
    }
    if (metadataFields > 0) {
        trace.metadata_fields = metadataFields;
    }
    return trace;
}
//# sourceMappingURL=pipeline-trace.js.map