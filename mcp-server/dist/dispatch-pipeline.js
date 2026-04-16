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
import { createSanitizer, createToSafeRecord, createMergeSafeRecords } from "./sanitize-utils.js";
const sanitizeMergeValue = createSanitizer();
const toSafeRecord = createToSafeRecord(sanitizeMergeValue);
const mergeSafeRecords = createMergeSafeRecords(toSafeRecord);
// ── Pipeline Runner ────────────────────────────────────────────────────────
export async function runPipeline(stages, ctx) {
    for (const stage of stages) {
        await stage(ctx);
        if (ctx.earlyReturn)
            return ctx.earlyReturn;
    }
    return ctx.finalResult;
}
// ── Stage: Normalize + Coerce ──────────────────────────────────────────────
export function stageNormalizeAndCoerce(ctx) {
    const { services: s, resolvedName } = ctx;
    ctx.normalizedArgs = s.normalizeParams(resolvedName, ctx.rawArgs);
    ctx.coercedArgs = s.coerceParams(resolvedName, ctx.normalizedArgs);
    const coercedKeys = Object.keys(ctx.coercedArgs).filter(k => ctx.coercedArgs[k] !== ctx.normalizedArgs[k]);
    if (coercedKeys.length > 0) {
        s.telemetryRecord("schema_coercion", resolvedName);
        ctx.traceRecorder?.annotate(`coerced:${coercedKeys.length}`);
    }
}
// ── Stage: Defaults ────────────────────────────────────────────────────────
export function stageApplyDefaults(ctx) {
    const { params, defaults_applied } = ctx.services.applyDefaults(ctx.resolvedName, ctx.coercedArgs);
    ctx.defaultedArgs = params;
    ctx.defaults_applied = defaults_applied;
    const count = Object.keys(defaults_applied).length;
    if (count > 0)
        ctx.traceRecorder?.annotate(`defaults:${count}`);
}
// ── Stage: Validate ────────────────────────────────────────────────────────
export function stageValidate(ctx) {
    const { services: s, resolvedName } = ctx;
    const error = s.validateParams(resolvedName, ctx.defaultedArgs);
    if (error) {
        s.sessionRecord({ tool: resolvedName, ok: false, duration_ms: 0, timestamp: Date.now(), error: error.error });
        s.telemetryRecord("validation_fail", resolvedName);
        s.telemetryRecordCall(resolvedName, false, 0);
        ctx.traceRecorder?.annotate("validation_fail");
        ctx.earlyReturn = error;
    }
}
// ── Stage: Context Inject ──────────────────────────────────────────────────
export function stageContextInject(ctx) {
    const { services: s, resolvedName } = ctx;
    const { params, inferred } = s.contextInject(resolvedName, ctx.defaultedArgs);
    ctx.contextArgs = params;
    ctx.inferred = inferred;
    ctx.finalParams = s.normalizeParams(resolvedName, ctx.contextArgs);
    const inferCount = Object.keys(inferred).length;
    if (inferCount > 0)
        ctx.traceRecorder?.annotate(`inferred:${inferCount}`);
}
// ── Stage: Prerequisite Check ──────────────────────────────────────────────
export function stagePrerequisite(ctx) {
    const { services: s, resolvedName } = ctx;
    const error = s.checkPrerequisite(resolvedName, ctx.finalParams);
    if (error) {
        s.sessionRecord({ tool: resolvedName, ok: false, duration_ms: 0, timestamp: Date.now(), error: error.error });
        s.telemetryRecord("prereq_block", resolvedName);
        s.telemetryRecordCall(resolvedName, false, 0);
        ctx.traceRecorder?.annotate("prereq_blocked");
        ctx.earlyReturn = error;
    }
}
// ── Stage: Pre-execution Analysis ──────────────────────────────────────────
export function stagePreExecAnalysis(ctx) {
    const { services: s, resolvedName } = ctx;
    const recent = s.getRecentSession(50);
    ctx.warnings.depWarning = s.checkDependencies(resolvedName, recent);
    const depGraph = s.getDependencyGraph();
    ctx.warnings.depValidation = depGraph.validate(resolvedName, recent);
    ctx.warnings.depGraph = depGraph;
    ctx.warnings.dupeWarning = s.checkIdempotency(resolvedName, ctx.finalParams, recent);
    ctx.warnings.conflict = s.detectConflict(resolvedName, ctx.finalParams);
    const budgetCheck = s.failureBudgetCheck(resolvedName);
    if (budgetCheck.blocked) {
        ctx.traceRecorder?.annotate("circuit_breaker");
        ctx.earlyReturn = { ok: false, error: budgetCheck.reason, _failure_budget: budgetCheck };
        return;
    }
    ctx.warnings.preflightWarning = s.preflightEntityCheck(resolvedName, ctx.finalParams, s.getSceneChanges(200));
    // Annotate which warnings were raised
    const flags = [];
    if (ctx.warnings.depWarning)
        flags.push("dep_warn");
    if (!ctx.warnings.depValidation?.satisfied)
        flags.push("dep_missing");
    if (ctx.warnings.dupeWarning)
        flags.push("idempotent");
    if (ctx.warnings.conflict)
        flags.push("conflict");
    if (ctx.warnings.preflightWarning)
        flags.push("preflight");
    if (flags.length > 0)
        ctx.traceRecorder?.annotate(flags.join(";"));
}
// ── Stage: Execute ─────────────────────────────────────────────────────────
export async function stageExecute(ctx) {
    const { services: s, resolvedName, handler } = ctx;
    ctx.startTime = performance.now();
    const throttle = s.getThrottleDecision();
    if (throttle.delay_ms > 0) {
        await new Promise((r) => setTimeout(r, throttle.delay_ms));
    }
    ctx.rawResult = await handler(ctx.finalParams);
    if (!ctx.rawResult.ok) {
        const enriched = s.enrichError(resolvedName, ctx.rawResult);
        const retry = s.shouldSmartRetry(enriched.error_category, enriched.retryable, 0);
        if (retry.shouldRetry) {
            await new Promise((r) => setTimeout(r, retry.delayMs));
            const retryResult = await handler(ctx.finalParams);
            if (retryResult.ok) {
                ctx.rawResult = mergeSafeRecords(toSafeRecord(retryResult), { _retried: true, _retry_reason: retry.reason });
                ctx.retried = true;
                ctx.traceRecorder?.annotate("retried_ok");
            }
            else {
                ctx.traceRecorder?.annotate("retried_fail");
            }
        }
        else {
            ctx.traceRecorder?.annotate("exec_fail");
        }
    }
    ctx.durationMs = Math.round(performance.now() - ctx.startTime);
    if (throttle.delay_ms > 0)
        ctx.traceRecorder?.annotate(`throttled:${throttle.delay_ms}ms`);
}
// ── Stage: Handle Failure ──────────────────────────────────────────────────
export function stageHandleFailure(ctx) {
    if (ctx.rawResult.ok)
        return;
    ctx.traceRecorder?.annotate("failure_path");
    const { services: s, resolvedName, durationMs } = ctx;
    const failEntry = { tool: resolvedName, ok: false, duration_ms: durationMs, timestamp: Date.now(), error: ctx.rawResult.error };
    s.sessionRecord(failEntry);
    s.latencyRecord(failEntry);
    s.throttleReport({ ok: false, duration_ms: durationMs, error: ctx.rawResult.error });
    s.failureBudgetRecord(resolvedName, false, ctx.rawResult.error);
    const enriched = s.enrichError(resolvedName, ctx.rawResult);
    const recovery = s.buildRecoveryAction(resolvedName, enriched.error_category, enriched.error);
    const callCount = s.getCallCount();
    const diagBlock = s.shouldAttachDiagnostics(enriched.error_category, callCount)
        ? s.gatherDiagnosticContext({
            category: enriched.error_category ?? "unknown",
            toolName: resolvedName,
            params: ctx.finalParams,
            error: enriched.error ?? "Unknown tool failure",
            sceneRecords: s.getSceneChanges(50),
            sessionRecords: s.getRecentSession(20),
            contextState: s.getContextSnapshot(),
        })
        : null;
    const failResponse = mergeSafeRecords(toSafeRecord(s.addTiming(enriched, ctx.startTime)), recovery ? { _recovery: recovery } : undefined, diagBlock ? { _diagnostics: diagBlock } : undefined, s.buildThrottleBlock() ?? undefined, { _session: s.getErrorContext() }, ctx.resolutionMeta);
    s.telemetryRecordCall(resolvedName, false, durationMs);
    s.telemetryRecordResponse(resolvedName, failResponse);
    if (recovery)
        ctx.traceRecorder?.annotate("recovery_attached");
    if (diagBlock)
        ctx.traceRecorder?.annotate("diagnostics_attached");
    ctx.earlyReturn = failResponse;
}
// ── Stage: Canonicalize + Extract Context ──────────────────────────────────
export function stageCanonicalizeAndExtract(ctx) {
    const { services: s, resolvedName } = ctx;
    let result = ctx.rawResult;
    if (result.result && typeof result.result === "object" && !Array.isArray(result.result)) {
        result = mergeSafeRecords(toSafeRecord(result), { result: s.canonicalizeOutput(resolvedName, result.result) });
        ctx.rawResult = result;
    }
    s.extractContext(resolvedName, result.result);
    const visionCtx = s.extractVisionContext(resolvedName, result);
    s.injectVisionContext(Object.entries(visionCtx));
}
// ── Stage: Transform Response ──────────────────────────────────────────────
export function stageTransformResponse(ctx) {
    const { services: s, resolvedName, durationMs } = ctx;
    const enhanced = s.enhanceVisionResponse(resolvedName, ctx.rawResult);
    const normalized = s.normalizeResponse(enhanced);
    if (normalized.result && typeof normalized.result === "object" && !Array.isArray(normalized.result)) {
        normalized.result = s.shapeResponse(resolvedName, normalized.result);
    }
    ctx.transformedResult = s.addWorkflowHint(resolvedName, normalized);
    // Record scene change + cost
    s.recordSceneChange(resolvedName, ctx.finalParams, Date.now());
    ctx.metadata._cost = s.classifyCostFromDuration(durationMs);
}
// ── Stage: Build Metadata ──────────────────────────────────────────────────
export function stageBuildMetadata(ctx) {
    const { services: s, resolvedName, metadata, warnings } = ctx;
    const safeResolutionMeta = toSafeRecord(ctx.resolutionMeta);
    for (const [key, value] of Object.entries(safeResolutionMeta)) {
        metadata[key] = value;
    }
    if (Object.keys(ctx.inferred).length > 0)
        metadata._inferred = toSafeRecord(ctx.inferred);
    if (Object.keys(ctx.defaults_applied).length > 0)
        metadata._defaults_applied = toSafeRecord(ctx.defaults_applied);
    if (warnings.depWarning)
        metadata._dependency_warning = warnings.depWarning;
    if (!warnings.depValidation?.satisfied) {
        metadata._dependency_graph = toSafeRecord({
            missing: warnings.depValidation.missing,
            suggested_sequence: warnings.depGraph.getCallSequence(resolvedName).sequence,
        });
    }
    if (warnings.dupeWarning)
        metadata._duplicate_warning = warnings.dupeWarning;
    if (warnings.conflict)
        metadata._conflict_warning = warnings.conflict.description;
    const safetyBlock = s.buildSafetyBlock();
    if (safetyBlock)
        metadata._scene_safety = safetyBlock;
    const throttleBlock = s.buildThrottleBlock();
    if (throttleBlock)
        metadata._throttle = throttleBlock;
    const echo = s.buildParamEcho(ctx.finalParams, ctx.defaults_applied, ctx.inferred);
    if (echo)
        metadata._param_echo = echo;
    if (warnings.preflightWarning)
        metadata._preflight_warning = warnings.preflightWarning;
    const durationEstimate = s.estimateToolDuration(resolvedName, s.getRecentSession(50));
    if (durationEstimate.confidence !== "none")
        metadata._duration_estimate = toSafeRecord(durationEstimate);
    ctx.finalResult = mergeSafeRecords(ctx.transformedResult, metadata);
}
// ── Stage: Record Success ──────────────────────────────────────────────────
export function stageRecordSuccess(ctx) {
    const { services: s, resolvedName, durationMs } = ctx;
    const sessionEntry = { tool: resolvedName, ok: true, duration_ms: durationMs, timestamp: Date.now(), cached: !!ctx.rawResult._cached };
    s.sessionRecord(sessionEntry);
    s.latencyRecord(sessionEntry);
    s.throttleReport({ ok: true, duration_ms: durationMs });
    s.failureBudgetRecord(resolvedName, true);
    const callCount = s.incrementCallCount();
    s.progressRecord(resolvedName, Date.now());
    ctx.metadata._callCount = callCount; // stash for digest stage
}
// ── Stage: Unlocked Tools + Digest + Health ────────────────────────────────
export function stageDigestAndHealth(ctx) {
    const { services: s } = ctx;
    const callCount = ctx.metadata._callCount;
    delete ctx.metadata._callCount;
    const recent200 = s.getRecentSession(200);
    const calledSet = new Set(recent200.filter((e) => e.ok).map((e) => e.tool));
    const depGraph = s.getDependencyGraph();
    const unlocked = depGraph.getUnlockedTools(calledSet);
    if (unlocked.length > 0) {
        ctx.finalResult = mergeSafeRecords(ctx.finalResult, { _unlocked_tools: unlocked });
    }
    const recent20 = s.getRecentSession(20);
    if (s.shouldAttachDigest(callCount, recent20)) {
        const rawDigest = s.buildDigest(callCount, recent20);
        const recent10 = s.getRecentSession(10);
        const digest = s.scopeDigest(rawDigest, recent10.map((e) => e.tool));
        ctx.finalResult = mergeSafeRecords(ctx.finalResult, { _digest: digest });
        const ctxSummary = s.buildContextSummary(s.getContextSnapshot());
        if (ctxSummary.field_count > 0)
            ctx.finalResult = mergeSafeRecords(ctx.finalResult, { _context: ctxSummary });
    }
    const recent15 = s.getRecentSession(15);
    if (s.shouldAttachHealth(callCount, recent15)) {
        ctx.finalResult = mergeSafeRecords(ctx.finalResult, { _health: s.buildHealthSummary(recent20) });
    }
}
// ── Stage: Delta Tracking ──────────────────────────────────────────────────
export function stageDeltaTracking(ctx) {
    const { services: s, resolvedName } = ctx;
    if (resolvedName.startsWith("observe") || resolvedName.startsWith("get_") || resolvedName.startsWith("list_")) {
        if (ctx.finalResult.result && typeof ctx.finalResult.result === "object" && !Array.isArray(ctx.finalResult.result)) {
            const delta = s.deltaTrack(resolvedName, ctx.finalResult.result);
            if (delta) {
                ctx.finalResult = mergeSafeRecords(ctx.finalResult, { _changed_since_last: delta });
                ctx.traceRecorder?.annotate("delta_found");
            }
        }
    }
}
// ── Stage: Micro-Verify ────────────────────────────────────────────────────
export async function stageMicroVerify(ctx) {
    const { services: s, resolvedName } = ctx;
    if (!s.hasMicroVerifyRule(resolvedName))
        return;
    const verifyResult = await s.microVerify(resolvedName, ctx.finalParams, async (tool, params) => {
        const handler = s.toolHandlers[tool];
        if (!handler)
            return { ok: false, error: `No handler for ${tool}` };
        return handler(params);
    });
    if (verifyResult) {
        if (verifyResult.verified) {
            ctx.finalResult = mergeSafeRecords(ctx.finalResult, {
                _verified: { ok: true, check: verifyResult.label, ms: verifyResult.duration_ms },
            });
            ctx.traceRecorder?.annotate("verify_pass");
        }
        else {
            ctx.finalResult = mergeSafeRecords(ctx.finalResult, {
                _verify_failed: { check: verifyResult.label, error: verifyResult.error, ms: verifyResult.duration_ms },
            });
            ctx.traceRecorder?.annotate("verify_fail");
        }
    }
}
// ── Stage: Finalize (telemetry + persist + timing) ─────────────────────────
export function stageFinalize(ctx) {
    const { services: s, resolvedName, durationMs } = ctx;
    s.telemetryRecordCall(resolvedName, true, durationMs);
    s.telemetryRecordResponse(resolvedName, ctx.finalResult);
    s.schedulePersist();
    ctx.finalResult = s.addTiming(ctx.finalResult, ctx.startTime);
}
// ── Default Pipeline ───────────────────────────────────────────────────────
/** The standard ordered stage list. */
export const DEFAULT_STAGES = [
    stageNormalizeAndCoerce,
    stageApplyDefaults,
    stageValidate,
    stageContextInject,
    stagePrerequisite,
    stagePreExecAnalysis,
    stageExecute,
    stageHandleFailure,
    stageCanonicalizeAndExtract,
    stageTransformResponse,
    stageBuildMetadata,
    stageRecordSuccess,
    stageDigestAndHealth,
    stageDeltaTracking,
    stageMicroVerify,
    stageFinalize,
];
/** Build a fresh DispatchContext ready for pipeline execution. */
export function createDispatchContext(resolvedName, rawArgs, handler, services, resolutionMeta) {
    return {
        resolvedName,
        rawArgs,
        handler,
        resolutionMeta,
        normalizedArgs: {},
        coercedArgs: {},
        defaultedArgs: {},
        defaults_applied: {},
        contextArgs: {},
        inferred: {},
        finalParams: {},
        warnings: {},
        startTime: 0,
        durationMs: 0,
        rawResult: { ok: false, error: "not executed" },
        retried: false,
        transformedResult: { ok: false },
        metadata: {},
        finalResult: { ok: false },
        services,
    };
}
//# sourceMappingURL=dispatch-pipeline.js.map