import { Server } from "@modelcontextprotocol/sdk/server/index.js";
import type { Tool } from "@modelcontextprotocol/sdk/types.js";
import { RiftbornResponse } from "./riftborn-types.js";
import type { SessionTracker } from "./system-enhancements.js";
export interface ToolCatalogEntry {
    name: string;
    description: string;
    tier: string;
    visible: boolean;
}
export interface CopilotOperatingContract {
    product: string;
    frontDoors: string[];
    loop: string[];
    workingRules: string[];
    discoveryResources: string[];
}
interface ResourceDeps {
    allTools: Tool[];
    visibleTools: () => Tool[];
    getToolCatalog: () => ToolCatalogEntry[];
    categoryMap: Record<string, string>;
    executeTool: (toolName: string, params?: object) => Promise<RiftbornResponse>;
    httpRequest: (method: "GET" | "POST", path: string, body?: object, timeoutMs?: number) => Promise<RiftbornResponse>;
    host: string;
    httpPort: number;
    tcpPort: number;
    brainPort: number;
    sessionTracker?: SessionTracker;
}
export declare function searchToolCatalog(catalog: ToolCatalogEntry[], query: string): ToolCatalogEntry[];
export declare function normalizeResourceSearchQuery(query: string): string;
export declare function normalizeResourceIdentifier(value: string): string;
export declare function decodeResourceComponent(component: string): string | null;
export declare function buildLocalServiceUrl(host: string, port: number, servicePath: string): string | null;
export declare function fetchLocalJsonResource(host: string, port: number, servicePath: string, label: string): Promise<Record<string, unknown>>;
export declare function getCopilotOperatingContract(): CopilotOperatingContract;
export declare function registerResourceHandlers(server: Server, { allTools, visibleTools, getToolCatalog, categoryMap, executeTool, httpRequest, host, httpPort, tcpPort, brainPort, sessionTracker }: ResourceDeps): void;
export {};
//# sourceMappingURL=mcp-resources.d.ts.map