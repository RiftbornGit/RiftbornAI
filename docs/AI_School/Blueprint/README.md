# RiftbornAI Blueprint School

**Read this track before creating or editing Blueprint classes, variables, components, functions, event graphs, or Blueprint gameplay wiring.**

Blueprints are not a dumping ground for “whatever was fastest.” They are a class architecture and graph language.

Good Blueprint work:

- keeps class responsibilities narrow
- uses components and functions to break logic apart
- uses events for causality instead of polling everything every frame
- stays readable enough that another agent or developer can safely modify it later

Bad Blueprint work compiles, but still creates a system nobody can reason about.

## Curriculum

1. **[01_Architecture_And_Communication.md](01_Architecture_And_Communication.md)** — Blueprint responsibility, composition, and communication patterns.
2. **[02_UE5_Blueprint_Systems.md](02_UE5_Blueprint_Systems.md)** — Graph types, editor-native context tools, mutation lanes, and current supported RiftbornAI tooling.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common graph-debt patterns and what to do instead.
4. **[04_Workflow.md](04_Workflow.md)** — The correct build order for class setup, graph edits, compile, and verification.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for Blueprint authoring, inspection, repair, and playtest verification.

## Core Rules

- One Blueprint class should have one clear job.
- Prefer component composition over giant inherited god-classes.
- Prefer functions and events over repeated node spaghetti.
- Use event-driven communication before reaching for Tick.
- Inspect the live Blueprint editor context before graph mutation.
- Compile and verify after meaningful changes, not just at the end.
