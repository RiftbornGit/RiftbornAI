# RiftbornAI — First-Run Setup

One-command installers for a fresh RiftbornAI install.

**Windows (PowerShell):**

```powershell
Setup\install.ps1
```

If the script is blocked by execution policy, either:

```powershell
PowerShell -ExecutionPolicy Bypass -File Setup\install.ps1
```

or unblock once with `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`.

**macOS / Linux:**

```bash
bash Setup/install.sh
```

## What it does

1. Checks Node.js 18+ is available.
2. Verifies the shipped `mcp-server/` is already runtime-ready. In binary beta
   releases this should pass immediately because the package now ships with
   production `node_modules/` included. If you're installing from a source tree
   instead, the installer falls back to `npm install --omit=dev` to materialize
   runtime dependencies.
3. Registers the MCP server in the user-global config of every supported
   client: **Claude Desktop**, **VS Code**, **Cursor**, **Windsurf**. Existing
   entries for other servers are left untouched.

After it finishes, restart your MCP client and open your Unreal project.
The RiftbornAI tab appears under the editor's Window menu, and the AI tools
show up in your MCP client's tool list.

## Uninstall

```powershell
node Scripts\install-mcp.mjs --uninstall    # Windows
```

```bash
node Scripts/install-mcp.mjs --uninstall    # macOS / Linux
```

Removes the RiftbornAI entries from every MCP client config, leaves the rest
in place.
