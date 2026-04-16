/**
 * Polyhaven Asset Integration for RiftbornAI
 *
 * Search and download free CC0 textures, HDRIs, and models from polyhaven.com.
 * Assets are downloaded to the project's Saved/Downloads folder,
 * then imported via import_asset_from_url or import_texture.
 */
const POLYHAVEN_API = "https://api.polyhaven.com";
// ---------------------------------------------------------------------------
// API helpers
// ---------------------------------------------------------------------------
async function fetchJson(url, timeoutMs = 10000) {
    const resp = await fetch(url, {
        signal: AbortSignal.timeout(timeoutMs),
        headers: { "User-Agent": "RiftbornAI/2.1 (Unreal Engine Plugin)" },
    });
    if (!resp.ok) {
        throw new Error(`Polyhaven API ${resp.status}: ${resp.statusText}`);
    }
    return resp.json();
}
// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------
export async function searchPolyhaven(query, assetType = "all", limit = 20) {
    try {
        // Polyhaven API: GET /assets?t=<type>
        const typeParam = assetType === "all" ? "" : `?t=${assetType}`;
        const data = await fetchJson(`${POLYHAVEN_API}/assets${typeParam}`);
        // data is { asset_id: { name, categories, tags, ... }, ... }
        const queryLower = query.toLowerCase();
        const matches = [];
        for (const [id, info] of Object.entries(data)) {
            const name = String(info.name || id);
            const cats = info.categories || [];
            const tags = info.tags || [];
            const allText = [name, id, ...cats, ...tags].join(" ").toLowerCase();
            if (allText.includes(queryLower)) {
                matches.push({
                    name,
                    id,
                    type: String(info.type || assetType),
                    categories: cats,
                    tags: tags.slice(0, 8),
                    download_url: `https://dl.polyhaven.org/file/ph-assets/${info.type === 1 ? "HDRIs" : info.type === 2 ? "Textures" : "Models"}/${id}/2k/${id}_2k.zip`,
                });
            }
            if (matches.length >= limit)
                break;
        }
        return {
            ok: true,
            result: JSON.stringify({
                query,
                asset_type: assetType,
                count: matches.length,
                assets: matches,
                hint: "Use download_polyhaven_texture with the asset id to download and import into UE.",
            }),
        };
    }
    catch (error) {
        return {
            ok: false,
            error: `Polyhaven search failed: ${error instanceof Error ? error.message : String(error)}`,
        };
    }
}
// ---------------------------------------------------------------------------
// Download texture maps
// ---------------------------------------------------------------------------
export async function downloadPolyhavenTexture(assetId, resolution = "2k", format = "jpg") {
    try {
        // Get the file list for this asset
        const files = await fetchJson(`${POLYHAVEN_API}/files/${assetId}`, 15000);
        // Extract texture map URLs
        const maps = {};
        if (files.Textures) {
            for (const [mapName, resolutions] of Object.entries(files.Textures)) {
                const resData = resolutions[resolution] || resolutions["2k"] || resolutions["1k"];
                if (resData) {
                    const formatData = resData[format] || resData["jpg"] || resData["png"];
                    if (formatData?.url) {
                        maps[mapName] = formatData.url;
                    }
                }
            }
        }
        if (Object.keys(maps).length === 0) {
            return {
                ok: false,
                error: `No texture maps found for '${assetId}' at ${resolution}/${format}. Available: ${JSON.stringify(Object.keys(files))}`,
            };
        }
        return {
            ok: true,
            result: JSON.stringify({
                asset_id: assetId,
                resolution,
                format,
                maps,
                map_count: Object.keys(maps).length,
                hint: "Use import_asset_from_url with each URL to download into UE. Diffuse/Color → albedo, Normal → normal map, Displacement → height map for silPOM, Rough → roughness.",
            }),
        };
    }
    catch (error) {
        return {
            ok: false,
            error: `Polyhaven download failed: ${error instanceof Error ? error.message : String(error)}`,
        };
    }
}
// ---------------------------------------------------------------------------
// Tool definitions
// ---------------------------------------------------------------------------
export const POLYHAVEN_TOOLS = [
    {
        name: "search_polyhaven",
        description: "Search Polyhaven for free CC0 textures, HDRIs, and 3D models. " +
            "Returns asset names, IDs, categories, and download URLs. " +
            "Use download_polyhaven_texture to get specific texture maps (albedo, normal, height, roughness).",
        inputSchema: {
            type: "object",
            properties: {
                query: {
                    type: "string",
                    description: "Search query (e.g., 'sand', 'rock', 'brick', 'forest floor')",
                },
                asset_type: {
                    type: "string",
                    enum: ["textures", "hdris", "models", "all"],
                    description: "Filter by asset type. Default: all",
                },
                limit: {
                    type: "number",
                    description: "Max results to return. Default: 20",
                },
            },
            required: ["query"],
        },
    },
    {
        name: "download_polyhaven_texture",
        description: "Get download URLs for all texture maps (albedo, normal, displacement, roughness, AO) " +
            "of a Polyhaven texture. Returns URLs that can be passed to import_asset_from_url. " +
            "Use search_polyhaven first to find the asset ID.",
        inputSchema: {
            type: "object",
            properties: {
                asset_id: {
                    type: "string",
                    description: "Polyhaven asset ID (from search_polyhaven results)",
                },
                resolution: {
                    type: "string",
                    enum: ["1k", "2k", "4k"],
                    description: "Texture resolution. Default: 2k",
                },
                format: {
                    type: "string",
                    enum: ["jpg", "png", "exr"],
                    description: "Image format. Default: jpg (png for normal maps recommended)",
                },
            },
            required: ["asset_id"],
        },
    },
];
// ---------------------------------------------------------------------------
// Handler map
// ---------------------------------------------------------------------------
export function createPolyhavenHandlers() {
    return {
        search_polyhaven: async (args) => searchPolyhaven(String(args.query || ""), args.asset_type || "all", Number(args.limit) || 20),
        download_polyhaven_texture: async (args) => downloadPolyhavenTexture(String(args.asset_id || ""), args.resolution || "2k", args.format || "jpg"),
    };
}
//# sourceMappingURL=polyhaven.js.map