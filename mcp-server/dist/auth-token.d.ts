export declare const ENV_TOKEN_NAMES: readonly ["RIFTBORN_AUTH_TOKEN", "RIFTBORN_API_KEY", "RIFTBORN_DEV_TOKEN"];
export interface AuthTokenResolution {
    token: string;
    source: string;
    searchedPaths: string[];
}
export declare function resolveBridgeAuthToken(options?: {
    env?: NodeJS.ProcessEnv;
    searchRoots?: string[];
}): AuthTokenResolution;
//# sourceMappingURL=auth-token.d.ts.map