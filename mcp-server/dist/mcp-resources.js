import { ListResourcesRequestSchema, ListResourceTemplatesRequestSchema, ReadResourceRequestSchema, } from "@modelcontextprotocol/sdk/types.js";
import { existsSync, readdirSync, readFileSync } from "node:fs";
import { homedir } from "node:os";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { stripProtoKeys } from "./bridge-reliability.js";
import { DOMAIN_DEFINITION_OF_DONE, getDomainDefinitionOfDoneContract, } from "./domain-proof-contract.js";
export { getDomainDefinitionOfDoneContract } from "./domain-proof-contract.js";
const MAX_RESOURCE_SEARCH_QUERY_LENGTH = 256;
const MAX_RESOURCE_IDENTIFIER_LENGTH = 512;
const MAX_LOCAL_JSON_RESPONSE_BYTES = 64 * 1024;
const LOCAL_SERVICE_HOST_PATTERN = /^(?:localhost|(?:\d{1,3}\.){3}\d{1,3}|\[[0-9a-f:.]+\]|[a-z0-9](?:[a-z0-9.-]*[a-z0-9])?)$/i;
function normalizeSkillPackKey(value) {
    return value
        .trim()
        .toLowerCase()
        .replace(/[\s-]+/g, "_")
        .replace(/[^a-z0-9_]/g, "");
}
function splitSkillPackFrontmatter(raw) {
    const lines = raw.split(/\r?\n/);
    if (lines[0]?.trim() !== "---") {
        return { frontmatter: "", body: raw.trim() };
    }
    let closingIndex = -1;
    for (let index = 1; index < lines.length; index += 1) {
        if (lines[index]?.trim() === "---") {
            closingIndex = index;
            break;
        }
    }
    if (closingIndex < 0) {
        return { frontmatter: "", body: raw.trim() };
    }
    return {
        frontmatter: lines.slice(1, closingIndex).join("\n").trim(),
        body: lines.slice(closingIndex + 1).join("\n").trim(),
    };
}
function trimQuotes(value) {
    const trimmed = value.trim();
    if (trimmed.length >= 2 &&
        ((trimmed.startsWith("\"") && trimmed.endsWith("\"")) ||
            (trimmed.startsWith("'") && trimmed.endsWith("'")))) {
        return trimmed.slice(1, -1).trim();
    }
    return trimmed;
}
function parseCommaSeparatedList(value) {
    return value
        .split(",")
        .map((part) => trimQuotes(part))
        .filter((part) => part.length > 0);
}
function parseSkillPackFile(raw, filename, source) {
    const { frontmatter, body } = splitSkillPackFrontmatter(raw);
    if (!frontmatter) {
        return null;
    }
    const entry = {
        name: filename.replace(/\.md$/i, ""),
        kind: "skill",
        summary: "",
        when: [],
        source,
        filename,
    };
    if (frontmatter) {
        for (const line of frontmatter.split(/\r?\n/)) {
            const trimmed = line.trim();
            if (!trimmed || trimmed.startsWith("#")) {
                continue;
            }
            const colonIndex = trimmed.indexOf(":");
            if (colonIndex <= 0) {
                continue;
            }
            const key = trimmed.slice(0, colonIndex).trim().toLowerCase();
            const value = trimQuotes(trimmed.slice(colonIndex + 1));
            if (key === "name") {
                entry.name = value;
            }
            else if (key === "kind") {
                entry.kind = normalizeSkillPackKey(value) || "skill";
            }
            else if (key === "summary" || key === "description") {
                entry.summary = value;
            }
            else if (key === "specialist" || key === "agent") {
                entry.specialist = normalizeSkillPackKey(value);
            }
            else if (key === "profile") {
                entry.profile = normalizeSkillPackKey(value);
            }
            else if (key === "when" || key === "triggers" || key === "keywords") {
                entry.when = parseCommaSeparatedList(value);
            }
        }
    }
    if (!entry.summary) {
        const firstBodyLine = body.split(/\r?\n/).find((line) => line.trim().length > 0) || "";
        entry.summary = firstBodyLine.replace(/^[-*]\s*/, "").trim();
    }
    return entry.name && body ? entry : null;
}
function parseInteger(value) {
    const parsed = Number.parseInt(value, 10);
    return Number.isFinite(parsed) ? parsed : undefined;
}
function parseNumber(value) {
    const parsed = Number.parseFloat(value);
    return Number.isFinite(parsed) ? parsed : undefined;
}
function parseBoolean(value) {
    const normalized = value.trim().toLowerCase();
    if (["true", "1", "yes"].includes(normalized)) {
        return true;
    }
    if (["false", "0", "no"].includes(normalized)) {
        return false;
    }
    return undefined;
}
function parseAgentManifestFile(raw, filename, source) {
    const { frontmatter, body } = splitSkillPackFrontmatter(raw);
    if (!frontmatter) {
        return null;
    }
    const entry = {
        name: filename.replace(/\.md$/i, ""),
        alias: normalizeSkillPackKey(filename.replace(/\.md$/i, "")),
        summary: "",
        when: [],
        skill_packs: [],
        tool_include: [],
        tool_exclude: [],
        source,
        filename,
    };
    for (const line of frontmatter.split(/\r?\n/)) {
        const trimmed = line.trim();
        if (!trimmed || trimmed.startsWith("#")) {
            continue;
        }
        const colonIndex = trimmed.indexOf(":");
        if (colonIndex <= 0) {
            continue;
        }
        const key = trimmed.slice(0, colonIndex).trim().toLowerCase();
        const value = trimQuotes(trimmed.slice(colonIndex + 1));
        if (key === "name") {
            entry.name = value;
        }
        else if (key === "alias" || key === "agent") {
            entry.alias = normalizeSkillPackKey(value);
        }
        else if (key === "summary" || key === "description") {
            entry.summary = value;
        }
        else if (key === "profile") {
            entry.profile = normalizeSkillPackKey(value);
        }
        else if (key === "when" || key === "triggers" || key === "keywords") {
            entry.when = parseCommaSeparatedList(value);
        }
        else if (key === "skill_packs" || key === "default_skill_packs" || key === "packs") {
            entry.skill_packs = parseCommaSeparatedList(value).map((pack) => normalizeSkillPackKey(pack));
        }
        else if (key === "allowed_tools" || key === "tool_include" || key === "tools") {
            entry.tool_include = parseCommaSeparatedList(value).map((tool) => normalizeSkillPackKey(tool));
        }
        else if (key === "disallowed_tools" || key === "tool_exclude" || key === "blocked_tools") {
            entry.tool_exclude = parseCommaSeparatedList(value).map((tool) => normalizeSkillPackKey(tool));
        }
        else if (key === "provider" || key === "preferred_provider" || key === "provider_bias") {
            entry.provider = value;
        }
        else if (key === "max_iterations") {
            entry.max_iterations = parseInteger(value);
        }
        else if (key === "timeout_seconds") {
            entry.timeout_seconds = parseNumber(value);
        }
        else if (key === "include_scene_context") {
            entry.include_scene_context = parseBoolean(value);
        }
        else if (key === "read_only_bias") {
            entry.read_only_bias = parseBoolean(value);
        }
        else if (key === "verification_bias") {
            entry.verification_bias = parseBoolean(value);
        }
        else if (key === "completion_style") {
            entry.completion_style = normalizeSkillPackKey(value);
        }
    }
    if (!entry.summary) {
        const firstBodyLine = body.split(/\r?\n/).find((line) => line.trim().length > 0) || "";
        entry.summary = firstBodyLine.replace(/^[-*]\s*/, "").trim();
    }
    return entry.name && entry.alias ? entry : null;
}
function getPluginRoot(explicitPluginRoot) {
    const configured = explicitPluginRoot?.trim() || process.env.RIFTBORN_PLUGIN_PATH?.trim();
    if (configured) {
        return resolve(configured);
    }
    return resolve(dirname(fileURLToPath(import.meta.url)), "..", "..");
}
function getProjectRoot(pluginRoot, explicitProjectRoot) {
    if (explicitProjectRoot?.trim()) {
        return resolve(explicitProjectRoot);
    }
    const parent = dirname(pluginRoot);
    const parentName = parent.split(/[\\/]/).at(-1)?.toLowerCase();
    if (parentName === "plugins") {
        return resolve(pluginRoot, "..", "..");
    }
    return pluginRoot;
}
function getUserRoot(explicitUserRoot) {
    const configured = explicitUserRoot?.trim()
        || process.env.RIFTBORN_USER_CONFIG_ROOT?.trim();
    if (configured) {
        return resolve(configured);
    }
    const localAppData = process.env.LOCALAPPDATA?.trim();
    if (localAppData) {
        return resolve(localAppData);
    }
    return resolve(homedir(), ".config");
}
export function readCopilotSkillPackCatalog(options) {
    const pluginRoot = getPluginRoot(options?.pluginRoot);
    const projectRoot = getProjectRoot(pluginRoot, options?.projectRoot);
    const userRoot = getUserRoot(options?.userRoot);
    const layers = [
        { source: "bundled", dir: resolve(pluginRoot, "Config", "SkillPacks") },
        { source: "project", dir: resolve(projectRoot, "Config", "RiftbornAI", "Skills") },
        { source: "user", dir: resolve(userRoot, "RiftbornAI", "SkillPacks") },
    ];
    const catalog = new Map();
    for (const layer of layers) {
        if (!existsSync(layer.dir)) {
            continue;
        }
        const files = readdirSync(layer.dir)
            .filter((entry) => entry.toLowerCase().endsWith(".md"))
            .sort((left, right) => left.localeCompare(right));
        for (const filename of files) {
            const raw = readFileSync(resolve(layer.dir, filename), "utf8");
            const parsed = parseSkillPackFile(raw, filename, layer.source);
            if (!parsed) {
                continue;
            }
            const stableKey = parsed.specialist || normalizeSkillPackKey(parsed.name) || normalizeSkillPackKey(filename);
            catalog.set(stableKey, parsed);
        }
    }
    return Array.from(catalog.values()).sort((left, right) => left.name.localeCompare(right.name));
}
export function searchSkillPackCatalog(catalog, query) {
    const needle = normalizeResourceSearchQuery(query).toLowerCase();
    if (!needle) {
        return [];
    }
    return catalog
        .filter((pack) => pack.name.toLowerCase().includes(needle) ||
        pack.summary.toLowerCase().includes(needle) ||
        (pack.specialist || "").toLowerCase().includes(needle) ||
        pack.when.some((phrase) => phrase.toLowerCase().includes(needle)))
        .sort((left, right) => left.name.localeCompare(right.name));
}
export function readCopilotAgentCatalog(options) {
    const pluginRoot = getPluginRoot(options?.pluginRoot);
    const projectRoot = getProjectRoot(pluginRoot, options?.projectRoot);
    const userRoot = getUserRoot(options?.userRoot);
    const layers = [
        { source: "bundled", dir: resolve(pluginRoot, "Config", "Agents") },
        { source: "project", dir: resolve(projectRoot, "Config", "RiftbornAI", "Agents") },
        { source: "user", dir: resolve(userRoot, "RiftbornAI", "Agents") },
    ];
    const catalog = new Map();
    for (const layer of layers) {
        if (!existsSync(layer.dir)) {
            continue;
        }
        const files = readdirSync(layer.dir)
            .filter((entry) => entry.toLowerCase().endsWith(".md"))
            .sort((left, right) => left.localeCompare(right));
        for (const filename of files) {
            const raw = readFileSync(resolve(layer.dir, filename), "utf8");
            const parsed = parseAgentManifestFile(raw, filename, layer.source);
            if (!parsed) {
                continue;
            }
            const stableKey = parsed.alias || normalizeSkillPackKey(parsed.name) || normalizeSkillPackKey(filename);
            catalog.set(stableKey, parsed);
        }
    }
    return Array.from(catalog.values()).sort((left, right) => left.name.localeCompare(right.name));
}
export function searchAgentCatalog(catalog, query) {
    const needle = normalizeResourceSearchQuery(query).toLowerCase();
    if (!needle) {
        return [];
    }
    return catalog
        .filter((agent) => agent.name.toLowerCase().includes(needle) ||
        agent.alias.toLowerCase().includes(needle) ||
        agent.summary.toLowerCase().includes(needle) ||
        (agent.profile || "").toLowerCase().includes(needle) ||
        (agent.provider || "").toLowerCase().includes(needle) ||
        (agent.completion_style || "").toLowerCase().includes(needle) ||
        agent.when.some((phrase) => phrase.toLowerCase().includes(needle)) ||
        agent.skill_packs.some((pack) => pack.toLowerCase().includes(needle)) ||
        agent.tool_include.some((tool) => tool.toLowerCase().includes(needle)) ||
        agent.tool_exclude.some((tool) => tool.toLowerCase().includes(needle)))
        .sort((left, right) => left.name.localeCompare(right.name));
}
export function searchToolCatalog(catalog, query) {
    const needle = normalizeResourceSearchQuery(query).toLowerCase();
    if (!needle) {
        return [];
    }
    return catalog
        .filter((tool) => tool.name.toLowerCase().includes(needle) ||
        tool.description.toLowerCase().includes(needle) ||
        (tool.reason || "").toLowerCase().includes(needle))
        .sort((left, right) => {
        const leftScore = (left.name.toLowerCase() === needle ? 2 : 0) +
            (left.visible ? 1 : 0);
        const rightScore = (right.name.toLowerCase() === needle ? 2 : 0) +
            (right.visible ? 1 : 0);
        if (leftScore !== rightScore) {
            return rightScore - leftScore;
        }
        return left.name.localeCompare(right.name);
    });
}
export function normalizeResourceSearchQuery(query) {
    return query
        .trim()
        .replace(/[\r\n\t\0]+/g, " ")
        .replace(/\s+/g, " ")
        .slice(0, MAX_RESOURCE_SEARCH_QUERY_LENGTH);
}
export function normalizeResourceIdentifier(value) {
    return value
        .trim()
        .replace(/[\r\n\t\0]+/g, " ")
        .replace(/\s+/g, " ")
        .slice(0, MAX_RESOURCE_IDENTIFIER_LENGTH);
}
export function decodeResourceComponent(component) {
    try {
        return decodeURIComponent(component);
    }
    catch {
        return null;
    }
}
export function buildLocalServiceUrl(host, port, servicePath) {
    const normalizedHost = typeof host === "string" ? host.trim() : "";
    if (!normalizedHost ||
        normalizedHost.length > 255 ||
        /[\/\\\s]/.test(normalizedHost) ||
        !LOCAL_SERVICE_HOST_PATTERN.test(normalizedHost)) {
        return null;
    }
    if (!Number.isInteger(port) || port < 1 || port > 65_535) {
        return null;
    }
    if (typeof servicePath !== "string" ||
        !servicePath.startsWith("/") ||
        /[\r\n\t\0\\]/.test(servicePath) ||
        servicePath.includes("://")) {
        return null;
    }
    return `http://${normalizedHost}:${port}${servicePath}`;
}
export async function fetchLocalJsonResource(host, port, servicePath, label) {
    const url = buildLocalServiceUrl(host, port, servicePath);
    if (!url) {
        return { ok: false, error: `${label} target is invalid` };
    }
    try {
        const response = await fetch(url, { signal: AbortSignal.timeout(3000) });
        if (!response.ok) {
            return { ok: false, error: `${label} returned HTTP ${response.status}` };
        }
        const contentType = response.headers.get("content-type") || "";
        if (!contentType.includes("application/json") && !contentType.includes("text/json")) {
            return { ok: false, error: `${label} returned non-JSON content` };
        }
        const text = await response.text();
        if (text.length > MAX_LOCAL_JSON_RESPONSE_BYTES) {
            return { ok: false, error: `${label} response exceeded ${MAX_LOCAL_JSON_RESPONSE_BYTES} bytes` };
        }
        const parsed = JSON.parse(text);
        if (typeof parsed !== "object" || parsed === null) {
            return { ok: false, error: `${label} returned malformed JSON` };
        }
        return stripProtoKeys(parsed);
    }
    catch {
        return { ok: false, error: `${label} not responding` };
    }
}
export function getCopilotOperatingContract() {
    return {
        product: "RiftbornAI governed Unreal Editor copilot",
        frontDoors: [
            "MCP server",
            "Built-in Slate copilot",
        ],
        loop: [
            "inspect",
            "plan",
            "act",
            "verify",
        ],
        workingRules: [
            "Inspect the current editor or scene state before mutating it.",
            "Prefer exact registered tools over guessed workflows or historical names.",
            "Use real project assets when available instead of recreating placeholders.",
            "After meaningful changes, verify with scene inspection, screenshots, logs, or PIE.",
            "Use tool discovery resources before concluding the product cannot perform a task.",
        ],
        discoveryResources: [
            "riftborn://copilot/operating-contract",
            "riftborn://copilot/domain-definition-of-done",
            "riftborn://copilot/domain-definition-of-done/{domain}",
            "riftborn://copilot/agents",
            "riftborn://copilot/agents/search/{query}",
            "riftborn://copilot/skill-packs",
            "riftborn://copilot/skill-packs/search/{query}",
            "riftborn://tools/categories",
            "riftborn://tools/deferred",
            "riftborn://tools/search/{query}",
            "riftborn://project/info",
            "riftborn://project/actors",
        ],
    };
}
export function registerResourceHandlers(server, { allTools, visibleTools, getToolCatalog, categoryMap, executeTool, httpRequest, host, httpPort, tcpPort, brainPort, sessionTracker }) {
    server.setRequestHandler(ListResourcesRequestSchema, async () => {
        const resources = [
            {
                uri: "riftborn://copilot/operating-contract",
                name: "Copilot Operating Contract",
                description: "Compact skill-style contract for external AI clients: the shared RiftbornAI operating loop, guardrails, and discovery resources.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://project/info",
                name: "Unreal Engine Project Info",
                description: "Current project name, engine version, game mode, player controller, and loaded level. Essential context for any UE operation.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://project/actors",
                name: "Level Actor Census",
                description: "All actors in the current level grouped by class with counts. Use to understand what's in the scene.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://bridge/health",
                name: "Bridge Health Status",
                description: "Connection status of HTTP bridge, TCP bridge, Brain API, and Ollama. Check before attempting operations.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://project/assets",
                name: "Project Asset Tree",
                description: "Top-level asset folders and counts under /Game/. Shows Blueprints, Materials, Maps, etc.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://governance/status",
                name: "Governance & Verification Status",
                description: "Current session taint state, proof mode, pending confirmations, and tool execution stats.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://tools/categories",
                name: "Tool Categories Summary",
                description: "All 26 tool categories with tool counts and brief descriptions. Helps discover available capabilities.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://tools/deferred",
                name: "Deferred Tool Catalog",
                description: "Hidden-by-default tools grouped by readiness tier. Use this instead of advertising the giant flat tool surface up front.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://session/history",
                name: "Session Tool History",
                description: "Recent tool calls in this session with timing, success/failure, and frequency. Use to understand what has been done and avoid redundant calls.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://copilot/domain-definition-of-done",
                name: "Domain Definition Of Done",
                description: "Minimum finish-line contracts for the primary authoring domains so workflows do not stop at compile-only proof.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://copilot/skill-packs",
                name: "Copilot Skill Packs",
                description: "Authorable bundled and project skill/workflow packs that can be injected into prompts or used as specialist aliases.",
                mimeType: "application/json",
            },
            {
                uri: "riftborn://copilot/agents",
                name: "Copilot Agent Manifests",
                description: "Authorable bundled and project specialist-agent manifests used by spawn_subagent for profile, provider, and playbook defaults.",
                mimeType: "application/json",
            },
        ];
        return { resources };
    });
    server.setRequestHandler(ListResourceTemplatesRequestSchema, async () => ({
        resourceTemplates: [
            {
                uriTemplate: "riftborn://asset/{assetPath}",
                name: "Asset Details",
                description: "Get details about a specific asset by path (e.g., /Game/Blueprints/BP_MyActor)",
                mimeType: "application/json",
            },
            {
                uriTemplate: "riftborn://actor/{actorName}",
                name: "Actor Details",
                description: "Get details about a specific actor in the level by name",
                mimeType: "application/json",
            },
            {
                uriTemplate: "riftborn://tools/search/{query}",
                name: "Search Deferred Tools",
                description: "Search the full tool catalog, including hidden-by-default deferred tools, by name or description",
                mimeType: "application/json",
            },
            {
                uriTemplate: "riftborn://copilot/domain-definition-of-done/{domain}",
                name: "Domain Definition Of Done By Domain",
                description: "Get the finish-line contract for one authoring domain such as Blueprint, UI, GAS, or SaveLoad",
                mimeType: "application/json",
            },
            {
                uriTemplate: "riftborn://copilot/skill-packs/search/{query}",
                name: "Search Skill Packs",
                description: "Search bundled and project skill/workflow packs by name, specialist alias, summary, or trigger phrases",
                mimeType: "application/json",
            },
            {
                uriTemplate: "riftborn://copilot/agents/search/{query}",
                name: "Search Agent Manifests",
                description: "Search bundled and project authored specialist agents by alias, name, summary, provider bias, or bound skill packs",
                mimeType: "application/json",
            },
        ],
    }));
    server.setRequestHandler(ReadResourceRequestSchema, async (request) => {
        const { uri } = request.params;
        // Static resources
        if (uri === "riftborn://copilot/operating-contract") {
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(getCopilotOperatingContract(), null, 2),
                    }],
            };
        }
        if (uri === "riftborn://copilot/domain-definition-of-done") {
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(getDomainDefinitionOfDoneContract(), null, 2),
                    }],
            };
        }
        if (uri === "riftborn://copilot/skill-packs") {
            const catalog = readCopilotSkillPackCatalog();
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify({
                            total_packs: catalog.length,
                            specialists: catalog.filter((pack) => Boolean(pack.specialist)).map((pack) => ({
                                specialist: pack.specialist,
                                name: pack.name,
                                profile: pack.profile ?? "editor_assistant",
                            })),
                            packs: catalog,
                        }, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://copilot/agents") {
            const catalog = readCopilotAgentCatalog();
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify({
                            total_agents: catalog.length,
                            agents: catalog,
                        }, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://project/info") {
            const result = await executeTool("get_project_info");
            if (!result.ok) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: result.error ?? "get_project_info failed", resource: uri }, null, 2),
                        }],
                };
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(result, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://project/actors") {
            const result = await executeTool("get_all_actors");
            if (!result.ok) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: result.error ?? "get_all_actors failed", resource: uri }, null, 2),
                        }],
                };
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(result, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://bridge/health") {
            const httpHealth = await httpRequest("GET", "/riftborn/health");
            const brainHealth = await fetchLocalJsonResource(host, brainPort, "/brain/health", "Brain API");
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify({
                            http_bridge: httpHealth,
                            brain_api: brainHealth,
                            ports: {
                                http: httpPort,
                                tcp: tcpPort,
                                brain: brainPort,
                            },
                        }, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://project/assets") {
            const result = await executeTool("list_assets", { folder: "/Game", recursive: false });
            if (!result.ok) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: result.error ?? "list_assets failed", resource: uri }, null, 2),
                        }],
                };
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(result, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://governance/status") {
            const result = await executeTool("get_verification_status");
            if (!result.ok) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: result.error ?? "get_verification_status failed", resource: uri }, null, 2),
                        }],
                };
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(result, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://tools/categories") {
            // Build category summary from the visible tool surface only.
            const categories = {};
            for (const tool of visibleTools()) {
                // Infer category from tool name prefix
                const prefix = tool.name.split("_")[0];
                const cat = categoryMap[prefix] || "Other";
                categories[cat] = (categories[cat] || 0) + 1;
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify({
                            total_tools: visibleTools().length,
                            hidden_tools: allTools.length - visibleTools().length,
                            categories,
                        }, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://tools/deferred") {
            const deferred = getToolCatalog().filter((tool) => !tool.visible);
            const byTier = {};
            for (const tool of deferred) {
                byTier[tool.tier] ||= [];
                byTier[tool.tier].push(tool);
            }
            for (const tools of Object.values(byTier)) {
                tools.sort((left, right) => left.name.localeCompare(right.name));
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify({
                            total_tools: getToolCatalog().length,
                            visible_tools: visibleTools().length,
                            deferred_tools: deferred.length,
                            deferred_by_tier: byTier,
                        }, null, 2),
                    }],
            };
        }
        // Template resources
        const assetMatch = uri.match(/^riftborn:\/\/asset\/(.+)$/);
        if (assetMatch) {
            const decodedAssetPath = decodeResourceComponent(assetMatch[1]);
            const assetPath = decodedAssetPath ? normalizeResourceIdentifier(decodedAssetPath) : "";
            if (!assetPath) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: "Invalid or empty asset resource identifier.", resource: uri }, null, 2),
                        }],
                };
            }
            const result = await executeTool("get_asset_info", { asset_path: assetPath });
            if (!result.ok) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: result.error ?? "get_asset_info failed", asset_path: assetPath }, null, 2),
                        }],
                };
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(result, null, 2),
                    }],
            };
        }
        const actorMatch = uri.match(/^riftborn:\/\/actor\/(.+)$/);
        if (actorMatch) {
            const decodedActorName = decodeResourceComponent(actorMatch[1]);
            const actorName = decodedActorName ? normalizeResourceIdentifier(decodedActorName) : "";
            if (!actorName) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: "Invalid or empty actor resource identifier.", resource: uri }, null, 2),
                        }],
                };
            }
            const result = await executeTool("get_actor_details", { actor_name: actorName });
            if (!result.ok) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: result.error ?? "get_actor_details failed", actor_name: actorName }, null, 2),
                        }],
                };
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(result, null, 2),
                    }],
            };
        }
        const toolSearchMatch = uri.match(/^riftborn:\/\/tools\/search\/(.+)$/);
        if (toolSearchMatch) {
            const decodedQuery = decodeResourceComponent(toolSearchMatch[1]);
            const query = decodedQuery ? normalizeResourceSearchQuery(decodedQuery) : "";
            if (!query) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: "Invalid or empty tool search query.", resource: uri }, null, 2),
                        }],
                };
            }
            const matches = searchToolCatalog(getToolCatalog(), query).slice(0, 50);
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify({
                            query,
                            total_matches: matches.length,
                            results: matches,
                        }, null, 2),
                    }],
            };
        }
        const dodMatch = uri.match(/^riftborn:\/\/copilot\/domain-definition-of-done\/(.+)$/);
        if (dodMatch) {
            const decodedDomain = decodeResourceComponent(dodMatch[1]);
            const domain = decodedDomain ? normalizeResourceIdentifier(decodedDomain) : "";
            if (!domain) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: "Invalid or empty domain identifier.", resource: uri }, null, 2),
                        }],
                };
            }
            const contract = getDomainDefinitionOfDoneContract(domain);
            if (!contract) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({
                                ok: false,
                                error: `Unknown domain '${domain}'.`,
                                available_domains: Object.values(DOMAIN_DEFINITION_OF_DONE).map((entry) => entry.domain),
                            }, null, 2),
                        }],
                };
            }
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(contract, null, 2),
                    }],
            };
        }
        const skillPackMatch = uri.match(/^riftborn:\/\/copilot\/skill-packs\/search\/(.+)$/);
        if (skillPackMatch) {
            const decodedQuery = decodeResourceComponent(skillPackMatch[1]);
            const query = decodedQuery ? normalizeResourceSearchQuery(decodedQuery) : "";
            if (!query) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: "Invalid or empty skill-pack search query.", resource: uri }, null, 2),
                        }],
                };
            }
            const catalog = readCopilotSkillPackCatalog();
            const matches = searchSkillPackCatalog(catalog, query).slice(0, 50);
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify({
                            query,
                            total_matches: matches.length,
                            results: matches,
                        }, null, 2),
                    }],
            };
        }
        const agentMatch = uri.match(/^riftborn:\/\/copilot\/agents\/search\/(.+)$/);
        if (agentMatch) {
            const decodedQuery = decodeResourceComponent(agentMatch[1]);
            const query = decodedQuery ? normalizeResourceSearchQuery(decodedQuery) : "";
            if (!query) {
                return {
                    contents: [{
                            uri,
                            mimeType: "application/json",
                            text: JSON.stringify({ ok: false, error: "Invalid or empty agent-manifest search query.", resource: uri }, null, 2),
                        }],
                };
            }
            const catalog = readCopilotAgentCatalog();
            const matches = searchAgentCatalog(catalog, query).slice(0, 50);
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify({
                            query,
                            total_matches: matches.length,
                            results: matches,
                        }, null, 2),
                    }],
            };
        }
        if (uri === "riftborn://session/history") {
            const history = sessionTracker?.getHistory() ?? { total_calls: 0, total_errors: 0, recent: [], tool_frequency: {} };
            return {
                contents: [{
                        uri,
                        mimeType: "application/json",
                        text: JSON.stringify(history, null, 2),
                    }],
            };
        }
        throw new Error(`Unknown resource: ${uri}`);
    });
}