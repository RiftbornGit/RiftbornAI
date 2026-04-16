# 05 — Tool Guide

Use exact registered animation tool names. If a name is uncertain, verify with `describe_tool`.

## Locomotion Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create 2D locomotion blend | `create_blend_space` | Speed × direction. Place clips at correct parameter values |
| Create 1D locomotion blend | `create_blend_space_1d` | Single parameter (speed, lean, etc.) |
| Create runtime animation controller | `create_anim_blueprint` | Entry point for all continuous animation logic |
| Add gameplay states | `add_anim_state` | States represent gameplay states, not individual clips |
| Wire state transitions | `add_anim_transition` | Set blend time per transition — do not use one default for everything |

Locomotion workflow: `create_blend_space` → `create_anim_blueprint` → `add_anim_state` → `add_anim_transition`

Design rules:

- keep state machines under 10-15 states; use sub-state machines for deeper branching
- blend spaces inside states handle clip variation; states handle gameplay logic
- tune blend times: responsive (0.1-0.15s), locomotion (0.2-0.3s), relaxed (0.3-0.5s)

## One-Shot Action Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create montage from clip | `create_anim_montage` | Configure sections for combo branching |
| Test action on live actor | `play_animation_montage` | Verify timing and return-to-locomotion |

One-shot workflow: `create_anim_montage` → integrate slot in AnimBP → `play_animation_montage` → verify

Design rules:

- montages for discrete actions only, never as locomotion substitute
- set up notifies for gameplay events (damage frame, VFX, sound)
- test interruption: what happens if the player acts during recovery

## Retargeting Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create source/target IK rig | `create_ik_rig` | One per skeleton. Defines bone chains for retargeting |
| Create retargeter between rigs | `create_ik_retargeter` | Maps chains from source rig to target rig |
| Add chain mapping | `add_ik_retarget_chain` | Manual chain mapping when auto-map is insufficient |
| Auto-map chains | `auto_map_retarget_chains` | Verify results — auto-mapping is a suggestion, not a guarantee |
| Set retarget reference pose | `create_retarget_pose` | Establish the reference pose for the retarget |
| Switch retarget pose | `set_current_retarget_pose` | Swap between reference poses |
| Transfer clips to target | `retarget_animation` | Output needs visual review, not just import confirmation |

Retargeting workflow: `create_ik_rig` (source) → `create_ik_rig` (target) → `create_ik_retargeter` → `add_ik_retarget_chain` (if needed) → `retarget_animation` → visual review

Design rules:

- compare source and target skeleton hierarchies before starting
- verify output on target character at gameplay camera distance
- check ground contact, stride length, hand placement, posture
- retarget is a starting point, not a finished result

## Pose Search Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create motion-matching database | `create_pose_search_database` | Infrastructure only — needs a runtime consumer graph |
| Populate with clips | `add_animation_to_pose_search_database` | Curate clips for pose coverage, not just quantity |
| Build search index | `build_pose_search_database_index` | Rebuild after adding or removing clips |
| Inspect database state | `inspect_pose_search_database` | Check coverage before runtime testing |

Pose search workflow: `create_pose_search_database` → `add_animation_to_pose_search_database` → `build_pose_search_database_index` → `inspect_pose_search_database`

Design rules:

- a database without a motion-matching graph is dead infrastructure
- clips need to cover the full pose space (locomotion, transitions, actions)
- incomplete coverage creates visible jumps between clips at runtime

## Actor Application Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Apply AnimBP to actor | `set_animation_blueprint` | Connects the authored system to a live skeletal mesh |
| Apply skeletal mesh | `set_skeletal_mesh` | Assigns the mesh before animation can apply |
| Play a montage on actor | `play_animation_montage` | Exercise one-shot actions on the runtime character |

Application workflow: `set_skeletal_mesh` (if needed) → `set_animation_blueprint` → `play_animation_montage` (test actions)

## Verification Tools

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Test in gameplay | `run_quick_playtest` | Required after every significant animation change |
| Viewport analysis | `observe_ue_project` | Captures screenshot + actor census + analysis |
| Position camera | `set_viewport_location` | Set to gameplay camera distance for review |
| Screenshot for review | `capture_viewport_sync` | Blocking capture with optional analysis |
| Focus on actor | `look_at_and_capture` | Moves camera to actor, captures, and analyzes |

Verification checklist:

- locomotion at gameplay speed (not slow preview)
- transitions between states (no pops, no sliding)
- foot contact and ground alignment
- montage timing in combat flow
- interruption scenarios (run → attack → dodge → idle)
- silhouette readability at camera distance

## Default Lanes

### Locomotion Lane

Use for idle, walk, run, directional travel, starts, stops, and turns.

Main tools: `create_blend_space`, `create_blend_space_1d`, `create_anim_blueprint`, `add_anim_state`, `add_anim_transition`, `set_animation_blueprint`

### One-Shot Action Lane

Use for attacks, interactions, reactions, emotes, and combo chains.

Main tools: `create_anim_montage`, `play_animation_montage`, `run_quick_playtest`

### Retargeting Lane

Use for reusing motion on a different rig or standardizing imported content to project rigs.

Main tools: `create_ik_rig`, `create_ik_retargeter`, `add_ik_retarget_chain`, `auto_map_retarget_chains`, `retarget_animation`

### Pose Search Lane

Use for authoring the database side of motion matching (only when runtime consumer exists).

Main tools: `create_pose_search_database`, `add_animation_to_pose_search_database`, `build_pose_search_database_index`, `inspect_pose_search_database`

### Verification Lane

Use for runtime proof — always the final step.

Main tools: `run_quick_playtest`, `observe_ue_project`, `set_viewport_location`, `capture_viewport_sync`, `look_at_and_capture`

## Proven Sequences

### Full Locomotion Setup

`create_blend_space` or `create_blend_space_1d` → `create_anim_blueprint` → `add_anim_state` (idle, walk, run, airborne) → `add_anim_transition` (tuned per pair) → `set_animation_blueprint` → `set_viewport_location` (gameplay distance) → `capture_viewport_sync` → `run_quick_playtest`

### Combat Action Pass

`create_anim_montage` (per action) → integrate montage slot into AnimBP → `play_animation_montage` (test each) → `look_at_and_capture` → `run_quick_playtest` (test interruption and combo flow)

### Retargeted Motion Reuse

`create_ik_rig` (source) → `create_ik_rig` (target) → `create_ik_retargeter` → `add_ik_retarget_chain` (if needed) → `retarget_animation` → review at gameplay distance → build blend space or montage from retargeted clips → `set_animation_blueprint` → `run_quick_playtest`

### Full Character Pipeline

Rig setup (if retargeting) → `create_blend_space` → `create_anim_blueprint` → `add_anim_state` → `add_anim_transition` → `create_anim_montage` (per action) → `set_animation_blueprint` → `play_animation_montage` (test actions) → `run_quick_playtest` → iterate

### Pose Search Database Pass

`create_pose_search_database` → `add_animation_to_pose_search_database` (curated clips) → `build_pose_search_database_index` → `inspect_pose_search_database`

## Rules

- Do not use montages as the default answer for locomotion.
- Do not retarget blind without making rig assumptions explicit.
- Do not judge motion only in close-up preview — always verify at gameplay camera distance.
- Do not build pose-search databases as a substitute for runtime graph design.
- Do not use one default blend time for all transitions — tune per transition type.
- Do not skip interruption testing for combat actions.
- Do not call animation done until the motion reads correctly in play.

## Key Takeaway

The animation surface is strongest when you separate locomotion logic, discrete action logic, retargeting, motion-matching database setup, and runtime proof into distinct lanes with the right tools for each. Trying to collapse those into one asset type or one workflow produces brittle motion.
