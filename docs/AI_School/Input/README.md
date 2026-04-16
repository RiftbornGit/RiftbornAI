# RiftbornAI Input School

**Read this track before creating or editing Enhanced Input actions, mapping contexts, key bindings, or control-scheme layouts.**

Input is not a button list. It is a control language:

- it turns player intent into reliable game verbs
- it teaches the player what the game expects
- it must stay consistent across modes and devices
- it only succeeds if the controls feel correct in play

Enhanced Input gives strong asset structure, but asset structure alone does not create a good control scheme.

## Curriculum

1. **[01_Player_Intent_And_Control_Grammar.md](01_Player_Intent_And_Control_Grammar.md)** — Player verbs, consistency, context, and why input should be modeled around intent.
2. **[02_UE5_Enhanced_Input_Systems.md](02_UE5_Enhanced_Input_Systems.md)** — Enhanced Input actions, contexts, mappings, beta-gated modifier depth, and the current supported RiftbornAI tool surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common control-scheme and mapping-context mistakes that make input confusing or brittle.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for action design, context setup, mapping, and runtime verification.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for creating input assets, adding mappings, and proving the controls in runtime.

## Core Rules

- Start from the player verb, not the key.
- Separate contexts by mode, state, or major role.
- Use clear action names and the correct value type from the start.
- Keep the control grammar learnable across keyboard, mouse, and controller expectations.
- Verify the controls in play, not only in the Content Browser.
