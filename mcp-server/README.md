# RiftbornAI MCP Server

Model Context Protocol (MCP) server that bridges external MCP clients to Unreal Engine via the RiftbornAI plugin.
The production boundary is a governed UE5 editor copilot for inspection and controlled scene automation. The raw generated schema count is not the same thing as the default customer-facing surface.

## Prerequisites

1. Unreal Engine 5.7 with RiftbornAI plugin loaded
2. Node.js 18+
3. Any MCP client that can launch a stdio server

## Installation

Binary beta releases ship the MCP server as a runtime-ready tree: `dist/` plus
production `node_modules/`. The installer self-tests that tree before it writes
any MCP config, so a shipped release should be usable without a networked npm
install.

Run the one-command installer at the plugin root (recommended):

```powershell
Setup\install.ps1   # Windows
```

```bash
bash Setup/install.sh   # macOS / Linux
```

To install manually instead:

```bash
cd mcp-server
npm install --omit=dev --ignore-scripts --no-audit --no-fund
```

Runtime verification:

```bash
node dist/index.js --self-test --json
```

Source rebuild (for plugin developers only — not needed for binary releases):

```bash
cd mcp-server
pnpm install
pnpm build
```

## Configuration

The repo ships editor-specific MCP config files such as `.vscode/mcp.json` so supported clients can auto-discover the MCP server when you open the plugin workspace.

If your game project is in a separate workspace, copy `.vscode/mcp.json` there and adjust the path:

```json
{
  "servers": {
    "riftborn": {
      "type": "stdio",
      "command": "node",
      "args": ["<path-to-plugin>/mcp-server/dist/index.js"]
    }
  }
}
```

## Environment Variables

- `RIFTBORN_HTTP_PORT`: HTTP bridge port (default: 8767)
- `RIFTBORN_TCP_PORT`: TCP bridge port (default: 8765)
- `RIFTBORN_HOST`: Bridge host (default: 127.0.0.1)
- `RIFTBORN_PLUGIN_PATH`: Override plugin root path (auto-detected)
- `RIFTBORN_TOOL_TIERS`: Tool tier filter (default: PRODUCTION)
- `RIFTBORN_ENABLE_INTERNAL_TOOLS`: Enable internal-only experimental tools (default: false)

## Capabilities

### Tools

The server merges manual tool definitions, vision helpers, and generated schemas, then filters them through the readiness gate. The default visible surface is the curated editor-copilot boundary, not the full generated schema set.

Shipped Beta builds clamp that visible surface to the locked 99-tool beta set; source builds and developer mode retain the broader production surface.

- **Project**: `list_assets`, `get_current_level`, `find_actor_by_label`
- **Editor Control**: `open_blueprint`, `start_pie`, `stop_pie`
- **Blueprints**: `create_blueprint`, `compile_blueprint`, `add_blueprint_variable`
- **Actors**: `spawn_actor`, `get_actor_info`, `move_actor`, `delete_actor`
- **Materials**: `create_material`, `create_material_instance`, `set_actor_material`
- **Animation**: `create_anim_blueprint`, `create_blend_space`, `play_animation_montage`
- **GAS**: `create_gameplay_ability`, `create_gameplay_effect`, `create_attribute_set`
- **VFX/Niagara**: `create_niagara_system`, `spawn_niagara_at_location`
- **Physics**: `set_physics_enabled`, `line_trace`, `add_physics_constraint`
- **Audio**: `create_sound_cue`, `play_sound_2d`, `play_sound_at_location`
- **Input**: `create_input_action`, `create_input_mapping_context`, `add_input_mapping`
- **Cinematics**: `create_level_sequence`, `add_sequence_track`, `open_sequence`
- **Lighting**: `create_light`, `set_component_property`, `set_post_process_settings`
- **PCG**: `create_pcg_graph`, `add_pcg_node`, `connect_pcg_nodes`
- **Console**: `execute_console_command`, `run_diagnostic`, `list_diagnostics`
- **Vision**: `analyze_scene_screenshot`, `observe_ue_project`, `look_at_and_capture`
- **Debugging**: `get_verification_status`, `diagnose_crash`

Internal-only experimental tools such as self-improvement and agent-job orchestration are excluded from production packaging unless `RIFTBORN_ENABLE_INTERNAL_TOOLS=true`.

### Resources (6 live + 2 templates)

Resources give MCP clients automatic context about your UE project:

- **Project Info**: `riftborn://project/info` — Engine version, game mode, player controller, current level
- **Level Actors**: `riftborn://project/actors` — All actors grouped by class with counts
- **Bridge Health**: `riftborn://bridge/health` — HTTP bridge connection and governance status
- **Asset Tree**: `riftborn://project/assets` — Top-level /Game/ folders and asset counts
- **Governance**: `riftborn://governance/status` — Session taint, proof mode, verification state
- **Tool Categories**: `riftborn://tools/categories` — Tool categories with counts

**Templates:**

- **Asset Details**: `riftborn://asset/{path}` — Details about any asset by path
- **Actor Details**: `riftborn://actor/{name}` — Details about any level actor by name

### Prompts (7 templates)

Reusable prompt templates for common tasks:

- **`create-ability`**: `ability_name`, `description`, `cost_type` — Full GAS ability with effect, cost, and cooldown
- **`fix-blueprints`**: `scope`, `dry_run` — Diagnose and repair broken Blueprints
- **`setup-character`**: `character_name`, `movement_type` — Character with mesh, animations, and input
- **`debug-issue`**: `problem` — Diagnose gameplay or editor issues
- **`create-game-mode`**: `mode_name`, `genre` — GameMode, GameState, PlayerController, and HUD scaffold
- **`add-vfx`**: `effect_type`, `target` — Niagara particle effects on actors or locations
- **`observe-scene`**: `focus` — Screenshot, vision analysis, and actor census

## How It Works

```text
External MCP Client <──MCP──> MCP Server <──HTTP──> RiftbornAI Plugin <──> Unreal Engine
                          (stdio)     (port 8767)  (C++ HTTP bridge)
```

The MCP server translates MCP tool calls into HTTP requests to the RiftbornAI HTTP bridge (port 8767), which routes them through the governance layer before executing in Unreal Engine.

## Troubleshooting

### "Bridge not responding"

- Make sure Unreal Editor is running with a project that has RiftbornAI plugin enabled
- Check: `curl http://localhost:8767/riftborn/health`

### "MCP server not discovered"

- Ensure `.vscode/mcp.json` exists and `dist/index.js` is built
- Restart your MCP client after adding the config
- Check your client logs for MCP-related errors

### "Tool execution timeout"

- The Python bridge might not be initialized
- Check Unreal Output Log for "Bridge: Server listening on port 8765"
- Try: `curl http://localhost:8767/riftborn/health`
