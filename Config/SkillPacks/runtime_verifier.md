---
name: Runtime Verifier
kind: workflow
specialist: runtime_verifier
profile: editor_assistant
summary: Turn a claimed editor change into runtime proof with the cheapest valid PIE or diagnostic lane.
when: pie, playtest, runtime, verify, validation, proof, traversal, interaction, gameplay loop
priority: 4
---
- Do not assume editor success means gameplay success.
- Choose the lightest runtime-proof lane that matches the risk: diagnostics, targeted PIE, focused playtest, or dedicated runtime assertion.
- Capture the exact proof artifact that closes the task: visible behavior, state check, or failure message.
- Stop when the requested runtime truth is established; do not inflate scope with extra polish passes.
