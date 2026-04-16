# 05 — RiftbornAI Tool Guide For VFX Work

Use exact registered tool names. If you are unsure a tool exists, confirm with `list_all_tools` or `describe_tool`.

## Start With Control And Context

| Task | Tool | Notes |
|---|---|---|
| Check for editor blockers | `get_editor_gui_state` | Use before and during VFX editing sessions |
| Assert no modal blockers | `assert_no_modal_blockers` | Prevent hidden dialogs from invalidating edits |
| Check active editor focus | `get_editor_focus_state` | Confirm the actual asset/editor lane |
| Bring a Niagara asset forward | `focus_asset_editor` | Prefer this over blind editor assumptions |
| Read Niagara editor state | `get_niagara_editor_context` | Grounded active-asset context |
| Read stack/selection state | `get_niagara_stack_context` | Use before fine-grained edits or reviews |
| Inspect modules and parameters | `list_niagara_modules` | Editor-native stack inventory |
| Prove compile health | `assert_niagara_compiles` | Compile is required before world verification |
| Check warnings/errors | `get_output_log_context` / `get_compile_diagnostics` | Use after asset edits and compile passes |

Design rules:

- always ground on editor context before editing inside the Niagara editor
- treat compile health as a gate — do not spawn an effect into the world before it compiles clean
- use `get_niagara_stack_context` to understand the current emitter and module structure before making targeted changes

## Discover And Inspect Existing Niagara Assets

| Task | Tool | Notes |
|---|---|---|
| List project systems | `list_niagara_systems` | Start here before creating a new system |
| Get emitter breakdown | `get_niagara_emitters` | Useful for understanding layer structure |
| Get exposed user params | `get_niagara_parameters` | Shows the runtime tuning surface |
| Deep-inspect a system | `inspect_niagara_system` | Returns emitter, module, renderer, and parameter data |
| Compare two systems | `diff_niagara_systems` | Use when reviewing variants or regressions |
| Save proof snapshot | `save_niagara_snapshot` | Capture state before large edits |

Design rules:

- always check existing systems before creating new ones — duplication drift is a real problem
- use snapshots before destructive edits to enable rollback
- inspect deeply before tuning — understand what emitters and modules already exist

## Create And Preview Niagara

| Task | Tool | Notes |
|---|---|---|
| Create empty system | `create_niagara_system` | Best when the effect structure is custom |
| Create from template | `create_niagara_from_template` | Good for first-pass fire, smoke, sparks, water, magic, explosion, blood, electric |
| Preview in editor | `preview_niagara` | Quick timing/silhouette pass before world spawn |

Design rules:

- use templates when one matches the structural intent — do not reinvent fire from scratch
- preview in the editor before spawning in-world — catches timing, shape, and color issues early
- custom empty systems are for effects where no template fits the gameplay role

## Spawn And Tune In World

| Task | Tool | Notes |
|---|---|---|
| Spawn world-space effect | `spawn_niagara_at_location` | Impacts, area effects, props, ambient points |
| Spawn attached effect | `spawn_niagara_attached` | Weapon trails, shield loops, actor buffs |
| Set runtime parameter | `set_niagara_parameter` | Prefer exposed `User.*` values for tuning |
| Activate/deactivate | `activate_niagara` | Toggle loops and conditional states |
| Restart effect | `reset_niagara` | Useful after parameter changes |

Design rules:

- choose world-space vs attached intentionally: impacts and area effects use location, actor-bound effects use attached
- tune via exposed `User.*` parameters rather than editing the graph for every iteration
- deactivate unused loops explicitly — do not leave dormant emitters running

## Ambient And World Mood

| Task | Tool | Notes |
|---|---|---|
| Spawn ambient forest particles | `spawn_forest_air_particles` | Dust motes, pollen, spores, fireflies |
| Retune ambient forest particles | `edit_forest_air_particles` | Resize, move, swap preset, density-retune |

## Visual Verification

| Task | Tool | Notes |
|---|---|---|
| Full scene observation | `observe_ue_project` | Best general verification loop |
| Targeted screenshot critique | `analyze_scene_screenshot` | Ask specifically about readability, clutter, brightness |
| Look at an actor or spot | `look_at_and_capture` | Good for attached or placed effects |
| Blocking viewport capture | `capture_viewport_sync` | Repeatable screenshot evidence |
| Quick play validation | `run_quick_playtest` | Check repetition, motion, overlap, timing |
| Performance snapshot | `get_performance_report` | Use before calling a gameplay-facing effect finished |

Design rules:

- verify at gameplay camera distance, not only close-up in the asset editor
- ask specific vision-analysis questions: "Is this readable during combat?" not just "Does it look good?"
- check performance cost for any effect that will appear in gameplay — particle count and overdraw matter

## Verification Checklist

Before calling a VFX surface done:

- [ ] effect compiles clean (`assert_niagara_compiles`)
- [ ] previewed in editor for timing and silhouette (`preview_niagara`)
- [ ] spawned in-world at correct position/attachment (`spawn_niagara_at_location` or `spawn_niagara_attached`)
- [ ] verified at gameplay camera distance (`analyze_scene_screenshot` or `look_at_and_capture`)
- [ ] readability confirmed during action (`run_quick_playtest`)
- [ ] performance cost checked (`get_performance_report`)
- [ ] no visual noise competing with gameplay-critical cues
- [ ] timing phases (anticipation, contact, decay) feel correct
- [ ] snapshot saved if effect is shared or important (`save_niagara_snapshot`)

## Recommended Default Flows

### New Gameplay Impact Effect

`list_niagara_systems` -> `create_niagara_from_template` or `create_niagara_system` -> `get_niagara_editor_context` -> `get_niagara_stack_context` -> `list_niagara_modules` -> `assert_niagara_compiles` -> `preview_niagara` -> `spawn_niagara_at_location` -> `analyze_scene_screenshot` -> `run_quick_playtest`

Use for: hit sparks, projectile impacts, area damage indicators, trap triggers.

### New Attached Buff Or Weapon Effect

`list_niagara_systems` -> `inspect_niagara_system` -> `spawn_niagara_attached` -> `set_niagara_parameter` -> `look_at_and_capture` -> `run_quick_playtest`

Use for: weapon trails, shield loops, burning aura, healing glow, buff indicators.

### Variant Tuning Of An Existing Effect

`inspect_niagara_system` -> `save_niagara_snapshot` -> `get_niagara_parameters` -> `set_niagara_parameter` or asset edit -> `assert_niagara_compiles` -> `diff_niagara_systems` -> `spawn_niagara_at_location` -> `get_performance_report`

Use for: creating fire/ice/lightning variants from a shared base, seasonal tweaks, difficulty-scaled effects.

### Ambient World Mood Pass

`list_niagara_systems` -> `spawn_forest_air_particles` or `create_niagara_from_template` -> `spawn_niagara_at_location` -> `set_niagara_parameter` -> `observe_ue_project` -> `get_performance_report`

Use for: dust motes, pollen, mist, embers, fireflies, environmental atmosphere.

## Rules

1. Do not skip context tools when editing inside the Niagara editor.
2. Do not skip world verification after a clean compile.
3. Do not solve readability problems with particle count alone.
4. Use snapshots and diffs when changing an important shared effect.
5. Verify at gameplay camera distance, not only in the isolated asset preview.
6. Budget particle count and overdraw before adding more layers.
7. Every effect needs a gameplay job before it needs a graph.

## Key Takeaway

The VFX surface is strongest when you separate: context inspection, asset authoring, world placement, visual verification, and performance proof. Skipping any layer produces effects that compile but fail in gameplay.
