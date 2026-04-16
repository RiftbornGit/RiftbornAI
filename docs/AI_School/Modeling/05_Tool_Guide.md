## Modeling Tool Guide

Use exact registered modeling tool names. If a name is uncertain, verify with `describe_tool`.

## Core Modeling Workspace

| Task | Tools | Why |
|------|-------|-----|
| Start an editable modeling workspace | `create_dynamic_mesh_actor` | Standard Geometry Script editing canvas |
| Convert an existing static mesh to editable geometry | `convert_static_to_dynamic` | Use when refining or repairing an existing mesh |
| Inspect mesh complexity and health | `get_dynamic_mesh_stats` | Good checkpoint before export or after heavy edits |
| Export the finished result to a static asset | `export_dynamic_to_static` | Promotes the edited result into a shippable asset boundary |

## Primitive And Profile Generation

| Task | Tools | Why |
|------|-------|-----|
| Boxy or planar form | `generate_box_mesh`, `generate_plane_mesh` | Clean starting point for many architectural and utility assets |
| Round base form | `generate_cylinder_mesh`, `generate_sphere_mesh`, `generate_torus_mesh` | Better than forcing those shapes from boxes |
| Outline extrude | `generate_extruded_polygon_mesh`, `generate_tapered_extruded_polygon_mesh` | Strong for pillars, obelisks, trims, and profile-driven forms |
| Revolved form | `generate_revolve_polygon_mesh`, `generate_spiral_revolve_polygon_mesh` | Good for lathed or radial forms |
| Swept form | `generate_swept_polygon_mesh`, `generate_spline_swept_polygon_mesh` | Good for rails, trims, pipes, and profile-following structures |
| Specialized foliage card mesh | `create_grass_card_mesh` | Purpose-built modeling helper, not a generic mesh route |

## Global Shape Operations

| Task | Tools | Why |
|------|-------|-----|
| Add or subtract major volume | `apply_mesh_boolean`, `apply_mesh_self_union` | Controlled structural cuts or merges |
| Cut or slice a mesh | `apply_mesh_plane_cut`, `apply_mesh_plane_slice` | Good for removing or separating larger form pieces |
| Mirror a structured asset | `apply_mesh_mirror` | Faster and cleaner than hand-editing symmetry |
| Add thickness | `apply_mesh_shell` | Useful for forms that should have real depth |
| Relax or normalize shape | `apply_mesh_smooth`, `apply_mesh_uniform_remesh` | Use after the form is mostly correct, not as the first fix |

## Selection-Based Local Editing

| Task | Tools | Why |
|------|-------|-----|
| Select by surface direction | `select_mesh_faces_by_normal` | Good for top, side, or front-facing region edits |
| Select by existing material region | `select_mesh_faces_by_material` | Useful when material regions map to editing regions |
| Select one connected area | `select_mesh_connected_region` | Limits changes to the real problem area |
| Push or pull a region | `extrude_mesh_selection` | Core local form tool |
| Add recess or frame | `inset_outset_mesh_selection` | Useful for panels, trims, and readable cut-ins |
| Refine edge transitions | `bevel_mesh_selection` | Use sparingly and intentionally |
| Duplicate or separate geometry | `duplicate_mesh_selection`, `disconnect_mesh_selection` | Helpful for repeated or detached forms |
| Remove bad geometry | `delete_mesh_selection` | Cleaner than trying to salvage everything |

## Cleanup And Repair

| Task | Tools | Why |
|------|-------|-----|
| Fill openings | `fill_mesh_holes` | Good cleanup step after cuts or deletes |
| Repair bad topology | `repair_mesh_degenerate_geometry`, `split_mesh_bowties` | Important after booleans or aggressive edits |
| Fix normals | `auto_repair_mesh_normals`, `recompute_mesh_normals` | Needed before material or lighting review |
| Create usable UVs | `auto_generate_mesh_uvs` | Important before final material assignment |

## Finish And Hardening

| Task | Tools | Why |
|------|-------|-----|
| Assign materials during or after finish | `set_mesh_material`, `assign_mesh_material` | Use when the mesh boundary is already clear |
| Generate collision | `generate_mesh_collision` | Required for many gameplay-facing meshes |
| Enable Nanite | `set_static_mesh_nanite` | Late hardening step once the asset form is stable |
| Generate LODs | `auto_generate_lods`, `auto_lod_chain` | Shipping-readiness step for repeated or distance-heavy assets |
| Inspect or tune LODs | `get_mesh_lod_info`, `set_lod_count`, `set_lod_screen_size`, `remove_lod` | Fine-tuning after initial LOD generation |
| Set LOD policy across assets | `list_lod_groups`, `get_lod_group_info`, `batch_set_lod_group` | Consistency pass for broader asset sets |
| Merge final meshes intentionally | `merge_static_meshes` | Use for deliberate aggregation, not to hide poor modular planning |

## Recommended Modeling Chains

### New modular mesh

`create_dynamic_mesh_actor` -> primitive or profile generator -> local selection edits -> cleanup tools -> `export_dynamic_to_static` -> material -> collision -> LOD/Nanite -> proof

### Repair existing mesh

`convert_static_to_dynamic` -> targeted shape fixes -> topology cleanup -> UV/normal repair -> `export_dynamic_to_static` -> hardening -> proof

### Stair, trim, or spline-following form

profile/sweep/stair generator -> local edits if needed -> cleanup -> `export_dynamic_to_static` -> collision/material -> proof

## Use This Track For

- authoring mesh form
- editing dynamic mesh topology
- cleaning and exporting modeled results
- finishing collision, UV, normal, LOD, and Nanite passes

## Do Not Use This Track As A Substitute For

- level blockout planning
- general environment ecology work
- material graph authoring
- physics authoring beyond mesh collision generation

Those domains have their own tracks.
