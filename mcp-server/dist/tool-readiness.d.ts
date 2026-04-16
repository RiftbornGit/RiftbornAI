/**
 * Tool Readiness Gate
 *
 * Classifies every MCP tool into a maturity tier so the ListTools response
 * only advertises tools the LLM can actually call successfully.
 *
 * This file is the MCP visibility projection, not the canonical shipped-surface
 * membership root. Shipped membership lives in
 * `Bridge/toolbook/public_surface.json`; see `docs/READINESS_TRUTH.md`.
 *
 * Tiers:
 *   PRODUCTION  – Curated handler, tested, safe to call.
 *   BETA        – Handler exists but uses generated passthrough; works when UE is running.
 *   EXPERIMENTAL– Generated tool with no curated handler; may fail or have rough edges.
 *   STUB        – Known broken or unimplemented; hidden by default.
 *   DEPRECATED  – Scheduled for removal; hidden by default.
 *
 * Usage in index.ts:
 *   import { filterToolsByReadiness, ReadinessTier, getToolReadiness } from "./tool-readiness.js";
 *   const visibleTools = filterToolsByReadiness(ALL_TOOLS, ["PRODUCTION", "BETA"]);
 */
import { Tool } from "@modelcontextprotocol/sdk/types.js";
export type ReadinessTier = "PRODUCTION" | "BETA" | "EXPERIMENTAL" | "STUB" | "DEPRECATED";
export interface ToolReadinessEntry {
    tier: ReadinessTier;
    reason?: string;
}
/**
 * Classify a tool that has no explicit override.
 * Rules:
 *   1. If the tool name appears in TOOL_HANDLERS (passed set), it gets BETA
 *      (has curated handler with non-trivial logic).
 *   2. If the tool is in the manual TOOLS array AND has a generated backend,
 *      it gets BETA (curated schema over a real executable surface).
 *   3. If the tool is generated only, it is EXPERIMENTAL/STUB depending on
 *      the expose-generated flag.
 *   4. Manual-only tools with no handler and no generated backend are STUB.
 */
export declare function classifyTool(toolName: string, handlerNames: Set<string>, manualToolNames?: Set<string>, generatedToolNames?: Set<string>): ReadinessTier;
/**
 * Get the readiness tier for a single tool.
 */
export declare function getToolReadiness(toolName: string, handlerNames: Set<string>, manualToolNames?: Set<string>, generatedToolNames?: Set<string>): ToolReadinessEntry;
/**
 * Filter a list of tools to those matching the requested tiers.
 * Also injects a [TIER] badge into the tool description so the LLM
 * knows the maturity level.
 */
export declare function filterToolsByReadiness(tools: Tool[], allowedTiers: ReadinessTier[], handlerNames: Set<string>, manualToolNames?: Set<string>, generatedToolNames?: Set<string>): Tool[];
/**
 * Return a summary of how many tools fall into each tier.
 * Useful for diagnostics and the bootstrap report.
 */
export declare function getReadinessSummary(tools: Tool[], handlerNames: Set<string>, manualToolNames?: Set<string>, generatedToolNames?: Set<string>): Record<ReadinessTier, number>;
//# sourceMappingURL=tool-readiness.d.ts.map