## AI School Task Playbooks

These playbooks turn the track system into concrete feature-building patterns.

Use them when a task spans multiple domains and you need a practical read order, build order, and proof set.

Do not treat them as replacements for the full tracks. They are shortcuts for common mixed-domain work.

## 1. Combat Ability Slice

Example tasks:

- dash, blink, slam, projectile, shield, heal, buff, debuff
- ability with HUD read, impact VFX, audio feedback, and multiplayer implications

Read:

1. `GAS`
2. `VFX`
3. `Audio`
4. `UI`
5. `Blueprint` or `CppArchitecture`
6. `Networking` if the ability matters in multiplayer

Build order:

1. define the gameplay contract: tags, cost, cooldown, target, failure state
2. choose the implementation lane: Blueprint or C++
3. author the ability, effect, and assignment scaffolding
4. add readable player feedback: VFX, audio, UI state
5. inspect replication assumptions if the feature is multiplayer-facing
6. prove the interaction on a real actor

Minimum proof:

- structure proof: confirm the edited ability/effect/UI asset and implementation lane
- asset-health proof: compile Blueprint assets and effect scaffolding as needed
- in-world proof: spawn or review feedback at gameplay distance
- runtime interaction proof: `run_quick_playtest`
- contract proof: `get_gas_assets`, `run_quick_playtest`, and networking checks when relevant

Most common failure:

- the ability technically exists, but timing, readability, or replication truth is still wrong

## 2. Traversal Mechanic Slice

Example tasks:

- sprint, dodge, mantle, climb, slide, glide, grapple, vault

Read:

1. `Input`
2. `Animation`
3. `Blueprint` or `CppArchitecture`
4. `UI` only if the mechanic changes prompts or HUD state
5. `Audio` or `VFX` only if feedback changes are part of the task

Build order:

1. define the verb and when it is allowed
2. author input actions and contexts
3. choose the animation role: locomotion, transition, or one-shot
4. implement state changes and gating in Blueprint or C++
5. add optional prompts or feedback layers
6. prove the mechanic under real movement conditions

Minimum proof:

- mapping proof for the authored input route
- gameplay-distance animation proof
- runtime interaction proof with `run_quick_playtest`

Most common failure:

- the input asset exists and the animation plays, but the mechanic still feels unreadable, interruptible in the wrong places, or hard to trigger consistently

## 3. Playable Area Slice

Example tasks:

- arena, biome pocket, dungeon wing, shrine clearing, settlement edge, traversal corridor

Read:

1. `LevelDesign`
2. `Environment`
3. `Audio`
4. `Streaming` if the area changes partition or data-layer behavior

Build order:

1. block out flow, sightlines, entrances, and traversal intent
2. build terrain, material, foliage, and world dressing in the correct environment order
3. add ambience only after the space reads visually
4. add or update streaming structure only when the spatial/state split is understood
5. test navigation, readability, and movement through the space

Minimum proof:

- viewport proof with `observe_ue_project`, `look_at_and_capture`, or `capture_viewport_sync`
- navigation proof with `build_navmesh` and `get_navmesh_status`
- runtime interaction proof with `run_quick_playtest`
- streaming proof when relevant through World Partition or data-layer inspection

Most common failure:

- the area looks dressed, but the route structure, cover logic, or traversal feel was never validated in motion

## 4. Cinematic Reveal Slice

Example tasks:

- boss intro, location reveal, quest payoff, travel montage, environmental story beat

Read:

1. `Cinematics`
2. `LevelDesign`
3. `VFX`
4. `Audio`

Build order:

1. define what each shot must communicate
2. place or confirm the world elements that support the shot
3. create cameras and bind the intended actors
4. add supporting VFX and audio accents
5. review the sequence before rendering
6. render only when the review pass is already clean

Minimum proof:

- structure proof with sequence context and binding inspection
- in-world review proof from camera framing
- output proof with `render_sequence` only when final output matters

Most common failure:

- the sequence renders successfully, but the shot order, emphasis, or bindings were never verified first

## 5. Destructible Interaction Slice

Example tasks:

- breakable crate, harvestable node, destructible wall, Chaos prop, impact-reactive object

Read:

1. `Physics`
2. `Blueprint` or `CppArchitecture`
3. `Audio`
4. `VFX`
5. `SaveLoad` if rollback, snapshots, or restoration matter during iteration

Build order:

1. define the affordance: block, break, harvest, collapse, or react
2. set up collision, fracture, or destruction structure
3. wire the interaction logic in Blueprint or C++
4. add feedback layers only after the physical behavior is believable
5. add rollback-safe capture when the iteration loop is destructive
6. prove the object in runtime

Minimum proof:

- collision or trace proof
- runtime interaction proof
- restoration proof when destructive iteration is part of the workflow

Most common failure:

- the object can break, but it does not communicate affordance clearly before impact or restore reliably after iteration

## 6. Reusable Mesh Or Prop Slice

Example tasks:

- modular wall bay, arch, pillar, stair piece, reusable prop, collision-aware utility mesh

Read:

1. `Modeling`
2. `Physics` if collision matters for gameplay or traversal
3. `LevelDesign` if the mesh's spatial role is still being shaped in context
4. `Environment` only if the work also changes ecological dressing or terrain integration

Build order:

1. define the mesh job, silhouette, and modular boundary
2. choose the correct modeling start: primitive, profile, sweep, revolve, or static-to-dynamic conversion
3. perform major form edits, then local region refinement
4. clean geometry, UVs, and normals
5. export to a static mesh asset
6. harden with material, collision, LOD, and Nanite passes as needed
7. prove the mesh in-world and through collision if relevant

Minimum proof:

- dynamic-mesh structure or stats proof
- cleanup/export proof
- in-world review proof
- collision proof when the mesh makes a gameplay promise

Most common failure:

- the mesh shape exists, but it is still blockout-grade, dirty, or not hardened for actual reuse

## 7. Text And Interface Update Slice

Example tasks:

- localized reward text, menu copy refresh, prompt cleanup, HUD wording update

Read:

1. `Localization`
2. `UI` if the task changes the actual surface layout or interaction

Build order:

1. define message intent and stable key structure
2. update the String Table entries deliberately
3. update the UI surface only if layout or interaction needs to change
4. verify that the text still reads correctly in context

Minimum proof:

- key and entry proof
- deliberate save proof
- PIE UI proof if the visible interface changed

Most common failure:

- the text is updated, but keys become unstable or the new wording breaks the actual widget layout

## How To Use These Playbooks

1. Choose the closest playbook.
2. Read the listed tracks in order.
3. Use the build order as a planning skeleton.
4. Use `VERIFICATION_LADDER.md` to confirm the task has enough proof.
5. If the task touches unsupported design-system concepts, read `COVERAGE_BOUNDARIES.md` before promising a route.

If none of these playbooks fit, fall back to:

- `TRACK_SELECTION.md` for routing
- the relevant domain tracks for craft judgment
- `VERIFICATION_LADDER.md` for proof expectations
