# 04 - Workflow

Cinematic work should move from shot purpose to camera setup to Sequencer proof to render.

## Phase 1: Define The Shot List

Before touching Sequencer, answer for each beat:

- what should the viewer notice in this shot
- what function does the shot serve (orient, reveal, emphasize, transition, payoff)
- what shot size is appropriate (wide for space, medium for action, close for emotion)
- what motion is needed (static for tension, push-in for discovery, pull-back for scale)
- what is the cut logic into and out of this shot

Write the shot list as a simple plan:

```
Beat 1: Wide establishing — show the arena from above. Static. 3 seconds.
Beat 2: Medium tracking — follow the character entering. Slow dolly. 4 seconds.
Beat 3: Close — character's face as they see the threat. Static, shallow focus. 2 seconds.
Beat 4: Wide reverse — reveal the threat from character POV. Push-in. 3 seconds.
```

Do not start from camera movement presets. Start from what the sequence needs to communicate.

## Phase 2: Prepare The Cameras

Use:

- `spawn_camera`
- `set_camera_properties`

to create the needed camera actors and tune their framing behavior.

This is where shot size and visual character start becoming concrete.

## Phase 3: Create The Sequence Container

Use:

- `create_level_sequence`

to create the Level Sequence asset that will hold the cinematic.

## Phase 4: Bind Actors And Tracks

Use:

- `add_sequence_track`

to bind the actors or cameras that the sequence actually needs.

## Phase 5: Inspect Sequencer Context

Use:

- `focus_asset_editor`
- `get_sequencer_editor_context`
- `list_sequence_bindings`
- `assert_sequence_binding_exists`

to verify that the active sequence and its bindings match the intended shot plan.

## Phase 6: Review The Frames Before Rendering

Use:

- `set_viewport_location` — position camera at each shot's key frame
- `capture_viewport_sync` — capture the frame for comparison
- `look_at_and_capture` — check subject readability from each camera position

Review each shot for:

- is the subject readable and in the intended position
- is the composition serving the shot's function
- does the framing convey the intended scale and emotion
- is there spatial continuity between adjacent shots (screen direction, eyelines)
- does the sequence have shot-size variety (not all medium-wide)

Build a review loop across cuts:

```
set_viewport_location (shot 1 key frame)
capture_viewport_sync
set_viewport_location (shot 2 key frame)
capture_viewport_sync
compare: does the cut deliver new information? Is continuity maintained?
```

This is the cheapest place to fix problems. Rendering without this step makes every correction cost a full render cycle.

## Phase 7: Render Final Proof

Use:

- `render_sequence`

only after the shots and bindings are already making sense.

## Recommended Sequences

### Simple Character Reveal

`spawn_camera` -> `set_camera_properties` (tight FOV, shallow focus) -> `create_level_sequence` -> `add_sequence_track` -> `get_sequencer_editor_context` -> `assert_sequence_binding_exists` -> frame review via `capture_viewport_sync` -> `render_sequence`

One camera, one subject, one beat. The simplest cinematic structure.

### Multi-Actor Beat

`create_level_sequence` -> bind the important actors and cameras with `add_sequence_track` -> `list_sequence_bindings` (verify all actors are bound) -> targeted review captures from each camera -> `render_sequence`

Scenes with dialogue, interaction, or ensemble staging need binding verification up front.

### Existing Sequence Revision

`focus_asset_editor` -> `get_sequencer_editor_context` -> `list_sequence_bindings` -> adjust the needed bindings or shot plan -> review frames via `capture_viewport_sync` -> `render_sequence`

Always inspect the current state before modifying an existing sequence.

### Multi-Angle Coverage Pass

`spawn_camera` (wide establishing) -> `spawn_camera` (medium action) -> `spawn_camera` (close detail) -> `set_camera_properties` on each -> `add_sequence_track` for all three -> review each angle via `set_viewport_location` + `capture_viewport_sync` -> compare coverage and choose the best edit points

Build coverage options before committing to a single cut.<br>

### Environment Establishing Shot

`spawn_camera` (high position, wide FOV) -> `set_camera_properties` -> `create_level_sequence` -> `add_sequence_track` -> slow dolly or static hold -> `capture_viewport_sync` -> verify the space reads clearly at this scale -> `render_sequence`

The viewer needs to understand the space before action begins.

## Key Takeaway

The safe order is:

define the shots first, prove the sequence second, render third.

If you reverse that order, the render queue becomes your first draft tool.
