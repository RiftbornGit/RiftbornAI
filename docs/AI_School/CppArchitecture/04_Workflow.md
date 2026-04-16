# 04 — Workflow

C++ work should move from class-boundary choice to generation/editing to the correct build lane to runtime proof.

## Phase 1: Decide The Class Boundary

Before touching a code tool, answer:

- is this a world object, locomotion pawn, reusable behavior, match rule class, or scoped service?
- what lifetime should it have?
- does it need transform ownership?

Then choose:

- Actor
- Character
- Component
- GameMode
- Subsystem

## Phase 2: Choose The Authoring Lane

### New Class Generation

Use the matching generator:

- `generate_actor_class_from_description`
- `generate_character_class_from_description`
- `generate_component_class_from_description`
- `generate_gamemode_class_from_description`
- `generate_subsystem_class_from_description`

### Direct Source Edit

Use:

- `create_source_file`
- `edit_source_file`

for bounded existing-source changes or when generation is not the right abstraction.

### Blueprint Migration

Use:

1. `check_blueprint_convertibility`
2. `convert_blueprint_to_cpp` only if the coverage is acceptable

## Phase 3: Check Build State Before Reloading Blindly

Use:

- `get_build_status`

This gives current project/build control-plane context before you choose compile or reload.

## Phase 4: Use The Right Build Lane

### Use `run_ubt` For

- new classes
- header changes
- reflection/UHT-sensitive changes
- compile situations where reliability matters more than iteration speed

### Use `trigger_live_coding` Or `hot_reload_cpp` For

- supported implementation-only changes while the editor is running

Do not use these as a substitute for full builds when the change type is broader.

## Phase 5: Read Errors Structurally

If compile or reload fails:

1. `get_build_errors`
2. patch the actual source issue
3. rerun the appropriate build lane

If the editor or reload crashes, use:

- `diagnose_crash`

before retrying.

## Phase 6: Validate Runtime Behavior

For gameplay-facing code:

- run `run_quick_playtest`

Compile success is necessary, not sufficient.

## Phase 7: Escalate To Packaging Validation When Needed

For broader shipping-readiness checks, use:

- `validate_for_packaging`
- `cook_project`
- `package_project`

These are later-stage proof tools, not the default first compile step.

## Recommended Sequences

### New Reusable Gameplay Behavior

`choose component boundary` -> `generate_component_class_from_description` -> `get_build_status` -> `run_ubt` -> `get_build_errors` if needed -> integrate/use the component -> `run_quick_playtest`

### New Match Rule Class

`choose GameMode boundary` -> `generate_gamemode_class_from_description` -> `run_ubt` -> `get_build_errors` if needed -> world integration -> `run_quick_playtest`

### Scoped Service

`choose subsystem type` -> `generate_subsystem_class_from_description` -> `run_ubt` -> `get_build_errors` if needed -> runtime verification

### Existing Source Patch

`get_build_status` -> `edit_source_file` -> `trigger_live_coding` or `hot_reload_cpp` when supported, otherwise `run_ubt` -> `get_build_errors` -> `run_quick_playtest`

### Blueprint Migration

`check_blueprint_convertibility` -> `convert_blueprint_to_cpp` -> inspect unsupported gaps -> `run_ubt` -> `get_build_errors` -> `run_quick_playtest`

## Key Takeaway

The safe order is:

choose boundary first, author second, build with the right lane third, verify in runtime fourth.

If you skip the first step, you can generate a lot of correct C++ in the wrong shape.
