/**
 * Managed tool dispatcher — builds the DispatchServices bundle from the
 * singletons owned by index.ts and runs the composable pipeline stages
 * (with trace recording and the error-path safe response builder).
 *
 * Extracted from index.ts so the entry-point stays focused on wiring.
 * All behavior is preserved verbatim; this file accepts its deps as a
 * single options bag instead of closing over module-scope globals.
 */
import type { RiftbornResponse, ToolHandler } from "./riftborn-types.js";
import { SessionTracker } from "./system-enhancements.js";
import { ContextPropagator } from "./pipeline-intelligence.js";
import { SchemaIntelligence } from "./schema-intelligence.js";
import { ToolResolver } from "./tool-resolution.js";
import { SceneDiffTracker } from "./vision-intelligence.js";
import { ProgressTracker } from "./session-intelligence.js";
import { SceneChangeLog } from "./scene-safety.js";
import { LatencyTracker } from "./performance-intelligence.js";
import { AdaptiveThrottle } from "./adaptive-throttle.js";
import { SessionPersistence } from "./session-persistence.js";
import { ResponseDeltaTracker, FailureBudget } from "./pipeline-refinements.js";
import { PipelineTelemetry } from "./pipeline-telemetry.js";
import { PipelineTraceStore } from "./pipeline-trace.js";
export interface DispatcherDeps {
    toolHandlers: Record<string, ToolHandler>;
    generatedToolNames: Set<string>;
    blockedTools: Set<string>;
    internalOnlyTools: Set<string>;
    getVisibleToolNames: () => Set<string>;
    enableInternalTools: boolean;
    allowHiddenTools: boolean;
    executeTool: (toolName: string, params?: object) => Promise<RiftbornResponse>;
    executeToolDirect: (toolName: string, params: Record<string, unknown>) => Promise<RiftbornResponse>;
    requireSchemaIntel: () => SchemaIntelligence;
    requireToolResolver: () => ToolResolver;
    contextPropagator: ContextPropagator;
    sceneDiffTracker: SceneDiffTracker;
    progressTracker: ProgressTracker;
    sceneChangeLog: SceneChangeLog;
    latencyTracker: LatencyTracker;
    adaptiveThrottle: AdaptiveThrottle;
    failureBudget: FailureBudget;
    responseDeltaTracker: ResponseDeltaTracker;
    pipelineTelemetry: PipelineTelemetry;
    pipelineTraceStore: PipelineTraceStore;
    sessionTracker: SessionTracker;
    sessionPersistence: SessionPersistence;
    getCallCount: () => number;
    incrementCallCount: () => number;
    digestInterval: number;
}
export declare function createManagedDispatcher(deps: DispatcherDeps): (name: string, args?: Record<string, unknown>) => Promise<RiftbornResponse>;
//# sourceMappingURL=dispatcher.d.ts.map