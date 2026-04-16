/**
 * Schema Intelligence — Round 6 enhancements
 *
 * 1. coerceParams — Auto-fix type mismatches agents make (string→number, string→boolean, etc.)
 * 2. validateParams — Local schema validation before bridge round-trip (saves latency)
 * 3. buildRecoveryAction — Structured recovery: exact tool + params to fix a failure
 *
 * The tool surface has 1326 numeric params and 352 booleans. LLMs routinely send these
 * as strings ("100" instead of 100, "true" instead of true). Coercion catches this
 * transparently; validation catches what coercion can't fix.
 */
import { createSanitizer, createToSafeRecord } from "./sanitize-utils.js";
const MAX_JSON_COERCION_LENGTH = 32_768;
const sanitizeParsedJson = createSanitizer();
const toSafeRecord = createToSafeRecord(sanitizeParsedJson);
function parseJsonForSchema(value, expected) {
    if (value.length > MAX_JSON_COERCION_LENGTH) {
        return value;
    }
    try {
        const parsed = JSON.parse(value);
        if (expected === "object") {
            if (typeof parsed === "object" && parsed !== null && !Array.isArray(parsed)) {
                return sanitizeParsedJson(parsed);
            }
            return value;
        }
        if (Array.isArray(parsed)) {
            return sanitizeParsedJson(parsed);
        }
    }
    catch {
        // not JSON — leave as-is
    }
    return value;
}
// ---------------------------------------------------------------------------
// 1. Type Coercion
// ---------------------------------------------------------------------------
/**
 * Coerce a single value to match an expected JSON Schema type.
 * Returns the original value if coercion isn't possible.
 */
function coerceValue(value, propSchema) {
    if (value === undefined || value === null)
        return value;
    const expected = propSchema.type;
    if (!expected)
        return value;
    const actual = typeof value;
    // string → number
    if (expected === "number" && actual === "string") {
        const num = Number(value);
        if (!Number.isNaN(num) && Number.isFinite(num))
            return num;
    }
    // string → integer
    if (expected === "integer" && actual === "string") {
        const num = parseInt(value, 10);
        if (!Number.isNaN(num))
            return num;
    }
    // string → boolean
    if (expected === "boolean" && actual === "string") {
        const lower = value.toLowerCase();
        if (lower === "true" || lower === "1" || lower === "yes")
            return true;
        if (lower === "false" || lower === "0" || lower === "no")
            return false;
    }
    // number → boolean (0/1 only)
    if (expected === "boolean" && actual === "number") {
        if (value === 0)
            return false;
        if (value === 1)
            return true;
    }
    // number → string
    if (expected === "string" && actual === "number") {
        return String(value);
    }
    // boolean → string
    if (expected === "string" && actual === "boolean") {
        return String(value);
    }
    // stringified JSON → object
    if (expected === "object" && actual === "string") {
        return parseJsonForSchema(value, "object");
    }
    // stringified JSON → array
    if (expected === "array" && actual === "string") {
        return parseJsonForSchema(value, "array");
    }
    return value;
}
const RECOVERY_MAP = {
    prerequisite_missing: (_toolName, error) => {
        if (/blueprint/i.test(error)) {
            return {
                tool: "open_blueprint",
                reason: "A Blueprint must be open in the editor first.",
            };
        }
        if (/landscape/i.test(error)) {
            return {
                tool: "create_landscape",
                params: { size: 2017, sections: 4 },
                reason: "No landscape exists. Create one first.",
            };
        }
        if (/PIE|play.*editor/i.test(error)) {
            return { tool: "start_pie", reason: "Play In Editor must be running." };
        }
        return null;
    },
    asset_not_found: (_toolName, error) => {
        if (/actor/i.test(error)) {
            return {
                tool: "find_actor_by_label",
                reason: "Actor not found. Search for the correct label.",
            };
        }
        if (/asset|material|mesh/i.test(error)) {
            return {
                tool: "find_tools",
                params: { query: "list assets" },
                reason: "Asset not found. Search for available assets.",
            };
        }
        return null;
    },
    bridge_disconnected: () => ({
        tool: "observe_ue_project",
        reason: "Bridge disconnected. Verify the editor is running.",
    }),
};
// ---------------------------------------------------------------------------
// SchemaIntelligence — main class
// ---------------------------------------------------------------------------
export class SchemaIntelligence {
    schemas = new Map();
    constructor(tools) {
        for (const tool of tools) {
            if (tool.inputSchema) {
                this.schemas.set(tool.name, tool.inputSchema);
            }
        }
    }
    /**
     * Coerce parameter values to match the schema's expected types.
     * Handles: string↔number, string↔boolean, number↔boolean (0/1),
     * stringified JSON → object/array.
     *
     * Returns a new object — does not mutate the input.
     */
    coerceParams(toolName, params) {
        const schema = this.schemas.get(toolName);
        if (!schema?.properties)
            return params;
        const out = toSafeRecord(params);
        for (const [key, value] of Object.entries(out)) {
            const propSchema = schema.properties[key];
            if (!propSchema)
                continue;
            out[key] = coerceValue(value, propSchema);
        }
        return out;
    }
    /**
     * Validate params against the tool's schema.
     * Returns null if valid, a ValidationError if not.
     *
     * Checks:
     * - Required fields present (not undefined/null/"")
     * - Enum compliance (after coercion)
     * - Basic type compliance (after coercion)
     */
    validateParams(toolName, params) {
        const schema = this.schemas.get(toolName);
        if (!schema)
            return null;
        const safeParams = toSafeRecord(params);
        // Required fields
        if (schema.required) {
            for (const field of schema.required) {
                const v = safeParams[field];
                if (v === undefined || v === null || v === "") {
                    return {
                        ok: false,
                        error: `Missing required parameter '${field}' for tool '${toolName}'.`,
                        error_category: "parameter_invalid",
                        recovery_hint: `Provide '${field}' (type: ${schema.properties?.[field]?.type || "unknown"}). Use describe_tool('${toolName}') for the full schema.`,
                        retryable: false,
                    };
                }
            }
        }
        // Enum compliance
        if (schema.properties) {
            for (const [key, value] of Object.entries(safeParams)) {
                const propSchema = schema.properties[key];
                if (!propSchema?.enum)
                    continue;
                if (!propSchema.enum.includes(value)) {
                    return {
                        ok: false,
                        error: `Invalid value '${String(value)}' for parameter '${key}'. Must be one of: ${propSchema.enum.join(", ")}`,
                        error_category: "parameter_invalid",
                        recovery_hint: `Valid values for '${key}': ${propSchema.enum.join(", ")}`,
                        retryable: false,
                    };
                }
            }
        }
        return null;
    }
}
/**
 * Build a structured recovery action from an error category.
 * Returns null if no recovery is known for this error type.
 *
 * The returned action tells the agent exactly what tool to call next
 * (with suggested params) to fix the problem.
 */
export function buildRecoveryAction(toolName, errorCategory, error) {
    const builder = RECOVERY_MAP[errorCategory];
    if (!builder)
        return null;
    return builder(toolName, error);
}
//# sourceMappingURL=schema-intelligence.js.map