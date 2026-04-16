/**
 * Vision Intelligence — Structured analysis of vision AI responses
 *
 * Turns free-text vision observations into actionable structured data:
 * - Issue detection with severity, category, and suggested fix
 * - Scene diffing via actor snapshot comparison
 * - Verification tracking across the build loop
 */
export interface VisionIssue {
    category: "lighting" | "material" | "placement" | "scale" | "composition" | "missing_content" | "performance";
    severity: "low" | "medium" | "high";
    description: string;
    suggested_tool?: string;
    suggested_args?: Record<string, unknown>;
}
export interface ParsedVisionResult {
    issues: VisionIssue[];
    quality: "poor" | "fair" | "good" | "excellent";
    actor_mentions: string[];
    screenshot_path?: string;
}
export interface ActorSnapshot {
    label: string;
    class?: string;
    location?: {
        x: number;
        y: number;
        z: number;
    };
}
export interface SceneDiff {
    actors_added: string[];
    actors_removed: string[];
    actors_moved: Array<{
        label: string;
        from: {
            x: number;
            y: number;
            z: number;
        };
        to: {
            x: number;
            y: number;
            z: number;
        };
    }>;
    timestamp: number;
}
export declare function parseVisionText(text: string): ParsedVisionResult;
export declare class SceneDiffTracker {
    private lastSnapshot;
    private lastTimestamp;
    /**
     * Record an actor list from an observation result.
     * Returns a diff if there was a previous snapshot, null otherwise.
     */
    update(actors: ActorSnapshot[]): SceneDiff | null;
    /** Get labels of all actors in the last snapshot. */
    get actorLabels(): string[];
    /** Check if a specific actor was seen in the last observation. */
    has(label: string): boolean;
    get snapshotSize(): number;
    clear(): void;
}
/**
 * Extract vision-related context values from a tool result.
 * Returns key-value pairs suitable for ContextPropagator storage.
 */
export declare function extractVisionContext(toolName: string, result: unknown): Record<string, string>;
/**
 * Enhance a vision tool result with structured analysis.
 * Called after a vision tool returns but before the response is sent to the agent.
 *
 * Adds `_vision` block with parsed issues, quality score, and scene diff.
 */
export declare function enhanceVisionResponse(toolName: string, result: unknown, diffTracker?: SceneDiffTracker): unknown;
/** Fields to keep for each vision-related tool in shapeResponse */
export declare const VISION_KEEP_FIELDS: Record<string, string[]>;
//# sourceMappingURL=vision-intelligence.d.ts.map