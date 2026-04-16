# 02 - UE5 World Partition And Data Layer Systems

RiftbornAI's current streaming surface is centered on World Partition inspection, data-layer authoring, actor-layer assignment, streaming-source configuration, and texture-streaming diagnostics.

## World Partition State Is Observable

Use:

- `get_world_partition_info`

to inspect the current world's partition setup.

This is the right first step for understanding:

- whether the world uses World Partition
- how many cells and layers exist
- what the current streaming state looks like

## Data Layers Are The Main State-Partition Tool

Use:

- `list_data_layers`
- `create_data_layer`
- `set_actor_data_layer`
- `create_city_state_data_layers`

for the main data-layer authoring lane.

Important distinction:

- `create_data_layer` creates and saves the asset, but does not add an instance to the current world
- `create_city_state_data_layers` creates coordinated runtime assets and matching world instances for multi-state orchestration

## Streaming Sources Control What Loads Near Important Presence

Use:

- `configure_streaming_source`

to define a streaming source on an actor.

This is the correct lane when content should stream in around:

- a player-related actor
- a traversal anchor
- a planned approach path

## Texture Streaming Has Diagnostics

Use:

- `get_texture_streaming_stats`

to inspect texture residency pressure and over-budget conditions.

This is part of streaming health, not a separate unrelated optimization concern.

## Be Careful With Adjacent Residency Helpers

The repo contains adjacent streaming-residency ideas, but not every one is a safe default lane.

Specifically:

- `optimize_streaming_residency` is marked `STUB` in the current readiness surface

That means it should not be documented as a default production workflow.

If you need an adjacent helper beyond the documented lane, verify it against the live registry first.

## Current Strength Of The Surface

The streaming tool surface is strongest for:

- partition inspection
- data-layer creation and listing
- actor-layer assignment
- coordinated city-state layer creation
- streaming-source configuration
- texture-streaming diagnostics

## Key Takeaway

Treat the current streaming lane as:

inspect partition -> define layers -> assign actors -> configure sources -> check residency

That order keeps the world-state logic understandable.
