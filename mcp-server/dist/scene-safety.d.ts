/**
 * Scene Safety — Change tracking, budget awareness, and undo planning
 *
 * Three capabilities that keep agents from overloading the scene:
 *
 * 1. SceneChangeLog — Records every mutation (create, delete, move, modify).
 *    Provides structured undo plans that reverse recent operations.
 *
 * 2. SceneBudget — Tracks running counts of actors, lights, materials, etc.
 *    Warns when the scene approaches UE performance limits.
 *
 * 3. ConflictDetector — Catches contradictory calls (e.g., deleting an actor
 *    that was just created, moving something that was already deleted).
 */
export type ChangeKind = "create" | "delete" | "move" | "modify";
export interface SceneChange {
    id: number;
    kind: ChangeKind;
    tool: string;
    label?: string;
    category: SceneCategory;
    params: Record<string, unknown>;
    timestamp: number;
}
export type SceneCategory = "actor" | "light" | "material" | "foliage" | "vfx" | "audio" | "landscape" | "postprocess" | "blueprint" | "other";
export interface UndoStep {
    description: string;
    tool: string;
    params: Record<string, unknown>;
}
export interface BudgetEntry {
    category: SceneCategory;
    count: number;
    warn_at: number;
    error_at: number;
    status: "ok" | "warning" | "over_budget";
}
export interface BudgetReport {
    entries: BudgetEntry[];
    warnings: string[];
}
export interface Conflict {
    description: string;
    recent_change_id: number;
}
export declare class SceneChangeLog {
    private changes;
    private nextId;
    private counts;
    /** Record a tool call as a scene change. Returns the change if tracked. */
    record(toolName: string, params: Record<string, unknown>, timestamp: number): SceneChange | null;
    /** Get the last N changes (most recent first). */
    recent(n?: number): SceneChange[];
    /** Get running budget report with warnings. */
    getBudget(): BudgetReport;
    /** Generate an undo plan for the last N changes. */
    getUndoPlan(n?: number): UndoStep[];
    /** Check if a proposed tool call conflicts with recent changes. */
    detectConflict(toolName: string, params: Record<string, unknown>): Conflict | null;
    /** Build a compact summary for injection into tool responses. */
    buildSafetyBlock(): Record<string, unknown> | null;
    /** Reset state (for testing or level changes). */
    reset(): void;
    /** Serialize state for persistence. */
    serialize(): {
        changes: SceneChange[];
        nextId: number;
        counts: Record<string, number>;
    };
    /** Restore state from a persisted snapshot. */
    restore(data: {
        changes: SceneChange[];
        nextId: number;
        counts: Record<string, number>;
    }): void;
}
//# sourceMappingURL=scene-safety.d.ts.map