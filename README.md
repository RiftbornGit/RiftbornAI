<p align="center">
  <img src="Resources/Icon128.png" alt="RiftbornAI" width="96" />
</p>

<h1 align="center">RiftbornAI</h1>

<p align="center">
  <strong>AI game director for Unreal Engine 5.7</strong><br>
  Governed editor automation. MCP-native. Vision-verified.
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-BSL--1.1-orange.svg" alt="Business Source License 1.1" /></a>
  <img src="https://img.shields.io/badge/UE-5.7-black?logo=unrealengine" alt="UE 5.7" />
  <img src="https://img.shields.io/badge/status-beta-orange" alt="Beta" />
  <img src="https://img.shields.io/badge/MCP-compatible-green" alt="MCP Compatible" />
</p>

---

Describe what you want in plain English. RiftbornAI helps build it inside a live Unreal Engine editor — terrain, lighting, materials, blueprints, characters, foliage, cinematics, and more. It sees what it builds through a vision loop and iterates until available checks say the result is acceptable or a real blocker is found.

Shipped Beta builds expose a locked 99-tool surface for the core workflows while source builds and developer mode keep the broader production surface available.

Works with MCP clients that support stdio servers: **Claude Code**, **Cursor**, **VS Code**, **Windsurf**, or anything else that speaks [Model Context Protocol](https://modelcontextprotocol.io).
It also ships with a native **C++/Slate copilot panel inside Unreal Editor** — a dockable RiftbornAI tab with chat, plans, approvals, a toolbook, and execution history running directly in UE 5.7.

Current repository releases are source-available under Business Source License
1.1 with Apache 2.0 as the change license. Using RiftbornAI to build your own
games or client deliverables is allowed. Selling a competing RiftbornAI-based
plugin, hosted service, OEM package, or white-label offering requires a
separate commercial license.

<p align="center">
  <img src="images/forest1.png" alt="Forest environment built entirely by AI prompts — dense canopy, volumetric light, playable character" width="720" />
</p>

<p align="center">
  <img src="images/forest2.png" alt="Forest clearing with water, realistic grass and ground cover — all AI-generated from natural language" width="720" />
</p>

<p align="center"><em>Example scenes built through RiftbornAI's governed editor loop from natural-language direction — terrain, trees, grass, lighting, water, post-process, and playable character.</em></p>

## What It Does

**In-Editor Copilot** — Native C++/Slate Unreal Editor panel. Open **Window -> RiftbornAI** or use the Level Editor toolbar button to chat with RiftbornAI directly inside UE. Review plans, approve steps, browse tools, and inspect execution without leaving the editor.

**Environment Building** — Landscapes, terrain sculpting, multi-layer materials, weight-painted foliage, procedural grass, atmospheric fog, water bodies, post-process volumes.

**Lighting & Visuals** — Directional, point, spot, and rect lights. Sky atmosphere. Exponential height fog. Post-process color grading, bloom, vignette, DoF. Vision-verified: the AI screenshots the viewport and can adjust against the requested mood or readability target.

**Blueprint & Gameplay** — Blueprint creation, component wiring, variable/event setup, compilation. Character spawning with animation, input, and camera. Navigation meshes, AI perception, behavior trees, GAS abilities.

**Materials** — PBR material graphs from texture maps or parameters. Material instances. Landscape materials with LandscapeLayerBlend. Expression node wiring.

**Animation & Rigging** — IK rigs, retargeters, control rigs, blend spaces, animation montages, pose search databases for motion matching.

**Procedural** — PCG graphs, Niagara systems, procedural foliage spawners, dynamic mesh generation (box, sphere, cylinder, extrusion), CSG booleans, mesh mirroring.

**Cinematics** — Level sequences, camera animation, actor bindings, keyframing, playback.

**C++ Generation** — UE C++ scaffolding and editing for gameplay-facing classes such as Actors, Characters, Components, Controllers, Game Modes, and Subsystems, with build and compile verification.

## Quick Start

Get the latest release zip from
[GitHub Releases](https://github.com/RiftbornGit/RiftbornAI/releases).

### 1. Copy the plugin

```
YourProject/
  Plugins/
    RiftbornAI/    <-- extract the release zip here
  YourProject.uproject
```

### 2. Run the one-command installer

The release ships with the MCP server prebuilt. The installer:
1. **Enables RiftbornAI in your `.uproject`** — required because several of
   RiftbornAI's UE plugin dependencies (ChaosMover, LearningAgents,
   ContextualAnimation, MassCrowd, etc.) are experimental and not enabled
   by default. UE only cascade-enables them when their parent plugin is
   itself listed in the project.
2. Verifies the shipped `mcp-server/` runtime. In packaged beta releases this
  should pass immediately because the release now ships a runtime-ready tree
  (`dist/` plus production `node_modules/`). For source checkouts, the
  installer falls back to `npm install --omit=dev --ignore-scripts --no-audit --no-fund`.
3. Registers the MCP server with every supported MCP client
   (Claude Desktop, VS Code, Cursor, Windsurf).

On Windows, the plugin also checks GitHub Releases after startup settles,
downloads packaged updates in the background, and queues them to apply the
next time the editor closes by replacing the plugin folder, rerunning setup,
and relaunching the project.

**Windows (PowerShell):**

```powershell
Setup\install.ps1
```

If blocked by execution policy:
`PowerShell -ExecutionPolicy Bypass -File Setup\install.ps1`

**macOS / Linux:**

```bash
bash Setup/install.sh
```

See [`Setup/README.md`](Setup/README.md) for what the installer does and how
to uninstall.

### 3. Open your project in UE 5.7

The plugin loads automatically. Check the Output Log for:

```
RiftbornAI bridge started on port 8767
```

### 4. Open the in-editor copilot panel

Inside Unreal Editor, open **Window -> RiftbornAI** or click the RiftbornAI
button in the Level Editor toolbar. This opens the native dockable Slate
copilot panel that lives inside UE.

### 5. (Already done by the installer) — connect external MCP clients

The installer wrote a `riftborn` server entry into your global config for
Claude Desktop / VS Code / Cursor / Windsurf. Restart the client to pick
it up. If you need to configure it manually:

```json
{
  "mcpServers": {
    "riftborn": {
      "command": "node",
      "args": ["<absolute-path>/Plugins/RiftbornAI/mcp-server/dist/index.js"]
    }
  }
}
```

### 6. Start building

```
"Create a landscape with rolling hills, paint it with grass and rock layers"
"Add golden hour lighting with atmospheric fog"
"Spawn a third person character and make it playable"
"Place 200 trees across the terrain"
```

See [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) for the full walkthrough.

## Requirements

- **Unreal Engine 5.7** (free from Epic Games Launcher)
- **Node.js 18+** for the MCP server
- **Python 3.10+** for the bridge (optional, auto-detected)
- **An MCP client** (optional for external control) — Claude Code, Cursor, VS Code, Windsurf, or any MCP-compatible tool

## Architecture

RiftbornAI has two operator surfaces:
- **External MCP clients** such as Claude Code, Cursor, VS Code, and Windsurf
- **A native in-editor C++/Slate copilot tab** docked directly inside Unreal Editor

Both sit on the same governed Unreal-side tool and verification layer.

```
External MCP Client (Claude Code, Cursor, etc.)   In-Editor Slate Copilot Tab
                    │                                          │
                    ▼                                          ▼
          MCP Server (TypeScript)                    C++ Plugin (UE 5.7)
                    │                                   ├─ Slate Copilot UI
                    ▼                                   ├─ Tool Registry
          Python Bridge ─── HTTP/TCP                   ├─ Governance Kernel
                    │                                   └─ Unreal Editor APIs
                    └──────────────────────┬───────────────────────────────
                                           ▼
                                Live Unreal Engine Editor
```

- **C++ Plugin** — native Slate copilot UI, tool registration, editor modes, and governance kernel with HMAC-signed proof bundles
- **Python Bridge** — Tool dispatch, flight recorder, crash autopsy, config management
- **MCP Server** — TypeScript server exposing the curated production surface and broader gated schema set via Model Context Protocol
- **Vision Loop** — Screenshots the viewport, analyzes with AI, iterates until correct

## Documentation

| Document | Description |
|----------|-------------|
| [Docs Index](docs/README.md) | Canonical map of the documentation corpus |
| [Getting Started](docs/GETTING_STARTED.md) | First-run setup and a realistic initial workflow |
| [User Tutorial](docs/USER_TUTORIAL.md) | Broader user-facing capability guide, including silPOM and multi-domain workflows |
| [Architecture](docs/ARCHITECTURE_MAP.md) | System design and module map |
| [Open Source Boundary](docs/OPEN_SOURCE_BOUNDARY.md) | What is open, what stays in the Unreal plugin boundary, and what becomes future hosted premium |
| [Tool Catalog](TOOL_REGISTRY.md) | All registered tools with readiness levels |
| [Governance](docs/GOVERNANCE_AND_SECURITY.md) | Security model, proof bundles, exec tokens |
| [Changelog](CHANGELOG.md) | Release history |

## Project Structure

```
Source/           UE5 C++ plugin, tool registration, and in-editor copilot
Content/          UE assets and configs
Bridge/           Python bridge + toolbook
mcp-server/       TypeScript MCP server
docs/             Product and architecture docs
ci/               CI gates and validation
Config/           Plugin configuration
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines. Issues and PRs welcome.

## License

[Business Source License 1.1](LICENSE) with
[Apache License, Version 2.0](LICENSE.Apache-2.0) as the change license.

The current repository is source-available, not OSI open source. Production use
to build your own games and client deliverables is allowed. Competing hosted,
OEM, white-label, embedded, or resale offerings require a separate commercial
license.

See [EULA.md](EULA.md) for the licensing notice,
[COMMERCIAL_LICENSE.md](COMMERCIAL_LICENSE.md) for commercial-license guidance,
[docs/OPEN_SOURCE_BOUNDARY.md](docs/OPEN_SOURCE_BOUNDARY.md) for the
current repository boundary and future split targets, and
[TRADEMARKS.md](TRADEMARKS.md) for trademark usage policy.
