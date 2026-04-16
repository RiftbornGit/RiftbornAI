import { Tool } from "@modelcontextprotocol/sdk/types.js";
import type { RiftbornResponse, ToolHandler } from "./riftborn-types.js";
type ExecuteToolFn = (toolName: string, params?: Record<string, unknown>) => Promise<RiftbornResponse>;
export declare const SCENE_ENHANCEMENT_TOOLS: Tool[];
export declare function createSceneEnhancementHandlers(executeTool: ExecuteToolFn): Record<string, ToolHandler>;
export {};
//# sourceMappingURL=scene-enhancement.d.ts.map