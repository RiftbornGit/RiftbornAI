## Modeling Workflow

Modeling work should move from form intent to dynamic mesh authoring to cleanup to export to hardening.

## Phase 1: Define The Mesh Job

Before touching geometry, decide:

- what the asset does
- whether it is modular or unique
- what silhouette must read at gameplay distance
- whether collision matters
- whether the result is meant to become a static asset

Typical outcomes:

- reusable architecture module
- hero prop
- utility prop
- collision-aware gameplay piece
- spline- or profile-driven structural piece

## Phase 2: Choose The Authoring Start

Choose the simplest start that matches the form:

- box, plane, cylinder, sphere, torus
- extruded profile
- tapered extrude
- revolve
- swept profile
- converted existing static mesh

Typical tool starts:

- `create_dynamic_mesh_actor`
- one of the primitive/profile generation tools
- or `convert_static_to_dynamic`

## Phase 3: Establish The Major Form

Build the large silhouette first.

Use:

- primitive generation
- sweep/revolve/extrude generation
- mirror where structural symmetry matters
- booleans only when they are the cleanest shaping step

At this stage, do not chase micro detail.

## Phase 4: Refine Selected Regions

Once the large form is correct, refine only the necessary regions.

Use:

- face selection tools
- `extrude_mesh_selection`
- `inset_outset_mesh_selection`
- `bevel_mesh_selection`
- `duplicate_mesh_selection`
- `disconnect_mesh_selection`
- `delete_mesh_selection`

This is where local shaping belongs.

## Phase 5: Cleanup And Repair

Before export, run a cleanup pass.

Use:

- `get_dynamic_mesh_stats`
- `fill_mesh_holes`
- `repair_mesh_degenerate_geometry`
- `split_mesh_bowties`
- `auto_generate_mesh_uvs`
- `auto_repair_mesh_normals`
- `recompute_mesh_normals`

If cleanup keeps surfacing new structural issues, return to the form phase instead of piling on more repairs.

## Phase 6: Export To Static Mesh

When the form is stable:

- `export_dynamic_to_static`

This marks the transition from active modeling to asset finish.

Do not export every early experiment.
Do not postpone export indefinitely once the mesh is clearly becoming a shipping asset.

## Phase 7: Asset Hardening

After export, finish the asset:

- assign material
- generate collision
- evaluate Nanite
- author LODs where needed

Typical hardening tools:

- `set_mesh_material` or `assign_mesh_material`
- `generate_mesh_collision`
- `set_static_mesh_nanite`
- `auto_generate_lods`
- `batch_set_lod_group`
- `set_lod_count`
- `set_lod_screen_size`

## Phase 8: Proof

The mesh is not done until it is proven in context.

Use:

- in-world placement and view review
- collision or trace proof when interaction matters
- gameplay placement proof when the asset affects traversal or combat read
- LOD inspection when the asset is meant for repeated or distance-heavy use

Typical end-to-end flows:

### New reusable structural mesh

`create_dynamic_mesh_actor` -> `generate_box_mesh` or profile generator -> local selection edits -> cleanup -> `export_dynamic_to_static` -> material assignment -> collision -> LOD/Nanite hardening -> in-world proof

### Existing mesh repair

`convert_static_to_dynamic` -> targeted cleanup or shaping -> UV/normal repair -> `export_dynamic_to_static` -> collision or material refresh -> proof

### Spline/profile-driven mesh

profile/sweep/revolve generator -> local corrections -> cleanup -> `export_dynamic_to_static` -> hardening -> proof

## Safe Order Summary

shape first, local refinement second, cleanup third, export fourth, hardening fifth, proof last.

If you invert that order, the mesh usually becomes harder to ship, not easier.
