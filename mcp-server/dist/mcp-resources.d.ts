import { Server } from "@modelcontextprotocol/sdk/server/index.js";
import type { Tool } from "@modelcontextprotocol/sdk/types.js";
import { RiftbornResponse } from "./riftborn-types.js";
import type { SessionTracker } from "./system-enhancements.js";
export { getDomainDefinitionOfDoneContract } from "./domain-proof-contract.js";
export interface ToolCatalogEntry {
    name: string;
    description: string;
    tier: string;
    reason?: string;
    visible: boolean;
}
export interface CopilotOperatingContract {
    product: string;
    frontDoors: string[];
    loop: string[];
    workingRules: string[];
    discoveryResources: string[];
}
export interface CopilotSkillPackEntry {
    name: string;
    kind: string;
    summary: string;
    specialist?: string;
    profile?: string;
    when: string[];
    source: string;
    filename: string;
}
export interface CopilotAgentManifestEntry {
    name: string;
    alias: string;
    summary: string;
    profile?: string;
    when: string[];
    skill_packs: string[];
    tool_include: string[];
    tool_exclude: string[];
    provider?: string;
    source: string;
    filename: string;
    max_iterations?: number;
    timeout_seconds?: number;
    include_scene_context?: boolean;
    read_only_bias?: boolean;
    verification_bias?: boolean;
    completion_style?: string;
}
export declare function readCopilotSkillPackCatalog(options?: {
    pluginRoot?: string;
    projectRoot?: string;
    userRoot?: string;
}): CopilotSkillPackEntry[];
export declare function searchSkillPackCatalog(catalog: CopilotSkillPackEntry[], query: string): CopilotSkillPackEntry[];
export declare function readCopilotAgentCatalog(options?: {
    pluginRoot?: string;
    projectRoot?: string;
    userRoot?: string;
}): CopilotAgentManifestEntry[];
export declare function searchAgentCatalog(catalog: CopilotAgentManifestEntry[], query: string): CopilotAgentManifestEntry[];
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