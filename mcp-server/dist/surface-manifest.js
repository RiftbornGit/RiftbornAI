import * as fs from "node:fs";
import * as path from "node:path";
import { fileURLToPath } from "node:url";
let cachedManifest = null;
const MAX_MANIFEST_BYTES = 1_048_576;
function manifestPath() {
    const moduleDir = path.dirname(fileURLToPath(import.meta.url));
    return path.resolve(moduleDir, "../../Bridge/toolbook/public_surface.json");
}
export function normalizeStringArray(value) {
    if (!Array.isArray(value)) {
        return [];
    }
    const seen = new Set();
    const out = [];
    for (const item of value) {
        if (typeof item !== "string" || seen.has(item)) {
            continue;
        }
        seen.add(item);
        out.push(item);
    }
    return out;
}
export function parseSurfaceManifest(raw) {
    try {
        const parsed = JSON.parse(raw);
        const readinessTruth = parsed.readiness_truth;
        const rawProductionTier = readinessTruth?.mcp_projection_defaults?.production_tools;
        const productionTier = (rawProductionTier === "PRODUCTION"
            || rawProductionTier === "BETA"
            || rawProductionTier === "EXPERIMENTAL"
            || rawProductionTier === "STUB"
            || rawProductionTier === "DEPRECATED") ? rawProductionTier : undefined;
        return {
            version: Number(parsed.version || 0),
            description: typeof parsed.description === "string" ? parsed.description : "",
            default_stage: typeof parsed.default_stage === "string" ? parsed.default_stage : undefined,
            readiness_truth: readinessTruth && typeof readinessTruth === "object" ? {
                canonical_generator_root: typeof readinessTruth.canonical_generator_root === "string"
                    ? readinessTruth.canonical_generator_root
                    : undefined,
                human_reference: typeof readinessTruth.human_reference === "string"
                    ? readinessTruth.human_reference
                    : undefined,
                mcp_projection_defaults: {
                    production_tools: productionTier,
                    beta_release_requires_surface_lock: readinessTruth.mcp_projection_defaults?.beta_release_requires_surface_lock === true,
                },
                documentation_paths: normalizeStringArray(readinessTruth.documentation_paths),
                downstream_artifacts: readinessTruth.downstream_artifacts && typeof readinessTruth.downstream_artifacts === "object"
                    ? Object.fromEntries(Object.entries(readinessTruth.downstream_artifacts)
                        .filter((entry) => typeof entry[1] === "string"))
                    : {},
                rules: normalizeStringArray(readinessTruth.rules),
            } : undefined,
            tools: normalizeStringArray(parsed.tools),
            production_tools: normalizeStringArray(parsed.production_tools),
            beta_release_tools: normalizeStringArray(parsed.beta_release_tools),
            internal_tools: normalizeStringArray(parsed.internal_tools),
            blocked_tools: normalizeStringArray(parsed.blocked_tools),
        };
    }
    catch {
        return null;
    }
}
export function loadSurfaceManifestFromPath(filePath) {
    try {
        const stats = fs.lstatSync(filePath);
        if (!stats.isFile() || stats.isSymbolicLink() || stats.size > MAX_MANIFEST_BYTES) {
            return null;
        }
        const raw = fs.readFileSync(filePath, "utf8");
        return parseSurfaceManifest(raw);
    }
    catch {
        return null;
    }
}
export function getSurfaceManifest() {
    if (cachedManifest) {
        return cachedManifest;
    }
    const parsed = loadSurfaceManifestFromPath(manifestPath());
    if (!parsed) {
        throw new Error("Failed to load surface manifest.");
    }
    cachedManifest = parsed;
    return cachedManifest;
}
export function getPublicToolNames() {
    return [...getSurfaceManifest().tools];
}
export function getProductionToolNames() {
    return [...getSurfaceManifest().production_tools];
}
export function getProductionToolNameSet() {
    return new Set(getSurfaceManifest().production_tools);
}
export function getDefaultProductionReadinessTier() {
    return getSurfaceManifest().readiness_truth?.mcp_projection_defaults?.production_tools || "PRODUCTION";
}
/** The locked beta-release surface. Source of truth lives in
 *  Bridge/toolbook/public_surface.json under `beta_release_tools`. The
 *  test in Tests/static/test_beta_release_surface_lock.py asserts both
 *  the exact count and the exact membership. */
export function getBetaReleaseToolNames() {
    return [...getSurfaceManifest().beta_release_tools];
}
export function getBetaReleaseToolNameSet() {
    return new Set(getSurfaceManifest().beta_release_tools);
}
export function getInternalToolNames() {
    return [...getSurfaceManifest().internal_tools];
}
export function getInternalToolNameSet() {
    return new Set(getSurfaceManifest().internal_tools);
}
export function getBlockedToolNames() {
    return [...getSurfaceManifest().blocked_tools];
}
export function getBlockedToolNameSet() {
    return new Set(getSurfaceManifest().blocked_tools);
}
export function getDefaultSurfaceStage() {
    const stage = getSurfaceManifest().default_stage?.toLowerCase();
    if (stage === "beta" || stage === "beta_release" || stage === "experimental") {
        return stage;
    }
    return "production";
}
export function getDefaultReadinessTierNames() {
    const stage = getDefaultSurfaceStage();
    if (stage === "experimental") {
        return ["PRODUCTION", "BETA", "EXPERIMENTAL"];
    }
    if (stage === "beta") {
        return ["PRODUCTION", "BETA"];
    }
    // `beta_release` uses the PRODUCTION readiness tier and then layers a
    // hard intersection with the locked beta-release set on top — see index.ts
    // for the actual filter wiring. Returning PRODUCTION here keeps the
    // tier filter from over-restricting before the lock applies.
    return [getDefaultProductionReadinessTier()];
}
//# sourceMappingURL=surface-manifest.js.map