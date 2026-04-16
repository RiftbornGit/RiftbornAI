# 05 — Tool Guide

Use exact registered C++ tool names. If a name is uncertain, verify with `describe_tool`.

## C++ Tool Map

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create a world-object class | `generate_actor_class_from_description` | Use when the class is fundamentally a world entity |
| Create a locomotion pawn class | `generate_character_class_from_description` | Use only when the class is really a Character |
| Create reusable behavior | `generate_component_class_from_description` | Prefer for reusable gameplay systems; use `is_scene_component` when transform is needed |
| Create match-rule authority | `generate_gamemode_class_from_description` | For game rules and match logic |
| Create scoped service | `generate_subsystem_class_from_description` | Choose `GameInstance`, `World`, or `LocalPlayer` scope intentionally |
| Create a source file directly | `create_source_file` | Use for bounded new-file work |
| Patch an existing source file | `edit_source_file` | Text-replacement edit lane for existing code |
| Check whether Blueprint migration is realistic | `check_blueprint_convertibility` | Use before promising conversion |
| Convert supported Blueprint portions to C++ | `convert_blueprint_to_cpp` | Reports unsupported nodes needing manual follow-up |
| Inspect current build/control-plane state | `get_build_status` | Use before choosing reload vs full build |
| Read structured compile failures | `get_build_errors` | Faster than guessing from generic log noise |
| Reload supported code changes quickly | `trigger_live_coding` | Use only for supported change types |
| Trigger hot reload on the module lane | `hot_reload_cpp` | Use intentionally, not as a blind retry |
| Run a reliable full build | `run_ubt` | Preferred for new classes, headers, and UHT-sensitive changes |
| Diagnose editor/build crash | `diagnose_crash` | Mandatory after relevant crash/timeout lanes |
| Validate packaging readiness | `validate_for_packaging` | Later-stage proof, not default iteration |
| Cook the project | `cook_project` | Shipping-readiness step |
| Package the project | `package_project` | Shipping-readiness step |
| Verify gameplay-facing runtime behavior | `run_quick_playtest` | Required after meaningful gameplay C++ work |

## Default Lanes

### New-Class Generation Lane

Use for:

- new gameplay classes
- new reusable components
- new scoped services

Main tools:

- `generate_actor_class_from_description`
- `generate_character_class_from_description`
- `generate_component_class_from_description`
- `generate_gamemode_class_from_description`
- `generate_subsystem_class_from_description`
- `run_ubt`
- `get_build_errors`

### Direct Source Patch Lane

Use for:

- bounded edits to existing files
- implementation refinements
- fixes after compile diagnostics

Main tools:

- `create_source_file`
- `edit_source_file`
- `get_build_status`
- `trigger_live_coding`
- `hot_reload_cpp`
- `run_ubt`
- `get_build_errors`

### Blueprint Migration Lane

Use for:

- checking whether a Blueprint is a good conversion candidate
- generating native source from supported graph portions

Main tools:

- `check_blueprint_convertibility`
- `convert_blueprint_to_cpp`
- `run_ubt`
- `get_build_errors`
- `run_quick_playtest`

## Proven Sequences

### New Component

`generate_component_class_from_description` -> `get_build_status` -> `run_ubt` -> `get_build_errors` if needed -> `run_quick_playtest`

Use for: reusable gameplay behaviors — health systems, interaction triggers, movement abilities, inventory logic.

### New Actor

`generate_actor_class_from_description` -> `run_ubt` -> `get_build_errors` -> spawn in level -> `run_quick_playtest`

Use for: world entities — pickups, hazards, interactable props, projectiles, spawners.

### Existing Source Fix

`get_build_status` -> `edit_source_file` -> `trigger_live_coding` or `hot_reload_cpp` when supported, otherwise `run_ubt` -> `get_build_errors`

Use for: bug fixes, implementation refinements, compile error resolution in existing code.

### New Gameplay System Service

`generate_subsystem_class_from_description` -> `run_ubt` -> `get_build_errors` -> integration work -> `run_quick_playtest`

Use for: scoped services — quest systems, save systems, input context managers, world state managers.

### Blueprint Conversion Pass

`check_blueprint_convertibility` -> `convert_blueprint_to_cpp` -> fill unsupported gaps -> `run_ubt` -> `get_build_errors` -> `run_quick_playtest`

Use for: migrating proven Blueprint logic to native code for performance or architectural reasons.

## Verification Checklist

Before calling a C++ authoring surface done:

- [ ] class boundary chosen intentionally (Actor vs Component vs Subsystem)
- [ ] generation completed with correct scope and dependencies
- [ ] full build passes (`run_ubt`, `get_build_errors`)
- [ ] no unresolved build errors or warnings
- [ ] crash check after build if UE was running (`diagnose_crash`)
- [ ] gameplay-facing behavior verified in runtime (`run_quick_playtest`)
- [ ] Blueprint conversion gaps filled if applicable

## Rules

- Do not choose the generator before choosing the architecture boundary.
- Do not use Character when Actor or Component would be cleaner.
- Do not treat GameMode as a generic service locator.
- Do not rely on Live Coding for new classes or header/UHT-sensitive changes.
- Do not assume Blueprint conversion means full behavioral parity.
- Do not stop at compile green for gameplay-facing code.

## Key Takeaway

The C++ surface is strongest when you separate:

- class-boundary choice
- generation or patching
- build-lane selection
- compile diagnostics
- runtime verification

That is what turns codegen into maintainable Unreal architecture.
