# 02 - UE5 Sequencer And Camera Systems

RiftbornAI's cinematic surface is centered on Level Sequence creation, camera placement, sequence-track authoring, live Sequencer context inspection, binding verification, and render output.

```
┌──────────────────────────────────────────────────────┐
│ Sequencer Pipeline                                    │
│                                                       │
│ Level Sequence (container)                            │
│   ├── Object Bindings (actor → track)                 │
│   │     ├── Transform track                           │
│   │     ├── Animation track                           │
│   │     └── Component property tracks                 │
│   ├── Camera Cuts (which camera is active per frame)  │
│   └── Audio / Event tracks                            │
│                                                       │
│ Camera Actors (scene objects)                          │
│   ├── CineCamera — filmback, focus, lens simulation   │
│   └── Standard Camera — basic position/rotation       │
│                                                       │
│ Output                                                │
│   └── render_sequence → image sequence or video       │
└──────────────────────────────────────────────────────┘
```

## Level Sequences Are The Main Container

Use:

- `create_level_sequence`
- `add_sequence_track`

for the default production authoring lane.

This is the core path for:

- making a cinematic asset
- binding actors into the sequence
- adding the needed tracks to drive those bindings

## Cameras Are Real Scene Actors

Use:

- `spawn_camera`
- `set_camera_properties`

to create and tune the camera actors the sequence will use.

This gives explicit control over:

- camera type
- FOV
- aspect ratio
- focal length
- focus and lens behavior

## Use Editor Context Tools Before Blind Edits

Use:

- `focus_asset_editor`
- `focus_editor_tab`
- `get_sequencer_editor_context`
- `list_sequence_bindings`
- `assert_sequence_binding_exists`

to inspect what Sequencer is actually showing and what is already bound.

This matters because cinematic work is often editor-local:

- which sequence is active
- which bindings exist
- which tracks or sections are selected

Those facts should be observed, not guessed.

## Rendering Is The Final Proof Lane

Use:

- `render_sequence`

when the sequence is ready for output proof.

Treat this as a later-stage validation step.
Do not use it as the first way you discover that the shot plan or bindings are wrong.

## Supporting Review Tools Matter

Use:

- `capture_viewport_sync`
- `set_viewport_location`
- `look_at_and_capture`

to review camera framing, composition, and scene readability before committing to full render passes.

## Current Strength Of The Surface

The cinematic tool surface is strongest for:

- sequence creation
- actor binding and track creation
- camera placement and tuning
- Sequencer context inspection
- binding verification
- final sequence rendering

There may be adjacent experimental or specialized helpers in the repo, but the default shipped lane is the explicit sequence-and-camera workflow above.

## Key Takeaway

Treat the current cinematic lane as:

shot plan -> cameras -> sequence bindings -> Sequencer proof -> render

If you skip the middle proof steps, rendering becomes an expensive debugging tool.
