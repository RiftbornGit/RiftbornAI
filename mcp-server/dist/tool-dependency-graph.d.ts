/**
 * Tool Dependency Graph — Round 16
 *
 * Static DAG of UE5 tool prerequisites. Replaces the flat DEPENDENCY_RULES
 * table in proactive-guards.ts with a proper graph that supports:
 *
 * 1. Transitive resolution — walks ancestors to find ALL prerequisites
 * 2. Ordering — returns the minimal call sequence to reach a target tool
 * 3. Multi-path validation — OR-branches (any of N tools satisfies a dep)
 * 4. Session-aware checking — validates against actual session history
 *
 * The graph is declared once and queried at runtime. It is intentionally
 * not dynamic — tool relationships in UE5 are structural, not discovered.
 */
import type { SessionEntry } from "./system-enhancements.js";
/** A single edge in the dependency graph. */
export interface DepEdge {
    /** One or more tools that satisfy this dependency (OR). */
    requires: string[];
    /** Human-readable reason for this dependency. */
    reason: string;
}
/** Resolved prerequisite chain for a tool. */
export interface PrereqChain {
    target: string;
    /** Ordered list from earliest prerequisite to the target itself. */
    sequence: string[];
    /** The OR-groups at each step (for tools with alternatives). */
    steps: Array<{
        tool: string;
        alternatives: string[];
    }>;
}
/** Result of validating a tool against session history. */
export interface DepValidation {
    tool: string;
    satisfied: boolean;
    /** Missing prerequisites (direct, not transitive). */
    missing: Array<{
        requires: string[];
        reason: string;
        suggestion: string;
    }>;
}
export declare class ToolDependencyGraph {
    private readonly edges;
    /** Reverse index: tool → tools that depend on it. */
    private readonly reverseMap;
    constructor(graph?: Record<string, DepEdge[]>);
    private buildReverseMap;
    /** Get direct prerequisites for a tool. */
    getDirectDeps(tool: string): DepEdge[];
    /** Get tools that directly depend on a given tool. */
    getDependents(tool: string): string[];
    /** Check whether a tool has any registered dependencies. */
    hasDeps(tool: string): boolean;
    /**
     * Collect ALL transitive prerequisites for a tool.
     *
     * Returns a flat set of tool names. For OR-groups, includes all
     * alternatives (any one satisfies the dep at runtime).
     *
     * Cycle-safe via visited tracking.
     */
    getAllPrereqs(tool: string): Set<string>;
    private collectPrereqs;
    /**
     * Build the recommended call sequence to reach a target tool.
     *
     * Returns tools in dependency order (earliest prerequisite first,
     * target tool last). For OR-groups, picks the first alternative.
     *
     * This is a topological sort of the reachable subgraph.
     */
    getCallSequence(tool: string): PrereqChain;
    private topoVisit;
    /**
     * Validate a tool against session history.
     *
     * Returns which direct dependencies are satisfied vs missing.
     * This replaces the flat DEPENDENCY_RULES lookup in proactive-guards.ts.
     */
    validate(tool: string, sessionHistory: SessionEntry[]): DepValidation;
    /**
     * Given a set of successfully called tools, find which tools are now
     * "unlocked" (all their dependencies are in the called set).
     *
     * Useful for suggesting next possible actions.
     */
    getUnlockedTools(calledTools: Set<string>): string[];
    /** How many tools are tracked in the graph. */
    get size(): number;
    /** All tool names that have dependency entries. */
    get tools(): string[];
}
export declare function getToolDependencyGraph(): ToolDependencyGraph;
/** Reset the singleton (for testing). */
export declare function resetToolDependencyGraph(): void;
//# sourceMappingURL=tool-dependency-graph.d.ts.map