# 02 - UE5 Collision Constraint And Destruction Systems

RiftbornAI's current physics surface is strongest around collision authoring, trace verification, physics constraints, and Chaos destruction setup.

## Collision Setup Is A First-Class Lane

Use:

- `create_collision_preset`
- `set_collision`
- `set_collision_preset`
- `set_collision_profile`
- `set_collision_response`
- `add_collision_shape`
- `generate_mesh_collision`
- `get_collision_channels`

for the default collision lane.

This is the path for:

- defining reusable collision behavior
- assigning actor-level collision rules
- adding simple collision geometry
- generating mesh collision where appropriate
- confirming which channels and presets exist

## Traces Prove Interaction Assumptions

Use:

- `line_trace`
- `sphere_trace`

to verify:

- reachability
- hit logic
- proximity or overlap assumptions
- whether a physical interaction path is actually present

These tools are proof, not just gameplay implementation.

## Constraints Express Mechanical Relationships

Use:

- `add_physics_constraint`

to define bounded physical relationships between actors.

This is the supported lane for:

- hinges
- ball sockets
- prismatic movement
- fixed joints
- break-force behavior

## Chaos Destruction Is An Authored Pipeline

Use:

- `create_geometry_collection`
- `fracture_mesh`
- `enable_destruction`
- `capture_destruction_snapshot`
- `restore_destruction_snapshot`

for destructible props and rollback-safe iteration.

This is the correct lane when the object should:

- exist as an intact prop
- fracture according to a chosen pattern
- simulate destruction at the right moment
- be restorable while iterating

## Current Strength Of The Surface

The physics tool surface is strongest for:

- collision setup
- constraint authoring
- trace-based proof
- destructible asset setup
- destruction rollback through snapshots

It is not a complete substitute for every deeper physical-material or ragdoll authoring case, so keep the school grounded in the verified production lane.

## Key Takeaway

Treat the current physics lane as:

collision rules -> interaction proof -> constraints or destruction -> rollback-safe validation

Skipping the proof step is how a physically configured object still behaves wrong in play.
