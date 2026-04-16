import type { BridgeConfig } from "./bridge-reliability.js";
export interface SessionTokenRequest {
    allowedTools?: string[];
    clientId: string;
    deniedTools?: string[];
    template: "artist" | "tech_artist" | "engineer" | "ci_runner";
    ttlSeconds: number;
}
export interface SessionTokenResponse {
    boundIp: string;
    expiresAt: string;
    expiresIn: number;
    role: string;
    status: string;
    tokenId: string;
}
export declare function exchangeScopedSessionToken(config: BridgeConfig, request: SessionTokenRequest): Promise<SessionTokenResponse>;
//# sourceMappingURL=session-token.d.ts.map