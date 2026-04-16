import { Server } from "@modelcontextprotocol/sdk/server/index.js";
export declare function normalizePromptArgument(value: string | undefined, fallback: string): string;
export declare const PROMPTS: {
    name: string;
    description: string;
    arguments: {
        name: string;
        description: string;
        required: boolean;
    }[];
}[];
export declare function buildPromptText(name: string, args?: Record<string, string | undefined>): string;
export declare function registerPromptHandlers(server: Server): void;
//# sourceMappingURL=mcp-prompts.d.ts.map