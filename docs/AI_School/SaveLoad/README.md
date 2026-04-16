# RiftbornAI Save / Load School

**Read this track before creating or editing level snapshots, scene checkpoints, world-state rollback flows, or save/load verification plans.**

Persistence is not just "save whatever changed." It is restore intent:

- it defines which state matters
- it defines what a valid restore means
- it protects iteration by making rollback reliable
- it must separate proof artifacts from a full player-facing save system

The current supported surface is strong for editor-world snapshots, scene checkpoints, and world-state digests. That is useful, but it is not the same as a complete runtime SaveGame authoring lane.

## Curriculum

1. **[01_Persistence_State_And_Restore_Intent.md](01_Persistence_State_And_Restore_Intent.md)** — State selection, continuity expectations, and why restoration needs explicit semantics.
2. **[02_UE5_Snapshot_Checkpoint_And_State_Digest_Systems.md](02_UE5_Snapshot_Checkpoint_And_State_Digest_Systems.md)** — Level Snapshots, scene checkpoints, state digests, and the current supported RiftbornAI save/load surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common persistence and restoration mistakes that make rollback unreliable or misleading.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for defining state, capturing snapshots or checkpoints, restoring, and verifying.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for snapshots, checkpoints, state digests, and restoration proof.

## Core Rules

- Define what state must survive before choosing a persistence tool.
- Distinguish rollback artifacts from player-facing save-system design.
- Use snapshots and checkpoints intentionally, not as a substitute for semantics.
- Capture proof of state before risky world changes.
- Always verify restoration instead of assuming the saved artifact is enough.
