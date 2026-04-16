## AI School Track Selection

AI School is modular by design.

Do not load every track for every task. Load the smallest set of tracks that matches the assets and behaviors you are about to change.

## Step 1: Pick The Primary Mutation Domain

Start with the asset or system that will be directly edited.

Examples:

- landscape, foliage, biome dressing -> `Environment`
- dynamic mesh topology, Geometry Script authoring, UV/normal cleanup, export, LOD/Nanite hardening -> `Modeling`
- Niagara system, hit sparks, ambient particles -> `VFX`
- ability, effect, attribute, gameplay tags -> `GAS`
- Widget Blueprint, HUD, menu flow -> `UI`
- AnimBP, montage, retargeting -> `Animation`
- Level Sequence, camera shots -> `Cinematics`
- World Partition, data layers -> `Streaming`
- replication audits, Iris grouping -> `Networking`
- snapshots, checkpoints, restore flows -> `SaveLoad`
- String Tables and localized text keys -> `Localization`
- gameplay C++ or Blueprint logic -> `CppArchitecture` or `Blueprint`

The primary domain should usually be read first.

## Step 2: Add Dependent Craft Tracks

Then add the tracks that shape the quality of the same feature.

Common combinations:

- reusable prop or modular architecture piece -> `Modeling` + `LevelDesign` if placement/read matters + `Physics` if collision promise is gameplay-critical
- combat ability -> `GAS` + `VFX` + `Audio` + `UI` + `Blueprint` or `CppArchitecture`
- traversal character update -> `Animation` + `Input` + `Blueprint` or `CppArchitecture`
- multiplayer ability -> `GAS` + `Networking` + `UI`
- cinematic set piece -> `Cinematics` + `LevelDesign` + `VFX` + `Audio`
- explorable map slice -> `LevelDesign` + `Environment` + `Streaming`
- interactive prop or destructible object -> `Physics` + `Blueprint` or `CppArchitecture` + `Audio` or `VFX` if feedback changes

If the task changes both logic and presentation, read both.

## Step 3: Add Cross-Cutting Contract Tracks Only When Needed

Some tracks are not the visible surface of the feature, but they still define important contracts.

Add them when the task changes:

- persistence expectations -> `SaveLoad`
- multiplayer truth or relevance -> `Networking`
- player-facing text or stable text keys -> `Localization`
- input verbs or control context -> `Input`

Do not add these tracks just because the feature exists in the same game. Add them when the task mutates their contract.

## Suggested Read Order

Use this order unless the task clearly demands otherwise:

1. primary mutation domain
2. supporting craft domains
3. logic domain: `Blueprint` or `CppArchitecture`
4. cross-cutting contract domains: `Networking`, `SaveLoad`, `Localization`, `Input`
5. `COVERAGE_BOUNDARIES.md` if a task mentions design-system names that may not have a public authoring lane

## How To Decide Quickly

Ask four questions:

1. What asset or system will actually be edited?
2. What player-facing read must improve at the same time?
3. What runtime contract could break if this changes?
4. What proof will be required before the task is truly done?

If you cannot answer those questions, you probably have not selected enough tracks.

## Do Not Over-Load Context

Bad routing:

- reading every track because the feature is "complex"
- loading `Networking` for a purely local editor-only art task
- loading `SaveLoad` for a feature that does not change persistence behavior
- loading `Localization` when no text or key structure is changing

Good routing:

- read the smallest set of tracks that governs the real mutations
- add one more track when the task crosses a domain boundary
- verify exact tool names with `list_all_tools` or `describe_tool` when a helper is uncertain

## Example Task Routing

`"Build a frost dash ability with HUD cooldown, impact burst, and multiplayer-safe behavior"`

Read:

- `GAS`
- `UI`
- `VFX`
- `Networking`
- `Blueprint` or `CppArchitecture`, depending on the implementation lane

`"Block out a haunted chapel area with fog, ambient sound, and streaming layers"`

Read:

- `LevelDesign`
- `Environment`
- `Audio`
- `Streaming`

`"Localize quest reward text and keep stable keys for future patches"`

Read:

- `Localization`

Add another track only if the task also changes the UI surface, gameplay logic, or save/load behavior tied to that text.

`"Author a modular stone arch mesh, generate collision, and harden it for reuse across a courtyard set"`

Read:

- `Modeling`
- `Physics` if the collision is gameplay-critical
- `LevelDesign` if the courtyard layout or sightline role is still being shaped
