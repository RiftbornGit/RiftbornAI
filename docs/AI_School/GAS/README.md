# RiftbornAI GAS School

**Read this track before creating gameplay abilities, gameplay effects, attribute sets, gameplay tags, or assigning abilities to actors.**

GAS is not just an asset factory. It is a game-state architecture:

- abilities decide who can do what, when, and under which conditions
- effects change state over time or instantly
- attributes store the numbers that matter
- gameplay tags express state, gating, and interaction rules
- the AbilitySystemComponent is the runtime authority holding it together

If you only create a `GA_*` asset and stop there, you have not built an ability system. You have created a file.

## Curriculum

1. **[01_Ability_Design_And_State_Model.md](01_Ability_Design_And_State_Model.md)** — Ability design as state transition and player contract.
2. **[02_UE5_GAS_Systems.md](02_UE5_GAS_Systems.md)** — ASC, abilities, effects, attributes, tags, and current RiftbornAI support.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common GAS mistakes that create brittle combat systems.
4. **[04_Workflow.md](04_Workflow.md)** — The correct build order from concept to actor-level verification.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact RiftbornAI tools for GAS creation, configuration, assignment, and runtime inspection.

## Core Rules

- Design interactions before creating assets.
- Define gameplay tags before relying on them for gating or state.
- Model cost, cooldown, targeting, and failure behavior explicitly.
- Do not assume asset creation equals finished gameplay logic.
- Verify abilities on a real actor with an AbilitySystemComponent.
- Treat replication and runtime playtest as first-class validation, not cleanup at the end.
