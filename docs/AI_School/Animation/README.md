# RiftbornAI Animation School

**Read this track before creating or editing Animation Blueprints, blend spaces, montages, retargeting assets, or pose-search databases.**

Animation is not just movement. It is a gameplay communication system:

- it shows intent before impact
- it sells weight, recovery, and timing
- it keeps locomotion readable at gameplay camera distance
- it helps players predict what happens next

UE gives many animation asset types, but choosing the wrong one creates brittle graphs and bad feel quickly.

## Curriculum

1. **[01_Motion_Intent_And_Readability.md](01_Motion_Intent_And_Readability.md)** — Motion roles, anticipation, recovery, readability, and state communication.
2. **[02_UE5_Animation_And_Retargeting_Systems.md](02_UE5_Animation_And_Retargeting_Systems.md)** — Animation Blueprints, blend spaces, montages, retargeting, pose search, and the current supported RiftbornAI tool surface.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — Common animation-authoring and retargeting mistakes that create poor runtime behavior.
4. **[04_Workflow.md](04_Workflow.md)** — The correct order for motion-role choice, asset authoring, retargeting, and runtime proof.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Exact tools for AnimBP authoring, locomotion setup, retargeting, motion-matching database setup, and verification.

## Core Rules

- Decide the gameplay role of the motion before choosing the asset type.
- Use blend spaces for parameter-driven continuous motion and montages for discrete actions.
- Retarget only when skeleton and rig assumptions are explicit.
- Build state machines around readable runtime states, not around a pile of clips.
- Verify motion on a live actor or in gameplay, not only in preview.
