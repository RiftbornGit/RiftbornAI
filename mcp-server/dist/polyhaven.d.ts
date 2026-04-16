/**
 * Polyhaven Asset Integration for RiftbornAI
 *
 * Search and download free CC0 textures, HDRIs, and models from polyhaven.com.
 * Assets are downloaded to the project's Saved/Downloads folder,
 * then imported via import_asset_from_url or import_texture.
 */
import { Tool } from "@modelcontextprotocol/sdk/types.js";
import { RiftbornResponse } from "./riftborn-types.js";
export declare function searchPolyhaven(query: string, assetType?: "textures" | "hdris" | "models" | "all", limit?: number): Promise<RiftbornResponse>;
export declare function downloadPolyhavenTexture(assetId: string, resolution?: "1k" | "2k" | "4k", format?: "jpg" | "png" | "exr"): Promise<RiftbornResponse>;
export declare const POLYHAVEN_TOOLS: Tool[];
export declare function createPolyhavenHandlers(): Record<string, (args: Record<string, unknown>) => Promise<RiftbornResponse>>;
//# sourceMappingURL=polyhaven.d.ts.map