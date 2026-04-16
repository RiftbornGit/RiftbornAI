# 04 - Workflow

Physics work should move from affordance definition to collision proof to constraint or destruction setup.

## Phase 1: Define The Physical Role

Before touching a physics tool, answer:

- is this object meant to block, overlap, constrain, break, or simulate debris
- what should the player infer from touching or hitting it
- what should happen when force is applied

## Phase 2: Set Collision Rules

Use:

- `create_collision_preset` when a reusable rule is needed
- `set_collision`
- `set_collision_preset`
- `set_collision_profile`
- `set_collision_response`
- `add_collision_shape`
- `generate_mesh_collision`

to establish the object's contact behavior.

## Phase 3: Prove Contact And Reachability

Use:

- `get_collision_channels`
- `line_trace`
- `sphere_trace`

to prove that the interaction path matches the intended design.

## Phase 4: Add Mechanical Behavior If Needed

Use:

- `add_physics_constraint`

when the object relationship is about bounded movement or breakable attachment.

## Phase 5: Add Destruction If Needed

Use:

- `create_geometry_collection`
- `fracture_mesh`
- `enable_destruction`

when the object should break rather than just collide.

## Phase 6: Protect Iteration With Snapshots

Use:

- `capture_destruction_snapshot`
- `restore_destruction_snapshot`

before and during destructive tuning passes.

## Phase 7: Verify In Runtime

Use playtest or the relevant gameplay validation path to judge:

- whether contact reads correctly
- whether force and break behavior feel right
- whether the player can understand the outcome

## Recommended Sequences

### New Solid Or Trigger Prop

`define affordance` -> collision setup tools -> `line_trace` or `sphere_trace` -> runtime proof

### Constrained Mechanical Object

collision setup -> `add_physics_constraint` -> trace or runtime verification

### Destructible Prop

collision setup -> `create_geometry_collection` -> `fracture_mesh` -> `capture_destruction_snapshot` -> `enable_destruction` -> runtime proof -> `restore_destruction_snapshot` when iterating

## Key Takeaway

The safe order is:

decide the physical truth first, prove the contact second, add breakage or joints third.

If you start from spectacle instead of affordance, the object usually becomes unclear.
