import type { BridgeConfig, RiftbornResponse } from "./bridge-reliability.js";
export declare enum RiskTier {
    SAFE = 0,
    RECOVERY = 1,
    MUTATING_REVERSIBLE = 2,
    MUTATING_PROJECT = 3,
    DESTRUCTIVE = 4
}
export interface ExecCtx {
    ctx_id: string;
    plan_id: string;
    step_id: number;
    tool_name: string;
    args_hash: string;
    risk_tier: number;
    caller: string;
    issued_at: number;
    expires_at: number;
    rollback_ready: boolean;
    rollback_proof_id: string;
    nonce: string;
    signature: string;
}
interface AgentStepToolResult {
    success?: boolean;
    result?: unknown;
    error?: string;
    metadata?: Record<string, unknown>;
    receipt?: Record<string, unknown>;
}
interface AgentStepBody {
    ok?: boolean;
    error?: string;
    error_code?: string;
    exec_ctx_id?: string;
    result?: {
        run_id?: string;
        executed_tool?: string;
        outcome?: string;
        execution_time_ms?: number;
        completed_at?: string;
        belief_updated?: boolean;
        goal_still_achievable?: boolean;
        next_action_summary?: string;
        delta?: Record<string, unknown>;
        tool_result?: AgentStepToolResult;
    };
}
interface FetchResponseLike {
    ok: boolean;
    status: number;
    headers: {
        get(name: string): string | null;
    };
    json(): Promise<unknown>;
    text(): Promise<string>;
}
type FetchLike = (input: string, init?: RequestInit) => Promise<FetchResponseLike>;
export declare function getToolRiskTier(toolName: string): RiskTier;
export declare function requiresGovernedExecution(toolName: string): boolean;
export declare function canonicalArgsJson(value: unknown): string;
export declare function hashArgs(args: Record<string, unknown>): string;
export declare function hashPlan(planJson: string): string;
export declare function computeExecCtxSignature(ctx: Omit<ExecCtx, "signature">): string;
export declare function flattenAgentStepResponse(raw: AgentStepBody): RiftbornResponse;
export declare class GovernedExecutionClient {
    private readonly config;
    private readonly callerId;
    private readonly fetchImpl;
    private readonly planSeed;
    private planContext;
    constructor(config: BridgeConfig, options?: {
        callerId?: string;
        fetchImpl?: FetchLike;
        planSeed?: string;
    });
    private ensurePlanContext;
    executeTool(toolName: string, args: Record<string, unknown>, riskTier: RiskTier, maxExecutionTimeMs: number): Promise<RiftbornResponse>;
}
export declare function getGovernedRouteTimeoutMs(toolName: string, baseTimeoutMs: number): number;
export declare function resetGovernedExecutionStateForTest(): void;
export {};
//# sourceMappingURL=governed-execution.d.ts.map