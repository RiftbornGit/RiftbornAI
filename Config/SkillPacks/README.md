# RiftbornAI Skill Packs

Skill packs are lightweight markdown playbooks that RiftbornAI can inject into
the system prompt when they match the current task.

Format:

```md
---
name: Blueprint Surgeon
kind: skill
specialist: blueprint_surgeon
profile: coding_agent
summary: One-line description
when: blueprint, compile, graph
priority: 4
---
- Short operational bullets
- Kept concise on purpose
```

Layer order:

1. Bundled plugin packs in `Config/SkillPacks`
2. Project packs in `Config/RiftbornAI/Skills`

Project packs override bundled packs with the same `specialist` or `name`.
