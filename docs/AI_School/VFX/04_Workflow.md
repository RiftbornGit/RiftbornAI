# 04 — The Correct VFX Workflow

Build VFX from gameplay need outward. Do not start by improvising emitters.

## Phase 1: Define The Effect Job

### Step 1.1 — Write The Gameplay Brief

Before touching Niagara, define:

- what event the effect represents
- whether it is a telegraph, impact, sustain loop, ambient effect, or mixed case
- who needs to read it: player, enemy, spectator, everyone
- where it is seen from: top-down, third-person, first-person, cinematic

If the effect has no clear gameplay job, stop and clarify that first.

### Step 1.2 — Set Readability Targets

Decide:

- strongest read from gameplay distance
- desired timing phases: anticipation, contact, sustain, decay
- priority level: gameplay-critical, state-based, or ambient
- approximate brightness and contrast budget relative to nearby effects

## Phase 2: Inspect Existing Assets First

### Step 2.1 — Find Reusable Baselines

Use:

- `list_niagara_systems`
- `inspect_niagara_system`
- `get_niagara_emitters`
- `get_niagara_parameters`

Look for an existing system that already solves part of the job.

### Step 2.2 — Open The Correct Editor Context

Use:

- `get_editor_focus_state`
- `focus_asset_editor`
- `get_niagara_editor_context`
- `get_niagara_stack_context`

Confirm the actual asset before editing. Do not mutate by assumption.

## Phase 3: Build The First Read

### Step 3.1 — Start From The Simplest Viable Structure

Use:

- `create_niagara_from_template` if a template is a good starting point
- `create_niagara_system` if you need a clean effect

For a first pass, aim for 1-2 emitters:

- one emitter for the primary read
- one emitter for motion accent or residual support if needed

### Step 3.2 — Get The Main Shape Working

Do not chase polish yet. Make sure the effect already communicates:

- direction
- area
- force
- duration

If the effect still does not read, more emitters will not fix it.

## Phase 4: Layer Supporting Emitters

### Step 4.1 — Add Only Layers With A Clear Job

Common second-pass layers:

- sparks or streaks for directional energy
- smoke or haze for aftermath
- ground burst for spatial anchoring
- embers, dust, or magical fallout for decay

### Step 4.2 — Keep The Hierarchy Clean

- primary layer brightest or clearest at the key moment
- support layers should never obscure the core read
- persistent loops should be quieter than the opening burst

## Phase 5: Expose Tuning Surface

### Step 5.1 — Promote Expected Variation To Parameters

Expose user-facing tuning for values likely to change:

- color
- spawn rate
- burst count
- scale
- lifetime
- velocity
- density

This is cheaper and safer than duplicating assets for small variants.

## Phase 6: Compile And Inspect

### Step 6.1 — Verify Asset Health

Use:

- `list_niagara_modules`
- `assert_niagara_compiles`

If it does not compile cleanly, it is not a valid milestone.

### Step 6.2 — Snapshot Before Big Rewrites

Use:

- `inspect_niagara_system`
- `save_niagara_snapshot`
- `diff_niagara_systems`

This gives you proof when tuning variants or undoing regressions.

## Phase 7: Preview In Motion

### Step 7.1 — Preview In The Editor

Use `preview_niagara` for a quick read on timing and silhouette.

Check:

- does the effect have a clear beginning and end?
- are phases visually distinct?
- is there too much dead time or too much overlap?

## Phase 8: Verify In World

### Step 8.1 — Spawn In Correct Context

Use:

- `spawn_niagara_at_location` for world-space effects
- `spawn_niagara_attached` for actor-bound effects

### Step 8.2 — Inspect In The Actual Scene

Use:

- `observe_ue_project`
- `analyze_scene_screenshot`
- `look_at_and_capture`

Check from real gameplay framing:

- bright background
- dark background
- cluttered combat space
- overlap with other VFX

## Phase 9: Playtest

### Step 9.1 — Validate Repetition And Overlap

Use `run_quick_playtest` after major gameplay-facing VFX changes.

You want to know:

- does repeated triggering become noise?
- does overlap stay readable?
- does the effect feel early enough, sharp enough, and brief enough?

## Phase 10: Budget And Finalize

### Step 10.1 — Inspect Cost

Use `get_performance_report` when the effect is close to final.

Watch for:

- too many simultaneous particle systems
- excessive brightness or translucent fill
- dynamic-light spam
- ambient effects competing with gameplay effects

### Step 10.2 — Final Pass

Only after readability and budget are solved should you spend time on:

- subtle secondary breakup
- richer color variation
- additional environmental tie-ins
- special-case variants

## Short Workflow Summary

`define gameplay job` -> `inspect existing assets` -> `build primary read` -> `layer support emitters` -> `expose user params` -> `assert_niagara_compiles` -> `preview_niagara` -> `spawn in world` -> `playtest` -> `check performance`

If a step fails readability, go back one phase. Do not continue stacking complexity on a weak base.
