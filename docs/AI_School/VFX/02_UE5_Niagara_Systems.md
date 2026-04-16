# 02 — UE5 Niagara Systems

Niagara is the implementation layer. It is not the design brief.

Do not start with "which module do I add?" Start with "what gameplay read do I need?" Then map that read onto Niagara in the simplest possible way.

## Niagara Building Blocks

### Niagara System

The full effect asset. Think "complete campfire" or "complete sword impact."

A system usually contains multiple emitters that each do one job.

### Niagara Emitter

One layer of the effect.

Examples:

- Flame body
- Sparks
- Smoke plume
- Ground embers
- Shock ring

This is the correct unit for effect layering.

### Modules

The behavior blocks inside spawn, update, and render stages.

Modules are where you express things like:

- spawn count or burst behavior
- velocity and gravity
- size over life
- drag and turbulence
- color or emissive changes over time

### Renderer

How particles are drawn.

Use renderer choice to reinforce the read:

- sprites for broad soft volumes
- ribbons or streaks for motion traces
- mesh renderers only when silhouette really benefits

### User Parameters

These are the tuning surface that should survive reuse.

Expose `User.*` parameters for the values you expect to retune:

- color
- spawn rate
- burst size
- lifetime
- scale
- velocity
- density

If you are duplicating a system only to change a few numbers, you probably need better user parameters.

## Layer Niagara Around The Effect Jobs

Most effects should be authored as cooperating emitters:

- Core emitter for the primary read
- Accent emitter for fast motion or spark energy
- Residual emitter for smoke, haze, dust, or magic linger
- Optional environmental tie-in for ground or space interaction

Keep emitter responsibilities separate enough that you can tune or remove a layer without breaking the whole effect.

## Editor-Native Niagara Lane

When working inside the Niagara editor, use the real asset-editor surface first.

Recommended order:

1. `get_editor_focus_state`
2. `focus_asset_editor`
3. `get_niagara_editor_context`
4. `get_niagara_stack_context`
5. `list_niagara_modules`
6. `assert_niagara_compiles`

Why this matters:

- You confirm the correct asset is actually focused.
- You get grounded editor state instead of guessing from screenshots.
- You can inspect the current stack, module count, selection, and compile health before editing.

Do not treat the Niagara editor like a black box.

## Authoring Surfaces In RiftbornAI

### Create Or Clone

Use:

- `create_niagara_system` when starting from an empty system
- `create_niagara_from_template` when an engine or project template gives you a faster baseline

Templates are starting points, not finished effects.

### Inspect Before Editing

Use:

- `list_niagara_systems`
- `get_niagara_emitters`
- `get_niagara_parameters`
- `inspect_niagara_system`
- `diff_niagara_systems`

These tools help you understand existing assets before you duplicate or mutate them.

### Preview And Spawn

Use:

- `preview_niagara` for quick editor preview
- `spawn_niagara_at_location` for world-space effects
- `spawn_niagara_attached` for socket or actor-bound effects

Attachment choice is design, not convenience:

- sword swing trail belongs on the weapon or hand
- campfire smoke belongs in world space
- shield loop belongs on the protected actor

### Runtime Tuning

Use:

- `set_niagara_parameter`
- `activate_niagara`
- `reset_niagara`

These are for tuning and validation after the system exists.

## Verification Surfaces Matter

Niagara compile success is necessary but not sufficient.

You still need to verify:

- Is the effect readable in the game camera?
- Is it too bright or too dim against the level background?
- Does it overlap badly with other combat effects?
- Does it hold up when spawned repeatedly?

For this, combine Niagara tools with:

- `observe_ue_project`
- `analyze_scene_screenshot`
- `look_at_and_capture`
- `run_quick_playtest`
- `get_performance_report`

## Use The Repo's Evaluation Mindset

RiftbornAI already contains VFX recipe, heuristics, and evaluation surfaces under `Source/RiftbornAI/Public/VFX/`.

That matters because the intended workflow is:

- store effect structure as repeatable logic
- apply heuristics as guidance, not blind authorship
- measure results with proof instead of taste alone

In practice, that means:

- build repeatable emitter structure
- expose tuning inputs
- compare variants
- verify against gameplay and performance goals

## Key Takeaway

Niagara is easiest to use badly when you jump straight into module tweaking.

Use it well by:

- defining the effect job first
- assigning one clear job per emitter
- exposing user parameters instead of cloning variants
- inspecting the editor state before editing
- compiling and then verifying in-world
