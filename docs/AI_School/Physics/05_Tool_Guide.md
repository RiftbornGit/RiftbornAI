# 05 - Tool Guide

Use exact registered physics tool names. If a name is uncertain, verify with `describe_tool`.

## Physics Tool Map

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create a reusable collision rule | `create_collision_preset` | Good for shared behaviors across many actors |
| Set basic collision mode | `set_collision` | Actor or component-level collision enable mode |
| Apply a preset quickly | `set_collision_preset`, `set_collision_profile` | Use when the actor should adopt a known collision behavior |
| Tune channel responses | `set_collision_response` | For explicit block/overlap/ignore rules |
| Add simple collision geometry | `add_collision_shape` | Box, sphere, or capsule support |
| Generate mesh collision | `generate_mesh_collision` | Use when authored collision needs bounded generation help |
| Inspect available channels and presets | `get_collision_channels` | Good first diagnostic step |
| Prove hit or overlap behavior | `line_trace`, `sphere_trace` | Use before trusting interaction assumptions |
| Add a physical joint | `add_physics_constraint` | Hinge, ball socket, prismatic, fixed, or free |
| Create a destructible asset | `create_geometry_collection` | Entry point for Chaos destruction authoring |
| Apply a fracture pattern | `fracture_mesh` | Voronoi, Planar, Radial, Brick, or Uniform |
| Toggle destruction simulation | `enable_destruction` | Runtime-ready destruction enable lane |
| Save and restore destruction states | `capture_destruction_snapshot`, `restore_destruction_snapshot` | Important for rollback-safe iteration |

## Default Lanes

### Collision Lane

Use for:

- solid objects
- triggers
- query behavior
- reusable collision patterns

Main tools:

- `create_collision_preset`
- `set_collision`
- `set_collision_preset`
- `set_collision_profile`
- `set_collision_response`
- `add_collision_shape`
- `generate_mesh_collision`

### Proof Lane

Use for:

- verifying hits
- checking overlap expectations
- confirming interaction reach

Main tools:

- `get_collision_channels`
- `line_trace`
- `sphere_trace`

### Constraint Lane

Use for:

- hinged or bounded relationships
- breakable joints

Main tools:

- `add_physics_constraint`

### Destruction Lane

Use for:

- authored breakable props
- reversible destruction iteration

Main tools:

- `create_geometry_collection`
- `fracture_mesh`
- `enable_destruction`
- `capture_destruction_snapshot`
- `restore_destruction_snapshot`

## Proven Sequences

### Trigger Or Obstacle Setup

`create_collision_preset` if needed -> `set_collision_preset` or `set_collision_profile` -> `line_trace`

### Mechanical Joint

collision setup -> `add_physics_constraint` -> runtime proof

### Destructible Object

`create_geometry_collection` -> `fracture_mesh` -> `capture_destruction_snapshot` -> `enable_destruction`

## Rules

- Do not guess collision behavior when traces can prove it.
- Do not skip presets when the same rule repeats.
- Do not fracture objects blindly without a gameplay reason.
- Do not iterate destructive changes without a rollback path.
- Do not call a physics task done until the behavior reads correctly in play.

## Key Takeaway

The physics surface is strongest when you separate:

- collision authoring
- interaction proof
- constraints
- destruction
- rollback

If those are mixed together carelessly, the object usually becomes physically noisy and semantically unclear.
