# 04 — Workflow

Level design should move from player intent to blockout to navigation proof to playtest.

## Phase 1: Define The Space Contract

Before creating geometry, answer:

- what is the space for?
- what is the player supposed to do here?
- is this a hub, arena, corridor, dungeon branch, traversal challenge, or reveal space?
- what is the desired pacing role?

If that is unclear, the geometry will drift into decorative guesswork.

## Phase 2: Create Or Branch The Map Container

Use:

- `create_level`
- `save_level_as`
- `set_gamemode_override`

This establishes the correct map boundary and runtime context before layout changes pile up.

## Phase 3: Choose The Layout Authoring Strategy

### Use Structured Blockout When It Fits

Use `generate_blockout` for:

- rooms
- hallways
- arenas
- towers
- courtyards
- bridges

This is usually the fastest way to test footprint, entrances, verticality, and circulation.

### Use Planning-First Dungeon Layout

For dungeon-style spaces:

1. `get_dungeon_layout`
2. inspect whether room/corridor rhythm is promising
3. `generate_dungeon` only when ready to commit

### Use Manual Placement Deliberately

Use:

- `spawn_actor`
- `spawn_actor_with_tags`
- `create_static_mesh_actor`

only when the structured blockout tools do not match the required shape or when refining a proven layout.

## Phase 4: Observe And Revise From Gameplay Angles

Use:

- `observe_ue_project` — full scene census and viewport screenshot
- `set_viewport_location` — position camera at player height near key spaces
- `capture_viewport_sync` — capture specific angles for comparison
- `look_at_and_capture` — review landmarks, entrances, and cover setups directly

Check from each major entrance:

- can the player see a landmark or destination from the threshold?
- is the critical-path legible without UI markers?
- is cover spacing appropriate for the expected engagement range?
- are there dead zones — oversized emptiness with no purpose?
- do corridor widths feel traversable or claustrophobic?

Build a multi-angle review loop:

```
set_viewport_location (entrance A, player height)
capture_viewport_sync
set_viewport_location (center arena, player height)
capture_viewport_sync
set_viewport_location (entrance B, player height)
capture_viewport_sync
compare: landmarks visible? sightlines fair? scale readable?
```

This is the cheapest place to catch layout mistakes. Revision here costs minutes; revision after decoration costs hours.

## Phase 5: Prove Navigation

Use:

- `build_navmesh`
- `get_navmesh_status`

Do this after meaningful blockout changes, especially when:

- ramps or stairs were added or moved
- doorway or corridor widths shifted
- openings were sealed or created
- combat cover or boundaries moved
- floor height changed in any connected space

After build, check `get_navmesh_status` for:

- agent type coverage (capsule radius vs corridor width)
- navmesh bounds actually enclosing the play area
- disconnected islands that would strand AI

A navmesh that builds without errors does not mean it covers the intended space. Verify coverage explicitly.

## Phase 6: Consider Large-Map Structure

If the layout scope grows:

- inspect `get_world_partition_info`
- create `create_hlod_layer` only when the map shape justifies streaming/aggregation thinking

Do not lead with optimization tools before the play space is worth optimizing.

## Phase 7: Playtest The Space

Use:

- `run_quick_playtest`

Look for:

- hesitation at route choices — if the player pauses, the landmark failed
- weak or unfair sightlines — one position dominates with no counterplay
- combat spaces that feel too open or too cramped for the intended engagement
- traversal that looks valid but feels slow or awkward
- pacing stretches with no meaningful change in spatial character
- jump or drop opportunities that navmesh allows but the player cannot read

Playtest is verification, not decoration. If something feels wrong in movement, the blockout is the problem, not the art.

## Recommended Sequences

### New Arena Or Courtyard

`create_level` -> `set_gamemode_override` -> `generate_blockout` -> `observe_ue_project` -> `set_viewport_location` -> `capture_viewport_sync` -> `build_navmesh` -> `get_navmesh_status` -> `run_quick_playtest` -> `save_level`

### New Dungeon Space

`create_level` -> `get_dungeon_layout` -> review route rhythm -> `generate_dungeon` -> `observe_ue_project` -> `build_navmesh` -> `run_quick_playtest` -> `save_level`

### Existing Layout Revision

`observe_ue_project` -> `look_at_and_capture` on problem areas -> `generate_blockout` refinement or deliberate manual placement -> `build_navmesh` -> `capture_viewport_sync` from revised angles -> `run_quick_playtest`

## Key Takeaway

The safe order is:

space purpose first, blockout second, navigation third, playtest fourth, polish later.

If you reverse that order, layout problems get more expensive every hour.
