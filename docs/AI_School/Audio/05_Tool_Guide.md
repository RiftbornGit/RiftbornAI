# 05 — Tool Guide

Use exact registered audio tool names. If a name is uncertain, verify with `describe_tool`.

## MetaSound Authoring Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `create_metasound_source` | Create a playable MetaSound source asset | Main path for world-playable procedural or hybrid sources. Use for ambient loops, one-shots, gameplay cues. |
| `create_metasound_preset` | Create a variation from an existing source | When only exposed defaults need to change. Keeps graph shared, prevents duplication drift. |
| `create_soundscape_metasound_source` | Scaffold controllable ambience | UE 5.7 builder lane. Best for atmosphere-first sound design with layer control. |
| `list_metasounds` | List project MetaSound assets | Discover existing assets before creating new ones. Prevents duplication. |
| `get_metasound_info` | Inspect graph structure and exposed inputs | Read nodes, connections, inputs, outputs, metadata. Use before editing existing graphs. |
| `list_metasound_node_types` | Discover available graph node classes | Use before graph edits. Do not guess node names. |
| `add_metasound_node` | Add a node to a MetaSound graph | Controlled graph construction. |
| `connect_metasound_nodes` | Wire MetaSound graph nodes | Deterministic graph wiring. |
| `set_metasound_input` | Set exposed defaults and tunable parameters | Tune inputs for variation, presets, and runtime control. |
| `play_metasound_preview` | Audition a MetaSound before world placement | Required before emitter placement. Catches graph mistakes early. |

Design rules:

- always preview before placement — fix graph problems before placement problems
- use `list_metasound_node_types` before assuming a node exists
- keep graphs as small as they can be while supporting the role
- use presets for variation instead of duplicating near-identical graphs

## Audio Import Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `import_audio` | Bring source audio material into the project | When using existing samples. Imported audio still needs MetaSound wrapping and placement work. |

Design rules:

- imported samples are not finished audio — they need role assignment, graph strategy, and placement
- wrap imported audio in a MetaSound source for world playback

## World Placement And Spatial Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `audio_spatial` | Analyze scene features and recommend emitter placement | Before spawning emitters. Gives placement plan based on environmental cues, not guesswork. |
| `spawn_audio_component` | Place a world audio emitter | After the graph and placement role are understood. |
| `set_audio_component` | Tune emitter attachment, autoplay, attenuation, behavior | After placement — refine the emitter's runtime behavior. |
| `analyze_acoustics` | Compute reverb from room geometry and spawn AudioVolume | For enclosed spaces — caves, tunnels, halls, interior rooms. Creates real UReverbEffect + AAudioVolume. |

Design rules:

- use `audio_spatial` to plan before spawning blindly
- emitters should align with gameplay path and listener distance, not just visual prop position
- do not overuse emitters — fewer well-placed sources beat many competing ones
- `analyze_acoustics` drives reverb from real geometry, not generic presets

## Verification Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `run_quick_playtest` | Listen during gameplay | Required after meaningful audio changes. Catches masking, distance, and readability problems. |
| `play_metasound_preview` | Audition in isolation | First check for graph correctness. Not a substitute for in-world verification. |

Design rules:

- solo preview is necessary but not sufficient — always verify in gameplay context
- listen for: cue readability during action, masking between layers, spatial believability, variation over time

## Default Lanes

### MetaSound Authoring Lane

Main tools: `create_metasound_source`, `create_metasound_preset`, `create_soundscape_metasound_source`, `list_metasounds`, `get_metasound_info`, `list_metasound_node_types`, `add_metasound_node`, `connect_metasound_nodes`, `set_metasound_input`, `play_metasound_preview`

Use for: building playable sources, graph shaping, exposing tunable parameters, previewing before level placement.

### World Placement And Acoustic Lane

Main tools: `audio_spatial`, `spawn_audio_component`, `set_audio_component`, `analyze_acoustics`, `run_quick_playtest`

Use for: deciding where audio should be heard from, spawning emitters, tuning room response, verifying in gameplay context.

## Verification Checklist

Before calling an audio surface done:

- [ ] source previewed and correct (`play_metasound_preview`)
- [ ] placement plan considered (`audio_spatial`)
- [ ] emitters placed and tuned (`spawn_audio_component`, `set_audio_component`)
- [ ] enclosed spaces tuned if applicable (`analyze_acoustics`)
- [ ] verified in gameplay context (`run_quick_playtest`)
- [ ] no masking of critical gameplay cues
- [ ] variation sufficient to avoid repetition fatigue
- [ ] mix headroom preserved for combat and interaction sounds

## Proven Sequences

### New Ambient Source

`create_metasound_source` or `create_soundscape_metasound_source` → `set_metasound_input` → `play_metasound_preview` → `audio_spatial` → `spawn_audio_component` → `set_audio_component` → `run_quick_playtest`

Use for: forest ambience, wind beds, water sounds, campfire loops.

### Existing MetaSound Cleanup Or Tuning

`list_metasounds` → `get_metasound_info` → `list_metasound_node_types` → `add_metasound_node` or `connect_metasound_nodes` → `set_metasound_input` → `play_metasound_preview`

Use for: fixing graph issues, adding variation, exposing new controls.

### Interior Space Pass

`audio_spatial` → `spawn_audio_component` → `analyze_acoustics` → `set_audio_component` → `run_quick_playtest`

Use for: cave ambience, dungeon rooms, indoor environments.

### Sample-Driven Import And Placement

`import_audio` → `create_metasound_source` → `play_metasound_preview` → `create_metasound_preset` for variants → `audio_spatial` → `spawn_audio_component`

Use for: using existing recordings, creating sample-based sound effects.

### Full Scene Audio Pass

Define role map → create all sources → preview each → plan spatial layout → place emitters → tune acoustics → playtest full scene

Use for: complete audio passes on new or existing levels.

## Rules

1. Do not place emitters before the underlying source has been previewed.
2. Do not use more emitters than the scene needs to read clearly.
3. Do not treat reverb as a generic "big space" shortcut.
4. Do not trust solo preview as final verification.
5. Use presets for variation when the graph should stay shared.
6. Do not add procedural complexity without audible payoff.
7. Every sound needs a role before it needs a graph.

## Key Takeaway

The audio surface is strongest when you separate: source authoring, graph control, world placement, acoustic tuning, and gameplay verification. Skipping any layer means the audio exists without serving the scene.
