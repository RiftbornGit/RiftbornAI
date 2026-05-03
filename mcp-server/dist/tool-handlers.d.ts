import type { Tool } from "@modelcontextprotocol/sdk/types.js";
import { RiftbornResponse, ToolHandler } from "./riftborn-types.js";
interface ToolHandlerDeps {
    executeTool: (toolName: string, params?: object) => Promise<RiftbornResponse>;
    dispatchTool?: (toolName: string, params?: Record<string, unknown>) => Promise<RiftbornResponse>;
    httpRequest: (method: "GET" | "POST", path: string, body?: object, timeoutMs?: number) => Promise<RiftbornResponse>;
    host: string;
    httpPort: number;
    visibleTools?: () => Tool[];
}
export declare function buildSafePlanWorkflowResult(plan: unknown, batchSteps: unknown): Record<string, unknown>;
export declare function createToolHandlers({ executeTool, dispatchTool, httpRequest, host: _host, httpPort: _httpPort, visibleTools }: ToolHandlerDeps): Record<string, ToolHandler>;
export {};