# 02 - UE5 Snapshot Checkpoint And State Digest Systems

RiftbornAI's current save/load surface is centered on Level Snapshots, scene checkpoints, and world-state digests.

## Level Snapshots Capture Editor World State

Use:

- `capture_level_snapshot`
- `restore_level_snapshot`
- `list_level_snapshots`

for the main snapshot lane.

This is the right path when the need is:

- branch authored world states
- restore a previous editor-world configuration
- inspect available snapshot artifacts

## Scene Checkpoints Are Richer Restore Bundles

Use:

- `save_scene_checkpoint`
- `restore_scene_checkpoint`

when the goal is a more complete restorable package.

These checkpoints can include:

- level snapshot data
- viewport state
- optional screenshot evidence
- world-state digest
- performance snapshot

That makes them strong for iteration, rollback, and proof bundles.

## World-State Digests Provide Stable Evidence

Use:

- `get_world_state_digest`

to capture a typed summary and hash of current world state.

This is valuable for:

- understanding what changed
- recording evidence before risky edits
- comparing restore expectations against reality

## Important Reality Check

This surface is strong for:

- editor-world state capture
- restoration
- checkpointing
- proof artifacts

It is not, by itself, a full player-facing runtime SaveGame authoring system.

If the task is true gameplay-slot persistence, that likely still depends on Blueprint or C++ implementation beyond the currently exposed save/load tools.

## Supporting Save And Load Operations Still Matter

Use surrounding project-state tools like:

- `save_level`
- `save_level_as`
- `load_level`
- `save_asset`
- `save_dirty_assets`

when the world or assets need to be saved consistently around the checkpoint flow.

## Key Takeaway

Treat the current save/load lane as:

snapshot or checkpoint capture -> digest or evidence -> restore verification

That keeps the persistence claims honest and useful.
