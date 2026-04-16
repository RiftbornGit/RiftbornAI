/**
 * Bootstrap: parse + validate environment variables and resolve the
 * bridge auth token. Extracted from index.ts to keep the entry-point
 * focused on wiring.
 *
 * Side effects (console.error logging about auth token resolution) stay here
 * so the bootstrap contract is centralized in one module.
 */
export interface BootstrapConfig {
    RIFTBORN_HTTP_PORT: number;
    RIFTBORN_TCP_PORT: number;
    RIFTBORN_BRAIN_PORT: number;
    RIFTBORN_HOST: string;
    RIFTBORN_AUTH_TOKEN: string;
    AUTH_TOKEN_SOURCE: string;
    ENABLE_INTERNAL_TOOLS: boolean;
    ALLOW_HIDDEN_TOOLS: boolean;
    ALLOW_UNAUTHENTICATED_LOCAL: boolean;
    DEV_MODE: boolean;
    INTERNAL_ONLY_TOOLS: Set<string>;
    BLOCKED_TOOLS: Set<string>;
}
export interface BootstrapOverrides {
    argv?: string[];
    cwd?: string;
    env?: NodeJS.ProcessEnv;
}
export declare function loadBootstrapConfig(entryFilename: string, overrides?: BootstrapOverrides): BootstrapConfig;
//# sourceMappingURL=bootstrap.d.ts.map