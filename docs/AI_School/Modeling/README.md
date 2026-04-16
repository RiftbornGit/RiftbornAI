# RiftbornAI Modeling School

**Read this track before creating or editing dynamic-mesh geometry, profile-generated meshes, topology operations, UV or normal cleanup, collision generation for modeled assets, or LOD/Nanite hardening passes.**

Modeling work is not just "make a mesh."

It is the discipline of turning clear form intent into usable game geometry:

- readable silhouette
- correct scale
- deliberate topology changes
- stable collision
- sane UVs and normals
- material assignment
- LOD and Nanite hardening when the asset is ready

Good modeling work:

- starts from function and silhouette
- uses the right primitive or profile for the job
- edits topology in a controlled order
- cleans the mesh before export
- treats collision and LODs as part of shipping-readiness, not afterthoughts

Bad modeling work often still "looks okay" in one screenshot while being wrong in every way that matters for production:

- messy topology
- broken normals
- missing UVs
- wrong collision
- over-detailed forms with weak silhouette
- blockout geometry mistaken for ship-ready assets

This track teaches how to use RiftbornAI's modeling surface deliberately instead of just chaining mesh operations until something exists.

## Curriculum

1. **[01_Form_Silhouette_And_Modularity.md](01_Form_Silhouette_And_Modularity.md)** — How game-ready modeled forms should be driven by silhouette, scale, reuse, and function.
2. **[02_UE5_Geometry_Script_And_Mesh_Hardening_Systems.md](02_UE5_Geometry_Script_And_Mesh_Hardening_Systems.md)** — Dynamic mesh authoring, Geometry Script operations, UV/normal repair, collision, export, LODs, and Nanite on the current supported surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Failure modes to avoid when authoring procedural or edited meshes.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for primitive setup, topology edits, cleanup, hardening, export, and proof.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for modeling, mesh cleanup, collision, LODs, and asset hardening.

## Core Rules

- Blockout and final modeling are not the same task.
- Start from silhouette and use-case before topology operations.
- Use dynamic mesh actors as the editing workspace, then export intentionally to static assets.
- Repair UVs, normals, and degenerate geometry before calling the mesh done.
- Collision, LODs, and Nanite are part of the modeling finish pass.
