function buildSessionBootstrapBody(request) {
    const body = {
        client_id: request.clientId,
        template: request.template,
        ttl_seconds: request.ttlSeconds,
    };
    if (request.allowedTools && request.allowedTools.length > 0) {
        body.allowed_tools = request.allowedTools;
    }
    if (request.deniedTools && request.deniedTools.length > 0) {
        body.denied_tools = request.deniedTools;
    }
    return body;
}
function getErrorMessage(payload, fallback) {
    if (payload && typeof payload === "object" && !Array.isArray(payload)) {
        const error = payload.error;
        if (typeof error === "string" && error.trim()) {
            return error;
        }
    }
    return fallback;
}
export async function exchangeScopedSessionToken(config, request) {
    if (!config.authToken) {
        throw new Error("Cannot exchange a session token without a bootstrap auth token.");
    }
    const response = await fetch(`http://${config.host}:${config.httpPort}/riftborn/auth/session`, {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${config.authToken}`,
        },
        body: JSON.stringify(buildSessionBootstrapBody(request)),
        signal: AbortSignal.timeout(5_000),
    });
    const rawText = await response.text();
    let payload = null;
    if (rawText.trim()) {
        try {
            payload = JSON.parse(rawText);
        }
        catch {
            payload = null;
        }
    }
    if (!response.ok) {
        throw new Error(getErrorMessage(payload, `Session bootstrap failed with HTTP ${response.status}.`));
    }
    if (!payload || payload.ok !== true) {
        throw new Error(getErrorMessage(payload, "Session bootstrap returned an invalid response payload."));
    }
    if (typeof payload.token_id !== "string"
        || !payload.token_id
        || typeof payload.expires_in !== "number"
        || !Number.isFinite(payload.expires_in)) {
        throw new Error("Session bootstrap response is missing token metadata.");
    }
    return {
        tokenId: payload.token_id,
        role: typeof payload.role === "string" ? payload.role : "",
        boundIp: typeof payload.bound_ip === "string" ? payload.bound_ip : "",
        expiresIn: payload.expires_in,
        expiresAt: typeof payload.expires_at === "string" ? payload.expires_at : "",
        status: typeof payload.status === "string" ? payload.status : "",
    };
}
//# sourceMappingURL=session-token.js.map