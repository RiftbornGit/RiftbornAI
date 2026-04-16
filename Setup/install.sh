#!/usr/bin/env bash
# RiftbornAI — one-command install for macOS / Linux.
# Run once after dropping the plugin into <YourProject>/Plugins/.

set -euo pipefail

PLUGIN_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"

echo ""
echo "=== RiftbornAI Setup ==="
echo "Plugin root: $PLUGIN_ROOT"
echo ""

# Step 1 — Node check
if ! command -v node >/dev/null 2>&1; then
    echo "[1/3] Node.js NOT FOUND."
    echo "      Install Node.js 18+ from https://nodejs.org and re-run this script."
    exit 1
fi
echo "[1/3] Node.js detected: $(node --version)"

# Step 2 — MCP server runtime deps
# Step 2 — MCP server runtime
echo "[2/3] Verifying MCP server runtime..."
MCP_DIR="$PLUGIN_ROOT/mcp-server"
if [ ! -d "$MCP_DIR" ]; then
    echo "      mcp-server/ not found at $MCP_DIR"
    exit 1
fi

MCP_ENTRY="$MCP_DIR/dist/index.js"
test_mcp_runtime() {
    if [ ! -f "$MCP_ENTRY" ]; then
        return 1
    fi
    RIFTBORN_AUTH_TOKEN="" RIFTBORN_API_KEY="" RIFTBORN_DEV_TOKEN="" \
    RIFTBORN_ALLOW_UNAUTHENTICATED_LOCAL="1" RIFTBORN_HOST="127.0.0.1" \
        node "$MCP_ENTRY" --self-test --json >/dev/null
}

if test_mcp_runtime; then
    echo "      Shipped MCP runtime already ready — skipping npm install."
else
    echo "      MCP runtime incomplete — installing production dependencies..."
    (cd "$MCP_DIR" && npm install --omit=dev --ignore-scripts --no-audit --no-fund)
    if ! test_mcp_runtime; then
        echo "      MCP server self-test failed after runtime install."
        exit 1
    fi
fi

# Step 3 — Register with MCP clients
echo "[3/3] Registering MCP server with installed clients..."
INSTALLER="$PLUGIN_ROOT/Scripts/install-mcp.mjs"
if [ ! -f "$INSTALLER" ]; then
    echo "      Scripts/install-mcp.mjs not found — cannot register automatically."
    echo "      Consult docs/GETTING_STARTED.md for manual configuration."
    exit 1
fi
node "$INSTALLER"

echo ""
echo "Done. Restart Claude Desktop / VS Code / Cursor / Windsurf to pick up the change."
echo "Open your Unreal project — the RiftbornAI tab appears under Window menu."
