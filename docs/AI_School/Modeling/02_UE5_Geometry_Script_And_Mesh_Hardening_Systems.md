## UE5 Geometry Script And Mesh Hardening Systems

RiftbornAI has a real modeling lane.

It is centered on the UE 5.7 Geometry Script surface plus downstream mesh hardening tools for collision, material assignment, export, LODs, and Nanite.

The strongest current production path is:

- create or convert to a dynamic mesh
- perform shape and topology operations there
- clean the geometry
- export to a static mesh asset
- harden the asset with collision, LOD, and Nanite settings

## Dynamic Mesh Is The Editing Workspace

Use:

- `create_dynamic_mesh_actor`
- `convert_static_to_dynamic`

to get into an editable mesh workspace.

This is where most modeling operations belong.

Think of the dynamic mesh actor as the modeling canvas, not the final shipping artifact.

## Primitive And Profile Generation

The primitive-generation lane includes:

- `generate_box_mesh`
- `generate_plane_mesh`
- `generate_cylinder_mesh`
- `generate_sphere_mesh`
- `generate_torus_mesh`

Profile-driven generation includes:

- `generate_extruded_polygon_mesh`
- `generate_tapered_extruded_polygon_mesh`
- `generate_revolve_polygon_mesh`
- `generate_spiral_revolve_polygon_mesh`
- `generate_swept_polygon_mesh`
- `generate_spline_swept_polygon_mesh`

Use primitive generation when the form is fundamentally boxy, cylindrical, planar, spherical, or toroidal.

Use profile-driven generation when the shape is defined by:

- an outline
- a path
- a revolution
- a taper

This is often cleaner than stacking many destructive edits onto a primitive.

## Mesh Operations

The core edit surface includes:

- `apply_mesh_boolean`
- `apply_mesh_self_union`
- `apply_mesh_plane_cut`
- `apply_mesh_plane_slice`
- `apply_mesh_mirror`
- `apply_mesh_shell`
- `apply_mesh_smooth`
- `apply_mesh_uniform_remesh`
- `apply_mesh_noise`

These operations are powerful, but they should follow form intent.

They are not substitutes for choosing the right starting form.

## Selection-Driven Local Editing

The selection lane includes:

- `select_mesh_faces_by_normal`
- `select_mesh_faces_by_material`
- `select_mesh_connected_region`

and then local edits:

- `extrude_mesh_selection`
- `inset_outset_mesh_selection`
- `bevel_mesh_selection`
- `duplicate_mesh_selection`
- `disconnect_mesh_selection`
- `delete_mesh_selection`

This is the right lane when only one region needs refinement.

Do not globally remesh or smooth a whole asset when the real problem lives in one connected region.

## Cleanup And Repair

The cleanup lane includes:

- `fill_mesh_holes`
- `repair_mesh_degenerate_geometry`
- `split_mesh_bowties`
- `auto_generate_mesh_uvs`
- `auto_repair_mesh_normals`
- `recompute_mesh_normals`
- `get_dynamic_mesh_stats`

This is the difference between "shape exists" and "mesh is usable."

Use stats and cleanup before export whenever a mesh has undergone several boolean, selection, or remesh operations.

## Export And Asset Finish

When the form is stable:

- `export_dynamic_to_static`

turns the modeled result into a static mesh asset.

After export, the finish pass often includes:

- `set_mesh_material` or `assign_mesh_material`
- `generate_mesh_collision`
- `set_static_mesh_nanite`
- `merge_static_meshes` where intentional aggregation is required

Do not export too early, but do not leave a finished asset trapped as an endlessly edited dynamic mesh either.

## LOD And Nanite Hardening

The asset-hardening lane includes:

- `auto_generate_lods`
- `auto_lod_chain`
- `batch_set_lod_group`
- `get_mesh_lod_info`
- `set_lod_count`
- `set_lod_screen_size`
- `remove_lod`
- `list_lod_groups`
- `get_lod_group_info`
- `set_static_mesh_nanite`

Use these after the mesh form is already correct.

LOD work is not a substitute for clean source form.

Nanite is also not a magic fix for weak silhouette, bad collision, or messy modular structure.

## Specialized Modeling Helpers

There are also specialized modeling routes such as:

- `create_grass_card_mesh`
- `generate_curved_stairs_mesh`
- `generate_linear_stairs_mesh`
- architecture compiler helpers

Use them when the form truly matches the tool's purpose.

Do not force a generic mesh workflow when a purpose-built stair or card mesh route is the cleaner solution.

## What This Surface Is Best At

It is strongest for:

- block-to-form modeling
- profile-driven mesh generation
- region-based topology edits
- cleanup and repair
- export to static mesh
- collision and LOD hardening

It is weaker as:

- a replacement for all DCC workflows
- a reason to over-model hero assets procedurally
- a guarantee that every generated form is automatically shippable without cleanup

The reliable path is:

form intent -> dynamic mesh authoring -> cleanup -> export -> hardening -> proof
