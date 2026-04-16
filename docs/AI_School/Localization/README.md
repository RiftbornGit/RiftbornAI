# RiftbornAI Localization School

**Read this track before creating or editing String Table assets, localization entry keys, or project text-localization structure.**

Localization is not just translation. It is meaning preservation:

- it keeps the player's understanding intact across cultures
- it depends on stable keys and clear context
- it must distinguish tone, UI fit, and gameplay clarity
- it breaks quickly when one string is forced to mean multiple things

The current tool surface is thin but real: it can create String Tables and author entries. That is enough to require discipline.

## Curriculum

1. **[01_Text_Intent_Keys_And_Cultural_Clarity.md](01_Text_Intent_Keys_And_Cultural_Clarity.md)** — Message intent, key stability, UI fit, and translation context.
2. **[02_UE5_String_Table_And_Culture_Systems.md](02_UE5_String_Table_And_Culture_Systems.md)** — String Tables, entry authoring, optional culture/table inspection helpers, and the current supported RiftbornAI localization surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common localization mistakes that create ambiguous keys or poor translated output.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for key design, String Table authoring, and verification.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for String Table creation, entry updates, and localization-safe verification.

## Core Rules

- Start from meaning and usage context, not from string duplication.
- Treat localization keys as stable contracts.
- Do not reuse one key for multiple meanings.
- Save and verify String Table changes deliberately.
- Verify any deeper culture/table helper against the live registry before relying on it.
