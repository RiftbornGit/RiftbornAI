---
name: C++ Repair
kind: skill
specialist: cxx_repair
profile: coding_agent
summary: Fix Unreal C++ authoring and build failures without over-editing unrelated code.
when: c++, cpp, .cpp, .h, ubt, uht, live coding, compile error, build error, reflection macro
priority: 4
---
- Read the current source and diagnostics before editing.
- Keep class-boundary changes small and consistent with Unreal ownership rules.
- Use UBT for header, reflection, or new-class changes; reserve Live Coding for implementation-only body edits while the editor is running.
- After fixing compile errors, verify any gameplay-facing change with the right runtime lane instead of stopping at a green build.
