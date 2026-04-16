# Getting Started with RiftbornAI

RiftbornAI lets an AI help build Unreal Engine games alongside you. You describe what you want in plain English, and it can create terrain, place lights, spawn actors, edit Blueprints, and playtest — all inside a live UE5 editor session.

This guide gets you from zero to a realistic first RiftbornAI session with the shipped local product.

## What You Need

- **Unreal Engine 5.7** (free from Epic Games Launcher)
- **Node.js 18+** ([nodejs.org](https://nodejs.org))
- **An MCP client** — Claude Code, Cursor, VS Code, Windsurf, or any MCP-compatible tool
- **A UE5 project** — any project, even the blank template

## Install (3 minutes)

### Step 1: Copy the plugin

Copy the `RiftbornAI` folder into your project's `Plugins/` directory:

```
YourProject/
  Content/
  Plugins/
    RiftbornAI/    <-- copy here
  YourProject.uproject
```

### Step 2: Open your project in UE5

The plugin loads automatically. Check the Output Log (`Window > Developer > Output Log`) for:

```
RiftbornAI bridge started on port 8767
```

If you see that, the plugin is running and waiting for commands.

### Step 3: Choose your front door

You can use RiftbornAI in either of these ways:

- **Built-in copilot panel inside Unreal Editor** — open **Window -> RiftbornAI**
- **External MCP client** — build and connect the MCP server

If you only want the in-editor copilot, you can stop here and open the panel.

### Step 4: Run the one-command installer (for external MCP clients)

The release ships with the MCP server as a runtime-ready tree. The installer
first verifies the shipped runtime and registers it with every supported MCP
client. If you are running from a source checkout instead of a packaged beta
release, it falls back to installing production runtime dependencies locally.

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

See [`Setup/README.md`](../Setup/README.md) for what the installer does and
how to uninstall.

### Step 5: Connect your AI client

**Claude Code** — add to your project's `.mcp.json`:

```json
{
  "mcpServers": {
    "riftborn": {
      "command": "node",
      "args": ["Plugins/RiftbornAI/mcp-server/dist/index.js"]
    }
  }
}
```

**Cursor** — add the same config to `.cursor/mcp.json`.

**VS Code** — add to `.vscode/mcp.json`:

```json
{
  "servers": {
    "riftborn": {
      "type": "stdio",
      "command": "node",
      "args": ["${workspaceFolder}/Plugins/RiftbornAI/mcp-server/dist/index.js"]
    }
  }
}
```

That is enough for the external-client path. If the editor is running and the bridge is healthy, the AI can now inspect and control your Unreal Editor through governed tools.

## Your First Scene (5 minutes)

Open your AI client and try these in order. Each command builds on the previous one.

### 1. Look around

> "What's in my level right now?"

The AI takes a viewport screenshot, enumerates scene state, and describes what it sees. This is the observation loop that gives the agent grounded context inside your editor.

### 2. Create terrain

> "Create a landscape with rolling hills, grass, and dirt paths"

This creates a 500m landscape with a multi-layer material (grass, dirt, rock), automatic grass placement via Landscape Grass Type, and sculpted terrain. The AI uses the bottom-up workflow: terrain first, then material, then foliage.

### 3. Add lighting

> "Light this scene for golden hour — warm sun, soft shadows, atmospheric fog"

The AI creates a directional light (sun), sky light, exponential height fog, and sky atmosphere. It can take screenshots after changes to verify the mood or readability against your request.

### 4. Place some trees

> "Scatter 50 oak trees across the hills, denser near the valleys"

Uses the foliage system to paint trees with density falloff based on terrain slope.

### 5. Add a playable character

> "Create a third-person character I can play as"

Spawns a character with skeletal mesh, animation blueprint, camera boom, and input bindings. Sets it as the default pawn.

### 6. Playtest

> "Start a playtest and tell me what you see"

The AI starts Play in Editor, observes the viewport, identifies issues such as lighting problems, collision gaps, or setup blockers, stops PIE, and reports findings. If PIE is blocked by compile errors or modals, RiftbornAI should surface that instead of pretending the playtest succeeded.

### 7. Save

> "Save the level"

## Skills (Slash Commands)

If your client or prompt library exposes slash commands, these are common workflow wrappers. They are convenience entrypoints, not the only way to use RiftbornAI.

| Command | What It Does |
|---------|-------------|
| `/observe` | Screenshot + actor census + AI vision analysis |
| `/create-terrain` | Landscape + material + layers + automatic grass |
| `/create-forest` | Full environment: terrain, grass, trees, fog, atmosphere |
| `/light-scene` | Analyze scene and add directional, sky, fog, post-process |
| `/polish` | Color grading, bloom, fog tuning, vignette |
| `/create-character` | Character: mesh, animation, input, camera, make playable |
| `/build-arena` | Playable arena: floor, walls, cover, lights, navmesh, spawn points |
| `/playtest` | Start PIE, observe gameplay, report issues |
| `/create-material` | PBR material from description |
| `/create-blueprint` | Blueprint with components, variables, events |
| `/checkpoint` | Save scene state + screenshot for later restore |

## How It Works

```
You (natural language) → AI Client → MCP Server → HTTP Bridge → UE5 Editor
                                                                    ↓
You (see changes)     ← AI Client ← MCP Server ← HTTP Bridge ← Tool Result
```

RiftbornAI exposes a large governed tool surface spanning:

- **Vision** — take screenshots, analyze scenes, observe with AI eyes
- **Actors** — spawn, move, rotate, scale, delete, duplicate, find
- **Terrain** — create landscapes, sculpt, paint layers, add grass
- **Materials** — create PBR materials, instances, set parameters
- **Lighting** — directional, point, spot, sky, post-process volumes
- **Blueprints** — create, add components/variables/events, compile
- **Characters** — create from template, set mesh, make playable
- **Testing** — start/stop PIE, line trace, run diagnostics

Every mutating tool goes through a governance system with signed execution tokens, risk tier classification, and audit logging.

## Vision AI (Optional)

The AI can *see* your scene through screenshots. To enable vision analysis, set one of these:

```bash
# OpenAI (recommended for vision)
export OPENAI_API_KEY=sk-...

# Anthropic
export ANTHROPIC_API_KEY=sk-ant-...

# Ollama (free, local, no internet required)
ollama pull llava
# Auto-detected at localhost:11434
```

Without a vision provider, the AI can still control the editor — it just can't analyze screenshots.

## Troubleshooting

**"Bridge disconnected"** — UE5 isn't running, or the plugin didn't load. Check Output Log for errors.

**No tools showing in an external client** — Re-run the installer (`Setup/install.ps1` or `Setup/install.sh`) and restart your MCP client. If you installed manually, confirm the MCP server's `node_modules/` exists (run `npm install --production` inside `mcp-server/`) and that your client config points at `mcp-server/dist/index.js`.

**"Unknown tool"** — The tool might be internal-only, beta-gated, or absent from the current build. Check `list_all_tools`, `describe_tool`, or the readiness docs before assuming it should exist.

**Vision returns nothing** — You need an API key (OpenAI, Anthropic) or local Ollama running.

**Health check:**

```bash
curl http://127.0.0.1:8767/riftborn/health
```

Should return `{"ok": true, ...}` when the editor is running.

## Next Steps

- Read [AI School](AI_School/01_Nature_And_Ecosystems.md) before building environments — it teaches the bottom-up workflow
- Check [ARCHITECTURE_MAP.md](ARCHITECTURE_MAP.md) to understand the system
- Use the root [TOOL_REGISTRY.md](../TOOL_REGISTRY.md) for the current tool list and [docs/README.md](README.md) for the canonical documentation map
