/**
 * Session Intelligence — Ambient awareness for long build sessions
 *
 * Three capabilities that keep agents oriented:
 *
 * 1. ProgressTracker — Maps tool calls to build milestones. Tracks what
 *    phase the project is in (terrain → lighting → materials → actors → gameplay).
 *
 * 2. ErrorPatternDetector — Spots consecutive failures, bridge instability,
 *    and repeated identical errors. Surfaces warnings before agents waste turns.
 *
 * 3. SessionDigest — Compact summary attached to every Nth response.
 *    Shows progress, open issues, and what to consider next.
 */
import type { SessionEntry } from "./system-enhancements.js";
export type BuildPhase = "terrain" | "lighting" | "materials" | "foliage" | "actors" | "gameplay" | "audio" | "vfx" | "testing";
export interface Milestone {
    phase: BuildPhase;
    label: string;
    toolName: string;
    timestamp: number;
}
export interface ErrorPattern {
    type: "streak" | "bridge_instability" | "repeated_error";
    message: string;
    tool?: string;
    count: number;
}
export interface Digest {
    call_number: number;
    milestones: Milestone[];
    phases_completed: BuildPhase[];
    phases_missing: BuildPhase[];
    error_patterns: ErrorPattern[];
    suggestion?: string;
}
export declare class ProgressTracker {
    private milestones;
    private completedPhases;
    /** Record a successful tool call. Returns a milestone if the tool maps to a build phase. */
    record(toolName: string, timestamp: number): Milestone | null;
    /** Get all milestones in order. */
    getMilestones(): Milestone[];
    /** Get phases that have been touched. */
    getCompletedPhases(): BuildPhase[];
    /** Get phases that haven't been touched, in build order. */
    getMissingPhases(): BuildPhase[];
    /** Get the most recent N milestones. */
    recent(n?: number): Milestone[];
    get totalMilestones(): number;
    clear(): void;
    /** Serialize milestones for persistence. */
    serialize(): Milestone[];
    /** Restore from persisted milestones. Rebuilds completedPhases automatically. */
    restoreFromMilestones(data: Milestone[]): void;
}
export declare function detectErrorPatterns(entries: SessionEntry[]): ErrorPattern[];
export declare function generateSuggestion(completedPhases: BuildPhase[], missingPhases: BuildPhase[], errorPatterns: ErrorPattern[]): string | undefined;
/**
 * Build a session digest from the progress tracker and session entries.
 * This is the main entry point for assembling the digest.
 */
export declare function buildDigest(callNumber: number, progress: ProgressTracker, recentEntries: SessionEntry[]): Digest;
/**
 * Decide whether to attach a digest to this response.
 * Attaches every `interval` calls, or when an error pattern is detected.
 */
export declare function shouldAttachDigest(callNumber: number, interval: number, recentEntries: SessionEntry[]): boolean;
//# sourceMappingURL=session-intelligence.d.ts.map