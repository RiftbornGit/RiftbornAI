# 02 — UE5 Level And Blockout Systems

This repo has a real level-design lane. Use it deliberately.

## Level Creation And Save Boundaries

Core map tools include:

- `create_level`
- `save_level`
- `save_level_as`
- `set_gamemode_override`

Use these to establish the level container and the intended runtime rules before piling on layout work.

## Blockout Has A Dedicated Tool

`generate_blockout` is a production-tier blockout tool.

It supports:

- room
- hallway
- arena
- tower
- courtyard
- bridge

It also supports structured features such as:

- openings
- arches
- ramps
- stairs
- columns
- multi-level tower-style blockouts

This matters because blockout should be faster and more intentional than manually spamming generic geometry every time.

## Manual Placement Still Matters

You still have:

- `spawn_actor`
- `spawn_actor_with_tags`
- `create_static_mesh_actor`

Use these when:

- the space needs one-off geometry or landmark pieces
- the blockout tool does not fit the required shape
- you are refining a proven layout with deliberate additions

Do not fall back to ad hoc placement as the default if a structured blockout tool already matches the job.

## Dungeon Planning Is Split Cleanly

For dungeon-style layouts, the surface distinguishes planning from commitment.

Use:

- `get_dungeon_layout` to generate rooms and corridors without spawning geometry
- `generate_dungeon` when you are ready to commit layout into world geometry and optional lights/navmesh

That split is important. Planning should happen before spawn-heavy commitment.

## Navigation Is A Design Surface

Traversal and AI route validity are not separate from layout quality.

Use:

- `build_navmesh`
- `get_navmesh_status`

These prove whether the blockout supports real movement rather than only looking navigable from above.

## Observation Is Part Of Level Design

You also have a strong verification loop:

- `observe_ue_project`
- `capture_viewport_sync`
- `look_at_and_capture`
- `set_viewport_location`
- `run_quick_playtest`

This means you can check:

- the overall composition
- route visibility from multiple angles
- whether entrances read clearly
- whether cover density feels right
- whether the level works in motion, not just in plan view

## World Partition And Streaming Matter At Larger Scale

For larger maps or streaming-sensitive spaces, use:

- `get_world_partition_info`
- `create_hlod_layer`

These are not first-step blockout tools, but they matter when the level scope grows beyond a single contained arena or room set.

## Cinematic Review Can Support Layout Review

`create_level_sequence` and `add_sequence_track` are not level-design tools by default, but they can help when you need a repeatable review flythrough or reveal pass for a layout.

Use them as support, not as a replacement for playtest.

## Key Takeaway

The repo's level-design surface has four layers:

1. map container setup
2. structured blockout or layout planning
3. navigation and movement proof
4. viewport and playtest verification

Treat all four as part of the job.
