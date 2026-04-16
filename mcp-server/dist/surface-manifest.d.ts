export type ManifestReadinessTier = "PRODUCTION" | "BETA" | "EXPERIMENTAL" | "STUB" | "DEPRECATED";
export interface SurfaceManifestReadinessTruth {
    canonical_generator_root?: string;
    human_reference?: string;
    mcp_projection_defaults?: {
        production_tools?: ManifestReadinessTier;
        beta_release_requires_surface_lock?: boolean;
    };
    documentation_paths?: string[];
    downstream_artifacts?: Record<string, string>;
    rules?: string[];
}
export interface SurfaceManifest {
    version: number;
    description: string;
    default_stage?: string;
    readiness_truth?: SurfaceManifestReadinessTruth;
    tools: string[];
    production_tools: string[];
    /** Locked beta-release surface — exactly the current hardened beta tools
     *  with the public Beta. See public_surface.json comment for the full
     *  contract. The array is the source of truth; `beta_release_tools_groups`
     *  in the JSON is documentation only. */
    beta_release_tools: string[];
    internal_tools: string[];
    blocked_tools: string[];
}
/** Surface stages.
 *
 *  `beta_release` is the locked 99-tool surface that ships with the public
 *  Beta. It is intentionally narrower than `production` (which is the larger
 *  curated set ~338 tools); both `beta_release_tools` AND the PRODUCTION
 *  readiness tier must accept a tool for it to be visible. Developers
 *  bypass everything via RIFTBORN_DEV_MODE=true (see index.ts).
 */
export type SurfaceStage = "production" | "beta" | "beta_release" | "experimental";
export declare function normalizeStringArray(value: unknown): string[];
export declare function parseSurfaceManifest(raw: string): SurfaceManifest | null;
export declare function loadSurfaceManifestFromPath(filePath: string): SurfaceManifest | null;
export declare function getSurfaceManifest(): SurfaceManifest;
export declare function getPublicToolNames(): string[];
export declare function getProductionToolNames(): string[];
export declare function getProductionToolNameSet(): Set<string>;
export declare function getDefaultProductionReadinessTier(): ManifestReadinessTier;
/** The locked beta-release surface. Source of truth lives in
 *  Bridge/toolbook/public_surface.json under `beta_release_tools`. The
 *  test in Tests/static/test_beta_release_surface_lock.py asserts both
 *  the exact count and the exact membership. */
export declare function getBetaReleaseToolNames(): string[];
export declare function getBetaReleaseToolNameSet(): Set<string>;
export declare function getInternalToolNames(): string[];
export declare function getInternalToolNameSet(): Set<string>;
export declare function getBlockedToolNames(): string[];
export declare function getBlockedToolNameSet(): Set<string>;
export declare function getDefaultSurfaceStage(): SurfaceStage;
export declare function getDefaultReadinessTierNames(): string[];
//# sourceMappingURL=surface-manifest.d.ts.map