# 02 — UE5 Animation And Retargeting Systems

RiftbornAI's animation surface covers Animation Blueprint scaffolding, state-machine structure, blend-space authoring, montage creation, retargeting setup, and pose-search database authoring.

## System Architecture

The runtime animation stack has clear layers:

```
Skeletal Mesh Component
  └── Animation Blueprint (AnimBP)
        ├── State Machine(s)
        │     ├── States (each plays a pose source)
        │     │     ├── Single animation asset
        │     │     ├── Blend Space (1D or 2D)
        │     │     └── Sub-state machine
        │     └── Transitions (rules + blend config)
        ├── Blend Layers (additive, per-bone, etc.)
        ├── Montage Slot Nodes (for one-shot overlays)
        └── IK / Post-Process nodes
```

The AnimBP is the runtime controller. Everything else (blend spaces, montages, state machines) feeds into it.

## Animation Blueprints Control Continuous Runtime Behavior

Use:

- `create_anim_blueprint`
- `add_anim_state`
- `add_anim_transition`

The AnimBP is the right lane for:

- idle, walk, run, and directional locomotion structure
- state-driven runtime behavior (grounded → airborne → landing)
- organizing which motion plays when, based on gameplay variables

Design rules for state machines:

- states should represent **gameplay states**, not individual clips
- keep state count manageable (under 10-15 per machine)
- use sub-state machines when a state has internal complexity (e.g., the "combat" state has sub-states for different weapon stances)
- transitions need explicit blend times and rules, not just "always true" with defaults

## Blend Spaces Support Parameter-Driven Motion

Use:

- `create_blend_space` — for 2D blending (speed × direction)
- `create_blend_space_1d` — for 1D blending (speed only, lean amount, etc.)

Blend spaces are the normal lane for locomotion blending. They take runtime parameters (speed, direction angle) and interpolate between placed clips.

Key considerations:

- clip placement in the blend space matters — clips at the wrong parameter values produce sliding or wrong speed
- blend spaces work best with clips that share compatible timing (similar foot contact phase)
- use sync markers on clips to prevent foot-phase mismatches during blending
- 2D blend spaces typically use speed on Y and direction on X for standard locomotion

## Montages Are For Discrete Actions

Use:

- `create_anim_montage`
- `play_animation_montage`

Montages are for:

- attacks, combos, and special moves
- emotes and interactions
- hit reactions and death animations
- any action with clear start, play, and end

Montages play through **slot nodes** in the AnimBP, overlaying or replacing the current pose. Key features:

- **sections** — named time ranges within the montage for branching (combo chains)
- **notifies** — fire gameplay events at specific frames (spawn VFX, enable damage, play sound)
- **blend in/out** — control how the montage merges with the underlying locomotion

Do not use montages as the main locomotion system. They lack parameter-driven blending.

## Applying Animation To Actors

Use:

- `set_animation_blueprint`

to assign the authored AnimBP to a skeletal mesh component on a live actor.

This is where the system stops being an asset library and starts being runtime behavior. Until an AnimBP is applied to an actor in a level, it is untested infrastructure.

## Retargeting Needs Explicit Rig Setup

The retargeting pipeline has three steps:

1. **Create IK Rigs** for source and target skeletons — `create_ik_rig`
2. **Create an IK Retargeter** that maps chains between the two rigs — `create_ik_retargeter`
3. **Retarget clips** from source to target — `retarget_animation`

Additional retargeting tools:

- `auto_map_retarget_chains` — automatic chain mapping (verify results)
- `create_retarget_pose` — set the reference pose for retargeting
- `set_current_retarget_pose` — switch between retarget poses
- `add_ik_retarget_chain` — manually add chain mappings

Retargeting is valuable when source content is high quality but the target character uses a different skeleton. However:

- proportional differences cause stride, reach, and contact issues
- bone hierarchy mismatches produce joint errors
- the retarget output must be reviewed on the target character, not just confirmed as "imported"

## Pose Search Is Database Authoring, Not Full Motion Matching

Use:

- `create_pose_search_database`
- `add_animation_to_pose_search_database`
- `build_pose_search_database_index`
- `inspect_pose_search_database`

These tools set up the searchable database side of motion matching.

Reality check:

- the database depends on real, high-quality animation clips that cover the needed pose space
- it depends on a real motion-matching animation graph that queries and selects poses
- database creation alone does not produce a complete runtime motion-matching solution
- incomplete pose coverage creates visible jumps between clips at runtime

## Runtime Proof Uses Gameplay Verification

Runtime proof is not optional. Use:

- `run_quick_playtest` — test the character in actual gameplay
- `observe_ue_project` — viewport analysis of the scene
- `set_viewport_location` — position camera at gameplay distance
- `capture_viewport_sync` — screenshot for transition review
- `look_at_and_capture` — focus on specific actor and analyze

Judge:

- locomotion at gameplay speed (not slow preview)
- transition quality between states
- foot contact and ground sliding
- montage timing in combat flow
- silhouette readability at camera distance

## Current Surface Strength

Strongest for:

- AnimBP scaffolding and state machine authoring
- locomotion blend-space setup (1D and 2D)
- montage creation and playback
- IK rig and retargeting pipeline
- pose-search database authoring

Not a substitute for:

- clip quality (the tools transport clips, they do not create motion)
- motion design taste (blend times, weight, timing are creative decisions)
- full animation graph architecture beyond the supported authoring primitives

## Key Takeaway

Pick the asset type that matches the motion role:

- AnimBP for continuous state logic
- blend spaces for parameter-driven locomotion
- montages for discrete actions
- IK retargeting tools for cross-rig transfer
- pose-search tools for database setup only

The tools build the structure. The quality comes from choosing the right structure for each motion job.
