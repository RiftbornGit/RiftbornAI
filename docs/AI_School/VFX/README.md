# RiftbornAI VFX School

**Read this track before creating or editing Niagara systems, gameplay VFX, ambient particle zones, or other effect-driven feedback.**

VFX is not decoration first. In a game, VFX is communication:

- It tells the player what is dangerous.
- It confirms that an action connected.
- It indicates state changes, buffs, debuffs, and interaction windows.
- It supports mood only after readability is solved.

If you build effects by stacking emitters until something looks "cool" in the asset editor, you will produce noise, not feedback.

## Curriculum

1. **[01_Gameplay_Feedback_And_Readability.md](01_Gameplay_Feedback_And_Readability.md)** — How players perceive effects, what visual hierarchy matters, and why camera distance changes everything.
2. **[02_UE5_Niagara_Systems.md](02_UE5_Niagara_Systems.md)** — Niagara concepts mapped to effect layering, user parameters, editor context tools, and compile checks.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common AI-authored VFX failure modes that look flashy in isolation but fail in game.
4. **[04_Workflow.md](04_Workflow.md)** — The correct build order: define gameplay job, build a readable core, layer support emitters, expose tuning, preview, verify, and budget.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact RiftbornAI tools for authoring, inspecting, spawning, previewing, and verifying Niagara work.

## Core Rules

- Solve gameplay readability before mood, polish, or complexity.
- Preview effects at gameplay camera distance, not only close-up in the Niagara editor.
- One emitter should do one clear job whenever possible.
- Expose tunable `User.*` parameters before duplicating systems for tiny variants.
- Use editor-native Niagara context tools before broad fallback routes.
- Compile, inspect, spawn, and verify in-world before declaring an effect done.
- Budget particles, lights, overdraw, and motion clutter before adding more layers.
