/**
 * Bootstrap: parse + validate environment variables and resolve the
 * bridge auth token. Extracted from index.ts to keep the entry-point
 * focused on wiring.
 *
 * Side effects (console.error logging about auth token resolution) stay here
 * so the bootstrap contract is centralized in one module.
 */
import * as path from "path";
import { ENV_TOKEN_NAMES, resolveBridgeAuthToken, } from "./auth-token.js";
import { getBlockedToolNameSet, getInternalToolNameSet, } from "./surface-manifest.js";
function isTruthyEnv(value) {
    const normalized = (value || "").trim().toLowerCase();
    return normalized === "1" || normalized === "true";
}
function isLoopbackHost(host) {
    const normalized = host.trim().toLowerCase();
    return normalized === "localhost"
        || normalized === "::1"
        || normalized === "[::1]"
        || normalized === "127.0.0.1"
        || normalized.startsWith("127.");
}
export function loadBootstrapConfig(entryFilename, overrides) {
    const env = overrides?.env ?? process.env;
    const argv = overrides?.argv ?? process.argv;
    const cwd = overrides?.cwd ?? process.cwd();
    const suppressAuthWarnings = argv.includes("--self-test")
        || env.RIFTBORN_SUPPRESS_AUTH_WARNINGS === "true";
    const RIFTBORN_HTTP_PORT = parseInt(env.RIFTBORN_HTTP_PORT || "8767");
    const RIFTBORN_TCP_PORT = parseInt(env.RIFTBORN_TCP_PORT || "8765");
    if (!Number.isInteger(RIFTBORN_HTTP_PORT) || RIFTBORN_HTTP_PORT < 1 || RIFTBORN_HTTP_PORT > 65535) {
        throw new Error(`[RiftbornAI] Invalid RIFTBORN_HTTP_PORT: ${env.RIFTBORN_HTTP_PORT}`);
    }
    if (!Number.isInteger(RIFTBORN_TCP_PORT) || RIFTBORN_TCP_PORT < 1 || RIFTBORN_TCP_PORT > 65535) {
        throw new Error(`[RiftbornAI] Invalid RIFTBORN_TCP_PORT: ${env.RIFTBORN_TCP_PORT}`);
    }
    const RIFTBORN_HOST = env.RIFTBORN_HOST || "127.0.0.1";
    const RIFTBORN_BRAIN_PORT = parseInt(env.RIFTBORN_BRAIN_PORT || "8768");
    if (!Number.isInteger(RIFTBORN_BRAIN_PORT) || RIFTBORN_BRAIN_PORT < 1 || RIFTBORN_BRAIN_PORT > 65535) {
        throw new Error(`[RiftbornAI] Invalid RIFTBORN_BRAIN_PORT: ${env.RIFTBORN_BRAIN_PORT}`);
    }
    if (!/^(?:localhost|(?:\d{1,3}\.){3}\d{1,3}|\[[0-9a-f:.]+\]|[a-z0-9](?:[a-z0-9.-]*[a-z0-9])?)$/i.test(RIFTBORN_HOST)) {
        throw new Error(`[RiftbornAI] Invalid RIFTBORN_HOST: ${RIFTBORN_HOST}`);
    }
    const entryDir = path.dirname(entryFilename);
    const authResolution = resolveBridgeAuthToken({
        env,
        searchRoots: [cwd, entryDir],
    });
    const RIFTBORN_AUTH_TOKEN = authResolution.token;
    const ALLOW_UNAUTHENTICATED_LOCAL = isTruthyEnv(env.RIFTBORN_ALLOW_UNAUTHENTICATED_LOCAL);
    const USING_DEV_BOOTSTRAP_TOKEN = RIFTBORN_AUTH_TOKEN.startsWith("riftborn_dev_");
    if (USING_DEV_BOOTSTRAP_TOKEN && !isLoopbackHost(RIFTBORN_HOST)) {
        throw new Error(`[RiftbornAI] Bootstrap dev tokens are loopback-only; refusing host ${RIFTBORN_HOST}. Configure a non-dev auth token for remote hosts.`);
    }
    if (ALLOW_UNAUTHENTICATED_LOCAL && !isLoopbackHost(RIFTBORN_HOST)) {
        throw new Error(`[RiftbornAI] RIFTBORN_ALLOW_UNAUTHENTICATED_LOCAL only supports loopback hosts; refusing host ${RIFTBORN_HOST}.`);
    }
    if (!RIFTBORN_AUTH_TOKEN) {
        const errorLines = [
            "[RiftbornAI] No bridge auth token found.",
            `[RiftbornAI] Checked env vars: ${ENV_TOKEN_NAMES.join(", ")}`,
        ];
        if (authResolution.searchedPaths.length > 0) {
            errorLines.push("[RiftbornAI] Checked token files:");
            for (const tokenPath of authResolution.searchedPaths) {
                errorLines.push(`  - ${tokenPath}`);
            }
        }
        if (!ALLOW_UNAUTHENTICATED_LOCAL) {
            errorLines.push("[RiftbornAI] Set RIFTBORN_AUTH_TOKEN or create Saved/RiftbornAI/.dev_token.");
            errorLines.push("[RiftbornAI] To intentionally run without auth on localhost only, set RIFTBORN_ALLOW_UNAUTHENTICATED_LOCAL=1.");
            throw new Error(errorLines.join("\n"));
        }
        if (!suppressAuthWarnings) {
            console.error("[RiftbornAI] WARNING: Running without a bridge auth token because RIFTBORN_ALLOW_UNAUTHENTICATED_LOCAL is enabled.");
            for (const line of errorLines.slice(1)) {
                console.error(line);
            }
        }
    }
    else if (!suppressAuthWarnings && authResolution.source === "env:RIFTBORN_API_KEY") {
        console.error("[RiftbornAI] Using RIFTBORN_API_KEY as auth token (RIFTBORN_AUTH_TOKEN not set).");
    }
    else if (!suppressAuthWarnings && authResolution.source === "env:RIFTBORN_DEV_TOKEN") {
        console.error("[RiftbornAI] Using RIFTBORN_DEV_TOKEN as bootstrap auth token.");
    }
    else if (!suppressAuthWarnings && authResolution.source.startsWith("file:")) {
        console.error(`[RiftbornAI] Using auth token from ${authResolution.source.slice(5)}.`);
    }
    if (!suppressAuthWarnings && RIFTBORN_AUTH_TOKEN && RIFTBORN_AUTH_TOKEN.startsWith("riftborn_dev_")) {
        console.error("[RiftbornAI] WARNING: Using a dev bootstrap token — the MCP client will exchange it for a short-lived session token.");
    }
    const ENABLE_INTERNAL_TOOLS = env.RIFTBORN_ENABLE_INTERNAL_TOOLS === "true";
    const ALLOW_HIDDEN_TOOLS = env.RIFTBORN_ALLOW_HIDDEN_TOOLS === "true";
    // Developer mode — exposes EVERY registered tool, bypassing the
    // locked beta-release surface. The shipped Beta build sets this false (the
    // default) so end users only see the hardened beta tool set. Developers
    // (the team building games on top of the plugin) flip it on with
    // RIFTBORN_DEV_MODE=true and get the full ~700+ surface back. Same gate
    // also implicitly enables internal tools and hidden-tool dispatch so
    // it's a single switch instead of three.
    const DEV_MODE = env.RIFTBORN_DEV_MODE === "true";
    return {
        RIFTBORN_HTTP_PORT,
        RIFTBORN_TCP_PORT,
        RIFTBORN_BRAIN_PORT,
        RIFTBORN_HOST,
        RIFTBORN_AUTH_TOKEN,
        AUTH_TOKEN_SOURCE: authResolution.source,
        ENABLE_INTERNAL_TOOLS: ENABLE_INTERNAL_TOOLS || DEV_MODE,
        ALLOW_HIDDEN_TOOLS: ALLOW_HIDDEN_TOOLS || DEV_MODE,
        ALLOW_UNAUTHENTICATED_LOCAL,
        DEV_MODE,
        INTERNAL_ONLY_TOOLS: getInternalToolNameSet(),
        BLOCKED_TOOLS: getBlockedToolNameSet(),
    };
}
//# sourceMappingURL=bootstrap.js.map