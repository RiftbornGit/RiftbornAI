import { ListResourcesRequestSchema, ListResourceTemplatesRequestSchema, ReadResourceRequestSchema, } from "@modelcontextprotocol/sdk/types.js";
import { stripProtoKeys } from "./bridge-reliability.js";
const MAX_RESOURCE_SEARCH_QUERY_LENGTH = 256;
const MAX_RESOURCE_IDENTIFIER_LENGTH = 512;
const MAX_LOCAL_JSON_RESPONSE_BYTES = 64 * 1024;
const LOCAL_SERVICE_HOST_PATTERN = /^(?:localhost|(?:\d{1,3}\.){3}\d{1,3}|\[[0-9a-f:.]+\]|[a-z0-9](?:[a-z0-9.-]*[a-z0-9])?)$/i;
export function searchToolCatalog(catalog, query) {
    const needle = normalizeResourceSearchQuery(query).toLowerCase();
    if (!needle) {
        return [];
    }
    return catalog
        .filter((tool) => tool.name.toLowerCase().includes(needle) ||
        tool.description.toLowerCase().includes(needle))
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
//# sourceMappingURL=mcp-resources.js.map