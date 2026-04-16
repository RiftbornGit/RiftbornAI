# 05 — Tool Guide

Use exact registered level-design tool names. If a name is uncertain, verify with `describe_tool`.

## Level Design Tool Map

### Map And Structure Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create a new map | `create_level` | Establishes the level container |
| Save current map | `save_level` | Use after meaningful verified progress |
| Branch map to a new asset | `save_level_as` | Good for variant layout exploration |
| Set the runtime rule context | `set_gamemode_override` | Apply before gameplay validation |

Design rules:

- save after verified progress, not after every edit — cluttered save history obscures rollback points
- use `save_level_as` to branch exploratory layouts instead of overwriting the main map

### Blockout And Layout Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Generate a structured blockout | `generate_blockout` | Supports room, hallway, arena, tower, courtyard, and bridge layouts |
| Place one-off geometry or landmarks | `spawn_actor`, `spawn_actor_with_tags`, `create_static_mesh_actor` | Use deliberately, not as the default replacement for structured blockout |
| Plan dungeon rooms and corridors without commit | `get_dungeon_layout` | Safe planning step before spawning |
| Generate a dungeon layout into world geometry | `generate_dungeon` | Can also build navmesh and place basic lights |

Design rules:

- prefer `generate_blockout` when the shape is standard — arenas, hallways, courtyards have proven generators
- use `get_dungeon_layout` to plan before committing geometry to the world
- manual `spawn_actor` is for refinements and landmarks, not mass layout — it produces unpredictable spacing

### Navigation And Streaming Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Build navigation | `build_navmesh` | Re-run after meaningful blockout changes |
| Inspect navigation state | `get_navmesh_status` | Verify bounds volumes and agent-ready coverage |
| Inspect streaming scope | `get_world_partition_info` | Relevant for larger or partitioned maps |
| Add HLOD policy layer | `create_hlod_layer` | Late-stage scale support, not first-step layout design |

Design rules:

- rebuild navmesh after any change that affects traversal: moved walls, new cover, changed floor heights
- check navmesh status after building to confirm agent coverage, not just green build output
- HLOD is polish, not layout — do not reach for it during blockout

### Visual Verification Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Inspect the whole scene | `observe_ue_project` | Viewport plus actor census and analysis |
| Review one angle precisely | `capture_viewport_sync` | Use for repeatable angle checks |
| Move review camera | `set_viewport_location` | Build a multi-angle review loop |
| Review a target space directly | `look_at_and_capture` | Useful for entrances, landmarks, and cover clusters |
| Verify by movement and interaction | `run_quick_playtest` | Required after meaningful layout work |
| Build a review flythrough if needed | `create_level_sequence`, `add_sequence_track` | Support tool, not a substitute for gameplay validation |

Design rules:

- verify from player height, not bird's-eye — the player will never see the space from above
- use multiple camera angles for spatial understanding: entrance sightlines, cover lanes, landmark visibility
- playtest is the real verification — screenshots catch composition, movement catches flow

## Default Lanes

### Map And Blockout Lane

Use for:

- creating the level container
- generating initial layout
- placing essential blockout refinements

Main tools:

- `create_level`
- `save_level_as`
- `set_gamemode_override`
- `generate_blockout`
- `spawn_actor`
- `create_static_mesh_actor`
- `get_dungeon_layout`
- `generate_dungeon`

### Navigation And Verification Lane

Use for:

- proving traversal
- checking composition from gameplay-relevant views
- validating the layout in motion

Main tools:

- `build_navmesh`
- `get_navmesh_status`
- `observe_ue_project`
- `set_viewport_location`
- `capture_viewport_sync`
- `look_at_and_capture`
- `run_quick_playtest`

## Verification Checklist

Before calling a level layout done:

- [ ] blockout structure generated or placed (`generate_blockout`, `generate_dungeon`)
- [ ] game mode set for runtime context (`set_gamemode_override`)
- [ ] navmesh built and agent coverage verified (`build_navmesh`, `get_navmesh_status`)
- [ ] reviewed from multiple gameplay-relevant angles (`set_viewport_location`, `capture_viewport_sync`)
- [ ] landmarks, entrances, and cover spacing checked (`look_at_and_capture`)
- [ ] full scene observed for composition (`observe_ue_project`)
- [ ] movement through the space feels clear and intentional (`run_quick_playtest`)
- [ ] level saved after verified progress (`save_level`)

## Proven Sequences

### Arena Blockout

`create_level` -> `set_gamemode_override` -> `generate_blockout` -> `observe_ue_project` -> `build_navmesh` -> `get_navmesh_status` -> `run_quick_playtest`

Use for: combat arenas, boss rooms, PvP spaces — enclosed areas with clear boundaries, cover, and sightlines.

### Corridor Or Traversal Space

`generate_blockout` -> `set_viewport_location` -> `capture_viewport_sync` -> `build_navmesh` -> `run_quick_playtest`

Use for: hallways, tunnels, bridges, linear connector spaces between major rooms.

### Dungeon Planning Pass

`get_dungeon_layout` -> review room/corridor structure -> `generate_dungeon` -> `observe_ue_project` -> `build_navmesh` -> `run_quick_playtest`

Use for: multi-room dungeons, cave networks, procedural interiors with connectivity requirements.

### Existing Space Revision

`observe_ue_project` -> `look_at_and_capture` -> targeted `generate_blockout` or manual refinement -> `build_navmesh` -> `capture_viewport_sync` -> `run_quick_playtest`

Use for: reworking cover placement, adjusting sightlines, fixing flow problems in an existing level.

### Multi-Angle Layout Review

`set_viewport_location` (entrance A) -> `capture_viewport_sync` -> `set_viewport_location` (center) -> `capture_viewport_sync` -> `set_viewport_location` (entrance B) -> `capture_viewport_sync` -> compare all three

Use for: verifying landmark visibility, entrance readability, and compositional balance from key positions.

## Rules

1. Do not start with polish when the layout has not survived playtest.
2. Do not rely on manual ad hoc placement when `generate_blockout` already fits the task.
3. Do not skip navigation rebuilds after traversal-affecting changes.
4. Do not judge layout only from beauty shots or top-down abstraction.
5. Do not call a level done before movement through it feels clear and intentional.
6. Verify from player height, not from editor overhead camera.
7. Landmarks, sightlines, and cover spacing matter more than decorative detail at the blockout stage.

## Key Takeaway

The level-design surface is strongest when you separate:

- map setup
- blockout or layout planning
- navigation proof
- viewport review
- playtest verification

Trying to compress all of that into ad hoc placement usually produces pretty confusion.
