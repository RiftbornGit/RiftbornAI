export function getToolAccessError(name, access) {
    if (access.blockedTools.has(name)) {
        return {
            ok: false,
            error: `Tool '${name}' is blocked by the public API manifest and is not available through MCP packaging.`,
        };
    }
    if (access.internalOnlyTools.has(name) && !access.enableInternalTools) {
        return {
            ok: false,
            error: `Tool '${name}' is internal-only and not available in production packaging. Set RIFTBORN_ENABLE_INTERNAL_TOOLS=true for internal builds.`,
        };
    }
    if (!access.visibleToolNames.has(name) && !access.allowHiddenTools) {
        return {
            ok: false,
            error: `Tool '${name}' is hidden by the readiness gate. Set RIFTBORN_ALLOW_HIDDEN_TOOLS=true to call hidden tools explicitly.`,
        };
    }
    return null;
}
function isCallableTool(name, toolHandlers, generatedToolNames) {
    return Boolean(toolHandlers[name]) || generatedToolNames.has(name);
}
export function resolveToolInvocation({ name, toolHandlers, generatedToolNames, toolResolver, }) {
    if (isCallableTool(name, toolHandlers, generatedToolNames)) {
        return { resolvedName: name };
    }
    const resolution = toolResolver.resolve(name);
    if (resolution.resolved &&
        (resolution.confidence === "alias" || resolution.confidence === "high") &&
        isCallableTool(resolution.resolved, toolHandlers, generatedToolNames)) {
        // SECURITY: Re-check access control after fuzzy resolution.
        // Without this, a blocked tool could be reached via a typo variant
        // (e.g., "blocked_too1" resolves to "blocked_tool" and bypasses the
        // access check that only ran on the original name).
        // Access config is not available here, so the caller must re-check.
        // We tag the resolution so the caller knows to verify.
        return {
            resolvedName: resolution.resolved,
            resolutionMeta: {
                _resolved_from: name,
                _resolved_to: resolution.resolved,
                _confidence: resolution.confidence,
                _requires_access_recheck: true,
            },
        };
    }
    const suggestions = resolution.suggestions.length > 0
        ? ` Did you mean: ${resolution.suggestions.join(", ")}?`
        : "";
    return {
        ok: false,
        error: `Unknown tool: ${name}.${suggestions}`,
    };
}
//# sourceMappingURL=dispatch-policy.js.map