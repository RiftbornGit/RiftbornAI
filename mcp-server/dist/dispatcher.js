/**
 * Managed tool dispatcher — builds the DispatchServices bundle from the
 * singletons owned by index.ts and runs the composable pipeline stages
 * (with trace recording and the error-path safe response builder).
 *
 * Extracted from index.ts so the entry-point stays focused on wiring.
 * All behavior is preserved verbatim; this file accepts its deps as a
 * single options bag instead of closing over module-scope globals.
 */
import { enrichError, normalizeParams, addWorkflowHint, checkPrerequisite, addTiming } from "./agent-assist.js";
import { normalizeResponse } from "./system-enhancements.js";
import { shouldSmartRetry } from "./pipeline-intelligence.js";
import { buildRecoveryAction } from "./schema-intelligence.js";
import { shapeResponse, classifyCostFromDuration } from "./tool-resolution.js";
import { applyDefaults, checkDependencies, checkIdempotency } from "./proactive-guards.js";
import { enhanceVisionResponse, extractVisionContext } from "./vision-intelligence.js";
import { buildDigest, shouldAttachDigest } from "./session-intelligence.js";
import { buildHealthSummary, shouldAttachHealth } from "./performance-intelligence.js";
import { getToolAccessError, resolveToolInvocation } from "./dispatch-policy.js";
import { gatherDiagnosticContext, shouldAttachDiagnostics } from "./error-recovery.js";
import { buildThrottleBlock } from "./adaptive-throttle.js";
import { getToolDependencyGraph } from "./tool-dependency-graph.js";
import { canonicalizeOutput, preflightEntityCheck, scopeDigest, buildParamEcho, estimateToolDuration, buildContextSummary, } from "./pipeline-refinements.js";
import { recordResponseTelemetry } from "./pipeline-telemetry.js";
import { createDispatchContext, DEFAULT_STAGES } from "./dispatch-pipeline.js";
import { TraceRecorder, runTracedPipeline, buildTrace } from "./pipeline-trace.js";
import { buildSafeCatchResponse } from "./response-builders.js";
export function createManagedDispatcher(deps) {
    const { toolHandlers: TOOL_HANDLERS, generatedToolNames: GENERATED_TOOL_NAMES, blockedTools: BLOCKED_TOOLS, internalOnlyTools: INTERNAL_ONLY_TOOLS, getVisibleToolNames, enableInternalTools, allowHiddenTools, executeTool, executeToolDirect, requireSchemaIntel, requireToolResolver, contextPropagator, sceneDiffTracker, progressTracker, sceneChangeLog, latencyTracker, adaptiveThrottle, failureBudget, responseDeltaTracker, pipelineTelemetry, pipelineTraceStore, sessionTracker, sessionPersistence, getCallCount, incrementCallCount, digestInterval, } = deps;
    async function dispatchManagedTool(requestedName, rawArgs = {}) {
        const resolution = resolveToolInvocation({
            name: requestedName,
            toolHandlers: TOOL_HANDLERS,
            generatedToolNames: GENERATED_TOOL_NAMES,
            toolResolver: requireToolResolver(),
        });
        if ("ok" in resolution) {
            return resolution;
        }
        const resolvedName = resolution.resolvedName;
        const accessError = getToolAccessError(resolvedName, {
            blockedTools: BLOCKED_TOOLS,
            internalOnlyTools: INTERNAL_ONLY_TOOLS,
            visibleToolNames: getVisibleToolNames(),
            enableInternalTools,
            allowHiddenTools,
        });
        if (accessError) {
            return accessError;
        }
        let handler = TOOL_HANDLERS[resolvedName];
        if (!handler && GENERATED_TOOL_NAMES.has(resolvedName)) {
            handler = async (toolArgs) => executeTool(resolvedName, toolArgs);
        }
        if (!handler) {
            return { ok: false, error: `Unknown tool: ${resolvedName}.` };
        }
        // Round 23: Dispatch through composable pipeline stages
        const hooks = await import("./dispatch-hooks.js");
        const services = {
            normalizeParams: (t, a) => normalizeParams(t, a),
            coerceParams: (t, a) => requireSchemaIntel().coerceParams(t, a),
            applyDefaults: (t, a) => applyDefaults(t, a),
            validateParams: (t, a) => requireSchemaIntel().validateParams(t, a),
            contextInject: (t, a) => contextPropagator.inject(t, a),
            checkPrerequisite: (t, a) => checkPrerequisite(t, a),
            checkDependencies: (t, r) => checkDependencies(t, r),
            getDependencyGraph: () => getToolDependencyGraph(),
            checkIdempotency: (t, a, r) => checkIdempotency(t, a, r),
            detectConflict: (t, a) => sceneChangeLog.detectConflict(t, a),
            failureBudgetCheck: (t) => failureBudget.check(t),
            preflightEntityCheck: (t, a, c) => preflightEntityCheck(t, a, c),
            getThrottleDecision: () => adaptiveThrottle.getDecision(),
            enrichError: (t, r) => enrichError(t, r),
            shouldSmartRetry: (cat, ret, att) => shouldSmartRetry(cat ?? "unknown", ret ?? false, att),
            buildRecoveryAction: (t, cat, err) => buildRecoveryAction(t, cat ?? "unknown", err ?? ""),
            shouldAttachDiagnostics: (cat, cc) => shouldAttachDiagnostics(cat ?? "unknown", cc),
            gatherDiagnosticContext: (info) => gatherDiagnosticContext(info),
            canonicalizeOutput: (t, r) => canonicalizeOutput(t, r),
            extractContext: (t, r) => contextPropagator.extract(t, r),
            extractVisionContext: (t, r) => extractVisionContext(t, r),
            injectVisionContext: (entries) => { const BLOCKED = new Set(['__proto__', 'constructor', 'prototype', 'toString', 'valueOf', 'hasOwnProperty', '__defineGetter__', '__defineSetter__']); for (const [k, v] of entries) {
                if (typeof k === 'string' && typeof v === 'string' && !BLOCKED.has(k))
                    contextPropagator.context[k] = v;
            } },
            enhanceVisionResponse: (t, r) => enhanceVisionResponse(t, r, sceneDiffTracker),
            normalizeResponse: (r) => normalizeResponse(r),
            shapeResponse: (t, r) => shapeResponse(t, r),
            addWorkflowHint: (t, r) => addWorkflowHint(t, r),
            classifyCostFromDuration: (ms) => classifyCostFromDuration(ms),
            recordSceneChange: (t, a, ts) => sceneChangeLog.record(t, a, ts),
            buildSafetyBlock: () => sceneChangeLog.buildSafetyBlock(),
            buildThrottleBlock: () => buildThrottleBlock(adaptiveThrottle),
            buildParamEcho: (a, d, i) => buildParamEcho(a, d, i),
            estimateToolDuration: (t, r) => estimateToolDuration(t, r),
            sessionRecord: (e) => sessionTracker.record(e),
            latencyRecord: (e) => latencyTracker.record(e),
            throttleReport: (o) => adaptiveThrottle.reportOutcome(o),
            failureBudgetRecord: (t, ok, err) => ok ? failureBudget.recordSuccess(t) : failureBudget.recordFailure(t, err),
            progressRecord: (t, ts) => progressTracker.record(t, ts),
            incrementCallCount: () => incrementCallCount(),
            getCallCount: () => getCallCount(),
            getRecentSession: (n) => sessionTracker.recent(n),
            getErrorContext: () => sessionTracker.getErrorContext(),
            getSceneChanges: (n) => sceneChangeLog.recent(n),
            getContextSnapshot: () => contextPropagator.snapshot(),
            buildDigest: (cc, r) => buildDigest(cc, progressTracker, r),
            shouldAttachDigest: (cc, r) => shouldAttachDigest(cc, digestInterval, r),
            scopeDigest: (d, r) => scopeDigest(d, r),
            buildContextSummary: (s) => buildContextSummary(s),
            shouldAttachHealth: (cc, r) => shouldAttachHealth(cc, undefined, r),
            buildHealthSummary: (r) => buildHealthSummary(latencyTracker, r),
            deltaTrack: (t, r) => responseDeltaTracker.track(t, r),
            microVerify: (t, a, exec) => hooks.microVerify(t, a, exec),
            hasMicroVerifyRule: (t) => hooks.hasMicroVerifyRule(t),
            executeToolDirect: (t, p) => executeToolDirect(t, p),
            telemetryRecord: (f, t) => pipelineTelemetry.record(f, t),
            telemetryRecordCall: (t, ok, ms) => pipelineTelemetry.recordCall(t, ok, ms),
            telemetryRecordResponse: (t, r) => recordResponseTelemetry(pipelineTelemetry, t, r),
            schedulePersist: () => sessionPersistence.scheduleSave(),
            addTiming: (r, st) => addTiming(r, st),
            toolHandlers: TOOL_HANDLERS,
        };
        const ctx = createDispatchContext(resolvedName, rawArgs, handler, services, resolution.resolutionMeta);
        const traceRecorder = new TraceRecorder();
        const tracedCtx = { ...ctx, traceRecorder };
        try {
            const pipelineStart = performance.now();
            const result = await runTracedPipeline(DEFAULT_STAGES, tracedCtx);
            const totalMs = Math.round((performance.now() - pipelineStart) * 100) / 100;
            const outcome = tracedCtx.earlyReturn ? "earlyReturn" : "success";
            pipelineTraceStore.record(buildTrace(tracedCtx, outcome, totalMs));
            return result;
        }
        catch (error) {
            const totalMs = Math.round((performance.now() - (tracedCtx.startTime || performance.now())) * 100) / 100;
            pipelineTraceStore.record(buildTrace(tracedCtx, "error", totalMs));
            const errMsg = error instanceof Error ? error.message : String(error);
            sessionTracker.record({ tool: resolvedName, ok: false, duration_ms: 0, timestamp: Date.now(), error: errMsg });
            adaptiveThrottle.reportOutcome({ ok: false, duration_ms: 0, error: errMsg });
            const enriched = enrichError(resolvedName, { ok: false, error: errMsg });
            const diagBlock = shouldAttachDiagnostics(enriched.error_category, getCallCount())
                ? gatherDiagnosticContext({
                    category: enriched.error_category,
                    toolName: resolvedName,
                    params: rawArgs,
                    error: errMsg,
                    sceneRecords: sceneChangeLog.recent(50),
                    sessionRecords: sessionTracker.recent(20),
                    contextState: contextPropagator.snapshot(),
                })
                : null;
            const catchResponse = buildSafeCatchResponse(enriched, {
                diagnostics: diagBlock ?? undefined,
                session: sessionTracker.getErrorContext(),
                resolutionMeta: resolution.resolutionMeta ?? undefined,
            });
            pipelineTelemetry.recordCall(resolvedName, false, 0);
            recordResponseTelemetry(pipelineTelemetry, resolvedName, catchResponse);
            return catchResponse;
        }
    }
    return dispatchManagedTool;
}
//# sourceMappingURL=dispatcher.js.map