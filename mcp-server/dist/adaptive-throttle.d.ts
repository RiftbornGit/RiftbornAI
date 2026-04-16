/**
 * Adaptive Throttle — Backpressure for degraded bridge conditions
 *
 * When the bridge is slow or failing, agents tend to rapid-fire retries which
 * makes the problem worse. This module maintains a throttle level based on
 * recent call outcomes and injects pre-call delays proportional to severity.
 *
 * Throttle levels:
 *   0 — normal (no delay)
 *   1 — light  (200ms delay, warned)
 *   2 — medium (800ms delay)
 *   3 — heavy  (2000ms delay)
 *
 * The level adjusts automatically:
 *   - Successful calls decay toward 0
 *   - Slow calls or errors ratchet toward 3
 *   - Complete bridge failure jumps to 3 immediately
 *
 * All state is local — no bridge calls, no persistence.
 */
export type ThrottleLevel = 0 | 1 | 2 | 3;
export interface ThrottleState {
    level: ThrottleLevel;
    delay_ms: number;
    reason: string;
    consecutive_ok: number;
    consecutive_bad: number;
}
export interface ThrottleDecision {
    delay_ms: number;
    level: ThrottleLevel;
    reason: string;
}
export interface CallOutcome {
    ok: boolean;
    duration_ms: number;
    error?: string;
}
export declare class AdaptiveThrottle {
    private level;
    private consecutiveOk;
    private consecutiveBad;
    private slowThresholdMs;
    private decayAfterOk;
    private escalateAfterBad;
    constructor(opts?: {
        slowThresholdMs?: number;
        decayAfterOk?: number;
        escalateAfterBad?: number;
    });
    /** Get current throttle decision (delay + reason). */
    getDecision(): ThrottleDecision;
    /** Get full internal state (for diagnostics/testing). */
    getState(): ThrottleState;
    /**
     * Report a call outcome. Adjusts throttle level based on success/failure
     * and latency patterns.
     */
    reportOutcome(outcome: CallOutcome): void;
    /** Reset to level 0 (for testing or bridge reconnection). */
    reset(): void;
}
/**
 * Build a compact throttle block for injection into tool responses.
 * Only returns non-null when throttle is active (level > 0).
 */
export declare function buildThrottleBlock(throttle: AdaptiveThrottle): Record<string, unknown> | null;
//# sourceMappingURL=adaptive-throttle.d.ts.map