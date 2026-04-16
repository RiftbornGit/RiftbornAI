# 05 - Tool Guide

Use exact registered cinematic tool names. If a name is uncertain, verify with `describe_tool`.

## Cinematic Tool Map

### Sequence Authoring

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create a new sequence asset | `create_level_sequence` | Main container for the cutscene |
| Bind an actor or camera into the sequence | `add_sequence_track` | Creates the track-binding lane on the explicit production surface |

Design rules:

- one sequence per scene or beat — do not pack unrelated beats into a single sequence
- bind only the actors the sequence actually controls; excess bindings cause confusion during playback review

### Camera Setup

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Spawn a cinematic camera | `spawn_camera` | Prefer `CineCamera` when shot control matters |
| Tune lens and framing properties | `set_camera_properties` | Use for FOV, aspect ratio, focal length, focus, and filmback |

Design rules:

- spawn cameras with shot purpose already decided — do not create orphan cameras to figure out later
- set FOV and focal length deliberately; defaults rarely match the intended shot character

### Sequencer Verification

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Bring the sequence editor forward | `focus_asset_editor`, `focus_editor_tab` | Helpful before editor-local inspection |
| Open the target Level Sequence asset | `open_sequence` | Useful before editor-local inspection when the target sequence asset is known |
| Inspect the active Sequencer state | `get_sequencer_editor_context` | Shows what sequence, tracks, sections, and bindings are active |
| List existing object bindings | `list_sequence_bindings` | Good for verifying the actual bound set |
| Assert a specific binding exists | `assert_sequence_binding_exists` | Use before assuming a subject is really in the sequence |
| Play the current sequence for timing review | `play_sequence` | Fast transport proof for cuts, timing, and camera motion |
| Stop the current sequence cleanly | `stop_sequence` | Reset transport between review passes |

Design rules:

- inspect Sequencer context before editing bindings blind — the editor state may not match assumptions
- assert important bindings explicitly rather than trusting memory from earlier steps

### Review And Render

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Review framing in editor | `set_viewport_location`, `capture_viewport_sync`, `look_at_and_capture` | Faster iteration than full render passes |
| Render final output proof | `render_sequence` | Late-stage proof, not first-pass iteration |

Design rules:

- review in viewport first, render last — rendering is slow and should validate, not discover
- capture multiple angles to check continuity between cuts

## Default Lanes

### Sequence Authoring Lane

Use for:

- creating the cinematic container
- binding actors and cameras

Main tools:

- `create_level_sequence`
- `add_sequence_track`

### Camera Setup Lane

Use for:

- placing shot cameras
- tuning lens and framing properties

Main tools:

- `spawn_camera`
- `set_camera_properties`

### Sequencer Verification Lane

Use for:

- checking the active sequence editor
- confirming the intended bindings exist

Main tools:

- `focus_asset_editor`
- `focus_editor_tab`
- `open_sequence`
- `get_sequencer_editor_context`
- `list_sequence_bindings`
- `assert_sequence_binding_exists`
- `play_sequence`
- `stop_sequence`

### Review And Render Lane

Use for:

- reviewing compositions and beats
- producing final output proof

Main tools:

- `set_viewport_location`
- `capture_viewport_sync`
- `look_at_and_capture`
- `render_sequence`

## Verification Checklist

Before calling a cinematic surface done:

- [ ] shot purpose defined for every camera in the sequence
- [ ] cameras spawned with appropriate lens properties (`spawn_camera`, `set_camera_properties`)
- [ ] sequence created and important actors bound (`create_level_sequence`, `add_sequence_track`)
- [ ] Sequencer context inspected and bindings confirmed (`get_sequencer_editor_context`, `list_sequence_bindings`)
- [ ] critical bindings asserted explicitly (`assert_sequence_binding_exists`)
- [ ] framing and composition reviewed from camera POV (`capture_viewport_sync`, `look_at_and_capture`)
- [ ] continuity between cuts checked (no spatial confusion at transition points)
- [ ] final render produced only after shot review is clean (`render_sequence`)

## Proven Sequences

### New Shot Pass

`spawn_camera` -> `set_camera_properties` -> `create_level_sequence` -> `add_sequence_track` -> `get_sequencer_editor_context` -> `assert_sequence_binding_exists`

Use for: creating a new cinematic from scratch — character reveal, establishing shot, action beat.

### Existing Sequence Audit

`focus_asset_editor` -> `get_sequencer_editor_context` -> `list_sequence_bindings` -> targeted review captures

Use for: diagnosing binding drift, checking what a sequence actually contains, preparing for revision.

### Final Render Pass

shot review complete -> bindings verified -> `render_sequence`

Use for: producing the final video output after all shots have been iteratively reviewed and approved.

### Multi-Camera Coverage Pass

`spawn_camera` (camera A) -> `spawn_camera` (camera B) -> `set_camera_properties` on each -> `add_sequence_track` for both -> review each angle -> `capture_viewport_sync` from each -> compare coverage

Use for: scenes requiring multiple coverage angles — dialogue over-the-shoulder pairs, action with wide and close options.

## Rules

1. Do not start with render output when the sequence has not been reviewed in viewport.
2. Do not assume actors are bound just because they exist in the level — assert bindings explicitly.
3. Do not let camera motion replace shot purpose — every move should serve discovery, energy, or tension.
4. Do not rely on editor memory; inspect the active Sequencer context before edits.
5. Do not call a cinematic finished until the beats read cleanly in viewport review.
6. Do not use the same framing for every shot — rhythm comes from contrast in size, duration, and movement.
7. Render is proof, not iteration — use viewport capture to iterate.

## Key Takeaway

The cinematic surface is strongest when you separate:

- shot planning
- camera setup
- sequence binding
- Sequencer proof
- final rendering

If you collapse those into one rushed pass, the cutscene usually looks expensive and confused.
