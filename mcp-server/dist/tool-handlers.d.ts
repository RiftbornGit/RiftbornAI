import { RiftbornResponse, ToolHandler } from "./riftborn-types.js";
interface ToolHandlerDeps {
    executeTool: (toolName: string, params?: object) => Promise<RiftbornResponse>;
    dispatchTool?: (toolName: string, params?: Record<string, unknown>) => Promise<RiftbornResponse>;
    httpRequest: (method: "GET" | "POST", path: string, body?: object, timeoutMs?: number) => Promise<RiftbornResponse>;
    host: string;
    httpPort: number;
}
export declare function buildSafePlanWorkflowResult(plan: unknown, batchSteps: unknown): Record<string, unknown>;
export declare function createToolHandlers({ executeTool, dispatchTool, httpRequest, host: _host, httpPort: _httpPort }: ToolHandlerDeps): Record<string, ToolHandler>;
export {};
//# sourceMappingURL=tool-handlers.d.ts.map