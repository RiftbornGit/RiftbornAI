/**
 * Round 20 — Dispatch Lifecycle Hooks
 *
 * Two capabilities that give agents fail-fast + auto-recovery:
 *
 * 1. Post-dispatch micro-verification — After any creation tool succeeds,
 *    immediately run a fast assert to confirm the thing actually exists.
 *    If it doesn't, annotate the response with _verify_failed so the agent
 *    knows not to build on a broken foundation.
 *
 * 2. Rollback execution — Read SceneChangeLog's undo plans and actually
 *    execute the reverse operations via dispatch. Two tools:
 *      undo_last  — undo the last N operations (default 1)
 *      rollback   — undo all changes matching a scope/label filter
 */
import type { SceneChange, SceneCategory, UndoStep } from "./scene-safety.js";
import type { RiftbornResponse } from "./riftborn-types.js";
export interface MicroVerifyResult {
    verified: boolean;
    tool: string;
    label: string;
    error?: string;
    duration_ms: number;
}
export interface RollbackStepResult {
    description: string;
    tool: string;
    ok: boolean;
    error?: string;
    duration_ms: number;
}
export interface RollbackReport {
    steps_attempted: number;
    steps_succeeded: number;
    steps_failed: number;
    results: RollbackStepResult[];
    duration_ms: number;
}
type Dispatch = (tool: string, params: Record<string, unknown>) => Promise<RiftbornResponse>;
/**
 * Run a fast micro-verification after a successful tool dispatch.
 * Returns null if the tool doesn't have a verify rule.
 */
export declare function microVerify(toolName: string, toolParams: Record<string, unknown>, dispatch: Dispatch): Promise<MicroVerifyResult | null>;
/**
 * Check if a tool has a micro-verify rule. Used by index.ts to decide
 * whether to run post-dispatch verification.
 */
export declare function hasMicroVerifyRule(toolName: string): boolean;
export declare function executeRollback(undoSteps: UndoStep[], dispatch: Dispatch): Promise<RollbackReport>;
/**
 * Filter undo steps by category or label for targeted rollback.
 */
export declare function filterUndoSteps(steps: UndoStep[], filter: {
    category?: SceneCategory;
    label?: string;
}, changes: SceneChange[]): UndoStep[];
export {};
//# sourceMappingURL=dispatch-hooks.d.ts.map