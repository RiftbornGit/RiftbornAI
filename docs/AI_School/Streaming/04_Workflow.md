# 04 - Workflow

Streaming work should move from world-state design to layer authoring to source configuration to residency checks.

## Phase 1: Define The Streaming Logic

Before touching a streaming tool, answer:

- what is spatially loaded because of proximity
- what changes because of world state
- which content must be present early for player orientation

## Phase 2: Inspect The Current World

Use:

- `get_world_partition_info`
- `list_data_layers`

to understand the current baseline before making changes.

## Phase 3: Create The Right Layers

Use:

- `create_data_layer`

for focused layer assets.

Use:

- `create_city_state_data_layers`

when the task is a coordinated set of named runtime state layers.

## Phase 4: Assign Actors Deliberately

Use:

- `set_actor_data_layer`

to put actors into meaningful layers based on the design logic from Phase 1.

## Phase 5: Configure Streaming Presence

Use:

- `configure_streaming_source`

when a player-relevant actor or helper should drive what loads first.

## Phase 6: Check Residency Health

Use:

- `get_texture_streaming_stats`

to confirm that the streaming setup is not producing obvious residency pressure.

## Recommended Sequences

### New Runtime Layer

`get_world_partition_info` -> `list_data_layers` -> `create_data_layer` -> `set_actor_data_layer`

### Coordinated City Or World-State Set

`get_world_partition_info` -> `create_city_state_data_layers` -> assign the relevant actors -> inspect again

### Streaming-Source Pass

inspect partition state -> `configure_streaming_source` -> `get_texture_streaming_stats`

## Key Takeaway

The safe order is:

understand the world first, encode the layer logic second, tune the loading behavior third.

If you start by randomly assigning actors, the world-state model becomes hard to recover.
