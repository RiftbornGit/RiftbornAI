# 03 — Anti-Patterns

These are the failure modes most likely to appear when AI authors VFX without discipline.

## NEVER: Build One Giant Emitter To Do Everything

**What goes wrong:**

- Spawn, motion, glow, debris, smoke, and residual behavior all get tangled together
- Small tuning changes break the whole effect
- You cannot tell which part is carrying the read

**Do instead:** Split the effect into clear jobs. Core, accent, residual, and environmental tie-in should usually be separate emitters.

## NEVER: Judge An Effect Only In The Asset Editor

**What goes wrong:**

- The effect looks good close-up against a neutral background
- It disappears or becomes messy at gameplay camera distance
- It collides visually with environment lighting and other effects

**Do instead:** Preview in the editor, then spawn in-world and verify at actual gameplay framing.

## NEVER: Make Ambient VFX Brighter Than Gameplay-Critical VFX

**What goes wrong:**

- Dust motes, sparkles, and decorative magic steal attention from telegraphs and impacts
- Combat readability collapses in busy scenes

**Do instead:** Reserve the strongest contrast and brightest peaks for gameplay-critical information.

## NEVER: Use More Particles To Solve A Readability Problem

**What goes wrong:**

- Overdraw increases
- Silhouette becomes muddy
- Frame cost rises while clarity gets worse

**Do instead:** Strengthen the primary shape, timing, and contrast first. Add particles only when they support a clear read.

## NEVER: Duplicate Systems For Minor Variants

**What goes wrong:**

- You create asset sprawl
- Small balance changes become repetitive manual edits
- Variants drift apart and become inconsistent

**Do instead:** Expose `User.*` parameters for color, burst size, density, scale, lifetime, and other expected tuning values.

## NEVER: Skip Niagara Introspection Before Editing

**What goes wrong:**

- You edit the wrong system
- You misread what modules or emitters already exist
- You claim success without compile proof

**Do instead:** Use `get_niagara_editor_context`, `get_niagara_stack_context`, `list_niagara_modules`, and `assert_niagara_compiles` as the default inspection loop.

## NEVER: Attach A World Effect Or World-Place An Attached Effect By Accident

**What goes wrong:**

- Slash trails lag behind the weapon
- Shield loops stay behind when actors move
- Campfire smoke follows a character because it was attached lazily

**Do instead:** Choose between `spawn_niagara_attached` and `spawn_niagara_at_location` based on what the player should perceive as the source.

## NEVER: Let Residual Loops Look Like Fresh Impacts

**What goes wrong:**

- A poison cloud is as bright as the initial detonation
- A burn aura reads like constant re-triggering
- The player cannot tell whether the dangerous moment has passed

**Do instead:** Use a strong initial peak, then reduce intensity and detail for the sustain phase.

## NEVER: Use VFX To Hide Weak Gameplay Timing

**What goes wrong:**

- Telegraphs are late, so the effect gets louder instead of earlier
- Hit confirms are vague, so the screen fills with noise
- Combat feels unfair even though it looks busy

**Do instead:** Fix the gameplay timing first. VFX should clarify state, not compensate for broken design timing.

## NEVER: Ignore Performance Until The End

**What goes wrong:**

- You build around expensive assumptions
- GPU emitters, translucency, and dynamic lights pile up
- Late optimization removes the very layers the effect depends on

**Do instead:** Budget from the first pass. Keep the core read effective even if support layers get reduced later.

## NEVER: Declare An Effect Done Without Motion And Overlap Checks

**What goes wrong:**

- The effect reads well in a still frame but collapses in action
- Several overlapping systems turn into visual soup
- Important gameplay information becomes indistinguishable

**Do instead:** Test repeated spawns, overlapping instances, and real motion before calling the effect finished.

## Golden Rule

**An effect is finished when it communicates clearly in context at acceptable cost.**

Not when it looks impressive in a close-up viewport.
