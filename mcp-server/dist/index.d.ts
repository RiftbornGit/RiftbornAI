#!/usr/bin/env node
/**
 * RiftbornAI MCP Server
 *
 * Model Context Protocol server that bridges VS Code Copilot to Unreal Engine
 * via the RiftbornAI HTTP bridge.
 *
 * This file is the ES-module entry point: it wires singletons together,
 * installs tool handlers, and owns the main() bootstrap. Larger pieces
 * have been split into sibling modules:
 *   - bootstrap.ts          — env parsing + auth token resolution
 *   - response-builders.ts  — safe response builders + sanitizer helpers
 *   - workflow-handlers.ts  — index-level tool handler overrides
 */
import { buildSafeCatchResponse, buildSafePlanWorkflowResponse } from "./response-builders.js";
export { buildSafeCatchResponse, buildSafePlanWorkflowResponse };
//# sourceMappingURL=index.d.ts.map