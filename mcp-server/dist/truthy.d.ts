/**
 * Tiny helpers for parsing environment-style truthy strings consistently.
 *
 * The C++ surface mirrors this in `Source/RiftbornAI/Public/Core/EnvUtils.h`
 * — keep both lists in sync. RIFTBORN_DEV_MODE, RIFTBORN_PROOF_MODE,
 * RIFTBORN_ENABLE_INTERNAL_TOOLS and similar boolean env knobs MUST flow
 * through `isTruthyEnv` so Bridge / SessionTaint / BetaReleaseSurface /
 * status endpoints / MCP server all agree on the same truthy value.
 */
/** True iff `value` matches one of `1`, `true`, `yes`, `on` (case-insensitive). */
export declare function isTruthy(value: string | undefined | null): boolean;
/** Convenience: read `env[name]` and apply `isTruthy`. */
export declare function isTruthyEnv(env: NodeJS.ProcessEnv | Record<string, string | undefined>, name: string): boolean;