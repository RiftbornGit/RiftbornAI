/**
 * Pipeline Intelligence — Round 4 enhancements for the RiftbornAI MCP Server
 *
 * 1. ToolRouter — Keyword search over the tool registry.
 *    Powers the `find_tools` meta-tool so agents can discover tools
 *    without scanning the full 672+ tool listing (~145K tokens).
 *
 * 2. shouldSmartRetry — Decides whether a failed tool call should auto-retry.
 *    Only retries on transient bridge failures (disconnected/timeout), max once.
 *
 * 3. ContextPropagator — Tracks outputs from successful tool calls and
 *    auto-fills missing parameters in subsequent calls.
 *    e.g. spawn_actor returns label → move_actor auto-fills label.
 */
export interface ToolSearchResult {
    name: string;
    description: string;
    required_params: string[];
    relevance: number;
    inputSchema?: any;
}
export declare function normalizeToolSearchQuery(query: string): string;
export declare function clampToolSearchResults(maxResults: number, fallback?: number): number;
export declare function shouldResetPipelineStateAfterTool(toolName: string): boolean;
export declare class ToolRouter {
    private index;
    constructor(tools: Array<{
        name: string;
        description?: string;
        inputSchema?: any;
    }>);
    /**
     * Search tools by keyword. Scores by name match (highest), parameter names
     * (medium), and description (lowest). Returns up to maxResults sorted by relevance.
     */
    search(query: string, maxResults?: number): ToolSearchResult[];
    /** Get tools by exact name prefix (e.g., "create_", "get_"). */
    byPrefix(prefix: string, maxResults?: number): string[];
    get size(): number;
}
export interface RetryDecision {
    shouldRetry: boolean;
    delayMs: number;
    reason: string;
}
/**
 * Decide whether a failed tool call should be auto-retried.
 * Only bridge_disconnected and bridge_timeout qualify.
 * Maximum 1 retry per call (attemptNumber must be 0).
 */
export declare function shouldSmartRetry(errorCategory: string, retryable: boolean, attemptNumber: number): RetryDecision;
export declare class ContextPropagator {
    private context;
    /** Extract and store context values from a successful tool response. */
    extract(toolName: string, result: unknown): Record<string, string>;
    /**
     * Inject stored context into missing tool parameters.
     * Returns augmented params and a map of inferred fields → source context key.
     * Never overwrites a param the agent already provided.
     */
    inject(toolName: string, params: Record<string, unknown>): {
        params: Record<string, unknown>;
        inferred: Record<string, string>;
    };
    /** Get current context snapshot (for debugging/resources). */
    snapshot(): Record<string, string>;
    /** Restore context from a serialized snapshot. */
    restore(data: Record<string, string>): void;
    /** Clear all stored context. */
    clear(): void;
    get size(): number;
}
//# sourceMappingURL=pipeline-intelligence.d.ts.map