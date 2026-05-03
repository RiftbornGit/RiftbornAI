## AI School Coverage Boundaries

This note explains where AI School coverage intentionally stops today.

The current AI School tracks are grounded in the live public tool surface. They teach craft, workflow, anti-patterns, and exact tool usage for domains that have a real callable lane in the repo.

Two design-system names still appear in [../DESIGN_SYSTEMS.md](../DESIGN_SYSTEMS.md):

- `GameDesignSystem`
- `AchievementDesignSystem`

Those names should not be treated as proof of a shipped authoring workflow.

## What Was Verified

The live public authoring surface was checked against:

- `Bridge/toolbook/public_surface.json`
- `Bridge/toolbook/contracts.json`
- the MCP readiness projection
- the generated MCP tool schema
- the registered C++ tool modules

Result:

- there is no grounded public `GameDesign` tool lane in the current shipped surface
- there is no grounded public `Achievement` or `trophy` authoring lane in the current shipped surface
- historical docs still mention `DesignToolsModule` and `ToolImpl_GameDesign`, but those names are not present in the current live source tree

## Operational Rule

Do not promise `GameDesignSystem` or `AchievementDesignSystem` workflows as callable RiftbornAI production lanes unless the exact tools can be verified in the live registry first.

Before claiming a planning or authoring route exists:

- use `list_all_tools`
- use `describe_tool`
- verify the tool is on the current public or otherwise intended callable surface

## What To Do Instead

For real project work, route through the concrete domain tracks that do have grounded authoring lanes:

- `LevelDesign` for space, flow, and blockouts
- `GAS` for abilities, effects, and attributes
- `UI` for widgets and HUD structure
- `Audio` for MetaSound and emitter placement
- `VFX` for Niagara design and verification
- `Localization` for String Tables and key design
- `SaveLoad`, `Networking`, `Input`, `Animation`, `Physics`, `Streaming`, `Cinematics`, and `CppArchitecture` as appropriate

If a future public route for `GameDesign` or `Achievements` is added, AI School should expand only after the tool surface is visible and verified.
