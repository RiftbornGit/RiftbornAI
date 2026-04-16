/**
 * Dispatch Pipeline — Composable middleware stages for tool dispatch.
 *
 * Replaces the monolithic dispatchManagedTool() with a typed pipeline where
 * each stage is a named function operating on a shared DispatchContext.
 *
 * Stages either:
 *   - Mutate ctx and return void (continue to next stage)
 *   - Set ctx.earlyReturn and return (short-circuit with that response)
 *
 * The runner calls stages sequentially. If a stage sets earlyReturn,
 * the pipeline stops and returns that response.
 */
import type { DuplicateWarning, DependencyWarning } from "./proactive-guards.js";
import type { DiagnosticContext, DiagnosticInput } from "./error-recovery.js";
import type { RetryDecision } from "./pipeline-intelligence.js";
import type { RecoveryAction } from "./schema-intelligence.js";
import type { ToolDependencyGraph } from "./tool-dependency-graph.js";
import type { RiftbornResponse } from "./riftborn-types.js";
import type { Digest } from "./session-intelligence.js";
import type { HealthSummary } from "./performance-intelligence.js";
import type { ContextSummary, DurationPrediction, ScopedDigest } from "./pipeline-refinements.js";
/** Minimal interface for the optional trace recorder (avoids circular import). */
export interface TraceAnnotator {
    annotate(decision: string): void;
}
export interface DispatchContext {
    resolvedName: string;
    rawArgs: Record<string, unknown>;
    handler: (args: Record<string, unknown>) => Promise<RiftbornResponse>;
    resolutionMeta?: Record<string, unknown>;
    traceRecorder?: TraceAnnotator;
    normalizedArgs: Record<string, unknown>;
    coercedArgs: Record<string, unknown>;
    defaultedArgs: Record<string, unknown>;
    defaults_applied: Record<string, unknown>;
    contextArgs: Record<string, unknown>;
    inferred: Record<string, unknown>;
    finalParams: Record<string, unknown>;
    warnings: Record<string, unknown>;
    startTime: number;
    durationMs: number;
    rawResult: RiftbornResponse;
    retried: boolean;
    transformedResult: RiftbornResponse;
    metadata: Record<string, unknown>;
    finalResult: RiftbornResponse;
    earlyReturn?: RiftbornResponse;
    services: DispatchServices;
}
/**
 * All external dependencies a stage might need — injected once at pipeline
 * creation so stages stay pure functions of (ctx) → void.
 */
export interface DispatchServices {
    normalizeParams: (tool: string, args: Record<string, unknown>) => Record<string, unknown>;
    coerceParams: (tool: string, args: Record<string, unknown>) => Record<string, unknown>;
    applyDefaults: (tool: string, args: Record<string, unknown>) => {
        params: Record<string, unknown>;
        defaults_applied: Record<string, unknown>;
    };
    validateParams: (tool: string, args: Record<string, unknown>) => RiftbornResponse | null;
    contextInject: (tool: string, args: Record<string, unknown>) => {
        params: Record<string, unknown>;
        inferred: Record<string, unknown>;
    };
    checkPrerequisite: (tool: string, args: Record<string, unknown>) => RiftbornResponse | null;
    checkDependencies: (tool: string, recent: any[]) => DependencyWarning | null;
    getDependencyGraph: () => ToolDependencyGraph;
    checkIdempotency: (tool: string, args: Record<string, unknown>, recent: any[]) => DuplicateWarning | null;
    detectConflict: (tool: string, args: Record<string, unknown>) => {
        description: string;
    } | null;
    failureBudgetCheck: (tool: string) => {
        blocked: boolean;
        reason?: string;
    };
    preflightEntityCheck: (tool: string, args: Record<string, unknown>, changes: any[]) => string | null;
    getThrottleDecision: () => {
        delay_ms: number;
    };
    enrichError: (tool: string, result: RiftbornResponse) => RiftbornResponse & {
        error_category?: string;
        retryable?: boolean;
    };
    shouldSmartRetry: (cat: string | undefined, retryable: boolean | undefined, attempt: number) => RetryDecision;
    buildRecoveryAction: (tool: string, cat: string | undefined, err: string | undefined) => RecoveryAction | null;
    shouldAttachDiagnostics: (cat: string | undefined, callCount: number) => boolean;
    gatherDiagnosticContext: (info: DiagnosticInput) => DiagnosticContext;
    canonicalizeOutput: (tool: string, result: Record<string, unknown>) => Record<string, unknown>;
    extractContext: (tool: string, result: unknown) => void;
    extractVisionContext: (tool: string, result: RiftbornResponse) => Record<string, unknown>;
    injectVisionContext: (entries: [string, unknown][]) => void;
    enhanceVisionResponse: (tool: string, result: RiftbornResponse) => RiftbornResponse;
    normalizeResponse: (result: RiftbornResponse) => RiftbornResponse;
    shapeResponse: (tool: string, result: Record<string, unknown>) => Record<string, unknown>;
    addWorkflowHint: (tool: string, result: RiftbornResponse) => RiftbornResponse;
    classifyCostFromDuration: (ms: number) => string;
    recordSceneChange: (tool: string, args: Record<string, unknown>, ts: number) => void;
    buildSafetyBlock: () => unknown | null;
    buildThrottleBlock: () => Record<string, unknown> | null;
    buildParamEcho: (args: Record<string, unknown>, defaults: Record<string, unknown>, inferred: Record<string, unknown>) => unknown | null;
    estimateToolDuration: (tool: string, recent: any[]) => DurationPrediction;
    sessionRecord: (entry: any) => void;
    latencyRecord: (entry: any) => void;
    throttleReport: (outcome: {
        ok: boolean;
        duration_ms: number;
        error?: string;
    }) => void;
    failureBudgetRecord: (tool: string, ok: boolean, error?: string) => void;
    progressRecord: (tool: string, ts: number) => void;
    incrementCallCount: () => number;
    getCallCount: () => number;
    getRecentSession: (n: number) => any[];
    getErrorContext: () => unknown;
    getSceneChanges: (n: number) => any[];
    getContextSnapshot: () => Record<string, unknown>;
    buildDigest: (callCount: number, recent: any[]) => Digest;
    shouldAttachDigest: (callCount: number, recent: any[]) => boolean;
    scopeDigest: (digest: Digest, recentTools: string[]) => ScopedDigest;
    buildContextSummary: (snapshot: Record<string, unknown>) => ContextSummary;
    shouldAttachHealth: (callCount: number, recent: any[]) => boolean;
    buildHealthSummary: (recent: any[]) => HealthSummary;
    deltaTrack: (tool: string, result: Record<string, unknown>) => Record<string, {
        was: unknown;
        now: unknown;
    }> | null;
    microVerify: (tool: string, args: Record<string, unknown>, execute: (t: string, p: Record<string, unknown>) => Promise<RiftbornResponse>) => Promise<{
        verified: boolean;
        label: string;
        duration_ms: number;
        error?: string;
    } | null>;
    hasMicroVerifyRule: (tool: string) => boolean;
    executeToolDirect: (tool: string, args: Record<string, unknown>) => Promise<RiftbornResponse>;
    telemetryRecord: (feature: string, tool: string) => void;
    telemetryRecordCall: (tool: string, ok: boolean, ms: number) => void;
    telemetryRecordResponse: (tool: string, response: Record<string, unknown>) => void;
    schedulePersist: () => void;
    addTiming: (result: RiftbornResponse, startTime: number) => RiftbornResponse;
    toolHandlers: Record<string, (args: Record<string, unknown>) => Promise<RiftbornResponse>>;
}
export type DispatchStage = (ctx: DispatchContext) => void | Promise<void>;
export declare function runPipeline(stages: DispatchStage[], ctx: DispatchContext): Promise<RiftbornResponse>;
export declare function stageNormalizeAndCoerce(ctx: DispatchContext): void;
export declare function stageApplyDefaults(ctx: DispatchContext): void;
export declare function stageValidate(ctx: DispatchContext): void;
export declare function stageContextInject(ctx: DispatchContext): void;
export declare function stagePrerequisite(ctx: DispatchContext): void;
export declare function stagePreExecAnalysis(ctx: DispatchContext): void;
export declare function stageExecute(ctx: DispatchContext): Promise<void>;
export declare function stageHandleFailure(ctx: DispatchContext): void;
export declare function stageCanonicalizeAndExtract(ctx: DispatchContext): void;
export declare function stageTransformResponse(ctx: DispatchContext): void;
export declare function stageBuildMetadata(ctx: DispatchContext): void;
export declare function stageRecordSuccess(ctx: DispatchContext): void;
export declare function stageDigestAndHealth(ctx: DispatchContext): void;
export declare function stageDeltaTracking(ctx: DispatchContext): void;
export declare function stageMicroVerify(ctx: DispatchContext): Promise<void>;
export declare function stageFinalize(ctx: DispatchContext): void;
/** The standard ordered stage list. */
export declare const DEFAULT_STAGES: DispatchStage[];
/** Build a fresh DispatchContext ready for pipeline execution. */
export declare function createDispatchContext(resolvedName: string, rawArgs: Record<string, unknown>, handler: (args: Record<string, unknown>) => Promise<RiftbornResponse>, services: DispatchServices, resolutionMeta?: Record<string, unknown>): DispatchContext;
//# sourceMappingURL=dispatch-pipeline.d.ts.map