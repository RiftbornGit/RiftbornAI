import * as crypto from "node:crypto";
import * as fs from "node:fs";
import * as path from "node:path";
import { fileURLToPath } from "node:url";
import { createSanitizer, createToSafeRecord } from "./sanitize-utils.js";
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const PLUGIN_ROOT = path.resolve(__dirname, "../..");
const CONTRACTS_PATH = path.resolve(PLUGIN_ROOT, "Bridge/toolbook/contracts.json");
const ARTIFACTS_DIR = path.resolve(PLUGIN_ROOT, "artifacts");
const SAVED_DIR = path.resolve(PLUGIN_ROOT, "Saved/RiftbornAI");
const sanitizeParsedJson = createSanitizer();
const toSafeRecord = createToSafeRecord(sanitizeParsedJson);
export var RiskTier;
(function (RiskTier) {
    RiskTier[RiskTier["SAFE"] = 0] = "SAFE";
    RiskTier[RiskTier["RECOVERY"] = 1] = "RECOVERY";
    RiskTier[RiskTier["MUTATING_REVERSIBLE"] = 2] = "MUTATING_REVERSIBLE";
    RiskTier[RiskTier["MUTATING_PROJECT"] = 3] = "MUTATING_PROJECT";
    RiskTier[RiskTier["DESTRUCTIVE"] = 4] = "DESTRUCTIVE";
})(RiskTier || (RiskTier = {}));
let cachedRiskMap = null;
let cachedContractsMissing = false;
let cachedSigningSecret = null;
function parseRiskTier(value) {
    switch ((value || "").toLowerCase()) {
        case "safe":
        case "read_only":
        case "verification_safe":
            return RiskTier.SAFE;
        case "recovery":
        case "system":
            return RiskTier.RECOVERY;
        case "mutating_reversible":
            return RiskTier.MUTATING_REVERSIBLE;
        case "mutating_irreversible":
        case "mutating_project":
            return RiskTier.MUTATING_PROJECT;
        case "destructive":
            return RiskTier.DESTRUCTIVE;
        default:
            return RiskTier.MUTATING_REVERSIBLE;
    }
}
function ensureRiskMapLoaded() {
    if (cachedRiskMap) {
        return;
    }
    cachedRiskMap = new Map();
    cachedContractsMissing = false;
    if (!fs.existsSync(CONTRACTS_PATH)) {
        cachedContractsMissing = true;
        return;
    }
    try {
        const payload = JSON.parse(fs.readFileSync(CONTRACTS_PATH, "utf8"));
        const rawTools = payload.tools || {};
        for (const [toolName, contract] of Object.entries(rawTools)) {
            const riskName = contract.classification?.risk_tier || contract.risk_tier;
            cachedRiskMap.set(toolName, parseRiskTier(riskName));
        }
    }
    catch {
        cachedContractsMissing = true;
        cachedRiskMap.clear();
    }
}
/** True iff contracts.json could not be loaded — exposed so MCP startup
 *  can fail closed (refuse to start) rather than route through with a
 *  permissive default that misclassifies mutating tools as safe. */
export function isContractsMissing() {
    ensureRiskMapLoaded();
    return cachedContractsMissing;
}
export function getToolRiskTier(toolName) {
    ensureRiskMapLoaded();
    if (cachedRiskMap?.has(toolName)) {
        return cachedRiskMap.get(toolName);
    }
    // Unknown tools always default to the highest non-destructive tier.
    // The previous "if contracts missing, default to SAFE" branch was
    // fail-OPEN — a packaged MCP without contracts would silently route
    // mutators (compile_project, patch_cpp_file, etc.) through direct
    // execution. Now contracts-missing is a startup error (see
    // assertContractsLoaded), so this code path treats every unknown
    // tool as high-risk and routes it through the governed lane.
    return RiskTier.MUTATING_PROJECT;
}
/** Throws when contracts.json is missing or unparseable. Call from MCP
 *  startup; without contracts the server cannot make safe routing
 *  decisions, so refusing to start is the only correct behavior. */
export function assertContractsLoaded() {
    ensureRiskMapLoaded();
    if (cachedContractsMissing) {
        throw new Error(`RiftbornAI MCP cannot start: contracts manifest missing or unparseable at ${CONTRACTS_PATH}. ` +
            "Without contracts the governed-execution layer cannot classify tools. " +
            "Re-package with Bridge/toolbook/contracts.json present, or run from a complete source tree.");
    }
}
export function requiresGovernedExecution(toolName) {
    return getToolRiskTier(toolName) > RiskTier.SAFE;
}
function normalizeForCanonical(value) {
    if (Array.isArray(value)) {
        return value.map((entry) => normalizeForCanonical(entry));
    }
    if (value && typeof value === "object") {
        const entries = Object.entries(value)
            .sort(([left], [right]) => left.localeCompare(right))
            .map(([key, entry]) => [key, normalizeForCanonical(entry)]);
        return Object.fromEntries(entries);
    }
    if (typeof value === "number") {
        if (!Number.isFinite(value)) {
            return 0;
        }
        if (Object.is(value, -0)) {
            return 0;
        }
        return Number.isInteger(value) ? Math.trunc(value) : value;
    }
    return value;
}
export function canonicalArgsJson(value) {
    return JSON.stringify(normalizeForCanonical(value));
}
export function hashArgs(args) {
    return crypto.createHash("sha256").update(canonicalArgsJson(args), "utf8").digest("hex");
}
export function hashPlan(planJson) {
    return crypto.createHash("sha256").update(planJson, "utf8").digest("hex");
}
function getSigningSecret() {
    if (cachedSigningSecret) {
        return cachedSigningSecret;
    }
    const envSecret = process.env.RIFTBORN_EXEC_SECRET;
    if (typeof envSecret === "string" && envSecret.length > 0) {
        const envBytes = Buffer.from(envSecret, "utf8");
        if (envBytes.length >= 16) {
            cachedSigningSecret = envBytes;
            return envBytes;
        }
        console.error(`[RiftbornAI] WARNING: RIFTBORN_EXEC_SECRET is set but too short (${envBytes.length} < 16 bytes). Falling back to file-based secret.`);
    }
    const candidates = [
        { filePath: path.resolve(ARTIFACTS_DIR, "exec_secret.hex"), format: "hex" },
        { filePath: path.resolve(ARTIFACTS_DIR, "exec_secret.key"), format: "raw" },
        { filePath: path.resolve(SAVED_DIR, "exec_secret.hex"), format: "hex" },
        { filePath: path.resolve(SAVED_DIR, "exec_secret.key"), format: "raw" },
    ];
    for (const candidate of candidates) {
        if (!fs.existsSync(candidate.filePath)) {
            continue;
        }
        const secret = candidate.format === "hex"
            ? Buffer.from(fs.readFileSync(candidate.filePath, "utf8").trim(), "hex")
            : fs.readFileSync(candidate.filePath);
        if (secret.length > 0) {
            cachedSigningSecret = secret;
            return secret;
        }
    }
    throw new Error("No ExecCtx signing secret found. Set RIFTBORN_EXEC_SECRET or generate artifacts/exec_secret.hex.");
}
function canonicalExecCtxPayload(ctx) {
    return [
        "{",
        `"args_hash":${JSON.stringify(ctx.args_hash)},`,
        `"caller":${JSON.stringify(ctx.caller)},`,
        `"ctx_id":${JSON.stringify(ctx.ctx_id)},`,
        `"expires_at":${ctx.expires_at.toFixed(1)},`,
        `"issued_at":${ctx.issued_at.toFixed(1)},`,
        `"nonce":${JSON.stringify(ctx.nonce)},`,
        `"plan_id":${JSON.stringify(ctx.plan_id)},`,
        `"risk_tier":${ctx.risk_tier},`,
        `"rollback_proof_id":${JSON.stringify(ctx.rollback_proof_id)},`,
        `"rollback_ready":${ctx.rollback_ready ? "true" : "false"},`,
        `"step_id":${ctx.step_id},`,
        `"tool_name":${JSON.stringify(ctx.tool_name)}`,
        "}",
    ].join("");
}
export function computeExecCtxSignature(ctx) {
    const secret = getSigningSecret();
    return crypto
        .createHmac("sha256", secret)
        .update(canonicalExecCtxPayload(ctx), "utf8")
        .digest("hex");
}
function buildExecCtx(callerId, planId, stepId, toolName, args, riskTier, rollbackReady, rollbackProofId, ttlSeconds) {
    const issuedAt = Number((Date.now() / 1000).toFixed(1));
    const expiresAt = Number((issuedAt + ttlSeconds).toFixed(1));
    const unsignedCtx = {
        ctx_id: crypto.randomUUID(),
        plan_id: planId,
        step_id: stepId,
        tool_name: toolName,
        args_hash: hashArgs(args),
        risk_tier: Number(riskTier),
        caller: callerId,
        issued_at: issuedAt,
        expires_at: expiresAt,
        rollback_ready: rollbackReady,
        rollback_proof_id: rollbackProofId,
        nonce: crypto.randomUUID().replace(/-/g, ""),
    };
    return {
        ...unsignedCtx,
        signature: computeExecCtxSignature(unsignedCtx),
    };
}
function buildConfirmationHint(metadata) {
    const token = typeof metadata?.confirmation_token === "string" ? metadata.confirmation_token : undefined;
    if (!token) {
        return undefined;
    }
    return {
        token,
        expires: typeof metadata?.confirmation_expires === "string" ? metadata.confirmation_expires : undefined,
        reason: typeof metadata?.confirmation_reason === "string" ? metadata.confirmation_reason : undefined,
    };
}
export function flattenAgentStepResponse(raw) {
    const safeRaw = sanitizeParsedJson(raw);
    if (!safeRaw.ok) {
        return {
            ok: false,
            error: safeRaw.error || "Governed step failed",
            error_code: safeRaw.error_code,
            exec_ctx_id: safeRaw.exec_ctx_id,
        };
    }
    const step = safeRaw.result;
    const toolResult = step?.tool_result;
    if (!step || !toolResult) {
        return { ok: false, error: "Governed step returned an unexpected response shape." };
    }
    const metadata = toolResult.metadata && typeof toolResult.metadata === "object"
        ? toSafeRecord(toolResult.metadata)
        : undefined;
    const confirmation = buildConfirmationHint(metadata);
    const flattened = {
        ok: toolResult.success === true,
        result: sanitizeParsedJson(toolResult.result),
        error: toolResult.error,
    };
    if (metadata && Object.keys(metadata).length > 0) {
        flattened.metadata = metadata;
    }
    if (toolResult.receipt && typeof toolResult.receipt === "object") {
        flattened.receipt = toSafeRecord(toolResult.receipt);
    }
    if (confirmation) {
        flattened.policy_decision = "needs_confirmation";
        flattened.policy_reason = confirmation.reason || toolResult.error;
        flattened.confirmation = confirmation;
    }
    flattened.governed = {
        run_id: step.run_id,
        outcome: step.outcome,
        execution_time_ms: step.execution_time_ms,
        completed_at: step.completed_at,
        executed_tool: step.executed_tool,
        next_action_summary: step.next_action_summary,
        goal_still_achievable: step.goal_still_achievable,
        belief_updated: step.belief_updated,
    };
    return flattened;
}
function buildAgentStepTimeoutMs(maxExecutionTimeMs) {
    return Math.min(3_600_000, Math.max(10_000, maxExecutionTimeMs + 5_000));
}
async function readResponseBody(response) {
    try {
        return await response.text();
    }
    catch {
        return "";
    }
}
function parseAgentStepBody(rawText) {
    if (!rawText.trim()) {
        return null;
    }
    try {
        return sanitizeParsedJson(JSON.parse(rawText));
    }
    catch {
        return null;
    }
}
export class GovernedExecutionClient {
    config;
    callerId;
    fetchImpl;
    planSeed;
    planContext = null;
    constructor(config, options) {
        this.config = config;
        this.callerId = options?.callerId || "mcp_server";
        this.fetchImpl = options?.fetchImpl || fetch;
        this.planSeed = options?.planSeed || crypto.randomUUID();
    }
    ensurePlanContext() {
        if (!this.planContext) {
            const planSeedJson = JSON.stringify({
                caller: this.callerId,
                plan_seed: this.planSeed,
            });
            const planId = hashPlan(planSeedJson);
            this.planContext = {
                planId,
                planHash: planId,
                nextStepIndex: 1,
            };
        }
        return this.planContext;
    }
    async executeTool(toolName, args, riskTier, maxExecutionTimeMs) {
        const plan = this.ensurePlanContext();
        const stepIndex = plan.nextStepIndex;
        const rollbackReady = riskTier >= RiskTier.MUTATING_PROJECT;
        const rollbackProofId = rollbackReady ? `${toolName}:${plan.planId}:${stepIndex}` : "";
        const execCtx = buildExecCtx(this.callerId, plan.planId, stepIndex, toolName, args, riskTier, rollbackReady, rollbackProofId, 60);
        plan.nextStepIndex = stepIndex + 1;
        const payload = {
            goal_id: crypto.randomUUID(),
            tool_name: toolName,
            arguments_json: canonicalArgsJson(args),
            step_number: stepIndex,
            max_execution_time_ms: maxExecutionTimeMs,
            strict_mode: false,
            plan_id: plan.planId,
            plan_hash: plan.planHash,
            plan_step_index: stepIndex,
            exec_ctx: execCtx,
        };
        const headers = {
            "Content-Type": "application/json",
        };
        if (this.config.authToken) {
            headers.Authorization = `Bearer ${this.config.authToken}`;
        }
        const url = `http://${this.config.host}:${this.config.httpPort}/riftborn/agent/step`;
        try {
            const response = await this.fetchImpl(url, {
                method: "POST",
                headers,
                body: JSON.stringify(payload),
                signal: AbortSignal.timeout(buildAgentStepTimeoutMs(maxExecutionTimeMs)),
            });
            const rawText = await readResponseBody(response);
            const parsed = parseAgentStepBody(rawText);
            if (!response.ok) {
                if (parsed?.error) {
                    return {
                        ok: false,
                        error: parsed.error,
                        error_code: parsed.error_code,
                        exec_ctx_id: parsed.exec_ctx_id,
                    };
                }
                return {
                    ok: false,
                    error: rawText ? `HTTP ${response.status}: ${rawText.slice(0, 200)}` : `HTTP ${response.status}`,
                };
            }
            if (!parsed) {
                return {
                    ok: false,
                    error: "HTTP 200 with invalid JSON body from /riftborn/agent/step",
                };
            }
            return flattenAgentStepResponse(parsed);
        }
        catch (error) {
            const message = error instanceof Error ? error.message : String(error);
            return {
                ok: false,
                error: `Governed step request failed: ${message.slice(0, 256)}`,
            };
        }
    }
}
export function getGovernedRouteTimeoutMs(toolName, baseTimeoutMs) {
    return requiresGovernedExecution(toolName)
        ? buildAgentStepTimeoutMs(baseTimeoutMs)
        : baseTimeoutMs;
}
export function resetGovernedExecutionStateForTest() {
    cachedRiskMap = null;
    cachedContractsMissing = false;
    cachedSigningSecret = null;
}