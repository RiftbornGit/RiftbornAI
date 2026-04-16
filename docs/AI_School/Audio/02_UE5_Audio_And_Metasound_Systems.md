# 02 — UE5 Audio And MetaSound Systems

This repo has a real Audio and MetaSound authoring lane. Use it deliberately.

## MetaSound Asset Types Matter

The supported tool surface distinguishes between:

- MetaSound sources
- MetaSound presets
- graph-level edits and previews

### MetaSound Source

Use `create_metasound_source` when you need a playable audio asset for world use.

This is the normal path for:

- ambient loops
- procedural one-shots
- reactive environmental sources
- gameplay-facing sound generators

### MetaSound Preset

Use `create_metasound_preset` when you already have a useful source/pattern and want variation through different exposed defaults rather than duplicating the whole graph.

### Soundscape Builder Source

Use `create_soundscape_metasound_source` when the job is a controllable ambience scaffold on the UE 5.7 MetaSound Builder surface.

This is especially useful for atmosphere-first sound design where exposed controls matter more than bespoke low-level graph building on day one.

## Supported MetaSound Graph Tools

The current graph surface includes:

- `list_metasounds`
- `get_metasound_info`
- `list_metasound_node_types`
- `add_metasound_node`
- `connect_metasound_nodes`
- `set_metasound_input`
- `play_metasound_preview`

This means you can:

- discover existing MetaSound assets
- inspect graph structure and exposed inputs
- add node types intentionally
- wire graphs deterministically
- set controllable defaults
- audition the result before world placement

## Preview Before Placement

`play_metasound_preview` exists because many graph mistakes should be caught before emitter placement.

Use preview to answer:

- does the graph actually produce the intended category of sound?
- do the exposed inputs respond meaningfully?
- is the loop or trigger behavior stable?

Do not skip directly to map placement unless you are reusing a proven source.

## Node Discovery Is Part Of The Workflow

`list_metasound_node_types` matters because procedural audio authoring depends on real available node classes, not guesses.

Use it before graph edits instead of assuming the engine exposes a node with the exact name you want.

## World Placement Is A Separate Concern

MetaSound authoring answers “what does the sound do?”

World placement answers “where should the player hear it from?”

For that lane, use:

- `spawn_audio_component`
- `set_audio_component`
- `audio_spatial`
- `analyze_acoustics`

### `audio_spatial`

This analyzes scene features and recommends emitter placement and attenuation strategy.

Use it when you need a placement plan for:

- water
- foliage/insects/birds
- fire
- enclosed room tone
- open terrain wind beds

### `analyze_acoustics`

This is not just a graph tool. It analyzes room geometry, computes reverb behavior, creates a `UReverbEffect`, and spawns an `AAudioVolume`.

Use it when the space itself should shape the sound.

## Imported Audio Still Needs Structure

`import_audio` helps bring source material into the project, but imported samples are not a finished design.

They still need:

- role assignment
- graph or preset strategy
- emitter placement
- playtest verification

## Runtime Emitters Need Intentional Settings

`spawn_audio_component` and `set_audio_component` are for in-world playback behavior.

This is where you shape:

- attachment or location
- autoplay behavior
- emitter presence in the level
- adjustments after placement

Do not confuse emitter existence with a finished spatial mix.

## Key Takeaway

Audio work in this repo has three layers:

1. source creation or import
2. MetaSound graph/preset shaping
3. world placement and acoustic verification

You need all three when building shippable audio, not just the first one that happens to succeed.
