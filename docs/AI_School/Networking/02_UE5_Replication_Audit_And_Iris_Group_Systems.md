# 02 - UE5 Replication Audit And Iris Group Systems

RiftbornAI's current networking surface is centered on safe replication audits, actor-level replication inspection, and Iris replication-group control.

## Project-Wide Replication Audit Exists

Use:

- `audit_net_replication`

to inspect replicated actor classes across the project.

This audit reports:

- estimated bandwidth-heavy classes
- large replicated data risks
- review points around replication conditions

It is static and safe.
It does not replace full runtime behavior testing, but it is the right first inspection step.

## Actor-Level Replication State Is Inspectable

Use:

- `inspect_actor_replication`

to inspect a specific actor's replication and Iris handle state.

This is the lane for checking what the system is actually doing for a concrete actor rather than relying on stale assumptions.

## Iris Replication Groups Are A Real Control Surface

Use:

- `create_replication_group`
- `add_actor_to_replication_group`
- `set_replication_group_filter_status`

to control relevance groups explicitly.

This is the current supported lane for:

- creating named replication groups
- assigning actors into those groups
- allowing or disallowing them through filter status

## Important Reality Check

These group tools only make sense when a valid replication system is active for the current world or PIE/net context.

That means:

- do not plan around them abstractly without a real network context
- do not treat group control as a substitute for authority design

## Current Strength Of The Surface

The networking tool surface is strongest for:

- project-wide replication inspection
- actor-level replication inspection
- Iris grouping and filter control

It is not a full session-management, RPC-authoring, or multiplayer gameplay implementation surface by itself.
Those concerns still lean on Blueprint, C++, GAS, and playtest verification.

## Key Takeaway

Treat the current networking lane as:

authority model -> static audit -> actor inspection -> grouping and filtering

If you skip the first step, the rest becomes optimization without design.
