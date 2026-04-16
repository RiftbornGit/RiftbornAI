import fs from "fs";
import path from "path";
export const ENV_TOKEN_NAMES = [
    "RIFTBORN_AUTH_TOKEN",
    "RIFTBORN_API_KEY",
    "RIFTBORN_DEV_TOKEN",
];
function uniquePaths(paths) {
    const seen = new Set();
    const out = [];
    for (const value of paths) {
        if (!value || seen.has(value))
            continue;
        seen.add(value);
        out.push(value);
    }
    return out;
}
function buildTokenSearchPaths(searchRoots, explicitFile) {
    const candidates = [];
    if (explicitFile?.trim()) {
        candidates.push(path.resolve(explicitFile.trim()));
    }
    for (const root of searchRoots) {
        let current = path.resolve(root);
        while (true) {
            candidates.push(path.join(current, "Saved", "RiftbornAI", ".dev_token"));
            const parent = path.dirname(current);
            if (parent === current)
                break;
            current = parent;
        }
    }
    return uniquePaths(candidates);
}
export function resolveBridgeAuthToken(options) {
    const env = options?.env ?? process.env;
    const searchRoots = options?.searchRoots ?? [process.cwd()];
    for (const envName of ENV_TOKEN_NAMES) {
        const token = env[envName]?.trim() ?? "";
        if (token) {
            return { token, source: `env:${envName}`, searchedPaths: [] };
        }
    }
    const searchedPaths = buildTokenSearchPaths(searchRoots, env.RIFTBORN_DEV_TOKEN_FILE);
    for (const tokenPath of searchedPaths) {
        try {
            if (!fs.existsSync(tokenPath))
                continue;
            const token = fs.readFileSync(tokenPath, "utf8").trim();
            if (token) {
                return { token, source: `file:${tokenPath}`, searchedPaths };
            }
        }
        catch {
            // Ignore unreadable candidates and continue.
        }
    }
    return { token: "", source: "", searchedPaths };
}
//# sourceMappingURL=auth-token.js.map