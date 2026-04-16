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
// ─── Constants ────────────────────────────────────────────────────────────────
const DELAY_BY_LEVEL = {
    0: 0,
    1: 200,
    2: 800,
    3: 2000,
};
const REASON_BY_LEVEL = {
    0: "normal",
    1: "light_throttle",
    2: "medium_throttle",
    3: "heavy_throttle",
};
/** Duration threshold (ms) above which a call is "slow". */
const SLOW_THRESHOLD_MS = 8000;
/** Consecutive OK calls needed to decay one level. */
const DECAY_AFTER_OK = 3;
/** Consecutive bad calls needed to escalate one level. */
const ESCALATE_AFTER_BAD = 2;
/** Bridge-fatal error patterns that jump directly to level 3. */
const FATAL_PATTERNS = [
    /ECONNREFUSED/i,
    /bridge is disconnected/i,
    /ETIMEDOUT/i,
    /socket hang up/i,
];
// ─── AdaptiveThrottle ─────────────────────────────────────────────────────────
export class AdaptiveThrottle {
    level = 0;
    consecutiveOk = 0;
    consecutiveBad = 0;
    slowThresholdMs;
    decayAfterOk;
    escalateAfterBad;
    constructor(opts) {
        this.slowThresholdMs = opts?.slowThresholdMs ?? SLOW_THRESHOLD_MS;
        this.decayAfterOk = opts?.decayAfterOk ?? DECAY_AFTER_OK;
        this.escalateAfterBad = opts?.escalateAfterBad ?? ESCALATE_AFTER_BAD;
    }
    /** Get current throttle decision (delay + reason). */
    getDecision() {
        return {
            delay_ms: DELAY_BY_LEVEL[this.level],
            level: this.level,
            reason: REASON_BY_LEVEL[this.level],
        };
    }
    /** Get full internal state (for diagnostics/testing). */
    getState() {
        return {
            level: this.level,
            delay_ms: DELAY_BY_LEVEL[this.level],
            reason: REASON_BY_LEVEL[this.level],
            consecutive_ok: this.consecutiveOk,
            consecutive_bad: this.consecutiveBad,
        };
    }
    /**
     * Report a call outcome. Adjusts throttle level based on success/failure
     * and latency patterns.
     */
    reportOutcome(outcome) {
        // Fatal bridge error → jump to max immediately
        if (!outcome.ok && outcome.error && isFatalBridgeError(outcome.error)) {
            this.level = 3;
            this.consecutiveOk = 0;
            this.consecutiveBad++;
            return;
        }
        if (outcome.ok && outcome.duration_ms < this.slowThresholdMs) {
            // Fast success → track toward decay
            this.consecutiveOk++;
            this.consecutiveBad = 0;
            if (this.consecutiveOk >= this.decayAfterOk && this.level > 0) {
                this.level = (this.level - 1);
                this.consecutiveOk = 0;
            }
        }
        else {
            // Failure or slow call → track toward escalation
            this.consecutiveBad++;
            this.consecutiveOk = 0;
            if (this.consecutiveBad >= this.escalateAfterBad && this.level < 3) {
                this.level = (this.level + 1);
                this.consecutiveBad = 0;
            }
        }
    }
    /** Reset to level 0 (for testing or bridge reconnection). */
    reset() {
        this.level = 0;
        this.consecutiveOk = 0;
        this.consecutiveBad = 0;
    }
}
// ─── Helpers ──────────────────────────────────────────────────────────────────
function isFatalBridgeError(error) {
    return FATAL_PATTERNS.some(p => p.test(error));
}
/**
 * Build a compact throttle block for injection into tool responses.
 * Only returns non-null when throttle is active (level > 0).
 */
export function buildThrottleBlock(throttle) {
    const state = throttle.getState();
    if (state.level === 0)
        return null;
    return {
        throttle_level: state.level,
        throttle_delay_ms: state.delay_ms,
        throttle_reason: state.reason,
    };
}
//# sourceMappingURL=adaptive-throttle.js.map