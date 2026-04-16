# RiftbornAI Audio School

**Read this track before creating or editing MetaSound assets, placing audio emitters in the world, tuning ambience, or making room/reverb decisions.**

Game audio is not a pile of sounds. It is a perception system:

- it tells the player what matters
- it supports distance, scale, and threat
- it keeps multiple sound roles from masking each other
- it connects the physical space to the emotional tone of the scene

MetaSound is a powerful graph tool, but it does not replace mix judgment.

## Curriculum

1. **[01_Game_Audio_And_Mix_Intent.md](01_Game_Audio_And_Mix_Intent.md)** — Gameplay readability, diegetic role, mix separation, and sonic hierarchy.
2. **[02_UE5_Audio_And_Metasound_Systems.md](02_UE5_Audio_And_Metasound_Systems.md)** — MetaSound asset types, graph flow, preview, emitter placement, and the current supported RiftbornAI tool surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common mistakes that create muddy, noisy, or context-free audio.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for audio design, graph authoring, placement, acoustics, and verification.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for MetaSound creation, placement, acoustics analysis, and playtest validation.

## Core Rules

- Decide the gameplay role of a sound before building the graph.
- Keep the mix readable; not every sound should compete for the same frequency and loudness space.
- Preview MetaSound assets before placing them in the world.
- Place emitters around the player experience, not just around visible props.
- Tune enclosed spaces with acoustic intent, not blanket reverb.
- Verify in gameplay context, not only in solo preview.
