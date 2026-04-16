## Form, Silhouette, And Modularity

Most game meshes fail before topology ever becomes the issue.

They fail because the form intent is weak.

The first questions are:

- what is this object for?
- what scale does the player read it at?
- what silhouette should be legible from gameplay distance?
- is this a one-off hero mesh or a reusable modular piece?

## Start With Function

A good game mesh has a job.

Examples:

- cover piece
- doorway frame
- staircase
- pillar
- trim piece
- roof segment
- decorative prop
- collision shell

That job determines:

- proportions
- thickness
- likely repetition
- collision expectations
- how much detail belongs in silhouette versus material

If you model before deciding the job, the asset often becomes visually busy but systemically weak.

## Silhouette Beats Surface Noise

At gameplay distance, players read:

- overall outline
- major cuts and angles
- openings
- support thickness
- top profile
- mass distribution

They do not primarily read:

- tiny edge perturbations
- dense bevel spam
- subtle secondary chamfers
- micro surface damage baked into the geometry

If the silhouette is weak, more topology usually makes the asset worse.

## Modular Pieces Need Clean Boundaries

Many production meshes should be authored as reusable pieces, not unique sculptures.

A good modular piece:

- has intentional end boundaries
- has consistent thickness and snapping logic
- avoids hidden shape drift between adjacent pieces
- can repeat without obvious form breakdown

Typical modular categories:

- walls
- floor sections
- beams
- arches
- stairs
- trim
- railings

The cleaner the module boundary, the less downstream pain in level assembly.

## Use Geometry For Form, Not For Everything

Put real geometry where it changes:

- silhouette
- collision
- opening structure
- player-readable depth

Do not spend topology on details that belong in:

- material variation
- normal data
- decals
- foliage dressing
- props layered onto the base mesh

This matters especially for stairs, walls, pillars, and repeated architecture.

## Editing Order Matters

A stable modeling task usually moves like this:

1. establish the primitive or profile
2. shape major proportions
3. cut or extrude major forms
4. mirror or duplicate structural repetition
5. refine selected regions
6. clean geometry
7. harden for shipping

If you jump around randomly between cuts, smoothing, booleans, remesh, and duplication, the mesh becomes harder to reason about.

## Hero Meshes And Utility Meshes Need Different Standards

A hero asset may justify:

- more silhouette complexity
- more controlled local variation
- more custom shaping

A utility mesh should bias toward:

- clarity
- reusability
- easier cleanup
- simpler collision
- stronger LOD behavior

Do not over-author a utility piece like it is a showcase hero asset.

## Collision Is Part Of Form Design

If the player can:

- walk on it
- hide behind it
- hit it
- climb it
- shoot through it

then collision expectations should shape the model.

That means:

- readable step heights
- believable thickness
- clear openings
- support geometry that matches the physical promise

A mesh that looks passable but collides badly is not a finished mesh.

## Ask These Questions Before Modeling

1. What is the mesh's gameplay or spatial job?
2. Is the silhouette clear from gameplay distance?
3. Is this a reusable module or a one-off form?
4. Which details belong in geometry, and which belong in materials?
5. What collision promise does this form make?
6. Will the asset need LOD and Nanite hardening later?

If you cannot answer those questions, you are not ready to start mesh operations.
