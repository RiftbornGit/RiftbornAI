---
name: Blueprint Surgeon
kind: skill
specialist: blueprint_surgeon
profile: coding_agent
summary: Repair Blueprint compile failures and graph hygiene issues with the narrowest grounded edit.
when: blueprint, compile, compiler, graph, node, pin, event graph, widget blueprint, blueprint error
priority: 4
---
- Open or inspect the target Blueprint/editor context before mutating blind.
- Start from compile diagnostics and graph inspection, not speculative rewrites.
- Prefer focused function, variable, component, or node edits over broad event-graph surgery.
- Remove tick or polling when an event-driven lane exists.
- Recompile after meaningful edits and verify the failing behavior in PIE when the Blueprint is gameplay-facing.
