# 04 — Workflow

Audio work should move from role to graph to space to verification.

## Phase 1: Define The Audio Role Map

Before touching MetaSound or emitter tools, decide:

- what category the sound belongs to (gameplay feedback, ambience, foley, combat punctuation, UI, music/mood)
- whether it is critical, supportive, or atmospheric
- whether it is world-space, player-feedback, or both
- how much mix priority it deserves relative to other sounds in the scene
- what frequency range it should own (low rumble, mid texture, high sparkle)

If that is unclear, the resulting graph and placement will be arbitrary.

Example role definitions:

- "campfire: atmospheric, world-space, low priority, mid-frequency crackle with low-end warmth, should not mask combat cues"
- "sword impact: critical gameplay feedback, player-space, high priority, transient mid-high punch with low-end thump, must cut through ambience"
- "cave wind: atmospheric, world-space, low priority, low-frequency drone with occasional mid whistle, should imply depth"

## Phase 2: Decide The Asset Strategy

Choose one:

- `import_audio` — bring existing samples into the project. Still needs graph and placement work.
- `create_metasound_source` — scaffold a playable procedural or hybrid source. Main path for gameplay-facing sounds.
- `create_metasound_preset` — derive a variation from an existing source. Use when only exposed defaults need to change.
- `create_soundscape_metasound_source` — scaffold an ambience-first MetaSound with controllable layers. Best for atmosphere work.

Decision guide:

- if you have a finished sample that needs no procedural behavior → `import_audio` then wrap in a simple MetaSound source
- if the sound needs pitch randomization, trigger variation, or parameter control → `create_metasound_source`
- if you already have a working source and need variations (different pitch, different filter, different volume) → `create_metasound_preset`
- if the task is ambient atmosphere with multiple controllable layers → `create_soundscape_metasound_source`

Do not build a large graph before deciding whether the sound should be sample-driven, preset-driven, or ambience-driven.

## Phase 3: Inspect Or Build The MetaSound

For existing assets:

1. `list_metasounds` — discover what already exists before creating new assets
2. `get_metasound_info` — read graph structure, nodes, connections, inputs, and outputs
3. `list_metasound_node_types` — discover available node classes before editing

For new or edited graphs:

1. `create_metasound_source` or `create_metasound_preset`
2. `list_metasound_node_types` — check available nodes before assuming names
3. `add_metasound_node` — add nodes intentionally
4. `connect_metasound_nodes` — wire the graph deterministically
5. `set_metasound_input` — set exposed defaults and tunable parameters

Graph authoring rules:

- keep the graph as small as it can be while supporting the role
- name exposed inputs meaningfully (not "Float1" — use "CrackleRate" or "WindIntensity")
- do not add procedural complexity without audible payoff
- if variation can be achieved through presets, prefer presets over graph duplication

## Phase 4: Preview Before World Placement

Use:

- `play_metasound_preview`

Confirm:

- the sound category is correct (does it sound like what it should be)
- loop/trigger behavior works (does it start, stop, and repeat correctly)
- exposed controls produce meaningful change (does adjusting "Intensity" actually sound different)
- the volume and character fit the intended role

Do not place a source in the map before the source itself behaves correctly. Fix graph problems before placement problems.

## Phase 5: Plan Spatial Placement

Before spawning emitters across a level, use:

- `audio_spatial`

This gives a placement plan based on environmental cues rather than pure guesswork.

Ask:

- where should the player perceive the source? (not necessarily where the visual prop is)
- which sounds need distinct localization? (rivers, specific machines, NPC voices)
- which sounds can be implied by a broader ambient bed? (general forest, wind, distant city)
- what is the gameplay route through this area? (path the player walks determines what they hear first)

Placement rules:

- emitters near gameplay paths need tighter attenuation (close, clear, distinct)
- background emitters can use wider attenuation (broad, ambient, supporting)
- do not place an emitter for every visual prop — decide which sources the player should actually perceive individually

## Phase 6: Place Emitters In World

Use:

- `spawn_audio_component` — place the emitter
- `set_audio_component` — tune attachment, autoplay, attenuation, and behavior

Emitter work should align with:

- gameplay path (the player will hear this from their walking route, not from a birds-eye view)
- camera/listener distance (what sounds like the right volume at editor distance may be wrong at gameplay distance)
- importance of the source (critical sources get tighter, louder emitters; ambient sources get wider, softer ones)

## Phase 7: Tune Enclosed Spaces

If the sound depends on room character, use:

- `analyze_acoustics`

This analyzes room geometry, computes reverb behavior, creates a `UReverbEffect`, and spawns an `AAudioVolume`.

This is the correct step for:

- caves
- tunnels
- halls and corridors
- interior rooms
- any space where the player should feel enclosure

Acoustic tuning rules:

- small spaces should feel tight and reflective, not just "reverby"
- large spaces should feel open and diffuse
- do not apply generic reverb — let room geometry drive the character
- transition between acoustic zones should feel natural (gradual change as the player moves between spaces)

## Phase 8: Verify In Context

Use:

- `run_quick_playtest`

Listen for:

- cue readability during gameplay (can the player hear what they need to hear while playing)
- masking between ambience and combat or interaction sounds (does anything get buried)
- believable spatial distance (does close sound close and far sound far)
- room response that matches the environment (does the cave sound like a cave)
- variation over time (does the world sound alive or robotic after 30 seconds)

If problems are found, return to the relevant phase:

- graph problems → Phase 3
- placement problems → Phase 5–6
- acoustic problems → Phase 7
- role or mix priorities wrong → Phase 1

## Recommended Sequences

### Ambient Zone

1. Define audio role map (atmospheric, low priority, broad frequency bed)
2. `create_soundscape_metasound_source` or `create_metasound_source`
3. `set_metasound_input` → tune layers and variation
4. `play_metasound_preview` → confirm it sounds correct
5. `audio_spatial` → get placement plan
6. `spawn_audio_component` → place emitters
7. `set_audio_component` → tune attenuation and behavior
8. `run_quick_playtest` → verify in gameplay context

### Reusable Procedural Source

1. `create_metasound_source`
2. `list_metasound_node_types` → discover available nodes
3. `add_metasound_node` → build graph
4. `connect_metasound_nodes` → wire
5. `set_metasound_input` → expose tunable parameters
6. `play_metasound_preview` → audition
7. `create_metasound_preset` → create variations if needed

### Interior Acoustic Pass

1. `audio_spatial` → analyze scene and plan placement
2. `spawn_audio_component` → place room-tone emitters
3. `analyze_acoustics` → compute reverb from room geometry
4. `set_audio_component` → tune emitter behavior
5. `run_quick_playtest` → verify room feels correct

### Sample-Driven Import And Placement

1. `import_audio` → bring sample into project
2. `create_metasound_source` → wrap in playable MetaSound
3. `play_metasound_preview` → confirm it works
4. `create_metasound_preset` → create variations if needed
5. `audio_spatial` → plan placement
6. `spawn_audio_component` → place in world

### Full Scene Audio Pass

1. Define role map for all sounds in the scene
2. Create or import all sources
3. Preview each source individually
4. Plan spatial placement for the whole scene
5. Place emitters
6. Tune enclosed spaces with acoustics
7. Playtest the full scene with all systems active

## Key Takeaway

The safe audio order is: role first, graph second, placement third, acoustics fourth, playtest last.

If you skip the role or placement steps, the scene may sound active without sounding intentional.
