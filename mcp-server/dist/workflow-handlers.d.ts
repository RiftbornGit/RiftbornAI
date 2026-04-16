/**
 * Workflow-level tool handler overrides — these are tools implemented
 * directly in the entry-point because they need live state from
 * singletons (sessionTracker, sceneChangeLog, etc.) plus access to the
 * managed dispatcher. Extracted from index.ts as a single install()
 * function so index.ts stays focused on wiring.
 */
import type { RiftbornResponse, ToolHandler } from "./riftborn-types.js";
import { SessionBookmarks } from "./pipeline-refinements.js";
import type { ContextPropagator } from "./pipeline-intelligence.js";
import type { ProgressTracker } from "./session-intelligence.js";
import type { SceneChangeLog } from "./scene-safety.js";
import type { SessionTracker } from "./system-enhancements.js";
import type { PipelineTelemetry } from "./pipeline-telemetry.js";
import type { PipelineTraceStore } from "./pipeline-trace.js";
export interface WorkflowHandlerDeps {
    toolHandlers: Record<string, ToolHandler>;
    dispatchManagedTool: (name: string, params?: Record<string, unknown>) => Promise<RiftbornResponse>;
    sessionTracker: SessionTracker;
    contextPropagator: ContextPropagator;
    sceneChangeLog: SceneChangeLog;
    progressTracker: ProgressTracker;
    sessionBookmarks: SessionBookmarks;
    pipelineTelemetry: PipelineTelemetry;
    pipelineTraceStore: PipelineTraceStore;
}
export declare function installWorkflowHandlers(deps: WorkflowHandlerDeps): void;
//# sourceMappingURL=workflow-handlers.d.ts.map