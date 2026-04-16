import type { ToolHandler } from "./riftborn-types.js";
import type { ToolResolver } from "./tool-resolution.js";
export interface ToolAccessConfig {
    blockedTools: Set<string>;
    internalOnlyTools: Set<string>;
    visibleToolNames: Set<string>;
    enableInternalTools: boolean;
    allowHiddenTools: boolean;
}
export interface ToolAccessError {
    ok: false;
    error: string;
}
export interface ResolutionSuccess {
    resolvedName: string;
    resolutionMeta?: {
        _resolved_from: string;
        _resolved_to: string;
        _confidence: "alias" | "high";
        _requires_access_recheck?: boolean;
    };
}
export interface ResolutionFailure {
    ok: false;
    error: string;
}
export declare function getToolAccessError(name: string, access: ToolAccessConfig): ToolAccessError | null;
interface ResolutionConfig {
    name: string;
    toolHandlers: Record<string, ToolHandler>;
    generatedToolNames: Set<string>;
    toolResolver: ToolResolver;
}
export declare function resolveToolInvocation({ name, toolHandlers, generatedToolNames, toolResolver, }: ResolutionConfig): ResolutionSuccess | ResolutionFailure;
export {};
//# sourceMappingURL=dispatch-policy.d.ts.map