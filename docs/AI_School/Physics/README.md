# RiftbornAI Physics School

**Read this track before creating or editing collision setups, physics constraints, traces, Chaos destruction assets, or destructible gameplay props.**

Physics is not just simulation. It is gameplay truth:

- it tells the player what is solid, dangerous, movable, or breakable
- it supports believable resistance and consequence
- it turns contact into readable outcomes
- it needs rollback-safe iteration when destruction is involved

The tools can create collision and destruction assets quickly, but fast setup without physical intent produces noisy or misleading behavior.

## Curriculum

1. **[01_Force_Collision_And_Physical_Readability.md](01_Force_Collision_And_Physical_Readability.md)** — Affordance, solidity, breakability, and why physical behavior must read clearly.
2. **[02_UE5_Collision_Constraint_And_Destruction_Systems.md](02_UE5_Collision_Constraint_And_Destruction_Systems.md)** — Collision presets, traces, constraints, Geometry Collections, destruction snapshots, and the current supported RiftbornAI physics surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common collision, constraint, and destruction mistakes that create unreliable gameplay.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for collision setup, physical behavior, fracture authoring, and rollback-safe proof.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for collision authoring, traces, constraints, destruction, and verification.

## Core Rules

- Start from the intended affordance, not from random physics toggles.
- Use collision presets and explicit responses deliberately.
- Prove interaction assumptions with traces instead of guessing.
- Treat destruction as a staged authored behavior, not as an uncontrolled explosion.
- Snapshot destructive states before risky iteration.
