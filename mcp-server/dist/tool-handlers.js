import { createVisionHandlers } from "./vision-loop.js";
import { diagnoseCrash } from "./crash-diagnosis.js";
import { buildAllTools } from "./manual-tools.js";
import { clampToolSearchResults, normalizeToolSearchQuery } from "./pipeline-intelligence.js";
import { getWorkflow, listWorkflows, normalizeWorkflowQuery } from "./tool-compression.js";
import { normalizeGoalQuery, planFromGoal, planToBatchSteps, listGoals } from "./call-planner.js";
import { createSanitizer, PROTO_BLOCKED_KEYS } from "./sanitize-utils.js";
import { createSceneEnhancementHandlers } from "./scene-enhancement.js";
import { createPolyhavenHandlers } from "./polyhaven.js";
import { promises as fs } from "fs";
import * as os from "os";
import * as path from "path";
const MAX_JSON_RESULT_LENGTH = 64 * 1024;
const MESHY_TEXT_TO_3D_ENDPOINT = "https://api.meshy.ai/openapi/v2/text-to-3d";
const MESHY_IMAGE_TO_3D_ENDPOINT = "https://api.meshy.ai/openapi/v1/image-to-3d";
const MESHY_MULTI_IMAGE_TO_3D_ENDPOINT = "https://api.meshy.ai/openapi/v1/multi-image-to-3d";
const MESHY_REQUEST_TIMEOUT_MS = 30_000;
const MESHY_TASK_TIMEOUT_MS = 15 * 60 * 1000;
const MESHY_POLL_INTERVAL_MS = 5_000;
const MESHY_MAX_PROMPT_LENGTH = 600;
const MESHY_MIN_TARGET_POLYCOUNT = 100;
const MESHY_MAX_TARGET_POLYCOUNT = 300_000;
const MESHY_ALLOWED_MODELS = new Set(["latest", "meshy-5", "meshy-6"]);
const MESHY_ALLOWED_MODEL_TYPES = new Set(["standard", "lowpoly"]);
const MESHY_ALLOWED_TOPOLOGIES = new Set(["triangle", "quad"]);
const MESHY_ALLOWED_ORIGIN_AT = new Set(["bottom", "center"]);
const MESHY_ALLOWED_POSE_MODE = new Set(["", "a-pose", "t-pose"]);
const OPENAI_IMAGE_ENDPOINT = "https://api.openai.com/v1/images/generations";
const OPENAI_IMAGE_EDIT_ENDPOINT = "https://api.openai.com/v1/images/edits";
const OPENAI_IMAGE_REQUEST_TIMEOUT_MS = 120_000;
const OPENAI_IMAGE_MAX_PROMPT_LENGTH = 32_000;
const OPENAI_ALLOWED_IMAGE_MODELS = new Set(["gpt-image-1.5", "gpt-image-1", "gpt-image-1-mini"]);
const OPENAI_ALLOWED_IMAGE_SIZES = new Set(["auto", "1024x1024", "1536x1024", "1024x1536"]);
const OPENAI_ALLOWED_IMAGE_QUALITIES = new Set(["auto", "low", "medium", "high"]);
const OPENAI_ALLOWED_IMAGE_BACKGROUNDS = new Set(["auto", "opaque", "transparent"]);
const OPENAI_ALLOWED_IMAGE_FORMATS = new Set(["png", "jpeg", "webp"]);
const OPENAI_ALLOWED_MODERATION = new Set(["auto", "low"]);
const OPENAI_ALLOWED_INPUT_FIDELITY = new Set(["low", "high"]);
const ELEVENLABS_SOUND_ENDPOINT = "https://api.elevenlabs.io/v1/sound-generation";
const ELEVENLABS_SOUND_REQUEST_TIMEOUT_MS = 120_000;
const ELEVENLABS_DEFAULT_SOUND_OUTPUT_FORMAT = "mp3_44100_128";
const ELEVENLABS_MIN_DURATION_SECONDS = 0.5;
const ELEVENLABS_MAX_DURATION_SECONDS = 30;
const IMAGE_MIME_BY_EXTENSION = {
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".webp": "image/webp",
    ".bmp": "image/bmp",
    ".gif": "image/gif",
    ".tga": "image/x-tga",
};
const sanitizeParsedJson = createSanitizer();
/** Parse hit Z from C++ line_trace text: "Location: (X, Y, Z)" */
function parseLineTraceZ(text) {
    // Try JSON first (future-proofing)
    const j = safeJsonParse(text);
    if (j && typeof j === "object") {
        return j.impact_z ?? j.impact_point?.z ?? j.impact_point?.Z ?? null;
    }
    // C++ format: "Location: (8000.0, 5000.0, -12.0)"
    const m = text.match(/Location:\s*\(\s*[\d.-]+,\s*[\d.-]+,\s*([\d.-]+)\s*\)/);
    return m ? parseFloat(m[1]) : null;
}
function safeJsonParse(text) {
    if (typeof text !== "string" || text.length > MAX_JSON_RESULT_LENGTH) {
        return null;
    }
    try {
        const parsed = JSON.parse(text);
        if (typeof parsed !== "object" || parsed === null || Array.isArray(parsed)) {
            return null;
        }
        return sanitizeParsedJson(parsed);
    }
    catch {
        return null;
    }
}
function toSafeRecord(value) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
        return {};
    }
    const out = {};
    for (const [key, entry] of Object.entries(value)) {
        if (PROTO_BLOCKED_KEYS.has(key)) {
            continue;
        }
        out[key] = entry;
    }
    return out;
}
export function buildSafePlanWorkflowResult(plan, batchSteps) {
    const result = toSafeRecord(plan);
    result.batch_steps = Array.isArray(batchSteps) ? sanitizeParsedJson(batchSteps) : [];
    return result;
}
const MAX_BATCH_STEPS = 10;
const MAX_CODE_LENGTH = 65_536;
function normalizeConsoleCommand(command) {
    if (typeof command !== "string") {
        return { ok: false, error: "execute_console_command requires a string 'command'." };
    }
    const trimmed = command.trim();
    if (!trimmed) {
        return { ok: false, error: "execute_console_command requires a non-empty command." };
    }
    if (trimmed.length > MAX_CODE_LENGTH) {
        return { ok: false, error: `execute_console_command input exceeds ${MAX_CODE_LENGTH} character limit.` };
    }
    if (/[\r\n\0]/.test(trimmed)) {
        return { ok: false, error: "execute_console_command only accepts a single-line command without NUL bytes." };
    }
    // SECURITY: Block shell metacharacters that could enable command injection
    // if the console command is passed through a shell context on the C++ side.
    // Matches the C++ ToolImpl_FileSystem.cpp metacharacter filter.
    if (/[&|;`$(){}]/.test(trimmed)) {
        return { ok: false, error: "execute_console_command: shell metacharacters (&|;`$(){}) are not allowed." };
    }
    return { ok: true, value: trimmed };
}
function normalizePythonCode(code) {
    if (typeof code !== "string") {
        return { ok: false, error: "execute_python requires a string 'code'." };
    }
    if (!code.trim()) {
        return { ok: false, error: "execute_python requires non-empty Python code." };
    }
    if (code.length > MAX_CODE_LENGTH) {
        return { ok: false, error: `execute_python input exceeds ${MAX_CODE_LENGTH} character limit.` };
    }
    if (code.includes("\0")) {
        return { ok: false, error: "execute_python does not accept NUL bytes." };
    }
    return { ok: true, value: code };
}
function normalizeOptionalString(value) {
    if (typeof value !== "string") {
        return undefined;
    }
    const trimmed = value.trim();
    return trimmed ? trimmed : undefined;
}
function normalizeStringArray(value, fieldName, maxItems) {
    if (value == null) {
        return { ok: true, value: [] };
    }
    if (typeof value === "string") {
        const trimmed = value.trim();
        if (!trimmed) {
            return { ok: true, value: [] };
        }
        if (trimmed.startsWith("[")) {
            try {
                return normalizeStringArray(JSON.parse(trimmed), fieldName, maxItems);
            }
            catch {
                return { ok: false, error: `${fieldName} must be a string array.` };
            }
        }
        return { ok: true, value: [trimmed] };
    }
    if (!Array.isArray(value)) {
        return { ok: false, error: `${fieldName} must be a string array.` };
    }
    if (value.length > maxItems) {
        return { ok: false, error: `${fieldName} supports at most ${maxItems} item(s).` };
    }
    const normalized = [];
    for (const entry of value) {
        if (typeof entry !== "string") {
            return { ok: false, error: `${fieldName} must only contain strings.` };
        }
        const trimmed = entry.trim();
        if (trimmed) {
            normalized.push(trimmed);
        }
    }
    return { ok: true, value: normalized };
}
function clampNumber(value, minValue, maxValue) {
    return Math.min(maxValue, Math.max(minValue, value));
}
function inferImageMimeType(source) {
    const normalized = source.split("?")[0] || source;
    return IMAGE_MIME_BY_EXTENSION[path.extname(normalized).toLowerCase()];
}
function isRemoteOrDataUri(source) {
    return /^https?:\/\//i.test(source) || /^data:/i.test(source);
}
async function resolveImageSource(source, fieldName) {
    const trimmed = source.trim();
    if (!trimmed) {
        return { ok: false, error: `${fieldName} must not be empty.` };
    }
    if (isRemoteOrDataUri(trimmed)) {
        return { ok: true, value: trimmed };
    }
    const mimeType = inferImageMimeType(trimmed);
    if (!mimeType) {
        return {
            ok: false,
            error: `${fieldName} must be a public URL, data URI, or readable local image path with a supported extension.`,
        };
    }
    try {
        const fileBytes = await fs.readFile(trimmed);
        return { ok: true, value: `data:${mimeType};base64,${fileBytes.toString("base64")}` };
    }
    catch {
        return {
            ok: false,
            error: `${fieldName} must be a public URL, data URI, or readable local image path.`,
        };
    }
}
async function resolveImageSources(sources, fieldName, maxItems) {
    if (sources.length > maxItems) {
        return { ok: false, error: `${fieldName} supports at most ${maxItems} item(s).` };
    }
    const resolved = [];
    for (let index = 0; index < sources.length; index += 1) {
        const entryResult = await resolveImageSource(sources[index], `${fieldName}[${index}]`);
        if (!entryResult.ok) {
            return entryResult;
        }
        resolved.push(entryResult.value);
    }
    return { ok: true, value: resolved };
}
async function writeTempBinaryFile(baseName, extension, bytes) {
    const safeBaseName = normalizeOptionalString(baseName) || "generated_asset";
    const tempFilePath = path.join(os.tmpdir(), `${safeBaseName}_${Date.now()}.${extension.replace(/^\./, "")}`);
    await fs.writeFile(tempFilePath, bytes);
    return tempFilePath;
}
function extractImportedAssetPath(result) {
    if (result && typeof result === "object" && !Array.isArray(result)) {
        const assetPath = result.asset_path;
        if (typeof assetPath === "string" && assetPath.trim()) {
            return assetPath.trim();
        }
    }
    if (typeof result !== "string") {
        return undefined;
    }
    const parsed = safeJsonParse(result);
    if (parsed && typeof parsed.asset_path === "string" && parsed.asset_path.trim()) {
        return parsed.asset_path.trim();
    }
    const match = result.match(/(\/Game\/[A-Za-z0-9_./]+)/);
    return match?.[1];
}
function normalizeBoundedPrompt(value, fieldName) {
    return normalizeBoundedPromptWithLimit(value, fieldName, MESHY_MAX_PROMPT_LENGTH);
}
function normalizeBoundedPromptWithLimit(value, fieldName, maxLength) {
    if (value == null) {
        return { ok: true };
    }
    if (typeof value !== "string") {
        return { ok: false, error: `${fieldName} must be a string.` };
    }
    const trimmed = value.trim();
    if (!trimmed) {
        return { ok: true };
    }
    return { ok: true, value: trimmed.slice(0, maxLength) };
}
function normalizeOptionalEnum(value, fieldName, allowed) {
    const normalized = normalizeOptionalString(value)?.toLowerCase();
    if (!normalized) {
        return { ok: true };
    }
    if (!allowed.has(normalized)) {
        return {
            ok: false,
            error: `${fieldName} must be one of: ${Array.from(allowed).join(", ")}.`,
        };
    }
    return { ok: true, value: normalized };
}
function normalizeTargetPolycount(value) {
    if (value == null || value === "") {
        return { ok: true };
    }
    const parsed = Number(value);
    if (!Number.isFinite(parsed)) {
        return { ok: false, error: "target_polycount must be a number." };
    }
    return {
        ok: true,
        value: Math.min(MESHY_MAX_TARGET_POLYCOUNT, Math.max(MESHY_MIN_TARGET_POLYCOUNT, Math.round(parsed))),
    };
}
function normalizeBooleanArg(value, defaultValue) {
    return typeof value === "boolean" ? value : defaultValue;
}
function deriveAssetNameFromPrompt(prompt) {
    const normalized = prompt
        .toLowerCase()
        .replace(/[^a-z0-9]+/g, "_")
        .replace(/^_+|_+$/g, "")
        .slice(0, 64);
    return normalized || "generated_model";
}
function extractMeshyError(payload, fallback) {
    const directMessage = payload?.message;
    if (typeof directMessage === "string" && directMessage.trim()) {
        return directMessage.trim();
    }
    const nestedMessage = payload?.task_error?.message;
    if (typeof nestedMessage === "string" && nestedMessage.trim()) {
        return nestedMessage.trim();
    }
    const nestedError = payload?.error;
    if (typeof nestedError === "string" && nestedError.trim()) {
        return nestedError.trim();
    }
    return fallback;
}
function extractOpenAIError(payload, fallback) {
    const nestedError = payload?.error?.message;
    if (typeof nestedError === "string" && nestedError.trim()) {
        return nestedError.trim();
    }
    const directMessage = payload?.message;
    if (typeof directMessage === "string" && directMessage.trim()) {
        return directMessage.trim();
    }
    return fallback;
}
function parseOpenAIModelAccessError(message) {
    if (typeof message !== "string" || !message.trim()) {
        return null;
    }
    const match = message.match(/Project ['"]([^'"]+)['"] does not have access to model ['"]([^'"]+)['"]/i);
    if (!match) {
        return null;
    }
    return {
        projectId: match[1],
        model: match[2],
    };
}
function formatOpenAIImageError(message, accessError) {
    if (!accessError) {
        return message;
    }
    return [
        `OpenAI image request was authenticated, but project '${accessError.projectId}' does not have access to model '${accessError.model}'.`,
        "Use an OPENAI_API_KEY from a project with image-model access, confirm the project can make model requests for images, complete any required organization verification, or try gpt-image-1-mini.",
        `OpenAI returned: ${message}`,
    ].join(" ");
}
async function meshyJsonRequest(apiKey, method, url, body) {
    try {
        const response = await fetch(url, {
            method,
            headers: {
                Authorization: `Bearer ${apiKey}`,
                Accept: "application/json",
                ...(body ? { "Content-Type": "application/json" } : {}),
            },
            body: body ? JSON.stringify(body) : undefined,
            signal: AbortSignal.timeout(MESHY_REQUEST_TIMEOUT_MS),
        });
        const text = await response.text();
        const parsed = safeJsonParse(text);
        if (!response.ok) {
            return {
                ok: false,
                error: extractMeshyError(parsed, `Meshy API request failed with HTTP ${response.status}.`),
            };
        }
        return { ok: true, result: parsed ?? { raw: text } };
    }
    catch (error) {
        return {
            ok: false,
            error: `Meshy API request failed: ${error instanceof Error ? error.message : String(error)}`,
        };
    }
}
async function waitForMeshyTask(apiKey, taskId) {
    return waitForMeshyTaskAt(apiKey, MESHY_TEXT_TO_3D_ENDPOINT, taskId);
}
async function waitForMeshyTaskAt(apiKey, endpoint, taskId) {
    const deadline = Date.now() + MESHY_TASK_TIMEOUT_MS;
    while (Date.now() < deadline) {
        const response = await meshyJsonRequest(apiKey, "GET", `${endpoint}/${encodeURIComponent(taskId)}`);
        if (!response.ok) {
            return response;
        }
        const task = toSafeRecord(response.result);
        const status = String(task.status || "").toUpperCase();
        if (status === "SUCCEEDED") {
            return { ok: true, result: task };
        }
        if (status === "FAILED" || status === "CANCELED") {
            return {
                ok: false,
                error: extractMeshyError(task, `Meshy task ${taskId} ${status.toLowerCase()}.`),
            };
        }
        await new Promise((resolve) => setTimeout(resolve, MESHY_POLL_INTERVAL_MS));
    }
    return {
        ok: false,
        error: `Meshy task ${taskId} timed out after ${Math.round(MESHY_TASK_TIMEOUT_MS / 60000)} minutes.`,
    };
}
async function openAIImageRequest(apiKey, body) {
    return openAIImageJsonRequest(apiKey, OPENAI_IMAGE_ENDPOINT, body, "generation");
}
async function openAIImageEditRequest(apiKey, body) {
    return openAIImageJsonRequest(apiKey, OPENAI_IMAGE_EDIT_ENDPOINT, body, "edit");
}
async function openAIImageJsonRequest(apiKey, endpoint, body, operation) {
    try {
        const response = await fetch(endpoint, {
            method: "POST",
            headers: {
                Authorization: `Bearer ${apiKey}`,
                Accept: "application/json",
                "Content-Type": "application/json",
            },
            body: JSON.stringify(body),
            signal: AbortSignal.timeout(OPENAI_IMAGE_REQUEST_TIMEOUT_MS),
        });
        const text = await response.text();
        const parsed = safeJsonParse(text);
        if (!response.ok) {
            const rawError = extractOpenAIError(parsed, `OpenAI image ${operation} failed with HTTP ${response.status}.`);
            const accessError = parseOpenAIModelAccessError(rawError);
            return {
                ok: false,
                error: formatOpenAIImageError(rawError, accessError),
                openai_error_message: rawError,
                openai_model_access_denied: Boolean(accessError),
                openai_project_id: accessError?.projectId,
                openai_denied_model: accessError?.model,
            };
        }
        return { ok: true, result: parsed ?? { raw: text } };
    }
    catch (error) {
        return {
            ok: false,
            error: `OpenAI image ${operation} failed: ${error instanceof Error ? error.message : String(error)}`,
        };
    }
}
async function elevenLabsSoundRequest(apiKey, body) {
    try {
        const response = await fetch(`${ELEVENLABS_SOUND_ENDPOINT}?output_format=${encodeURIComponent(ELEVENLABS_DEFAULT_SOUND_OUTPUT_FORMAT)}`, {
            method: "POST",
            headers: {
                "xi-api-key": apiKey,
                Accept: "audio/mpeg",
                "Content-Type": "application/json",
            },
            body: JSON.stringify(body),
            signal: AbortSignal.timeout(ELEVENLABS_SOUND_REQUEST_TIMEOUT_MS),
        });
        if (!response.ok) {
            const errorText = await response.text();
            const parsed = safeJsonParse(errorText);
            return {
                ok: false,
                error: extractOpenAIError(parsed, `ElevenLabs sound generation failed with HTTP ${response.status}.`),
            };
        }
        return {
            ok: true,
            result: Buffer.from(await response.arrayBuffer()),
            contentType: response.headers.get("content-type") || "audio/mpeg",
        };
    }
    catch (error) {
        return {
            ok: false,
            error: `ElevenLabs sound generation failed: ${error instanceof Error ? error.message : String(error)}`,
        };
    }
}
function searchToolsLocally(query, maxResults) {
    const terms = normalizeToolSearchQuery(query)
        .toLowerCase()
        .split(/\s+/)
        .map((term) => term.trim())
        .filter(Boolean);
    if (terms.length === 0) {
        return [];
    }
    const results = [];
    for (const tool of buildAllTools()) {
        const haystack = `${tool.name} ${tool.description ?? ""}`.toLowerCase();
        let score = 0;
        for (const term of terms) {
            if (tool.name.toLowerCase().includes(term)) {
                score += 10;
            }
            else if (haystack.includes(term)) {
                score += 3;
            }
        }
        if (score > 0) {
            results.push({
                name: tool.name,
                description: tool.description ?? "",
                score,
            });
        }
    }
    return results
        .sort((a, b) => b.score - a.score || a.name.localeCompare(b.name))
        .slice(0, clampToolSearchResults(maxResults));
}
export function createToolHandlers({ executeTool, dispatchTool, httpRequest, host: _host, httpPort: _httpPort }) {
    return {
        // ── Name Remaps (MCP name ≠ C++ tool name) ──
        get_current_level: async () => executeTool("get_level_info"),
        generate_and_import_3d_model: async (args) => {
            const url = normalizeOptionalString(args.url);
            const promptResult = normalizeBoundedPrompt(args.prompt, "prompt");
            if (!promptResult.ok) {
                return { ok: false, error: promptResult.error };
            }
            const texturePromptResult = normalizeBoundedPrompt(args.texture_prompt, "texture_prompt");
            if (!texturePromptResult.ok) {
                return { ok: false, error: texturePromptResult.error };
            }
            const referenceImagesResult = normalizeStringArray(args.reference_images, "reference_images", 4);
            if (!referenceImagesResult.ok) {
                return { ok: false, error: referenceImagesResult.error };
            }
            const prompt = promptResult.value;
            const texturePrompt = texturePromptResult.value;
            const referenceImages = referenceImagesResult.value;
            const textureImage = normalizeOptionalString(args.texture_image);
            if ([Boolean(url), Boolean(prompt), referenceImages.length > 0].filter(Boolean).length > 1) {
                return {
                    ok: false,
                    error: "generate_and_import_3d_model accepts exactly one input source: prompt, reference_images, or url.",
                };
            }
            if (!url && !prompt && referenceImages.length === 0) {
                return {
                    ok: false,
                    error: "generate_and_import_3d_model requires one input source: prompt, reference_images, or url.",
                };
            }
            if (url) {
                return executeTool("generate_and_import_3d_model", {
                    url,
                    destination_path: args.destination_path || undefined,
                    asset_name: args.asset_name || undefined,
                    place_in_scene: args.place_in_scene === true,
                    place_x: Number(args.place_x) || 0,
                    place_y: Number(args.place_y) || 0,
                    place_z: Number(args.place_z) || 0,
                    enable_nanite: args.enable_nanite === true,
                    generate_collision: args.generate_collision !== false,
                });
            }
            const apiKey = normalizeOptionalString(process.env.MESHY_API_KEY);
            if (!apiKey) {
                return {
                    ok: false,
                    error: "generate_and_import_3d_model prompt mode requires MESHY_API_KEY in the MCP server environment.",
                };
            }
            const aiModelResult = normalizeOptionalEnum(args.ai_model, "ai_model", MESHY_ALLOWED_MODELS);
            if (!aiModelResult.ok) {
                return { ok: false, error: aiModelResult.error };
            }
            const modelTypeResult = normalizeOptionalEnum(args.model_type, "model_type", MESHY_ALLOWED_MODEL_TYPES);
            if (!modelTypeResult.ok) {
                return { ok: false, error: modelTypeResult.error };
            }
            const topologyResult = normalizeOptionalEnum(args.topology, "topology", MESHY_ALLOWED_TOPOLOGIES);
            if (!topologyResult.ok) {
                return { ok: false, error: topologyResult.error };
            }
            const originAtResult = normalizeOptionalEnum(args.origin_at, "origin_at", MESHY_ALLOWED_ORIGIN_AT);
            if (!originAtResult.ok) {
                return { ok: false, error: originAtResult.error };
            }
            const poseModeResult = normalizeOptionalEnum(args.pose_mode, "pose_mode", MESHY_ALLOWED_POSE_MODE);
            if (!poseModeResult.ok) {
                return { ok: false, error: poseModeResult.error };
            }
            const targetPolycountResult = normalizeTargetPolycount(args.target_polycount);
            if (!targetPolycountResult.ok) {
                return { ok: false, error: targetPolycountResult.error };
            }
            let resolvedTextureImage;
            if (textureImage) {
                const textureImageResult = await resolveImageSource(textureImage, "texture_image");
                if (!textureImageResult.ok) {
                    return { ok: false, error: textureImageResult.error };
                }
                resolvedTextureImage = textureImageResult.value;
            }
            const shouldTexture = normalizeBooleanArg(args.should_texture, true) || Boolean(texturePrompt) || args.enable_pbr === true || Boolean(resolvedTextureImage);
            const shouldRemesh = normalizeBooleanArg(args.should_remesh, false) || topologyResult.value !== undefined || targetPolycountResult.value !== undefined;
            const aiModel = aiModelResult.value || "latest";
            const assetName = normalizeOptionalString(args.asset_name)
                || (prompt ? deriveAssetNameFromPrompt(prompt) : referenceImages.length > 1 ? "multi_image_model" : "image_model");
            if (originAtResult.value && args.auto_size !== true) {
                return { ok: false, error: "origin_at requires auto_size=true." };
            }
            let finalTask = {};
            let sourceMode = "meshy_text_to_3d";
            let refineTaskId = null;
            let previewTaskId = null;
            if (prompt) {
                const previewRequest = {
                    mode: "preview",
                    prompt,
                    ai_model: aiModel,
                    should_remesh: shouldRemesh,
                    target_formats: ["glb"],
                };
                if (modelTypeResult.value) {
                    previewRequest.model_type = modelTypeResult.value;
                }
                if (topologyResult.value && shouldRemesh) {
                    previewRequest.topology = topologyResult.value;
                }
                if (targetPolycountResult.value && shouldRemesh) {
                    previewRequest.target_polycount = targetPolycountResult.value;
                }
                const previewCreate = await meshyJsonRequest(apiKey, "POST", MESHY_TEXT_TO_3D_ENDPOINT, previewRequest);
                if (!previewCreate.ok) {
                    return previewCreate;
                }
                previewTaskId = String(previewCreate.result?.result || "");
                if (!previewTaskId) {
                    return { ok: false, error: "Meshy preview response did not include a task id." };
                }
                const previewTaskResponse = await waitForMeshyTask(apiKey, previewTaskId);
                if (!previewTaskResponse.ok) {
                    return { ok: false, error: `Meshy preview stage failed: ${previewTaskResponse.error}` };
                }
                finalTask = toSafeRecord(previewTaskResponse.result);
                if (shouldTexture) {
                    const refineRequest = {
                        mode: "refine",
                        preview_task_id: previewTaskId,
                        ai_model: aiModel,
                        target_formats: ["glb"],
                        enable_pbr: args.enable_pbr === true,
                    };
                    if (texturePrompt) {
                        refineRequest.texture_prompt = texturePrompt;
                    }
                    else if (resolvedTextureImage) {
                        refineRequest.texture_image_url = resolvedTextureImage;
                    }
                    const refineCreate = await meshyJsonRequest(apiKey, "POST", MESHY_TEXT_TO_3D_ENDPOINT, refineRequest);
                    if (!refineCreate.ok) {
                        return refineCreate;
                    }
                    refineTaskId = String(refineCreate.result?.result || "");
                    if (!refineTaskId) {
                        return { ok: false, error: "Meshy refine response did not include a task id." };
                    }
                    const refineTaskResponse = await waitForMeshyTask(apiKey, refineTaskId);
                    if (!refineTaskResponse.ok) {
                        return { ok: false, error: `Meshy refine stage failed: ${refineTaskResponse.error}` };
                    }
                    finalTask = toSafeRecord(refineTaskResponse.result);
                }
            }
            else {
                const resolvedReferenceImages = await resolveImageSources(referenceImages, "reference_images", 4);
                if (!resolvedReferenceImages.ok) {
                    return { ok: false, error: resolvedReferenceImages.error };
                }
                const endpoint = resolvedReferenceImages.value.length > 1
                    ? MESHY_MULTI_IMAGE_TO_3D_ENDPOINT
                    : MESHY_IMAGE_TO_3D_ENDPOINT;
                if (resolvedReferenceImages.value.length > 1 && modelTypeResult.value) {
                    return { ok: false, error: "model_type is only supported for single-image Meshy generation." };
                }
                const imageRequest = {
                    ai_model: aiModel,
                    should_remesh: shouldRemesh,
                    should_texture: shouldTexture,
                    enable_pbr: args.enable_pbr === true,
                    moderation: args.moderation === true,
                    image_enhancement: normalizeBooleanArg(args.image_enhancement, true),
                    remove_lighting: normalizeBooleanArg(args.remove_lighting, true),
                    auto_size: args.auto_size === true,
                    target_formats: ["glb"],
                };
                if (resolvedReferenceImages.value.length === 1) {
                    imageRequest.image_url = resolvedReferenceImages.value[0];
                    if (modelTypeResult.value) {
                        imageRequest.model_type = modelTypeResult.value;
                    }
                }
                else {
                    imageRequest.image_urls = resolvedReferenceImages.value;
                }
                if (topologyResult.value && shouldRemesh) {
                    imageRequest.topology = topologyResult.value;
                }
                if (targetPolycountResult.value && shouldRemesh) {
                    imageRequest.target_polycount = targetPolycountResult.value;
                }
                if (poseModeResult.value && poseModeResult.value !== "") {
                    imageRequest.pose_mode = poseModeResult.value;
                }
                if (originAtResult.value && args.auto_size === true) {
                    imageRequest.origin_at = originAtResult.value;
                }
                if (texturePrompt) {
                    imageRequest.texture_prompt = texturePrompt;
                }
                else if (resolvedTextureImage) {
                    imageRequest.texture_image_url = resolvedTextureImage;
                }
                const createTaskResponse = await meshyJsonRequest(apiKey, "POST", endpoint, imageRequest);
                if (!createTaskResponse.ok) {
                    return createTaskResponse;
                }
                const taskId = String(createTaskResponse.result?.result || "");
                if (!taskId) {
                    return { ok: false, error: "Meshy image-to-3D response did not include a task id." };
                }
                const taskResponse = await waitForMeshyTaskAt(apiKey, endpoint, taskId);
                if (!taskResponse.ok) {
                    return { ok: false, error: `Meshy image-to-3D failed: ${taskResponse.error}` };
                }
                finalTask = toSafeRecord(taskResponse.result);
                sourceMode = resolvedReferenceImages.value.length > 1 ? "meshy_multi_image_to_3d" : "meshy_image_to_3d";
            }
            const modelUrls = toSafeRecord(finalTask.model_urls);
            const modelUrl = typeof modelUrls.glb === "string" ? modelUrls.glb : undefined;
            if (!modelUrl) {
                return { ok: false, error: "Meshy task succeeded but did not return a GLB download URL." };
            }
            const importResponse = await executeTool("generate_and_import_3d_model", {
                url: modelUrl,
                destination_path: args.destination_path || undefined,
                asset_name: assetName,
                place_in_scene: args.place_in_scene === true,
                place_x: Number(args.place_x) || 0,
                place_y: Number(args.place_y) || 0,
                place_z: Number(args.place_z) || 0,
                enable_nanite: args.enable_nanite === true,
                generate_collision: args.generate_collision !== false,
            });
            if (!importResponse.ok) {
                return importResponse;
            }
            const imported = typeof importResponse.result === "string"
                ? safeJsonParse(importResponse.result) ?? { raw: importResponse.result }
                : toSafeRecord(importResponse.result);
            return {
                ok: true,
                result: {
                    ...toSafeRecord(imported),
                    source_mode: sourceMode,
                    provider: "meshy",
                    prompt,
                    preview_task_id: previewTaskId,
                    refine_task_id: refineTaskId,
                    reference_image_count: referenceImages.length,
                    generated_model_url: modelUrl,
                    thumbnail_url: finalTask.thumbnail_url,
                    texture_urls: finalTask.texture_urls,
                },
            };
        },
        generate_texture: async (args) => {
            const url = normalizeOptionalString(args.url);
            const promptResult = normalizeBoundedPromptWithLimit(args.prompt, "prompt", OPENAI_IMAGE_MAX_PROMPT_LENGTH);
            if (!promptResult.ok) {
                return { ok: false, error: promptResult.error };
            }
            const referenceImagesResult = normalizeStringArray(args.reference_images, "reference_images", 16);
            if (!referenceImagesResult.ok) {
                return { ok: false, error: referenceImagesResult.error };
            }
            const prompt = promptResult.value;
            const referenceImages = referenceImagesResult.value;
            const maskImage = normalizeOptionalString(args.mask_image);
            if (url && (prompt || referenceImages.length > 0 || maskImage)) {
                return { ok: false, error: "generate_texture direct-import mode only supports url plus import settings." };
            }
            if (!url && !prompt) {
                return { ok: false, error: "generate_texture requires a prompt for generation/editing, or a url for direct import." };
            }
            if (maskImage && referenceImages.length === 0) {
                return { ok: false, error: "mask_image requires at least one reference image." };
            }
            if (url) {
                return executeTool("generate_texture", {
                    url,
                    destination_path: args.destination_path || undefined,
                    asset_name: args.asset_name || undefined,
                    compression: args.compression || undefined,
                });
            }
            const apiKey = normalizeOptionalString(process.env.OPENAI_API_KEY);
            if (!apiKey) {
                return {
                    ok: false,
                    error: "generate_texture prompt mode requires OPENAI_API_KEY in the MCP server environment.",
                };
            }
            const modelResult = normalizeOptionalEnum(args.model, "model", OPENAI_ALLOWED_IMAGE_MODELS);
            if (!modelResult.ok) {
                return { ok: false, error: modelResult.error };
            }
            const sizeResult = normalizeOptionalEnum(args.size, "size", OPENAI_ALLOWED_IMAGE_SIZES);
            if (!sizeResult.ok) {
                return { ok: false, error: sizeResult.error };
            }
            const qualityResult = normalizeOptionalEnum(args.quality, "quality", OPENAI_ALLOWED_IMAGE_QUALITIES);
            if (!qualityResult.ok) {
                return { ok: false, error: qualityResult.error };
            }
            const backgroundResult = normalizeOptionalEnum(args.background, "background", OPENAI_ALLOWED_IMAGE_BACKGROUNDS);
            if (!backgroundResult.ok) {
                return { ok: false, error: backgroundResult.error };
            }
            const formatResult = normalizeOptionalEnum(args.output_format, "output_format", OPENAI_ALLOWED_IMAGE_FORMATS);
            if (!formatResult.ok) {
                return { ok: false, error: formatResult.error };
            }
            const moderationResult = normalizeOptionalEnum(args.moderation, "moderation", OPENAI_ALLOWED_MODERATION);
            if (!moderationResult.ok) {
                return { ok: false, error: moderationResult.error };
            }
            const inputFidelityResult = normalizeOptionalEnum(args.input_fidelity, "input_fidelity", OPENAI_ALLOWED_INPUT_FIDELITY);
            if (!inputFidelityResult.ok) {
                return { ok: false, error: inputFidelityResult.error };
            }
            const rawCompression = args.output_compression;
            const outputCompression = rawCompression == null || rawCompression === ""
                ? undefined
                : Math.min(100, Math.max(0, Math.round(Number(rawCompression))));
            if (rawCompression != null && !Number.isFinite(Number(rawCompression))) {
                return { ok: false, error: "output_compression must be a number between 0 and 100." };
            }
            const outputFormat = formatResult.value || "png";
            if (backgroundResult.value === "transparent" && outputFormat === "jpeg") {
                return { ok: false, error: "Transparent background requires output_format png or webp." };
            }
            const requestedModel = modelResult.value || "gpt-image-1.5";
            let modelUsed = requestedModel;
            let fallbackWarning;
            let response;
            let sourceMode = "openai_image_generation";
            const runOpenAIImageRequest = async (model) => {
                if (referenceImages.length > 0) {
                    const resolvedReferenceImages = await resolveImageSources(referenceImages, "reference_images", 16);
                    if (!resolvedReferenceImages.ok) {
                        return { ok: false, error: resolvedReferenceImages.error };
                    }
                    let resolvedMaskImage;
                    if (maskImage) {
                        const maskResult = await resolveImageSource(maskImage, "mask_image");
                        if (!maskResult.ok) {
                            return { ok: false, error: maskResult.error };
                        }
                        resolvedMaskImage = maskResult.value;
                    }
                    const requestBody = {
                        images: resolvedReferenceImages.value.map((image_url) => ({ image_url })),
                        prompt,
                        model,
                        n: 1,
                        size: sizeResult.value || "1024x1024",
                        quality: qualityResult.value || "auto",
                        background: backgroundResult.value || "auto",
                        output_format: outputFormat,
                        moderation: moderationResult.value || "auto",
                        input_fidelity: inputFidelityResult.value || "low",
                    };
                    if (outputCompression !== undefined && outputFormat !== "png") {
                        requestBody.output_compression = outputCompression;
                    }
                    if (resolvedMaskImage) {
                        requestBody.mask = { image_url: resolvedMaskImage };
                    }
                    sourceMode = resolvedMaskImage ? "openai_image_inpaint" : "openai_image_edit";
                    return openAIImageEditRequest(apiKey, requestBody);
                }
                const requestBody = {
                    prompt,
                    model,
                    n: 1,
                    size: sizeResult.value || "1024x1024",
                    quality: qualityResult.value || "auto",
                    background: backgroundResult.value || "auto",
                    output_format: outputFormat,
                    moderation: moderationResult.value || "auto",
                };
                if (outputCompression !== undefined && outputFormat !== "png") {
                    requestBody.output_compression = outputCompression;
                }
                sourceMode = "openai_image_generation";
                return openAIImageRequest(apiKey, requestBody);
            };
            response = await runOpenAIImageRequest(modelUsed);
            if (!response.ok &&
                modelUsed === "gpt-image-1.5" &&
                response.openai_model_access_denied === true) {
                modelUsed = "gpt-image-1-mini";
                const fallbackResponse = await runOpenAIImageRequest(modelUsed);
                if (fallbackResponse.ok) {
                    response = fallbackResponse;
                    fallbackWarning =
                        "Requested model gpt-image-1.5 is not available to this OpenAI project; generated with gpt-image-1-mini instead.";
                }
                else {
                    response = {
                        ...fallbackResponse,
                        error: [
                            response.error,
                            `Automatic fallback to '${modelUsed}' also failed.`,
                            fallbackResponse.error,
                        ].filter(Boolean).join(" "),
                    };
                }
            }
            if (!response.ok) {
                return response;
            }
            const payload = toSafeRecord(response.result);
            const firstImage = Array.isArray(payload.data) ? payload.data[0] : undefined;
            const imagePayload = (firstImage && typeof firstImage === "object") ? toSafeRecord(firstImage) : {};
            let imageBase64 = typeof imagePayload.b64_json === "string" ? imagePayload.b64_json : undefined;
            // Fallback: some OpenAI image endpoints return `url` instead of base64
            // (depends on account tier / model default). Fetch the URL and convert.
            if (!imageBase64) {
                const imageUrl = typeof imagePayload.url === "string" ? imagePayload.url : undefined;
                if (imageUrl) {
                    try {
                        const urlResp = await fetch(imageUrl, { signal: AbortSignal.timeout(OPENAI_IMAGE_REQUEST_TIMEOUT_MS) });
                        if (urlResp.ok) {
                            const buf = Buffer.from(await urlResp.arrayBuffer());
                            imageBase64 = buf.toString("base64");
                        }
                    }
                    catch {
                        // fall through to error
                    }
                }
            }
            if (!imageBase64) {
                return { ok: false, error: "OpenAI image generation succeeded but response contained neither b64_json nor a reachable url." };
            }
            const assetName = normalizeOptionalString(args.asset_name) || deriveAssetNameFromPrompt(prompt || "generated_texture");
            const extension = outputFormat === "jpeg" ? "jpg" : outputFormat;
            const tempFilePath = await writeTempBinaryFile(assetName, extension, Buffer.from(imageBase64, "base64"));
            try {
                const importResponse = await executeTool("import_texture", {
                    file_path: tempFilePath,
                    destination: args.destination_path || "/Game/Generated/Textures",
                    compression: args.compression || "Default",
                });
                if (!importResponse.ok) {
                    return importResponse;
                }
                const imported = typeof importResponse.result === "string"
                    ? safeJsonParse(importResponse.result) ?? { raw: importResponse.result }
                    : toSafeRecord(importResponse.result);
                return {
                    ok: true,
                    result: {
                        ...toSafeRecord(imported),
                        source_mode: sourceMode,
                        provider: "openai",
                        prompt,
                        model_requested: requestedModel,
                        model_used: modelUsed,
                        reference_image_count: referenceImages.length,
                        revised_prompt: imagePayload.revised_prompt,
                        output_format: outputFormat,
                        ...(fallbackWarning ? { warning: fallbackWarning, fallback_model: modelUsed } : {}),
                    },
                };
            }
            finally {
                await fs.unlink(tempFilePath).catch(() => undefined);
            }
        },
        generate_audio: async (args) => {
            const promptResult = normalizeBoundedPromptWithLimit(args.prompt, "prompt", 2_000);
            if (!promptResult.ok || !promptResult.value) {
                return { ok: false, error: promptResult.ok ? "generate_audio requires a prompt." : promptResult.error };
            }
            const apiKey = normalizeOptionalString(process.env.ELEVENLABS_API_KEY);
            if (!apiKey) {
                return {
                    ok: false,
                    error: "generate_audio requires ELEVENLABS_API_KEY in the MCP server environment.",
                };
            }
            const rawDuration = args.duration_seconds;
            let durationSeconds;
            if (rawDuration != null && rawDuration !== "") {
                const parsed = Number(rawDuration);
                if (!Number.isFinite(parsed)) {
                    return { ok: false, error: "duration_seconds must be a number." };
                }
                durationSeconds = clampNumber(parsed, ELEVENLABS_MIN_DURATION_SECONDS, ELEVENLABS_MAX_DURATION_SECONDS);
            }
            const rawPromptInfluence = args.prompt_influence;
            const promptInfluence = rawPromptInfluence == null || rawPromptInfluence === ""
                ? 0.3
                : Number(rawPromptInfluence);
            if (!Number.isFinite(promptInfluence)) {
                return { ok: false, error: "prompt_influence must be a number between 0 and 1." };
            }
            const requestBody = {
                text: promptResult.value,
                loop: normalizeBooleanArg(args.loop, false),
                prompt_influence: clampNumber(promptInfluence, 0, 1),
                model_id: normalizeOptionalString(args.model_id) || "eleven_text_to_sound_v2",
            };
            if (durationSeconds !== undefined) {
                requestBody.duration_seconds = durationSeconds;
            }
            const generationResponse = await elevenLabsSoundRequest(apiKey, requestBody);
            if (!generationResponse.ok) {
                return generationResponse;
            }
            const audioBytes = generationResponse.result;
            if (!(audioBytes instanceof Uint8Array) && !Buffer.isBuffer(audioBytes)) {
                return { ok: false, error: "ElevenLabs did not return audio bytes." };
            }
            const assetName = normalizeOptionalString(args.asset_name) || deriveAssetNameFromPrompt(promptResult.value);
            const tempFilePath = await writeTempBinaryFile(assetName, "mp3", audioBytes);
            try {
                const importResponse = await executeTool("import_audio", {
                    file_path: tempFilePath,
                    destination: args.destination_path || "/Game/Generated/Audio",
                });
                if (!importResponse.ok) {
                    return importResponse;
                }
                const soundPath = extractImportedAssetPath(importResponse.result);
                const actorLabel = normalizeOptionalString(args.actor_label);
                let playback;
                if (soundPath && actorLabel) {
                    const spawnResponse = await executeTool("spawn_audio_component", {
                        sound_path: soundPath,
                        actor_label: actorLabel,
                        auto_play: normalizeBooleanArg(args.auto_play, false),
                        loop: normalizeBooleanArg(args.loop, false),
                    });
                    if (!spawnResponse.ok) {
                        return spawnResponse;
                    }
                    playback = {
                        mode: "attached_component",
                        actor_label: actorLabel,
                        auto_play: normalizeBooleanArg(args.auto_play, false),
                        loop: normalizeBooleanArg(args.loop, false),
                        result: spawnResponse.result,
                    };
                }
                else if (soundPath && args.play_2d === true) {
                    const playResponse = await executeTool("play_sound_2d", {
                        sound_path: soundPath,
                    });
                    if (!playResponse.ok) {
                        return playResponse;
                    }
                    playback = {
                        mode: "play_2d",
                        result: playResponse.result,
                    };
                }
                return {
                    ok: true,
                    result: {
                        import_result: typeof importResponse.result === "string"
                            ? importResponse.result
                            : toSafeRecord(importResponse.result),
                        source_mode: "elevenlabs_sound_generation",
                        provider: "elevenlabs",
                        prompt: promptResult.value,
                        duration_seconds: durationSeconds,
                        sound_path: soundPath,
                        playback,
                    },
                };
            }
            finally {
                await fs.unlink(tempFilePath).catch(() => undefined);
            }
        },
        generate_blockout: async (args) => {
            const description = normalizeOptionalString(args.description) || "";
            const descriptionLower = description.toLowerCase();
            const explicitTemplate = normalizeOptionalString(args.template)?.toLowerCase();
            const inferTemplate = () => {
                if (explicitTemplate === "room" || explicitTemplate === "hallway" || explicitTemplate === "arena" || explicitTemplate === "tower" || explicitTemplate === "courtyard" || explicitTemplate === "bridge") {
                    return explicitTemplate;
                }
                if (descriptionLower.includes("bridge") || descriptionLower.includes("catwalk")) {
                    return "bridge";
                }
                if (descriptionLower.includes("courtyard")) {
                    return "courtyard";
                }
                if (descriptionLower.includes("hall") || descriptionLower.includes("corridor")) {
                    return "hallway";
                }
                if (descriptionLower.includes("arena")) {
                    return "arena";
                }
                if (descriptionLower.includes("tower") || descriptionLower.includes("vertical")) {
                    return "tower";
                }
                return "room";
            };
            const template = inferTemplate();
            const defaultsByTemplate = {
                room: { width: 2400, depth: 2400, height: 420, levels: 1 },
                hallway: { width: 1200, depth: 6000, height: 360, levels: 1 },
                arena: { width: 6000, depth: 6000, height: 520, levels: 1 },
                tower: { width: 2200, depth: 2200, height: 360, levels: 3 },
                courtyard: { width: 7000, depth: 7000, height: 420, levels: 1 },
                bridge: { width: 1600, depth: 8000, height: 180, levels: 1 },
            };
            const parseDescriptionHints = () => {
                const hints = {};
                const footprintMatch = descriptionLower.match(/(\d+(?:\.\d+)?)\s*(?:x|by)\s*(\d+(?:\.\d+)?)(?:\s*(m|cm))?/i);
                if (footprintMatch) {
                    const scale = footprintMatch[3] === "m" ? 100 : 1;
                    hints.width = Number(footprintMatch[1]) * scale;
                    hints.depth = Number(footprintMatch[2]) * scale;
                }
                const heightMatch = descriptionLower.match(/(?:height|wall(?:s)?|ceiling)\s*(?:of\s*)?(\d+(?:\.\d+)?)\s*(m|cm)/i)
                    || descriptionLower.match(/(\d+(?:\.\d+)?)\s*(m|cm)\s*(?:tall|high)/i);
                if (heightMatch) {
                    const scale = heightMatch[2] === "m" ? 100 : 1;
                    hints.height = Number(heightMatch[1]) * scale;
                }
                const levelsMatch = descriptionLower.match(/(\d+)\s*(?:levels|stories|storeys|floors)/i);
                if (levelsMatch) {
                    hints.levels = Number(levelsMatch[1]);
                }
                if (descriptionLower.includes("stairs") || descriptionLower.includes("staircase")) {
                    hints.addStairs = true;
                }
                if (descriptionLower.includes("ramp") || descriptionLower.includes("incline")) {
                    hints.addRamp = true;
                }
                if (descriptionLower.includes("arch") || descriptionLower.includes("archway") || descriptionLower.includes("gateway")) {
                    hints.addArches = true;
                }
                if (descriptionLower.includes("column") || descriptionLower.includes("pillar")) {
                    hints.addColumns = true;
                }
                if (descriptionLower.includes("curved stair") || descriptionLower.includes("spiral") || descriptionLower.includes("winding stair")) {
                    hints.curvedStairs = true;
                    hints.addStairs = true;
                }
                const sideMatchers = [
                    ["north", /north/],
                    ["south", /south/],
                    ["east", /east/],
                    ["west", /west/],
                ];
                const hintedOpenSides = [];
                for (const [side, matcher] of sideMatchers) {
                    if (new RegExp(`(?:open|entrance|entry|gate|door|arch).{0,12}${side}`).test(descriptionLower)
                        || new RegExp(`${side}.{0,12}(?:open|entrance|entry|gate|door|arch)`).test(descriptionLower)
                        || (matcher.test(descriptionLower) && (descriptionLower.includes("opening") || descriptionLower.includes("entry")))) {
                        hintedOpenSides.push(side);
                    }
                }
                if (hintedOpenSides.length > 0) {
                    hints.openSides = hintedOpenSides;
                }
                return hints;
            };
            const normalizeOpenSides = (value) => {
                const rawSidesResult = normalizeStringArray(value, "open_sides", 4);
                if (!rawSidesResult.ok) {
                    return rawSidesResult;
                }
                const allowedSides = new Set(["north", "south", "east", "west"]);
                const normalized = new Set();
                for (const rawSide of rawSidesResult.value) {
                    for (const token of rawSide.split(/[\s,]+/).map((entry) => entry.trim().toLowerCase()).filter(Boolean)) {
                        const canonical = token === "n" ? "north"
                            : token === "s" ? "south"
                                : token === "e" ? "east"
                                    : token === "w" ? "west"
                                        : token;
                        if (!allowedSides.has(canonical)) {
                            return { ok: false, error: `open_sides only supports north, south, east, and west. Received '${token}'.` };
                        }
                        normalized.add(canonical);
                    }
                }
                return { ok: true, value: Array.from(normalized) };
            };
            const hints = parseDescriptionHints();
            const openSidesResult = normalizeOpenSides(args.open_sides);
            if (!openSidesResult.ok) {
                return { ok: false, error: openSidesResult.error };
            }
            const defaults = defaultsByTemplate[template];
            const width = Math.max(200, Number(args.width) || hints.width || defaults.width);
            const depth = Math.max(200, Number(args.depth) || hints.depth || defaults.depth);
            const height = Math.max(120, Number(args.height) || hints.height || defaults.height);
            const wallThickness = Math.max(10, Number(args.wall_thickness) || 40);
            const floorThickness = Math.max(5, Number(args.floor_thickness) || 20);
            const levels = Math.max(1, Math.round(Number(args.levels) || hints.levels || defaults.levels));
            const stairStyle = normalizeOptionalString(args.stair_style)?.toLowerCase() === "curved" || hints.curvedStairs === true ? "curved" : "linear";
            const addStairs = normalizeBooleanArg(args.add_stairs, template === "tower" || hints.addStairs === true);
            const addRamp = normalizeBooleanArg(args.add_ramp, template === "bridge" || hints.addRamp === true);
            const addArches = normalizeBooleanArg(args.add_arches, hints.addArches === true);
            const addColumns = normalizeBooleanArg(args.add_columns, template === "courtyard" || hints.addColumns === true);
            const createCeiling = typeof args.create_ceiling === "boolean" ? args.create_ceiling : !(template === "arena" || template === "courtyard" || template === "bridge");
            const defaultOpenSides = template === "hallway" || template === "bridge" ? ["north", "south"] : [];
            const openSides = Array.from(new Set([...defaultOpenSides, ...(hints.openSides || []), ...openSidesResult.value]));
            const totalWallHeight = Math.max(height, height * levels);
            const effectiveWallHeight = template === "bridge" ? Math.max(100, Math.min(180, height)) : totalWallHeight;
            const openingWidth = clampNumber(Number(args.opening_width) || Math.round(Math.min(width, depth) * (template === "bridge" ? 0.45 : 0.3)), 150, Math.max(160, Math.min(width, depth) - 120));
            const openingHeight = clampNumber(Number(args.opening_height) || Math.round(effectiveWallHeight * (addArches ? 0.75 : 0.65)), 120, Math.max(140, effectiveWallHeight - 20));
            const originX = Number(args.origin_x) || 0;
            const originY = Number(args.origin_y) || 0;
            const originZ = Number(args.origin_z) || 0;
            const actorPrefix = normalizeOptionalString(args.actor_prefix) || `${template}_blockout_${Date.now().toString().slice(-6)}`;
            const wallBaseZ = originZ + floorThickness;
            const runTool = async (toolName, params) => {
                const response = await executeTool(toolName, params);
                if (!response.ok) {
                    return {
                        ok: false,
                        error: `${toolName} failed while generating blockout '${actorPrefix}': ${response.error ?? "unknown error"}`,
                    };
                }
                return response;
            };
            const createdActors = [];
            const createBoxPiece = async (suffix, pieceX, pieceY, pieceZ, pieceWidth, pieceDepth, pieceHeight) => {
                const actorName = `${actorPrefix}_${suffix}`;
                const createActor = await runTool("create_dynamic_mesh_actor", {
                    actor_name: actorName,
                    loc_x: pieceX,
                    loc_y: pieceY,
                    loc_z: pieceZ,
                });
                if (!createActor.ok) {
                    return createActor;
                }
                const createMesh = await runTool("generate_box_mesh", {
                    actor_name: actorName,
                    width: pieceWidth,
                    depth: pieceDepth,
                    height: pieceHeight,
                    reset_mesh: true,
                    origin: "base",
                });
                if (!createMesh.ok) {
                    return createMesh;
                }
                createdActors.push(actorName);
                return { ok: true, result: actorName };
            };
            const createCylinderPiece = async (suffix, pieceX, pieceY, pieceZ, radius, pieceHeight) => {
                const actorName = `${actorPrefix}_${suffix}`;
                const createActor = await runTool("create_dynamic_mesh_actor", {
                    actor_name: actorName,
                    loc_x: pieceX,
                    loc_y: pieceY,
                    loc_z: pieceZ,
                });
                if (!createActor.ok) {
                    return createActor;
                }
                const createMesh = await runTool("generate_cylinder_mesh", {
                    actor_name: actorName,
                    radius,
                    height: pieceHeight,
                    radial_steps: 20,
                    height_steps: 1,
                    reset_mesh: true,
                    origin: "base",
                });
                if (!createMesh.ok) {
                    return createMesh;
                }
                createdActors.push(actorName);
                return { ok: true, result: actorName };
            };
            const createRampPiece = async (suffix, pieceX, pieceY, pieceZ, rampWidth, rampRun, rampHeight, rollDegrees) => {
                const actorName = `${actorPrefix}_${suffix}`;
                const createActor = await runTool("create_dynamic_mesh_actor", {
                    actor_name: actorName,
                    loc_x: pieceX,
                    loc_y: pieceY,
                    loc_z: pieceZ,
                });
                if (!createActor.ok) {
                    return createActor;
                }
                const createMesh = await runTool("generate_box_mesh", {
                    actor_name: actorName,
                    width: rampWidth,
                    depth: rampRun,
                    height: rampHeight,
                    reset_mesh: true,
                    origin: "base",
                });
                if (!createMesh.ok) {
                    return createMesh;
                }
                const rotateResult = await runTool("rotate_actor", {
                    label: actorName,
                    roll: rollDegrees,
                });
                if (!rotateResult.ok) {
                    return rotateResult;
                }
                return { ok: true, result: actorName };
            };
            const createNorthSouthWall = async (side, centerY) => {
                const suffixBase = side === "north" ? "Wall_North" : "Wall_South";
                const wallWidth = width + wallThickness * 2;
                if (!openSides.includes(side)) {
                    return createBoxPiece(suffixBase, originX, centerY, wallBaseZ, wallWidth, wallThickness, effectiveWallHeight);
                }
                const usableOpeningWidth = Math.min(openingWidth, Math.max(150, wallWidth - 120));
                const segmentWidth = Math.max(50, (wallWidth - usableOpeningWidth) * 0.5);
                if (segmentWidth > 40) {
                    const left = await createBoxPiece(`${suffixBase}_Left`, originX - (usableOpeningWidth + segmentWidth) * 0.5, centerY, wallBaseZ, segmentWidth, wallThickness, effectiveWallHeight);
                    if (!left.ok) {
                        return left;
                    }
                    const right = await createBoxPiece(`${suffixBase}_Right`, originX + (usableOpeningWidth + segmentWidth) * 0.5, centerY, wallBaseZ, segmentWidth, wallThickness, effectiveWallHeight);
                    if (!right.ok) {
                        return right;
                    }
                }
                const lintelHeight = Math.max(30, effectiveWallHeight - Math.min(openingHeight, effectiveWallHeight - 10));
                if (addArches || lintelHeight > 50) {
                    return createBoxPiece(`${suffixBase}_Top`, originX, centerY, wallBaseZ + Math.min(openingHeight, effectiveWallHeight - 10), usableOpeningWidth, wallThickness, lintelHeight);
                }
                return { ok: true, result: suffixBase };
            };
            const createEastWestWall = async (side, centerX) => {
                const suffixBase = side === "east" ? "Wall_East" : "Wall_West";
                if (!openSides.includes(side)) {
                    return createBoxPiece(suffixBase, centerX, originY, wallBaseZ, wallThickness, depth, effectiveWallHeight);
                }
                const usableOpeningWidth = Math.min(openingWidth, Math.max(150, depth - 120));
                const segmentDepth = Math.max(50, (depth - usableOpeningWidth) * 0.5);
                if (segmentDepth > 40) {
                    const northSegment = await createBoxPiece(`${suffixBase}_North`, centerX, originY + (usableOpeningWidth + segmentDepth) * 0.5, wallBaseZ, wallThickness, segmentDepth, effectiveWallHeight);
                    if (!northSegment.ok) {
                        return northSegment;
                    }
                    const southSegment = await createBoxPiece(`${suffixBase}_South`, centerX, originY - (usableOpeningWidth + segmentDepth) * 0.5, wallBaseZ, wallThickness, segmentDepth, effectiveWallHeight);
                    if (!southSegment.ok) {
                        return southSegment;
                    }
                }
                const lintelHeight = Math.max(30, effectiveWallHeight - Math.min(openingHeight, effectiveWallHeight - 10));
                if (addArches || lintelHeight > 50) {
                    return createBoxPiece(`${suffixBase}_Top`, centerX, originY, wallBaseZ + Math.min(openingHeight, effectiveWallHeight - 10), wallThickness, usableOpeningWidth, lintelHeight);
                }
                return { ok: true, result: suffixBase };
            };
            const baseFloor = await createBoxPiece("Floor_00", originX, originY, originZ, width, depth, floorThickness);
            if (!baseFloor.ok) {
                return baseFloor;
            }
            for (let levelIndex = 1; levelIndex < levels; levelIndex += 1) {
                const intermediateFloor = await createBoxPiece(`Floor_${levelIndex.toString().padStart(2, "0")}`, originX, originY, originZ + floorThickness + (levelIndex * height), width, depth, floorThickness);
                if (!intermediateFloor.ok) {
                    return intermediateFloor;
                }
            }
            const northWall = await createNorthSouthWall("north", originY + (depth * 0.5) + (wallThickness * 0.5));
            if (!northWall.ok) {
                return northWall;
            }
            const southWall = await createNorthSouthWall("south", originY - (depth * 0.5) - (wallThickness * 0.5));
            if (!southWall.ok) {
                return southWall;
            }
            const eastWall = await createEastWestWall("east", originX + (width * 0.5) + (wallThickness * 0.5));
            if (!eastWall.ok) {
                return eastWall;
            }
            const westWall = await createEastWestWall("west", originX - (width * 0.5) - (wallThickness * 0.5));
            if (!westWall.ok) {
                return westWall;
            }
            if (createCeiling) {
                const roof = await createBoxPiece("Ceiling", originX, originY, originZ + floorThickness + totalWallHeight, width, depth, floorThickness);
                if (!roof.ok) {
                    return roof;
                }
            }
            if (addColumns) {
                const columnRadius = Math.max(20, Math.round(wallThickness * 0.55));
                const columnInsetX = Math.max(columnRadius + 20, (width * 0.5) - wallThickness - columnRadius - 10);
                const columnInsetY = Math.max(columnRadius + 20, (depth * 0.5) - wallThickness - columnRadius - 10);
                const columnPositions = [
                    ["Column_NE", originX + columnInsetX, originY + columnInsetY],
                    ["Column_NW", originX - columnInsetX, originY + columnInsetY],
                    ["Column_SE", originX + columnInsetX, originY - columnInsetY],
                    ["Column_SW", originX - columnInsetX, originY - columnInsetY],
                ];
                for (const [suffix, columnX, columnY] of columnPositions) {
                    const column = await createCylinderPiece(suffix, columnX, columnY, wallBaseZ, columnRadius, effectiveWallHeight);
                    if (!column.ok) {
                        return column;
                    }
                }
            }
            if (addRamp) {
                const rampSegments = levels > 1 ? levels - 1 : 1;
                const rampWidth = Math.max(220, Math.min(width, depth) - wallThickness * 2 - 120);
                const rampThickness = Math.max(20, floorThickness);
                const rampAngleDegrees = template === "bridge" ? 10 : 15;
                const theoreticalRun = Math.round((height + floorThickness) / Math.tan((rampAngleDegrees * Math.PI) / 180));
                const rampRun = Math.min(Math.max(800, theoreticalRun), Math.max(800, depth - wallThickness * 2 - 60));
                for (let rampIndex = 0; rampIndex < rampSegments; rampIndex += 1) {
                    const ramp = await createRampPiece(`Ramp_${rampIndex.toString().padStart(2, "0")}`, originX + (template === "bridge" ? 0 : width * 0.18), originY - (depth * 0.5) + Math.min(depth * 0.35, rampRun * 0.5) + wallThickness, wallBaseZ + (rampIndex * height), rampWidth, rampRun, rampThickness, -rampAngleDegrees);
                    if (!ramp.ok) {
                        return ramp;
                    }
                }
            }
            if (addStairs) {
                const stairLevels = levels > 1 ? levels - 1 : 1;
                if (stairStyle === "curved") {
                    const stairWidth = Math.max(160, Math.min(width, depth) * 0.3);
                    const innerRadius = Math.max(100, Math.min(width, depth) * 0.16);
                    const stepHeight = 18;
                    const numSteps = clampNumber(Math.round(height / stepHeight), 5, 40);
                    const curveAngle = template === "tower" ? 270 : 180;
                    for (let stairIndex = 0; stairIndex < stairLevels; stairIndex += 1) {
                        const actorName = `${actorPrefix}_CurvedStairs_${stairIndex.toString().padStart(2, "0")}`;
                        const createActor = await runTool("create_dynamic_mesh_actor", {
                            actor_name: actorName,
                            loc_x: originX,
                            loc_y: originY,
                            loc_z: wallBaseZ + (stairIndex * height),
                        });
                        if (!createActor.ok) {
                            return createActor;
                        }
                        const createStairs = await runTool("generate_curved_stairs_mesh", {
                            actor_name: actorName,
                            step_width: stairWidth,
                            step_height: stepHeight,
                            inner_radius: innerRadius,
                            curve_angle: curveAngle,
                            num_steps: numSteps,
                            floating: false,
                            reset_mesh: true,
                        });
                        if (!createStairs.ok) {
                            return createStairs;
                        }
                        createdActors.push(actorName);
                    }
                }
                else {
                    const stairWidth = Math.max(160, Math.min(width, depth) - wallThickness * 2 - 120);
                    const stepHeight = 18;
                    const numSteps = clampNumber(Math.round(height / stepHeight), 4, 40);
                    const stairRun = numSteps * 30;
                    for (let stairIndex = 0; stairIndex < stairLevels; stairIndex += 1) {
                        const actorName = `${actorPrefix}_Stairs_${stairIndex.toString().padStart(2, "0")}`;
                        const createActor = await runTool("create_dynamic_mesh_actor", {
                            actor_name: actorName,
                            loc_x: originX - (width * 0.5) + wallThickness + (stairWidth * 0.5) + 60,
                            loc_y: originY - (depth * 0.5) + wallThickness + (stairRun * 0.5) + 60,
                            loc_z: wallBaseZ + (stairIndex * height),
                        });
                        if (!createActor.ok) {
                            return createActor;
                        }
                        const createStairs = await runTool("generate_linear_stairs_mesh", {
                            actor_name: actorName,
                            step_width: stairWidth,
                            step_height: stepHeight,
                            step_depth: 30,
                            num_steps: numSteps,
                            floating: false,
                            reset_mesh: true,
                        });
                        if (!createStairs.ok) {
                            return createStairs;
                        }
                        createdActors.push(actorName);
                    }
                }
            }
            const uniqueActors = Array.from(new Set(createdActors.filter(Boolean)));
            return {
                ok: true,
                result: {
                    source_mode: "procedural_blockout_composite",
                    template,
                    actor_prefix: actorPrefix,
                    description,
                    width,
                    depth,
                    height,
                    levels,
                    wall_thickness: wallThickness,
                    floor_thickness: floorThickness,
                    open_sides: openSides,
                    add_stairs: addStairs,
                    stair_style: stairStyle,
                    add_ramp: addRamp,
                    add_arches: addArches,
                    add_columns: addColumns,
                    opening_width: openingWidth,
                    opening_height: openingHeight,
                    create_ceiling: createCeiling,
                    actors: uniqueActors,
                },
            };
        },
        // ── Spawn with snap-to-ground ──
        spawn_actor: async (args) => {
            const loc = (args.location || {});
            const x = Number(loc.x) || 0;
            const y = Number(loc.y) || 0;
            let z = Number(loc.z) || 0;
            if (args.snap_to_ground === true) {
                const traceResult = await executeTool("line_trace", {
                    start_x: x, start_y: y, start_z: 50000,
                    end_x: x, end_y: y, end_z: -50000,
                    channel: "WorldStatic",
                });
                if (traceResult.ok && traceResult.result) {
                    const parsed = parseLineTraceZ(String(traceResult.result));
                    if (parsed !== null)
                        z = parsed;
                }
            }
            // Map MCP "actor_class" to C++ "class_name"
            return executeTool("spawn_actor", {
                class_name: args.actor_class,
                x, y, z,
                label: args.label || undefined,
                mesh_path: args.mesh_path || undefined,
                color: args.color || undefined,
                folder: args.folder || undefined,
            });
        },
        create_static_mesh_actor: async (args) => {
            const x = Number(args.x) || 0;
            const y = Number(args.y) || 0;
            let z = Number(args.z) || 0;
            if (args.snap_to_ground === true) {
                const traceResult = await executeTool("line_trace", {
                    start_x: x, start_y: y, start_z: 50000,
                    end_x: x, end_y: y, end_z: -50000,
                    channel: "WorldStatic",
                });
                if (traceResult.ok && traceResult.result) {
                    const parsed = parseLineTraceZ(String(traceResult.result));
                    if (parsed !== null)
                        z = parsed;
                }
            }
            return executeTool("create_static_mesh_actor", {
                mesh: args.mesh,
                label: args.label || "StaticMeshActor",
                folder: args.folder || undefined,
                x, y, z,
            });
        },
        // ── Physics & Traces ──
        line_trace: async (args) => executeTool("line_trace", {
            start_x: Number(args.start_x) || 0,
            start_y: Number(args.start_y) || 0,
            start_z: Number(args.start_z) || 0,
            end_x: Number(args.end_x) || 0,
            end_y: Number(args.end_y) || 0,
            end_z: Number(args.end_z) || 0,
            channel: args.channel || "Visibility",
            complex: args.complex === true,
        }),
        // ── Actor Management ──
        get_actor_info: async (args) => executeTool("get_actor_info", {
            label: args.label,
        }),
        get_actor_transform: async (args) => executeTool("get_actor_transform", {
            actor_name: args.actor_name,
        }),
        move_actor: async (args) => executeTool("move_actor", {
            label: args.label,
            x: Number(args.x) || 0,
            y: Number(args.y) || 0,
            z: Number(args.z) || 0,
        }),
        rotate_actor: async (args) => executeTool("rotate_actor", {
            label: args.label,
            pitch: Number(args.pitch) || 0,
            yaw: Number(args.yaw) || 0,
            roll: Number(args.roll) || 0,
        }),
        scale_actor: async (args) => executeTool("scale_actor", {
            label: args.label,
            scale: args.scale != null ? Number(args.scale) : undefined,
            x: args.x != null ? Number(args.x) : undefined,
            y: args.y != null ? Number(args.y) : undefined,
            z: args.z != null ? Number(args.z) : undefined,
        }),
        delete_actor: async (args) => executeTool("delete_actor", {
            label: args.label,
            dry_run: args.dry_run === true,
        }),
        set_actor_transform: async (args) => executeTool("set_actor_transform", {
            label: args.label,
            loc_x: Number(args.loc_x) || 0,
            loc_y: Number(args.loc_y) || 0,
            loc_z: Number(args.loc_z) || 0,
            rot_pitch: Number(args.rot_pitch) || 0,
            rot_yaw: Number(args.rot_yaw) || 0,
            rot_roll: Number(args.rot_roll) || 0,
            scale_x: args.scale_x != null ? Number(args.scale_x) : 1,
            scale_y: args.scale_y != null ? Number(args.scale_y) : 1,
            scale_z: args.scale_z != null ? Number(args.scale_z) : 1,
        }),
        duplicate_actor: async (args) => executeTool("duplicate_actor", {
            label: args.label,
            new_label: args.new_label || undefined,
            offset_x: args.offset_x != null ? Number(args.offset_x) : 100,
            offset_y: Number(args.offset_y) || 0,
            offset_z: Number(args.offset_z) || 0,
        }),
        find_actor_by_label: async (args) => executeTool("find_actor_by_label", {
            label: args.label,
        }),
        get_selected_actors: async () => executeTool("get_selected_actors"),
        set_actor_material: async (args) => executeTool("set_actor_material", {
            actor_label: args.actor_label,
            material_path: args.material_path,
            slot_index: Number(args.slot_index) || 0,
        }),
        set_actor_color: async (args) => executeTool("set_actor_color", {
            actor_label: args.actor_label,
            color: args.color,
            roughness: args.roughness != null ? Number(args.roughness) : 0.5,
            metallic: args.metallic != null ? Number(args.metallic) : 0.0,
        }),
        set_actor_property: async (args) => executeTool("set_actor_property", {
            label: args.label,
            property: args.property,
            value: String(args.value),
        }),
        get_actors_in_radius: async (args) => executeTool("get_actors_in_radius", {
            x: Number(args.x) || 0,
            y: Number(args.y) || 0,
            z: Number(args.z) || 0,
            radius: Number(args.radius) || 1000,
            class_filter: args.class_filter || undefined,
        }),
        select_actors: async (args) => executeTool("select_actors", {
            labels: args.labels,
        }),
        // ── Viewport & Camera Control ──
        set_viewport_location: async (args) => executeTool("set_viewport_location", {
            x: Number(args.x) || 0,
            y: Number(args.y) || 0,
            z: Number(args.z) || 0,
            pitch: Number(args.pitch) || 0,
            yaw: Number(args.yaw) || 0,
        }),
        focus_actor: async (args) => executeTool("focus_actor", {
            label: args.label,
        }),
        look_at_and_capture: async (args) => executeTool("look_at_and_capture", {
            target_label: args.target_label || undefined,
            target_x: Number(args.target_x) || 0,
            target_y: Number(args.target_y) || 0,
            target_z: Number(args.target_z) || 0,
            distance: args.distance != null ? Number(args.distance) : 1500,
            yaw: args.yaw != null ? Number(args.yaw) : 45,
            pitch: args.pitch != null ? Number(args.pitch) : -25,
            filename: args.filename || undefined,
            analyze: args.analyze === true,
            prompt: args.prompt || undefined,
        }),
        capture_viewport_sync: async (args) => executeTool("capture_viewport_sync", {
            filename: args.filename || undefined,
            analyze: args.analyze === true,
            prompt: args.prompt || undefined,
        }),
        capture_viewport_safe: async (args) => executeTool("capture_viewport_safe", {
            filename: args.filename || undefined,
            width: Number(args.width) || 0,
            height: Number(args.height) || 0,
            analyze: args.analyze === true,
            prompt: args.prompt || undefined,
        }),
        // ── Lighting ──
        create_light: async (args) => executeTool("create_light", {
            type: args.type,
            x: Number(args.x) || 0,
            y: Number(args.y) || 0,
            z: args.z != null ? Number(args.z) : 200,
            intensity: args.intensity != null ? Number(args.intensity) : 5000,
            color: args.color || "255,255,255",
            label: args.label || undefined,
        }),
        // NOTE: set_light_properties, get_light_actors, get_light_info DE-REGISTERED in C++ —
        // use set_component_property / execute_python instead.
        // NOTE: create_sky_atmosphere, create_sky_light, set_atmosphere_properties DE-REGISTERED in C++ —
        // use spawn_actor / set_component_property instead.
        // NOTE: create_exponential_height_fog, set_fog_properties DE-REGISTERED in C++ —
        // use spawn_actor / set_component_property instead.
        // ── Post-Processing ──
        create_post_process_volume: async (args) => executeTool("create_post_process_volume", {
            x: Number(args.x) || 0,
            y: Number(args.y) || 0,
            z: Number(args.z) || 0,
            infinite_extent: args.infinite_extent !== false,
        }),
        // NOTE: set_bloom, set_exposure, set_color_grading DE-REGISTERED in C++ —
        // use set_post_process_settings instead.
        set_post_process_settings: async (args) => executeTool("set_post_process_settings", {
            actor_name: args.actor_name,
            bloom_intensity: args.bloom_intensity != null ? Number(args.bloom_intensity) : undefined,
            exposure_compensation: args.exposure_compensation != null ? Number(args.exposure_compensation) : undefined,
            saturation: args.saturation != null ? Number(args.saturation) : undefined,
            contrast: args.contrast != null ? Number(args.contrast) : undefined,
            white_balance_temp: args.white_balance_temp != null ? Number(args.white_balance_temp) : undefined,
            vignette_intensity: args.vignette_intensity != null ? Number(args.vignette_intensity) : undefined,
            dof_fstop: args.dof_fstop != null ? Number(args.dof_fstop) : undefined,
            dof_focal_distance: args.dof_focal_distance != null ? Number(args.dof_focal_distance) : undefined,
        }),
        // ── Landscape & Terrain ──
        create_landscape: async (args) => executeTool("create_landscape", {
            size_x: Number(args.size_x) || 255,
            size_y: Number(args.size_y) || 255,
            x: Number(args.x) || 0,
            y: Number(args.y) || 0,
            z: Number(args.z) || 0,
            scale: Number(args.scale) || 100,
            label: args.label || "Landscape",
            terrain_style: args.terrain_style || "hills",
        }),
        sculpt_landscape: async (args) => executeTool("sculpt_landscape", {
            landscape_label: args.landscape_label,
            operation: args.operation,
            center_x: Number(args.center_x ?? args.x) || 0,
            center_y: Number(args.center_y ?? args.y) || 0,
            radius: Number(args.radius) || 1000,
            strength: args.strength != null ? Number(args.strength) : 0.5,
            height: Number(args.height) || 0,
            height_delta: args.height_delta != null ? Number(args.height_delta) : undefined,
        }),
        paint_landscape_layer: async (args) => executeTool("paint_landscape_layer", {
            landscape_label: args.landscape_label,
            layer_name: args.layer_name,
            center_x: Number(args.center_x ?? args.x) || 0,
            center_y: Number(args.center_y ?? args.y) || 0,
            radius: Number(args.radius) || 1000,
            strength: args.strength != null ? Number(args.strength) : 1.0,
            falloff_power: args.falloff_power != null ? Number(args.falloff_power) : undefined,
            refresh_grass: args.refresh_grass,
        }),
        draw_landscape_path: async (args) => executeTool("draw_landscape_path", {
            landscape_label: args.landscape_label,
            layer_name: args.layer_name || "",
            path_points: args.path_points,
            width: Number(args.width) || 400,
            strength: args.strength != null ? Number(args.strength) : 1.0,
            falloff_power: args.falloff_power != null ? Number(args.falloff_power) : 2.0,
            height_delta: args.height_delta != null ? Number(args.height_delta) : 0,
            refresh_grass: args.refresh_grass,
            save_level: args.save_level,
        }),
        create_procedural_river: async (args) => executeTool("create_procedural_river", {
            landscape_label: args.landscape_label,
            path_points: args.path_points,
            width: Number(args.width) || 800,
            depth: Number(args.depth) || 150,
            flow_speed: Number(args.flow_speed) || 120,
            layer_name: args.layer_name || "Dirt",
            falloff_power: args.falloff_power != null ? Number(args.falloff_power) : 1.5,
            endpoint_taper_distance: args.endpoint_taper_distance != null ? Number(args.endpoint_taper_distance) : -1,
            surface_z_offset: args.surface_z_offset != null ? Number(args.surface_z_offset) : 10,
            water_material_path: args.water_material_path || "",
            refresh_grass: args.refresh_grass,
            label: args.label || "River",
            save_level: args.save_level,
        }),
        add_landscape_layer: async (args) => executeTool("add_landscape_layer", {
            landscape_label: args.landscape_label,
            layer_name: args.layer_name,
            is_weight_blended: args.is_weight_blended !== false,
        }),
        apply_landscape_material: async (args) => executeTool("apply_landscape_material", {
            landscape_name: args.landscape_name || undefined,
            material_path: args.material_path,
        }),
        // ── Static Meshes & Components ──
        // NOTE: set_static_mesh, add_component DE-REGISTERED in C++ —
        // use set_component_property / execute_python instead.
        set_component_property: async (args) => executeTool("set_component_property", {
            actor_name: args.actor_name,
            component_name: args.component_name,
            property_name: args.property_name,
            value: String(args.value),
        }),
        // ── Level Management ──
        save_level: async () => executeTool("save_level"),
        save_scene_checkpoint: async (args) => executeTool("save_scene_checkpoint", {
            name: args.name || undefined,
            overwrite: args.overwrite === true,
            save_level: args.save_level !== false,
            include_screenshot: args.include_screenshot !== false,
            include_digest: args.include_digest !== false,
            include_performance: args.include_performance !== false,
            width: Number(args.width) || 0,
            height: Number(args.height) || 0,
        }),
        restore_scene_checkpoint: async (args) => executeTool("restore_scene_checkpoint", {
            checkpoint_path: args.checkpoint_path || undefined,
            checkpoint_id: args.checkpoint_id || undefined,
            dry_run: args.dry_run === true,
            reload_level: args.reload_level !== false,
            apply_viewport: args.apply_viewport !== false,
        }),
        save_level_as: async (args) => executeTool("save_level_as", {
            new_path: args.new_path,
        }),
        create_level: async (args) => executeTool("create_level", {
            level_name: args.level_name,
            destination: args.destination || "/Game/Maps",
        }),
        load_level: async (args) => executeTool("load_level", {
            path: args.path,
        }),
        // NOTE: set_default_map DE-REGISTERED in C++ — use set_project_setting instead.
        // ── Play In Editor (PIE) ──
        start_pie: async (args) => executeTool("start_pie", {
            mode: args.mode || "viewport",
            num_players: Number(args.num_players) || 1,
        }),
        stop_pie: async () => executeTool("stop_pie"),
        // ── Blueprint Editing ──
        open_blueprint: async (args) => executeTool("open_blueprint", {
            path: args.path,
        }),
        add_blueprint_component: async (args) => executeTool("add_blueprint_component", {
            component_type: args.component_type,
            component_name: args.component_name,
            parent_component: args.parent_component || undefined,
        }),
        add_blueprint_variable: async (args) => executeTool("add_blueprint_variable", {
            name: args.name,
            type: args.type,
            default_value: args.default_value || undefined,
            is_exposed: args.is_exposed !== false,
        }),
        add_blueprint_event: async (args) => executeTool("add_blueprint_event", {
            blueprint: args.blueprint,
            event_type: args.event_type,
            component: args.component || undefined,
            filter_class: args.filter_class || undefined,
            once: args.once === true,
            idempotency_key: args.idempotency_key,
        }),
        compile_blueprint: async (args) => executeTool("compile_blueprint", {
            blueprint_path: args.blueprint_path || undefined,
        }),
        // ── Python Execution ──
        // CRITICAL: Do NOT use tcpExecute() here — that bypasses all risk tiers,
        // proof bundles, and execution gateway. Route through executeTool() which
        // goes via the governed HTTP bridge on port 8767.
        execute_python: async (args) => {
            const normalized = normalizePythonCode(args.code);
            if (!normalized.ok) {
                return { ok: false, error: normalized.error };
            }
            return executeTool("execute_python", { code: normalized.value });
        },
        // ── Handlers with explicit default args / transforms ──
        generate_project_insights: async (args) => executeTool("generate_project_insights", {
            format: args.format || 'full',
            modules: args.modules || 'all',
        }),
        // Console Commands — routed through governed HTTP bridge to prevent code injection.
        // SECURITY: All console/diagnostic tools route through executeTool() (audit findings S1, S5).
        // NEVER use tcpExecute() with user-supplied string interpolation.
        execute_console_command: async (args) => {
            const normalized = normalizeConsoleCommand(args.command);
            if (!normalized.ok) {
                return { ok: false, error: normalized.error };
            }
            return executeTool("execute_console_command", {
                command: normalized.value,
                mode: args.mode || 'DEBUG',
                hypothesis: args.hypothesis || null,
                expected_outcome: args.expected_outcome || null,
            });
        },
        run_diagnostic: async (args) => {
            const name = args.name;
            const commandSets = {
                profile_performance: ["stat fps", "stat unit", "stat gpu", "stat memory", "stat scenerendering"],
                gas_debug: ["showdebug abilitysystem", "AbilitySystem.Debug.NextCategory"],
                collision_debug: ["show collision", "p.VisualizeWorldQueries 1"],
                ai_debug: ["ai.debug", "showdebug behavortree", "showdebug eqs"],
                network_debug: ["net.ListActorsInPackageMap", "stat net", "net.RepGraph.DebugActor"],
            };
            const commands = commandSets[name];
            if (!commands) {
                return { ok: false, error: `Unknown diagnostic: ${name}. Available: ${Object.keys(commandSets).join(", ")}` };
            }
            const results = [];
            for (const cmd of commands) {
                try {
                    const r = await executeTool("execute_console_command", { command: cmd });
                    results.push(`[${cmd}] ${r.ok ? "OK" : r.error || "failed"}`);
                }
                catch (e) {
                    results.push(`[${cmd}] error: ${e}`);
                }
            }
            return { ok: true, result: `Diagnostic '${name}' executed ${commands.length} commands:\n${results.join("\n")}` };
        },
        list_diagnostics: async (_args) => ({
            ok: true,
            result: JSON.stringify({
                diagnostics: [
                    { name: "profile_performance", description: "Capture FPS, draw calls, memory, GPU/CPU timings" },
                    { name: "gas_debug", description: "Debug Gameplay Ability System: active abilities, effects, attributes" },
                    { name: "collision_debug", description: "Debug collision channels, overlaps, and trace results" },
                    { name: "ai_debug", description: "Debug AI: behavior trees, blackboard, EQS, perception" },
                    { name: "network_debug", description: "Debug networking: replication, RPCs, bandwidth, packet loss" },
                ],
            }),
        }),
        get_verification_status: async (_args) => {
            try {
                const result = await executeTool("get_verification_status", {});
                if (result.ok)
                    return result;
                return { ok: false, error: result.error ?? "get_verification_status failed" };
            }
            catch { /* fall through */ }
            return { ok: false, error: "Verification status unavailable because the bridge is not responding." };
        },
        // ── Vision / Viewport — explicit defaults + type casts ──
        analyze_scene_screenshot: async (args) => executeTool("analyze_scene_screenshot", {
            auto_capture: args.auto_capture !== false,
            prompt: args.prompt || "",
            image_path: args.image_path || "",
        }),
        observe_ue_project: async (args) => executeTool("observe_ue_project", {
            capture_screenshot: args.capture_screenshot !== false,
            include_vision: args.include_vision !== false,
            prompt: args.prompt || "",
            max_actor_sample: Number(args.max_actor_sample) || 40,
            max_class_buckets: Number(args.max_class_buckets) || 12,
        }),
        // ── Pure passthrough handlers removed ──
        // Geometry modeling, level snapshots, crowd, niagara channels, metasound,
        // movie graph, data layers, foundry, chaos mover, variants, remote control,
        // smart objects, pose search, control rig, neural morph, USD, field system,
        // gameplay cameras, chooser, data registry, contextual anim, learning agents,
        // VCam, take recorder, live link, replication, and world rules —
        // all routed by the index.ts generated-tool fallback.
        // ── Volumetric Material Engine ──
        create_vme_material: async (args) => executeTool("create_vme_material", {
            material_name: args.material_name || "M_VME_Terrain",
            destination: args.destination || "/Game/Materials",
            phase: Number(args.phase) || 4,
            apply_to_terrain: args.apply_to_terrain !== false,
            landscape_mode: args.landscape_mode === true,
        }),
        // ── Build Monitoring ──
        // wait_for_build passes through to C++ which creates a latent job.
        // The C++ tool returns {ok, job_id, poll_url} — the caller polls via
        // get_latent_job_status using that job_id.
        wait_for_build: async (args) => executeTool("wait_for_build", {
            timeout_seconds: Number(args.timeout_seconds) || 120,
        }),
        // ── Build Monitoring (passthrough to C++ build-event tracker) ──
        get_build_events: async (args) => executeTool("get_build_events", {
            max_count: Number(args.max_count) || 20,
            clear: args.clear === true,
        }),
        get_build_errors: async (args) => executeTool("get_build_errors", {
            max_count: Number(args.max_count) || 50,
            source: args.source || "all",
        }),
        get_error_summary: async (args) => executeTool("get_error_summary", args),
        get_errors_since: async (args) => executeTool("get_errors_since", {
            since_timestamp: Number(args.since_timestamp) || 0,
            severity: args.severity || "all",
            max_count: Number(args.max_count) || 50,
        }),
        // ── Modal Dismissal ──
        dismiss_modal_dialog: async (args) => executeTool("dismiss_modal_dialog", {
            classification: args.classification || "auto",
            force: args.force === true,
        }),
        // ── Editor Control ──
        set_viewport_mode: async (args) => executeTool("set_viewport_mode", {
            mode: args.mode,
        }),
        navigate_content_browser: async (args) => executeTool("navigate_content_browser", {
            path: args.path || "",
            sync_to_asset: args.sync_to_asset || "",
        }),
        set_editor_pref: async (args) => executeTool("set_editor_pref", {
            section: args.section,
            key: args.key,
            value: args.value,
        }),
        click_editor_button: async (args) => executeTool("click_editor_button", {
            button_text: args.button_text,
        }),
        // ── Crash Diagnosis (bridge-independent — reads disk directly) ──
        diagnose_crash: async (args) => {
            const result = diagnoseCrash(args.project_dir, args.max_age_seconds ? Number(args.max_age_seconds) : undefined);
            return { ok: true, result: JSON.stringify(result) };
        },
        // ── Tool Discovery / Workflow Discovery ──
        // index.ts installs richer overrides that use the readiness-filtered visible catalog.
        // These local implementations keep the tools callable and test-discoverable even
        // before index.ts decorates the handler map.
        find_tools: async (args) => {
            const query = normalizeToolSearchQuery(String(args.query || ""));
            const maxResults = Number(args.max_results) || 20;
            const tools = searchToolsLocally(query, maxResults);
            return { ok: true, result: { matches: tools.length, tools } };
        },
        get_workflow: async (args) => {
            const query = normalizeWorkflowQuery(String(args.query || ""));
            if (query === "list" || query === "") {
                return { ok: true, result: { workflows: listWorkflows() } };
            }
            const workflow = getWorkflow(query);
            if (!workflow) {
                return { ok: false, error: `Unknown workflow '${query}'. Use query='list' to see available workflows.` };
            }
            return { ok: true, result: workflow };
        },
        // ── Batch Execution — reduces agent round-trips ──
        batch_execute: async (args) => {
            const invokeTool = dispatchTool ?? executeTool;
            const steps = args.steps;
            if (!Array.isArray(steps) || steps.length === 0) {
                return { ok: false, error: "batch_execute requires a non-empty 'steps' array." };
            }
            if (steps.length > MAX_BATCH_STEPS) {
                return { ok: false, error: `batch_execute supports at most ${MAX_BATCH_STEPS} steps, got ${steps.length}.` };
            }
            if (steps.some((step) => step?.tool === "batch_execute")) {
                return { ok: false, error: "batch_execute does not allow nested batch_execute steps." };
            }
            const continueOnError = args.continue_on_error === true;
            const results = [];
            let allOk = true;
            for (const step of steps) {
                if (!step.tool || typeof step.tool !== "string" || !step.tool.trim()) {
                    results.push({ tool: "?", ok: false, error: "Each step must have a 'tool' string.", _duration_ms: 0 });
                    allOk = false;
                    if (!continueOnError)
                        break;
                    continue;
                }
                if (step.args !== undefined && (typeof step.args !== "object" || step.args === null || Array.isArray(step.args))) {
                    results.push({ tool: step.tool, ok: false, error: "Each step.args must be an object when provided.", _duration_ms: 0 });
                    allOk = false;
                    if (!continueOnError)
                        break;
                    continue;
                }
                const start = performance.now();
                try {
                    const stepResult = await invokeTool(step.tool.trim(), toSafeRecord(step.args));
                    const duration = Math.round(performance.now() - start);
                    results.push({
                        tool: step.tool.trim(),
                        ok: stepResult.ok,
                        result: stepResult.ok ? stepResult.result : undefined,
                        error: stepResult.ok ? undefined : (stepResult.error ?? "unknown error"),
                        _duration_ms: duration,
                    });
                    if (!stepResult.ok) {
                        allOk = false;
                        if (!continueOnError)
                            break;
                    }
                }
                catch (e) {
                    const duration = Math.round(performance.now() - start);
                    results.push({
                        tool: step.tool,
                        ok: false,
                        error: e instanceof Error ? e.message : String(e),
                        _duration_ms: duration,
                    });
                    allOk = false;
                    if (!continueOnError)
                        break;
                }
            }
            return {
                ok: allOk,
                result: {
                    steps_requested: steps.length,
                    steps_completed: results.length,
                    all_succeeded: allOk,
                    results,
                },
            };
        },
        // ── Call Planner — dependency-aware workflow planning ──
        plan_workflow: async (args) => {
            const goal = normalizeGoalQuery(String(args.goal || ""));
            if (!goal) {
                return { ok: false, error: "plan_workflow requires a 'goal' string." };
            }
            if (goal.toLowerCase() === "list") {
                return { ok: true, result: { goals: listGoals() } };
            }
            // Build plan — uses empty session/context here; index.ts overrides with live state
            const plan = planFromGoal(goal, []);
            if (!plan) {
                return {
                    ok: false,
                    error: `Unrecognized goal '${goal}'. Use goal='list' to see available goals, or pass an exact tool name.`,
                };
            }
            return {
                ok: true,
                result: buildSafePlanWorkflowResult(plan, planToBatchSteps(plan)),
            };
        },
        // ── Vision Loop — Claude's eyes inside Unreal Engine ──
        ...createSceneEnhancementHandlers(executeTool),
        ...createVisionHandlers(executeTool, httpRequest),
        // ── Polyhaven — free CC0 textures, HDRIs, models ──
        ...createPolyhavenHandlers(),
        // ── Teaching Mode — annotated tool execution for learning ──
        explain_tool_execution: async (args) => {
            const toolName = String(args.tool_name || "").trim();
            if (!toolName) {
                return { ok: false, error: "explain_tool_execution requires a 'tool_name' string." };
            }
            // Look up the tool in generated tools
            const { GENERATED_TOOLS } = await import("./generated-tools.js");
            const tool = GENERATED_TOOLS.find((t) => t.name === toolName);
            if (!tool) {
                return { ok: false, error: `Tool '${toolName}' not found in registry.` };
            }
            const schema = tool.inputSchema;
            const params = schema?.properties ?? {};
            const required = schema?.required ?? [];
            // Build structured explanation
            const paramDocs = Object.entries(params).map(([name, def]) => ({
                name,
                type: def.type ?? "string",
                description: def.description ?? "",
                required: required.includes(name),
                default: def.default,
            }));
            const explanation = {
                tool_name: toolName,
                description: tool.description,
                parameter_count: Object.keys(params).length,
                required_parameters: required,
                parameters: paramDocs,
                usage_tips: [
                    required.length > 0
                        ? `Required parameters: ${required.join(", ")}`
                        : "All parameters are optional.",
                    "Parameters are passed as key-value pairs in the tool call.",
                    "Results are returned as JSON with 'ok' and 'result' fields.",
                ],
                category: toolName.includes("_") ? toolName.split("_")[0] : "general",
            };
            return { ok: true, result: explanation };
        },
        // ── Scene Quality Report — combines multiple analysis tools ──
        generate_scene_report: async (args) => {
            const invokeTool = dispatchTool ?? executeTool;
            // Step 1: Observe the scene
            const observe = await invokeTool("observe_ue_project", {
                capture_screenshot: true,
                include_vision: true,
                prompt: "Provide a detailed assessment of this Unreal Engine scene.",
            });
            // Step 2: Score quality
            const score = await invokeTool("score_scene_quality", {
                categories: args.categories || "lighting,composition,realism,contrast,color_grading",
            });
            // Step 3: Performance snapshot
            const perf = await invokeTool("get_performance_snapshot", {});
            return {
                ok: true,
                result: {
                    observation: typeof observe.result === "string" ? safeJsonParse(observe.result) ?? observe.result : observe.result,
                    quality_scores: typeof score.result === "string" ? safeJsonParse(score.result) ?? score.result : score.result,
                    performance: typeof perf.result === "string" ? safeJsonParse(perf.result) ?? perf.result : perf.result,
                    generated_at: new Date().toISOString(),
                },
            };
        },
        // ── Semantic Dependency Graph — blast radius analysis ──
        query_dependencies: async (args) => {
            const packageName = String(args.package_name || "").trim();
            if (!packageName) {
                return { ok: false, error: "query_dependencies requires a 'package_name' string." };
            }
            const maxDepth = Number(args.max_depth) || 3;
            const pyCode = `
import sys, json
sys.path.insert(0, '${process.cwd().replace(/\\/g, "/")}/../Bridge/core')
from semantic_graph import SemanticGraphDB
db = SemanticGraphDB()
result = db.simulate_refactor_blast_radius('${packageName.replace(/'/g, "\\'")}', ${maxDepth})
print(json.dumps(result))
`.trim();
            const result = await executeTool("execute_python", { code: pyCode });
            const text = typeof result.result === "string" ? result.result : JSON.stringify(result.result);
            const parsed = safeJsonParse(text);
            return { ok: true, result: parsed ?? { raw: text } };
        },
        blast_radius: async (args) => {
            const packageName = String(args.package_name || "").trim();
            if (!packageName) {
                return { ok: false, error: "blast_radius requires a 'package_name' string." };
            }
            const maxDepth = Number(args.max_depth) || 5;
            const pyCode = `
import sys, json
sys.path.insert(0, '${process.cwd().replace(/\\/g, "/")}/../Bridge/core')
from semantic_graph import SemanticGraphDB
db = SemanticGraphDB()
result = db.simulate_refactor_blast_radius('${packageName.replace(/'/g, "\\'")}', ${maxDepth})
print(json.dumps(result))
`.trim();
            const result = await executeTool("execute_python", { code: pyCode });
            const text = typeof result.result === "string" ? result.result : JSON.stringify(result.result);
            const parsed = safeJsonParse(text);
            if (parsed && typeof parsed === "object") {
                parsed.warning = parsed.blast_radius_count > 10
                    ? "Large blast radius — review affected packages before proceeding."
                    : undefined;
            }
            return { ok: true, result: parsed ?? { raw: text } };
        },
    };
}
//# sourceMappingURL=tool-handlers.js.map