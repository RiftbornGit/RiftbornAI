# RiftbornAI Streaming School

**Read this track before creating or editing World Partition data layers, streaming sources, streaming-state layouts, or texture-streaming diagnostics.**

Streaming is not just performance plumbing. It is world continuity management:

- it decides what can exist near the player
- it controls how world states appear and disappear
- it protects scale without losing orientation
- it must avoid pop-in, contradiction, and broken presence

The tools can create layers and inspect partition state quickly, but good streaming still starts from clear spatial and state logic.

## Curriculum

1. **[01_Spatial_Loading_And_Player_Presence.md](01_Spatial_Loading_And_Player_Presence.md)** — How streaming should preserve continuity, anticipation, and world-state clarity.
2. **[02_UE5_World_Partition_And_Data_Layer_Systems.md](02_UE5_World_Partition_And_Data_Layer_Systems.md)** — World Partition, data layers, streaming sources, city-state layers, and the current supported RiftbornAI streaming surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common world-streaming and data-layer mistakes that create pop-in or logic confusion.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for streaming-state design, layer creation, actor assignment, and residency checks.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for data layers, streaming sources, World Partition inspection, and diagnostics.

## Core Rules

- Decide whether separation is spatial, state-based, or both before creating layers.
- Use data layers to express meaningful world-state partitions.
- Inspect the live partition state before reassigning actors blind.
- Treat streaming diagnostics as design feedback, not just optimization trivia.
- Do not rely on stubbed or non-default residency helpers as the main workflow.
