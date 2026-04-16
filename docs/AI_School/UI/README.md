# RiftbornAI UI School

**Read this track before creating or editing Widget Blueprints, HUD shells, menus, prompts, overlays, or controller-navigation flows.**

UI is not decoration. It is a decision interface:

- it tells the player what matters now
- it controls attention, urgency, and mental load
- it translates game state into clear available actions
- it must survive mouse, keyboard, and gamepad use

UMG gives layout freedom, but freedom without hierarchy and focus discipline produces broken UX quickly.

## Curriculum

1. **[01_Interface_Hierarchy_And_Player_Focus.md](01_Interface_Hierarchy_And_Player_Focus.md)** — Attention, hierarchy, state, and why UI should reduce decision cost.
2. **[02_UE5_Widget_And_UI_Verification_Systems.md](02_UE5_Widget_And_UI_Verification_Systems.md)** — Widget Blueprint authoring, widget-tree inspection, layout audit, navigation, and the current supported RiftbornAI tool surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common HUD, menu, and flow mistakes that make UI noisy or unreliable.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for UI intent, structure, compile, navigation, and live verification.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for widget creation, hierarchy inspection, compile, navigation, and proof in PIE.

## Core Rules

- Start from the player task and urgency, not from a blank canvas.
- Give the most important state the strongest visual weight and the shortest path to comprehension.
- Define gamepad and keyboard focus intentionally; never leave navigation to chance.
- Compile and audit layout after meaningful hierarchy changes.
- Verify widgets in PIE, not only in the designer.
