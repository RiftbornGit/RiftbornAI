/**
 * Tool Resolution & Response Shaping — Round 7 enhancements
 *
 * 1. ToolResolver — Fuzzy tool name resolution. When agents call a wrong name
 *    (e.g. "spawn_light" → "create_light"), auto-resolves and executes the
 *    correct tool instead of returning "Unknown tool". Uses Levenshtein distance
 *    + prefix/suffix matching for high confidence, otherwise returns suggestions.
 *
 * 2. ResponseShaper — Per-tool extraction of the fields agents actually need.
 *    e.g. spawn_actor returns 15+ fields from UE; agents only need label, class, location.
 *    Reduces token consumption without losing actionable information.
 *
 * 3. CostTagger — Tags each response with execution cost class so agents can
 *    plan: "cheap" (<1s reads), "moderate" (1-5s mutations), "expensive" (>5s compiles/builds).
 */
import type { Tool } from "@modelcontextprotocol/sdk/types.js";
export interface ResolveResult {
    /** The resolved tool name (null if no match found). */
    resolved: string | null;
    /** Confidence: "exact" | "alias" | "high" | "medium" | "none" */
    confidence: "exact" | "alias" | "high" | "medium" | "none";
    /** Top suggestions if not auto-resolved. */
    suggestions: string[];
}
export declare class ToolResolver {
    private names;
    private nameSet;
    constructor(tools: Tool[]);
    /**
     * Resolve a tool name that wasn't found in the registry.
     *
     * Resolution order:
     * 1. Exact match (already handled upstream — this is a fallback)
     * 2. Static alias lookup
     * 3. Levenshtein distance ≤ 2 (high confidence auto-resolve)
     * 4. Prefix/suffix overlap (medium confidence — return suggestions)
     */
    resolve(name: string): ResolveResult;
}
/**
 * Shape a tool response to only include the fields agents need.
 * Falls back to the full response if:
 * - Tool has no shaping rule
 * - Result is not an object
 * - Result is a string or array (handled by normalizeResponse)
 */
export declare function shapeResponse(toolName: string, result: unknown): unknown;
export type CostClass = "cheap" | "moderate" | "expensive";
/**
 * Classify a tool's expected execution cost.
 * Based on prefix patterns: reads are cheap, compiles/builds are expensive,
 * everything else is moderate.
 */
export declare function classifyCost(toolName: string): CostClass;
/**
 * Classify cost from actual execution duration.
 * Overrides the prefix heuristic with real timing data.
 */
export declare function classifyCostFromDuration(durationMs: number): CostClass;
//# sourceMappingURL=tool-resolution.d.ts.map