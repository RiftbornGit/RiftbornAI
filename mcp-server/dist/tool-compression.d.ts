/**
 * Tool Compression & Workflow Graph — Round 5 enhancements
 *
 * 1. compressToolListing — Shrinks the ListTools payload by ~40%:
 *    - Shortens long descriptions to first sentence
 *    - Strips parameter-level descriptions from inputSchema
 *    - Preserves property names, types, required, enum, default
 *    Full schemas remain available via find_tools and describe_tool.
 *
 * 2. WorkflowGraph — Encodes common multi-step sequences that agents
 *    struggle to discover independently. Powers the `get_workflow` tool.
 */
import type { Tool } from "@modelcontextprotocol/sdk/types.js";
export declare function normalizeWorkflowQuery(name: string): string;
/**
 * Compress a tool listing for the ListTools response.
 * Returns a new array — does not mutate the original tools.
 *
 * - Descriptions shortened to first sentence (max 150 chars)
 * - Parameter descriptions removed from inputSchema
 * - Type info, required fields, enums, defaults preserved
 */
export declare function compressToolListing(tools: Tool[]): Tool[];
export interface WorkflowStep {
    tool: string;
    description: string;
    key_params?: string[];
    notes?: string;
}
export interface Workflow {
    name: string;
    description: string;
    steps: WorkflowStep[];
}
/**
 * Get a named workflow (multi-step build sequence).
 * Returns null if workflow name is not found.
 */
export declare function getWorkflow(name: string): Workflow | null;
/**
 * List all available workflow names with descriptions.
 */
export declare function listWorkflows(): Array<{
    name: string;
    description: string;
    step_count: number;
}>;
//# sourceMappingURL=tool-compression.d.ts.map