# 03 — Anti-Patterns

Audio usually fails through overuse, not absence.

These are the patterns that make scenes noisy, repetitive, or spatially unbelievable.

## Anti-Pattern: One Loop Per Visible Thing

Symptoms:

- every torch, fan, waterfall, or prop gets its own full-strength loop
- the map is full of emitters with no mix hierarchy

Why it is bad:

- creates constant masking
- destroys headroom
- makes the world louder instead of clearer

Do instead:

- decide which sources must read individually
- let secondary detail be implied by a smaller number of intentional emitters

## Anti-Pattern: Auditioning Only In Solo Preview

Symptoms:

- the sound seems great in `play_metasound_preview`
- it disappears or becomes annoying once placed in the real scene

Why it is bad:

- isolated preview hides mix conflict and distance problems

Do instead:

- preview first, then verify in-world and in playtest

## Anti-Pattern: Blanket Reverb

Symptoms:

- caves, halls, rooms, and tunnels all get similar wash
- reverb is used as a vibe slider instead of a space description

Why it is bad:

- weakens spatial storytelling
- muddies already busy scenes

Do instead:

- use `analyze_acoustics` when the room shape should drive the tuning
- think about reflectivity, size, and distance cues before adding reverb

## Anti-Pattern: Full-Spectrum Layering Everywhere

Symptoms:

- ambience, combat, foley, and music all occupy the same band and intensity
- every new sound is added on top instead of making room

Why it is bad:

- important gameplay cues stop reading
- the scene sounds “busy” rather than believable

Do instead:

- assign sonic jobs and frequency space
- reduce or remove competing layers before adding more

## Anti-Pattern: Procedural Complexity Without Purpose

Symptoms:

- large MetaSound graphs built because procedural equals advanced
- many nodes with little audible payoff

Why it is bad:

- increases authoring and debug cost
- makes future tuning harder without improving player experience

Do instead:

- start with the smallest graph that supports the role
- add variation only when repetition or responsiveness actually needs it

## Anti-Pattern: Preset Explosion

Symptoms:

- new assets are cloned constantly for tiny variations
- graph maintenance becomes fragmented across many near-duplicates

Why it is bad:

- fixes and improvements stop propagating cleanly

Do instead:

- keep a reusable source
- use `create_metasound_preset` when only exposed defaults need to vary

## Anti-Pattern: Placement By Visual Guessing Alone

Symptoms:

- emitters are dropped near visible props without thinking about where the player should hear them from
- attenuation and environmental role are secondary

Why it is bad:

- spatial audio stops matching player perception
- the scene feels fake even if the raw sounds are good

Do instead:

- use `audio_spatial` to get a placement plan
- place emitters for perceived source and gameplay route, not only for asset location

## Anti-Pattern: Static Ambience With No Variation

Symptoms:

- loops repeat identically for long stretches
- the world sounds mechanical

Why it is bad:

- repetition becomes more noticeable than atmosphere

Do instead:

- use exposed inputs, presets, or graph variation where needed
- keep identity stable while varying timing or texture

## Anti-Pattern: Declaring Done Without Gameplay Verification

Symptoms:

- the audio exists in the map
- no one checks how it interacts with movement, combat, or other emitters

Why it is bad:

- many audio problems only appear while the player is moving or under load

Do instead:

- run `run_quick_playtest`
- confirm the intended sound still reads while gameplay is happening

## Key Takeaway

The common audio failure is not “too little.” It is “too much, too often, without hierarchy.”

Fight that by keeping role, space, and mix clarity ahead of novelty.
