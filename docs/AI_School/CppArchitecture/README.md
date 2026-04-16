# RiftbornAI C++ Architecture School

**Read this track before creating or editing Unreal C++ gameplay classes, source files, module-facing class boundaries, or Blueprint-to-C++ conversions.**

Unreal C++ quality is mostly about putting behavior in the right kind of class.

Good C++ work:

- chooses the smallest ownership boundary that matches the behavior
- keeps reusable logic in components or services instead of giant actor classes
- respects build and reload limits instead of treating every change as hot-reloadable
- proves behavior with compile diagnostics and runtime verification

Bad C++ work often compiles while still creating the wrong architecture.

## Curriculum

1. **[01_Class_Boundaries_And_Responsibilities.md](01_Class_Boundaries_And_Responsibilities.md)** — Actor, Character, Component, GameMode, and Subsystem responsibility rules.
2. **[02_UE5_CPP_Authoring_And_Build_Systems.md](02_UE5_CPP_Authoring_And_Build_Systems.md)** — The current supported generation, source-edit, conversion, build, and reload lanes.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common Unreal C++ architecture and workflow mistakes.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for class choice, generation/editing, compile, reload, and runtime verification.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for C++ generation, editing, conversion, build, and validation.

## Core Rules

- Pick the class type that matches lifetime and responsibility, not whichever generator is most convenient.
- Prefer reusable components over copying behavior across actors.
- Use subsystems for scoped services, not for everything global by habit.
- Treat GameMode as match-rule authority, not as a dumping ground for unrelated systems.
- Use the right build lane for the kind of code change you made.
- Do not treat Blueprint-to-C++ conversion as automatic feature parity.
