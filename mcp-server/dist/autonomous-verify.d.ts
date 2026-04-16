/**
 * Round 19 — Autonomous Verification Engine
 *
 * After an AI agent builds things via RiftbornAI, this module generates and
 * runs structured verification checks based on what was actually done.
 *
 * Core idea: SceneChangeLog knows every create/modify/delete. ContextPropagator
 * knows the last landscape/actor/material/blueprint. This module reads that
 * tracked state and converts it into assert_* tool calls that run in UE,
 * returning structured PASS/FAIL — no expensive vision AI needed.
 *
 * Two user-facing tools:
 *   verify_session  — auto-generate + run checks from session history
 *   smoke_test_pie  — start PIE → structural checks → stop PIE → report
 */
import type { SceneChange } from "./scene-safety.js";
import type { Milestone } from "./session-intelligence.js";
import type { RiftbornResponse } from "./riftborn-types.js";
export interface VerificationCheck {
    tool: string;
    params: Record<string, unknown>;
    label: string;
    severity: "error" | "warning";
}
export interface VerificationPlan {
    checks: VerificationCheck[];
    generated_from: string;
    change_count: number;
}
export interface CheckResult {
    label: string;
    tool: string;
    passed: boolean;
    error?: string;
    duration_ms: number;
}
export interface VerificationReport {
    overall: "PASS" | "FAIL" | "WARN";
    total: number;
    passed: number;
    failed: number;
    warned: number;
    checks: CheckResult[];
    duration_ms: number;
    suggestion?: string;
}
export interface PieSmokeResult {
    overall: "PASS" | "FAIL" | "CRASH";
    pre_checks: CheckResult[];
    pie_started: boolean;
    runtime_checks: CheckResult[];
    post_checks: CheckResult[];
    duration_ms: number;
    suggestion?: string;
}
type Dispatch = (tool: string, params: Record<string, unknown>) => Promise<RiftbornResponse>;
export interface PlanInput {
    sceneChanges: SceneChange[];
    milestones?: Milestone[];
    context?: Record<string, string>;
}
export declare function generatePlan(input: PlanInput): VerificationPlan;
export declare function runPlan(plan: VerificationPlan, dispatch: Dispatch, sceneChanges?: SceneChange[]): Promise<VerificationReport>;
export interface PieSmokeOptions {
    /** How long to let PIE run before runtime checks (seconds, 2–30). Default 3. */
    duration_seconds?: number;
    /** Max errors allowed in output log post-PIE. Default 0. */
    max_errors?: number;
}
export declare function pieSmokeTest(dispatch: Dispatch, opts?: PieSmokeOptions): Promise<PieSmokeResult>;
export {};
//# sourceMappingURL=autonomous-verify.d.ts.map