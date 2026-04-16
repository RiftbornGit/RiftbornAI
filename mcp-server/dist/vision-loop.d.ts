/**
 * RiftbornAI Vision Loop — Gives Claude eyes inside Unreal Engine
 *
 * Architecture:
 *   OBSERVE → REASON → PLAN → BUILD → VERIFY → ITERATE
 *
 * Claude captures viewport screenshots, analyzes them with vision AI,
 * reasons about what needs to change, executes tools, then verifies
 * the result visually. This creates a closed-loop visual builder.
 */
import { Tool } from "@modelcontextprotocol/sdk/types.js";
export declare const VISION_LOOP_TOOLS: Tool[];
export declare function createVisionHandlers(executeTool: (name: string, args: object) => Promise<any>, _httpRequest: (method: "GET" | "POST", path: string, body?: object) => Promise<any>): {
    vision_observe: (args: any) => Promise<{
        ok: boolean;
        error: string;
        observation?: undefined;
        tip?: undefined;
    } | {
        ok: boolean;
        observation: any;
        tip: string;
        error?: undefined;
    }>;
    vision_compare: (args: any) => Promise<{
        ok: boolean;
        error: string;
        after_screenshot?: undefined;
        analysis?: undefined;
    } | {
        ok: boolean;
        error: string;
        after_screenshot: any;
        analysis?: undefined;
    } | {
        ok: boolean;
        after_screenshot: any;
        analysis: any;
        error?: undefined;
    }>;
    vision_inspect_actor: (args: any) => Promise<{
        ok: boolean;
        error: string;
        actor?: undefined;
        inspection?: undefined;
    } | {
        ok: boolean;
        actor: any;
        inspection: any;
        error?: undefined;
    }>;
    vision_playtest: (args: any) => Promise<{
        ok: boolean;
        error: string;
        details: any;
        before_screenshot?: undefined;
        gameplay_screenshot?: undefined;
        analysis?: undefined;
        tip?: undefined;
    } | {
        ok: boolean;
        error: string;
        details?: undefined;
        before_screenshot?: undefined;
        gameplay_screenshot?: undefined;
        analysis?: undefined;
        tip?: undefined;
    } | {
        ok: any;
        before_screenshot: any;
        gameplay_screenshot: any;
        analysis: any;
        tip: string;
        error?: undefined;
        details?: undefined;
    }>;
    vision_sweep: (args: any) => Promise<{
        ok: boolean;
        sweep_count: number;
        failed_angles: number;
        views: any[];
        tip: string;
    }>;
    vision_build_and_verify: (args: any) => Promise<{
        ok: boolean;
        error: string;
        tool_result?: undefined;
        before_screenshot?: undefined;
        after_screenshot?: undefined;
        visual_verification?: undefined;
        verified?: undefined;
    } | {
        ok: any;
        tool_result: any;
        before_screenshot: any;
        after_screenshot: any;
        visual_verification: any;
        verified: any;
        error?: undefined;
    }>;
};
//# sourceMappingURL=vision-loop.d.ts.map