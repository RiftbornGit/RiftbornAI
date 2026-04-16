# RiftbornAI Networking School

**Read this track before creating or editing replication groups, replication-filter behavior, actor replication policy assumptions, or network-readiness audits.**

Networking is not just data movement. It is shared world truth:

- it decides who is authoritative
- it decides what each client must know
- it determines what costs bandwidth and update budget
- it must preserve fairness and coherence under latency

The current tool surface is strong for replication audits and Iris group control, but those tools only help when the authority model is clear first.

## Curriculum

1. **[01_Authority_Visibility_And_Network_Truth.md](01_Authority_Visibility_And_Network_Truth.md)** — Authority, relevance, prediction boundaries, and why networked state must have a clear owner.
2. **[02_UE5_Replication_Audit_And_Iris_Group_Systems.md](02_UE5_Replication_Audit_And_Iris_Group_Systems.md)** — Replication audits, actor-level replication inspection, Iris groups, and the current supported RiftbornAI networking surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common replication and grouping mistakes that waste bandwidth or corrupt shared truth.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for authority planning, replication audit, grouping, and runtime-proof planning.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for audits, actor replication inspection, and Iris group control.

## Core Rules

- Decide authority first.
- Replicate only what remote clients truly need.
- Inspect real replication state before guessing about cost.
- Use grouping to express relevance deliberately, not as a random optimization pass.
- Treat multiplayer truth as a design contract, not just a transport concern.
