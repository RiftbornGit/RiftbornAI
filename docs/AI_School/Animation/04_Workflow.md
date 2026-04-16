# 04 — Workflow

Animation work should move from motion role to asset plan to correct authoring lane to runtime proof.

## Phase 1: Define The Motion Role

Before touching an animation tool, answer:

- is this continuous locomotion, a transition, or a one-shot action?
- which gameplay state should it communicate?
- what must the player be able to read from this motion at gameplay camera distance?
- what weight class is the character? (determines blend times, anticipation, recovery)

That choice determines the correct asset type:

- continuous locomotion → blend space + AnimBP state machine
- one-shot action → montage
- parameter-driven variation → blend space (1D or 2D)
- cross-rig reuse → retarget pipeline
- motion matching → pose search database

## Phase 2: Decide The Source Asset Plan

Choose whether you are:

- authoring around existing clips on the same skeleton
- retargeting from another rig (requires IK rig setup first)
- assembling a motion-matching database from real clips

Do not start the runtime graph before the source-asset plan is clear. If clips do not exist yet, the animation system is infrastructure waiting for content — acknowledge that instead of pretending the system is complete.

## Phase 3: Set Up The Skeleton And Rig Foundation

If retargeting is involved:

1. `create_ik_rig` for the source skeleton
2. `create_ik_rig` for the target skeleton
3. `create_ik_retargeter` to map chains between them
4. `add_ik_retarget_chain` for any chains not auto-mapped
5. `retarget_animation` to produce clips on the target skeleton

If working with an existing skeleton, skip to Phase 4.

Verify retarget output before building runtime systems on top of it:

- check posture, stride, hand placement
- check ground contact (no floating or penetrating feet)
- review at gameplay camera distance, not close-up

## Phase 4: Build Locomotion Structure

For parameter-driven locomotion:

1. `create_blend_space` or `create_blend_space_1d` — set up the parameter-to-clip mapping
2. `create_anim_blueprint` — create the runtime controller
3. `add_anim_state` — add locomotion states (idle, moving, airborne, landing)
4. `add_anim_transition` — wire transitions with appropriate blend times

State machine design rules:

- states represent gameplay states, not individual clips
- idle → moving transition should have a start animation or short blend
- moving → idle should have a stop animation
- include directional changes (turn-in-place, pivot)
- blend times: responsive actions (0.1-0.15s), locomotion changes (0.2-0.3s), relaxed transitions (0.3-0.5s)

## Phase 5: Build One-Shot Actions

For discrete actions:

1. `create_anim_montage` — create the montage from a source clip
2. Configure sections if the montage has combo branching
3. Set up notifies for gameplay events (damage frames, VFX spawn, sound cues)
4. Integrate the montage slot into the AnimBP

Test each montage independently:

- `play_animation_montage` on the target actor
- verify timing, interruption, and return-to-locomotion behavior

## Phase 6: Apply To Actor And Exercise

Connect the animation system to a real character:

1. `set_animation_blueprint` — apply the AnimBP to the actor's skeletal mesh
2. `play_animation_montage` — exercise one-shot actions manually
3. Verify the full state machine activates correctly

This is where asset authoring becomes runtime behavior. Problems that were invisible in isolation (wrong transitions, broken state logic, montage conflicts) become visible here.

## Phase 7: Verify In Gameplay Context

Use the full verification loop:

1. `set_viewport_location` — position camera at gameplay distance
2. `capture_viewport_sync` — screenshot locomotion, transitions, actions
3. `look_at_and_capture` — focus on the character and analyze motion
4. `run_quick_playtest` — test in actual gameplay

Judge:

- readability at gameplay camera distance
- transition quality (no pops, no sliding, no weight loss)
- timing of one-shot actions in combat flow
- foot contact and ground alignment
- interruption scenarios (run → attack → dodge → idle)
- state machine behavior under rapid input changes

## Phase 8: Optional Motion-Matching Database Lane

If the task truly needs pose search:

1. `create_pose_search_database` — create the database asset
2. `add_animation_to_pose_search_database` — populate with curated clips
3. `build_pose_search_database_index` — build the search index
4. `inspect_pose_search_database` — verify coverage and quality

Only do this after the clip set and runtime motion-matching graph are real. A database without a consumer is dead infrastructure.

## Recommended Sequences

### New Locomotion Controller

`create_blend_space` or `create_blend_space_1d` → `create_anim_blueprint` → `add_anim_state` (idle, walk, run) → `add_anim_transition` (with tuned blend times) → `set_animation_blueprint` → `set_viewport_location` (gameplay distance) → `capture_viewport_sync` → `run_quick_playtest`

### New Combat Action Set

`create_anim_montage` (per action) → integrate montage slots into AnimBP → `play_animation_montage` (test each) → `look_at_and_capture` → `run_quick_playtest` (test interruption and combo flow)

### Cross-Rig Reuse Pipeline

`create_ik_rig` (source) → `create_ik_rig` (target) → `create_ik_retargeter` → `add_ik_retarget_chain` (if needed) → `retarget_animation` → review retargeted clips → build blend space or montage from retargeted clips → `set_animation_blueprint` → `run_quick_playtest`

### Pose Search Database Setup

`create_pose_search_database` → `add_animation_to_pose_search_database` (curated clips) → `build_pose_search_database_index` → `inspect_pose_search_database`

### Full Character Animation Pipeline

Phase 3 (rig setup if needed) → Phase 4 (locomotion) → Phase 5 (actions) → Phase 6 (apply to actor) → Phase 7 (verify in gameplay) → iterate

## Key Takeaway

The safe order is: choose the motion role first, build the matching asset type second, apply to a real actor third, verify in gameplay fourth.

If you start from tools instead of motion role, the asset graph usually ends up backwards. If you skip runtime proof, you ship placeholder animation that compiles but does not communicate.
