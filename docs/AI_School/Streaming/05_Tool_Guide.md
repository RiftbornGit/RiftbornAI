# 05 - Tool Guide

Use exact registered streaming tool names. If a name is uncertain, verify with `describe_tool`.

## Streaming Tool Map

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Inspect the current World Partition state | `get_world_partition_info` | Best first step before making streaming changes |
| List existing data layers | `list_data_layers` | Shows current layer names, types, states, and actor counts |
| Create a focused data-layer asset | `create_data_layer` | Creates and saves the asset, but does not add a world instance by itself |
| Create coordinated runtime state layers | `create_city_state_data_layers` | Strong lane for multi-state city or world-state orchestration |
| Assign an actor to a layer | `set_actor_data_layer` | Use deliberately, based on actual state logic |
| Configure a World Partition streaming source | `configure_streaming_source` | Shapes what loads first around an actor or helper |
| Check texture-streaming pressure | `get_texture_streaming_stats` | Confirms residency health and over-budget conditions |
| Verify exact helper availability | `list_all_tools`, `describe_tool` | Use before depending on adjacent non-default residency helpers |

## Default Lanes

### Partition And Layer Inspection Lane

Use for:

- understanding the current world
- reviewing existing layer structure

Main tools:

- `get_world_partition_info`
- `list_data_layers`

### Layer Authoring Lane

Use for:

- creating new layers
- building coordinated runtime state sets
- assigning actors into those sets

Main tools:

- `create_data_layer`
- `create_city_state_data_layers`
- `set_actor_data_layer`

### Streaming Presence Lane

Use for:

- shaping what loads around important actors or paths

Main tools:

- `configure_streaming_source`

### Residency Diagnostics Lane

Use for:

- checking texture-streaming health

Main tools:

- `get_texture_streaming_stats`

## Proven Sequences

### New State Layer

`get_world_partition_info` -> `list_data_layers` -> `create_data_layer` -> `set_actor_data_layer`

### City-State Orchestration

`get_world_partition_info` -> `create_city_state_data_layers` -> actor assignment pass

### Streaming Source Setup

inspect world state -> `configure_streaming_source` -> `get_texture_streaming_stats`

## Rules

- Do not create layers before you know what they mean.
- Do not assign actors to layers blind.
- Do not depend on stubbed residency helpers as the primary workflow.
- Do not ignore texture-streaming diagnostics once the spatial logic is in place.
- Do not call a streaming task done until the partition and residency state make sense together.

## Key Takeaway

The streaming surface is strongest when you separate:

- partition inspection
- layer design
- actor assignment
- source configuration
- residency diagnostics

If those are mixed together casually, the world usually becomes hard to stream and hard to reason about.
