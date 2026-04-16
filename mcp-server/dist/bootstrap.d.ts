/**
 * Bootstrap: parse + validate environment variables and resolve the
 * bridge auth token. Extracted from index.ts to keep the entry-point
 * focused on wiring.
 *
 * Side effects (console.error logging about auth token resolution) are
 * preserved here so behavior is identical to the pre-split module.
 */
import { fileURLToPath } from "url";
export interface BootstrapConfig {
    RIFTBORN_HTTP_PORT: number;
    RIFTBORN_TCP_PORT: number;
    RIFTBORN_BRAIN_PORT: number;
    RIFTBORN_HOST: string;
    RIFTBORN_AUTH_TOKEN: string;
    ENABLE_INTERNAL_TOOLS: boolean;
    ALLOW_HIDDEN_TOOLS: boolean;
    DEV_MODE: boolean;
    INTERNAL_ONLY_TOOLS: Set<string>;
    BLOCKED_TOOLS: Set<string>;
}
export declare function loadBootstrapConfig(entryFilename: string): BootstrapConfig;
export { fileURLToPath };
//# sourceMappingURL=bootstrap.d.ts.map