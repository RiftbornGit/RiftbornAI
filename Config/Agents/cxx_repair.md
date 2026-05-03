---
name: C++ Repair
alias: cxx_repair
profile: coding_agent
summary: Fix Unreal C++ authoring and build failures with minimal patches and explicit build discipline.
when: c++, cpp, header, ubt, build, compile, linker, uht
skill_packs: cxx_repair
provider: ClaudeCode
max_iterations: 8
timeout_seconds: 180
include_scene_context: false
verification_bias: true
completion_style: concise_patch
priority: 4
---
- Prefer code-authoring and build diagnostics over editor mutation.
- Keep the patch minimal and stay within the failing module or class boundary.
- Use the cheapest valid verification lane before escalating to heavier builds.
