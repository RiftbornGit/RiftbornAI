# RiftbornAI — Frequently Asked Questions

## Setup

### "Bridge disconnected" / "Connection refused on port 8767"

The Unreal editor isn't running, or the RiftbornAI plugin failed to load.

1. Open the UE project. The plugin loads automatically at editor start.
2. Check the **Output Log** for `RiftbornAI bridge started on port 8767`.
3. If you don't see it, search the log for `LogRiftbornAI` errors.
4. Common cause: another instance of UE is already on port 8767 — close it.

### `Setup\install.ps1` is blocked by execution policy

Run it via:

```powershell
PowerShell -ExecutionPolicy Bypass -File Setup\install.ps1
```

Or unblock once:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

### Installer says "Node.js NOT FOUND"

Install [Node.js 18 or newer](https://nodejs.org/) and re-run the installer.
Node is required for the MCP server that connects RiftbornAI to MCP clients
(Claude Desktop, VS Code, Cursor, Windsurf). It's not used at runtime by UE
itself — the editor plugin runs without it.

### My MCP client doesn't see RiftbornAI tools

1. Restart the MCP client after running `Setup/install.ps1` — config changes
   only apply on next launch.
2. Re-run the installer to confirm the registration step succeeded:
   `node Scripts/install-mcp.mjs`
3. Verify `mcp-server/node_modules/` exists. If not:
   `cd mcp-server && npm install --production`
4. Confirm UE is running and the bridge is up (see "Bridge disconnected" above).
5. Open your client's MCP debug log to see whether the `riftborn` server
   started and what error (if any) it reported.

### Where does the installer write MCP client config?

| Client | Path (Windows) | Path (macOS) |
|---|---|---|
| Claude Desktop | `%APPDATA%\Claude\claude_desktop_config.json` | `~/Library/Application Support/Claude/claude_desktop_config.json` |
| VS Code | `%APPDATA%\Code\User\mcp.json` | `~/.config/Code/User/mcp.json` |
| Cursor | `%USERPROFILE%\.cursor\mcp.json` | `~/.cursor/mcp.json` |
| Windsurf | `%USERPROFILE%\.codeium\windsurf\mcp_config.json` | `~/.codeium/windsurf/mcp_config.json` |

The installer adds a single `riftborn` server entry and leaves any other
servers untouched.

### How do I uninstall?

```powershell
node Scripts\install-mcp.mjs --uninstall    # Windows
```

```bash
node Scripts/install-mcp.mjs --uninstall    # macOS / Linux
```

Then delete the `Plugins/RiftbornAI/` folder from your project.

---

## LLM providers

### Which LLM does RiftbornAI use?

You bring your own. Supported providers:

| Provider | What you need |
|---|---|
| **Claude** (Anthropic) | API key from console.anthropic.com |
| **GPT** (OpenAI) | API key from platform.openai.com |
| **Gemini** (Google) | API key from aistudio.google.com |
| **Ollama** (local) | Ollama installed locally — no key, no cost, no cloud |

Configure your choice in `Config/UserRiftbornAI.ini` (read environment
variables by default — see [`docs/LLM_PROVIDERS.md`](LLM_PROVIDERS.md)).

### How much will the LLM cost me?

It depends on how much you build. Rough order of magnitude for a focused
session against Claude 3.5 Sonnet:

| Activity | Approx tokens | Approx cost |
|---|---|---|
| Build a small landscape with foliage and lighting | 50K–200K | $0.15–$0.60 |
| Build a playable arena from scratch (terrain, walls, lighting, character, navmesh) | 200K–800K | $0.60–$2.40 |
| Generate a single material from a prompt | 5K–20K | $0.02–$0.06 |

Local Ollama runs are free but slower and lower quality on small models.

### Why Ollama if cloud is cheap?

Privacy. Some studios cannot send code or asset descriptions to a third-party
API. Ollama runs entirely on your machine — RiftbornAI ships with provider
support for it out of the box.

---

## Tools & capabilities

### How many tools does RiftbornAI have?

About 500 built-in tools across environment building, lighting, materials,
Blueprints, characters, animation, PCG, Niagara, sequencer, and more. Some
are gated on optional UE plugins — RiftbornAI hides them automatically when
the underlying module isn't enabled.

### How do I add my own tools?

See [`docs/TOOL_AUTHORING.md`](TOOL_AUTHORING.md). Short version: write a
companion `.uplugin` that depends on `RiftbornAI`, register your tool with
`FClaudeToolRegistry::Get().RegisterTool(...)` in `StartupModule()`, ship.
The reference implementation is at
[`Examples/RiftbornAI-ExampleTool/`](../Examples/RiftbornAI-ExampleTool).

### Can it modify Blueprint visual scripts?

Yes — Blueprint creation, component wiring, variable/event setup, node
graph editing, compilation. See `docs/USER_TUTORIAL.md` for examples.

### Does it work in packaged builds (shipped games)?

No. RiftbornAI is editor-only — it loads in the Unreal Editor and is
excluded from cooked / packaged game builds. The AI work happens at
authoring time; the assets it creates ship like any other UE asset.

---

## Performance & system requirements

### What hardware do I need?

If you can run the UE 5.7 editor with a non-trivial project, you can run
RiftbornAI. The plugin adds ~50 MB to editor RAM and an HTTP server on
localhost:8767. Tool calls are sub-second for simple operations; vision
analysis of viewport screenshots takes 1–5 seconds depending on provider.

### Will it slow down the editor?

In idle: no — the bridge thread is at-rest. During an active agent session:
expect normal editor responsiveness; tool calls run on the game thread and
are gated by frame budget. Vision capture briefly stalls the renderer
(~50 ms per screenshot).

### What about my project's existing assets / Blueprints?

RiftbornAI never deletes or overwrites without an explicit confirmation.
Mutating tools (anything beyond read-only inspection) go through the
governance pipeline: risk-tiered policy, undo strategy declared at
registration, optional confirmation tokens for destructive operations. See
[`docs/GOVERNANCE_AND_SECURITY.md`](GOVERNANCE_AND_SECURITY.md).

---

## Beta-specific

### Is this stable enough for a serious project?

For experimentation and prototyping, yes. For a project on a hard deadline,
keep your project under source control and snapshot before long agent
sessions — that's good practice with any AI tool.

### What's the upgrade path from this beta?

On Windows, RiftbornAI now checks GitHub Releases after editor startup
settles, downloads the packaged release zip in the background, and queues the
updater automatically. The next time the editor closes, RiftbornAI replaces
the plugin folder, reruns `Setup/install.ps1`, and relaunches the project.

Manual fallback still works: drop the new release zip into
`Plugins/RiftbornAI/`, replacing the old folder, then rerun
`Setup/install.ps1` if `mcp-server/` changed.

### Where do I report bugs?

GitHub Issues: https://github.com/RiftbornGit/RiftbornAI/issues

Include: UE editor version, your OS, the prompt you sent, the Output Log
line that errored, and (if possible) the proof-bundle ID printed in the log
for the failing tool call.

### Can I get the source?

Not as part of this beta. The plugin ships as a binary-only release with
public SDK headers for tool authoring. Source-access licensing is available
under separate commercial terms — see [`COMMERCIAL_LICENSE.md`](../COMMERCIAL_LICENSE.md).
