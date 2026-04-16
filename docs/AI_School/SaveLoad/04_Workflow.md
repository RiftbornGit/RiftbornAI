# 04 - Workflow

Save/load work should move from restore intent to capture to restoration proof.

## Phase 1: Define The Restore Goal

Before touching a persistence tool, answer:

- what state must survive
- who needs the restore
- whether this is rollback, checkpointing, or a player-facing persistence plan

## Phase 2: Prepare The World State

Use:

- `save_level`
- `save_level_as`
- `save_asset`
- `save_dirty_assets`

when the current authored state needs to be made consistent before capture.

## Phase 3: Choose The Capture Lane

### Snapshot Lane

Use:

- `capture_level_snapshot`

when the goal is a UE Level Snapshot artifact.

### Checkpoint Lane

Use:

- `save_scene_checkpoint`

when the goal is a richer restorable package with more evidence.

## Phase 4: Capture State Evidence

Use:

- `get_world_state_digest`
- `list_level_snapshots`

to record or inspect the available proof of state.

## Phase 5: Restore And Verify

Use:

- `restore_level_snapshot`
- `restore_scene_checkpoint`
- `load_level` when appropriate

to restore the chosen artifact and verify that the intended state is actually back.

## Recommended Sequences

### Risky World Edit Rollback

`save_level` -> `capture_level_snapshot` -> risky change -> `restore_level_snapshot` if needed -> verify

### Rich Iteration Checkpoint

prepare current state -> `save_scene_checkpoint` -> mutate -> `restore_scene_checkpoint` if needed

### State Evidence Pass

`get_world_state_digest` -> `save_scene_checkpoint`

## Key Takeaway

The safe order is:

define the restore intent first, capture second, restore and verify third.

If you skip the first step, persistence artifacts accumulate without meaning.
