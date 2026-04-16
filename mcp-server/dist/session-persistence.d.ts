/**
 * Round 18 — Session Persistence
 *
 * Serializes key pipeline state to a temp file so MCP reconnections
 * don't lose context.  Four state sources are persisted:
 *
 *   1. ContextPropagator  — last-known output values (actor labels, paths, etc.)
 *   2. SessionTracker     — recent call ring buffer + counters
 *   3. SceneChangeLog     — tracked scene mutations + category counts
 *   4. ProgressTracker    — build-phase milestones
 *
 * State is written lazily (debounced) after each tool dispatch.
 * On startup, state is restored only if the file is < MAX_AGE_MS old.
 */
import type { SessionTracker } from "./system-enhancements.js";
import type { ContextPropagator } from "./pipeline-intelligence.js";
import type { SceneChangeLog } from "./scene-safety.js";
import type { ProgressTracker } from "./session-intelligence.js";
import type { SessionEntry } from "./system-enhancements.js";
import type { SceneChange } from "./scene-safety.js";
import type { Milestone } from "./session-intelligence.js";
export interface SessionSnapshot {
    version: number;
    saved_at: number;
    context: Record<string, string>;
    session: {
        buffer: SessionEntry[];
        totalCalls: number;
        totalErrors: number;
    };
    scene: {
        changes: SceneChange[];
        nextId: number;
        counts: Record<string, number>;
    };
    progress: {
        milestones: Milestone[];
    };
}
export interface PersistableSources {
    contextPropagator: ContextPropagator;
    sessionTracker: SessionTracker;
    sceneChangeLog: SceneChangeLog;
    progressTracker: ProgressTracker;
}
export declare function collectSnapshot(sources: PersistableSources): SessionSnapshot;
export declare function applySnapshot(snapshot: SessionSnapshot, targets: PersistableSources): {
    restored: true;
    age_ms: number;
} | {
    restored: false;
    reason: string;
};
export declare function saveSnapshot(snapshot: SessionSnapshot, filePath?: string): boolean;
export declare function loadSnapshot(filePath?: string): SessionSnapshot | null;
export declare function deleteSnapshot(filePath?: string): void;
export declare class SessionPersistence {
    private sources;
    private filePath;
    private timer;
    private debounceMs;
    constructor(sources: PersistableSources, opts?: {
        filePath?: string;
        debounceMs?: number;
    });
    /**
     * Try to restore state from disk.
     * Returns result object indicating success/failure + age.
     */
    tryRestore(): {
        restored: boolean;
        reason?: string;
        age_ms?: number;
    };
    /**
     * Schedule a debounced write.  Safe to call after every tool dispatch —
     * only one write happens within the debounce window.
     */
    scheduleSave(): void;
    /** Force an immediate write (e.g. on graceful shutdown). */
    saveNow(): boolean;
    /** Cancel any pending write and remove the state file. */
    dispose(): void;
    /** Expose for tests. */
    get stateFilePath(): string;
}
//# sourceMappingURL=session-persistence.d.ts.map