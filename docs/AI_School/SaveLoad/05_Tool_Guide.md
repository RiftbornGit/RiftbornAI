# 05 - Tool Guide

Use exact registered save/load tool names. If a name is uncertain, verify with `describe_tool`.

## Save / Load Tool Map

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Capture a UE Level Snapshot | `capture_level_snapshot` | Main editor-world snapshot lane |
| Restore a UE Level Snapshot | `restore_level_snapshot` | Recovery lane for previously captured snapshots |
| List available snapshots | `list_level_snapshots` | Useful before choosing a restore target |
| Save a richer checkpoint bundle | `save_scene_checkpoint` | Includes snapshot, viewport, optional screenshot, digest, and performance evidence |
| Restore a checkpoint bundle | `restore_scene_checkpoint` | Can restore the scene and optionally reload the level and viewport |
| Capture a stable world-state summary | `get_world_state_digest` | Good evidence lane before or after risky edits |
| Save the current map | `save_level`, `save_level_as` | Useful to stabilize authored state around checkpoint capture |
| Save current assets | `save_asset`, `save_dirty_assets` | Keeps asset state consistent around persistence operations |
| Load the current map explicitly | `load_level` | Useful when restoration or comparison needs a specific level load |

## Default Lanes

### Snapshot Lane

Use for:

- editor-world rollback
- branching authored state

Main tools:

- `capture_level_snapshot`
- `restore_level_snapshot`
- `list_level_snapshots`

### Checkpoint Lane

Use for:

- richer restore bundles
- proof-heavy iteration flows

Main tools:

- `save_scene_checkpoint`
- `restore_scene_checkpoint`
- `get_world_state_digest`

### Supporting Save State Lane

Use for:

- stabilizing the world and assets around capture

Main tools:

- `save_level`
- `save_level_as`
- `save_asset`
- `save_dirty_assets`
- `load_level`

## Proven Sequences

### Snapshot Rollback

`save_level` -> `capture_level_snapshot` -> mutate -> `restore_level_snapshot`

### Rich Checkpoint Pass

prepare state -> `save_scene_checkpoint` -> mutate -> `restore_scene_checkpoint`

### Evidence Capture

`get_world_state_digest` -> `save_scene_checkpoint`

## Rules

- Do not call a snapshot a full save system.
- Do not restore without verification.
- Do not skip state-digest evidence when rollback confidence matters.
- Do not confuse map saving with restore semantics.
- Do not make persistence claims stronger than the current tool surface supports.

## Key Takeaway

The save/load surface is strongest when you separate:

- restore intent
- snapshot capture
- checkpoint capture
- state evidence
- restoration proof

If those are blurred together, persistence becomes ambiguous fast.
