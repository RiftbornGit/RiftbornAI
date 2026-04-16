## Modeling Anti-Patterns

## Anti-Pattern 1: Treating Blockout As Finished Modeling

`generate_blockout` is a space-planning tool.

It helps define:

- layout
- traversal
- room proportions
- cover
- spatial rhythm

It is not the same thing as authored final mesh geometry.

**Do instead:** Use blockout to prove space first. Then use the modeling surface to author real reusable forms, clean topology, collision, and asset hardening.

## Anti-Pattern 2: Starting With Boolean Tricks Instead Of Shape Intent

When the first instinct is to boolean everything, the asset usually ends up with:

- weak silhouette
- unclear structural logic
- more cleanup work than necessary

**Do instead:** Start with the primitive or profile that matches the intended form. Use booleans as a controlled shaping step, not as the entire modeling strategy.

## Anti-Pattern 3: Editing The Wrong Mesh Because Context Was Not Verified

Modeling tools mutate geometry fast.

If the wrong dynamic mesh or converted mesh is in focus, the damage is immediate.

**Do instead:** Confirm the actual editing target before each major operation. Use stats, selection tools, and context checks to prove you are operating on the intended mesh.

## Anti-Pattern 4: Global Cleanup For A Local Problem

If one face group is bad, globally smoothing or remeshing the whole mesh often destroys parts that were already fine.

**Do instead:** Select the affected region first, then use local extrude, inset, bevel, disconnect, or delete operations where the real issue lives.

## Anti-Pattern 5: Exporting Before UV, Normal, And Topology Cleanup

A mesh can look acceptable in one viewport angle while still containing:

- holes
- degenerate geometry
- broken normals
- bad UV assumptions

**Do instead:** Run the cleanup pass before export. Repair geometry, recompute or repair normals as needed, and generate UVs deliberately.

## Anti-Pattern 6: Calling The Mesh Done Before Collision Exists

If the mesh supports traversal, hits, cover, interaction, or placement rules, missing collision means it is still unfinished.

**Do instead:** Treat `generate_mesh_collision` and collision verification as part of the finish pass.

## Anti-Pattern 7: Using Nanite Or LODs To Hide A Weak Source Mesh

LOD and Nanite are shipping-readiness tools.

They do not fix:

- weak silhouette
- bad modular boundaries
- overcomplicated source form
- collision mismatch

**Do instead:** Fix the source mesh first. Then harden with LOD and Nanite once the form is correct.

## Anti-Pattern 8: Over-Detailing Utility Meshes

Most reusable environment or gameplay meshes do not need hero-level complexity.

Too much geometry causes:

- slower iteration
- weaker reuse
- harder cleanup
- noisier silhouette

**Do instead:** Put detail where the player reads structure. Let material and dressing carry the secondary and tertiary richness.

## Anti-Pattern 9: Forgetting The Modular Seam

A piece may look fine alone but fail completely when repeated.

Common signs:

- drifting edge thickness
- mismatched endpoints
- inconsistent profile
- poor repetition

**Do instead:** Check the mesh as a repeatable module before calling it stable.

## Anti-Pattern 10: Endless Dynamic-Mesh Iteration Without Export Discipline

Dynamic mesh is the right place to edit, but not every mesh should live there forever.

Without an intentional export point:

- material assignment stays fuzzy
- downstream hardening is delayed
- the asset boundary stays unclear

**Do instead:** When the form is stable, export to a static mesh asset and finish the hardening pass there.
