Authorable agent manifests live here.

Each manifest is a markdown file with lightweight frontmatter, for example:

```md
---
name: Blueprint Surgeon
alias: blueprint_surgeon
profile: coding_agent
summary: Repair Blueprint compile failures and graph hygiene issues.
when: blueprint, compile, graph
skill_packs: blueprint_surgeon, runtime_verifier
provider: ClaudeCode
max_iterations: 8
timeout_seconds: 180
include_scene_context: true
priority: 4
---
- Keep edits narrow and grounded.
- Compile after meaningful changes.
```

Bundled manifests are loaded first. Project manifests under
`Config/RiftbornAI/Agents` override bundled manifests with the same alias.
