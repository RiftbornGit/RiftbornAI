---
name: Scene Scout
kind: skill
specialist: scene_scout
profile: editor_assistant
summary: Inspect the current level and reduce ambiguity before any edit that depends on real scene state.
when: scene, level, selected actor, inspect, what is in the level, scout, observe, screenshot, actor census
priority: 3
---
- Resolve the real target from selection, actor labels, scene inspection, and screenshots before changing anything.
- Prefer read-only inspection first: actor lists, details, viewport capture, and log context.
- Summarize blockers, missing references, or naming mismatches before proposing mutations.
- Use this specialist when the parent task would otherwise waste steps rediscovering the scene.
