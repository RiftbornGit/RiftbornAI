/**
 * Bootstrap: parse + validate environment variables and resolve the
 * bridge auth token. Extracted from index.ts to keep the entry-point
 * focused on wiring.
 *
 * Side effects (console.error logging about auth token resolution) are
 * preserved here so behavior is identical to the pre-split module.
 */
import * as path from "path";
import { fileURLToPath } from "url";
import { ENV_TOKEN_NAMES, resolveBridgeAuthToken, } from "./auth-token.js";
import { getBlockedToolNameSet, getInternalToolNameSet, } from "./surface-manifest.js";
export function loadBootstrapConfig(entryFilename) {
    const suppressAuthWarnings = process.argv.includes("--self-test")
        || process.env.RIFTBORN_SUPPRESS_AUTH_WARNINGS === "true";
    const RIFTBORN_HTTP_PORT = parseInt(process.env.RIFTBORN_HTTP_PORT || "8767");
    const RIFTBORN_TCP_PORT = parseInt(process.env.RIFTBORN_TCP_PORT || "8765");
    if (!Number.isInteger(RIFTBORN_HTTP_PORT) || RIFTBORN_HTTP_PORT < 1 || RIFTBORN_HTTP_PORT > 65535) {
        throw new Error(`[RiftbornAI] Invalid RIFTBORN_HTTP_PORT: ${process.env.RIFTBORN_HTTP_PORT}`);
    }
    if (!Number.isInteger(RIFTBORN_TCP_PORT) || RIFTBORN_TCP_PORT < 1 || RIFTBORN_TCP_PORT > 65535) {
        throw new Error(`[RiftbornAI] Invalid RIFTBORN_TCP_PORT: ${process.env.RIFTBORN_TCP_PORT}`);
    }
    const RIFTBORN_HOST = process.env.RIFTBORN_HOST || "127.0.0.1";
    const RIFTBORN_BRAIN_PORT = parseInt(process.env.RIFTBORN_BRAIN_PORT || "8768");
    if (!Number.isInteger(RIFTBORN_BRAIN_PORT) || RIFTBORN_BRAIN_PORT < 1 || RIFTBORN_BRAIN_PORT > 65535) {
        throw new Error(`[RiftbornAI] Invalid RIFTBORN_BRAIN_PORT: ${process.env.RIFTBORN_BRAIN_PORT}`);
    }
    if (!/^(?:localhost|(?:\d{1,3}\.){3}\d{1,3}|\[[0-9a-f:.]+\]|[a-z0-9](?:[a-z0-9.-]*[a-z0-9])?)$/i.test(RIFTBORN_HOST)) {
        throw new Error(`[RiftbornAI] Invalid RIFTBORN_HOST: ${RIFTBORN_HOST}`);
    }
    const entryDir = path.dirname(entryFilename);
    const authResolution = resolveBridgeAuthToken({
        env: process.env,
        searchRoots: [process.cwd(), entryDir],
    });
    const RIFTBORN_AUTH_TOKEN = authResolution.token;
    if (!RIFTBORN_AUTH_TOKEN) {
        if (!suppressAuthWarnings) {
            console.error("[RiftbornAI] WARNING: No bridge auth token found — bridge requests will be unauthenticated.");
            console.error(`[RiftbornAI] Checked env vars: ${ENV_TOKEN_NAMES.join(", ")}`);
            if (authResolution.searchedPaths.length > 0) {
                console.error("[RiftbornAI] Checked token files:");
                for (const tokenPath of authResolution.searchedPaths) {
                    console.error(`  - ${tokenPath}`);
                }
            }
            console.error("[RiftbornAI] Set RIFTBORN_AUTH_TOKEN or create Saved/RiftbornAI/.dev_token.");
        }
    }
    else if (!suppressAuthWarnings && authResolution.source === "env:RIFTBORN_API_KEY") {
        console.error("[RiftbornAI] Using RIFTBORN_API_KEY as auth token (RIFTBORN_AUTH_TOKEN not set).");
    }
    else if (!suppressAuthWarnings && authResolution.source === "env:RIFTBORN_DEV_TOKEN") {
        console.error("[RiftbornAI] Using RIFTBORN_DEV_TOKEN as auth token.");
    }
    else if (!suppressAuthWarnings && authResolution.source.startsWith("file:")) {
        console.error(`[RiftbornAI] Using auth token from ${authResolution.source.slice(5)}.`);
    }
    if (!suppressAuthWarnings && RIFTBORN_AUTH_TOKEN && RIFTBORN_AUTH_TOKEN.startsWith("riftborn_dev_")) {
        console.error("[RiftbornAI] WARNING: Using a dev auth token — do not use in production.");
    }
    const ENABLE_INTERNAL_TOOLS = process.env.RIFTBORN_ENABLE_INTERNAL_TOOLS === "true";
    const ALLOW_HIDDEN_TOOLS = process.env.RIFTBORN_ALLOW_HIDDEN_TOOLS === "true";
    // Developer mode — exposes EVERY registered tool, bypassing the
    // locked beta-release surface. The shipped Beta build sets this false (the
    // default) so end users only see the hardened beta tool set. Developers
    // (the team building games on top of the plugin) flip it on with
    // RIFTBORN_DEV_MODE=true and get the full ~700+ surface back. Same gate
    // also implicitly enables internal tools and hidden-tool dispatch so
    // it's a single switch instead of three.
    const DEV_MODE = process.env.RIFTBORN_DEV_MODE === "true";
    return {
        RIFTBORN_HTTP_PORT,
        RIFTBORN_TCP_PORT,
        RIFTBORN_BRAIN_PORT,
        RIFTBORN_HOST,
        RIFTBORN_AUTH_TOKEN,
        ENABLE_INTERNAL_TOOLS: ENABLE_INTERNAL_TOOLS || DEV_MODE,
        ALLOW_HIDDEN_TOOLS: ALLOW_HIDDEN_TOOLS || DEV_MODE,
        DEV_MODE,
        INTERNAL_ONLY_TOOLS: getInternalToolNameSet(),
        BLOCKED_TOOLS: getBlockedToolNameSet(),
    };
}
// Re-exported for callers that used to pull these directly from index.ts
export { fileURLToPath };
//# sourceMappingURL=bootstrap.js.map