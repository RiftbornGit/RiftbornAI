# 05 — Tool Guide

Use exact registered Blueprint tools. Do not invent aliases.

## Core Rule

Start with context for existing Blueprint work. Start with scaffolding for new Blueprint work.

## Blueprint Tool Map

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create a new Blueprint class | `create_blueprint` | Use when the asset does not exist yet |
| Open Blueprint on the legacy authoring lane | `open_blueprint` | Required before several older component/variable/function edits |
| Add class components | `add_blueprint_component` | Prefer this before graph-level work |
| Add explicit state | `add_blueprint_variable` | Create variables before wiring behavior around them |
| Add reusable logic surface | `add_blueprint_function` | Prefer named functions over repeated node islands |
| Add bounded event entry logic | `add_blueprint_event` | Restricted safe event lane; not universal graph authoring |
| Remove event wiring created on that lane | `remove_blueprint_event` | Use when undoing or cleaning restricted event hookups |
| Add individual graph nodes | `add_blueprint_node` | Use for controlled graph construction |
| Connect graph nodes | `connect_blueprint_nodes` | Pair with `add_blueprint_node` on the authoring lane |
| Inspect current graph content on legacy lane | `get_blueprint_graph` | Helpful for a quick graph readout on the older flow |
| Set pin values on legacy lane | `set_blueprint_pin_value` | Legacy authoring-lane default/value edits |
| Set up bounded event patterns | `setup_blueprint_events` | Use where the older event setup helper matches the need |
| Compile a Blueprint directly | `compile_blueprint` | Use after meaningful authoring-lane changes |
| Check if Blueprint should move to C++ | `check_blueprint_convertibility` | Planning/assessment tool before conversion |
| Convert Blueprint to C++ | `convert_blueprint_to_cpp` | Structural step, not a casual cleanup tool |
| Read focused Blueprint editor state | `get_editor_focus_state`, `focus_asset_editor`, `get_blueprint_editor_context` | First step for existing-graph mutation |
| Enumerate graphs | `list_blueprint_graphs` | Grounds exact graph names before edits |
| Enumerate nodes | `list_blueprint_nodes` | Grounds exact node ids before edits |
| Search nodes by pattern | `find_blueprint_nodes` | Faster than scanning all node output manually |
| Focus an exact node in the editor | `focus_blueprint_node` | Useful before targeted replacements or visual confirmation |
| Read compile failures precisely | `get_blueprint_compile_diagnostics` | Use before repair when the problem is unclear |
| Prove compile health | `assert_blueprint_compiles` | Fast safety check after refactors or repairs |
| Remove a node exactly | `remove_blueprint_node` | Destructive; use only with grounded node ids |
| Replace a node exactly | `replace_blueprint_node` | Preferred over loose manual surgery when the swap is known |
| Change exact pin defaults | `set_blueprint_pin_default` | Editor-native value/default mutation |
| Rename a Blueprint variable | `rename_blueprint_variable` | Compile immediately after |
| Change class parent | `reparent_blueprint` | Architectural move; verify downstream assumptions |
| Compile multiple Blueprints | `batch_compile_blueprints` | Good for broader repair passes |
| Repair compile errors | `repair_blueprint_compile_errors` | Deterministic recovery step for broken assets |
| Run repair loop | `validate_and_repair_blueprint_loop` | Use when compile issues span more than a single obvious fix |
| Validate overall Blueprint health | `validate_blueprint_health` | Broader health gate, not just raw compile output |
| Save recovery point | `create_blueprint_snapshot` | Use before larger graph surgery |
| Compare before/after | `compare_blueprint_snapshots` | Review structural changes safely |
| Export Blueprint as text | `export_blueprint_text` | Useful for review, diffing, and recovery support |
| List saved snapshots | `list_blueprint_snapshots` | Helps manage recovery points |
| Verify runtime behavior | `run_quick_playtest` | Required after meaningful gameplay Blueprint edits |

## When To Prefer Which Lane

### Use The Open-Blueprint Authoring Lane For

- new Blueprint creation
- adding components, variables, and functions
- simple event-response gameplay scaffolding
- bounded node wiring you can describe clearly ahead of time

Main tools:

- `create_blueprint`
- `open_blueprint`
- `add_blueprint_component`
- `add_blueprint_variable`
- `add_blueprint_function`
- `add_blueprint_event`
- `add_blueprint_node`
- `connect_blueprint_nodes`
- `compile_blueprint`

### Use The Editor-Native Lane For

- existing Blueprint inspection
- exact node mutation
- compile diagnosis
- refactors, repairs, and reparenting

Main tools:

- `get_editor_focus_state`
- `focus_asset_editor`
- `get_blueprint_editor_context`
- `list_blueprint_graphs`
- `list_blueprint_nodes`
- `find_blueprint_nodes`
- `get_blueprint_compile_diagnostics`
- `assert_blueprint_compiles`
- `remove_blueprint_node`
- `replace_blueprint_node`
- `set_blueprint_pin_default`
- `rename_blueprint_variable`
- `reparent_blueprint`
- `repair_blueprint_compile_errors`

## Proven Sequences

### New Interaction Blueprint

`create_blueprint` -> `open_blueprint` -> `add_blueprint_component` -> `add_blueprint_variable` -> `add_blueprint_function` -> `add_blueprint_event` -> `add_blueprint_node` -> `connect_blueprint_nodes` -> `compile_blueprint` -> `validate_blueprint_health` -> `run_quick_playtest`

Use for: interactive objects, doors, switches, chests, traps, pickups — any Blueprint with components, state, and event-driven behavior.

### Existing Graph Cleanup

`get_editor_focus_state` -> `focus_asset_editor` -> `get_blueprint_editor_context` -> `find_blueprint_nodes` -> `create_blueprint_snapshot` -> `replace_blueprint_node` or `remove_blueprint_node` -> `get_blueprint_compile_diagnostics` -> `repair_blueprint_compile_errors` -> `assert_blueprint_compiles` -> `run_quick_playtest`

Use for: fixing broken nodes, replacing deprecated patterns, removing dead graph branches, refactoring tangled event graphs.

### Batch Project Repair

`batch_compile_blueprints` -> `get_blueprint_compile_diagnostics` on the highest-signal failures -> `repair_blueprint_compile_errors` -> `validate_and_repair_blueprint_loop` where needed -> `assert_blueprint_compiles` -> `run_quick_playtest`

Use for: post-engine-upgrade fixes, mass repair after API changes, project-wide Blueprint health restoration.

### Blueprint Variable Or Function Refactor

`get_blueprint_editor_context` -> `list_blueprint_graphs` -> `find_blueprint_nodes` -> `create_blueprint_snapshot` -> `rename_blueprint_variable` or node surgery -> `assert_blueprint_compiles` -> `compare_blueprint_snapshots` -> `run_quick_playtest`

Use for: renaming variables, restructuring functions, consolidating duplicate logic.

## Verification Checklist

Before calling a Blueprint surface done:

- [ ] components added with correct types and hierarchy
- [ ] variables created with meaningful names and correct types
- [ ] functions encapsulate reusable logic (not duplicated in event graph)
- [ ] event graph wiring is minimal and delegates to functions
- [ ] Blueprint compiles clean (`assert_blueprint_compiles`)
- [ ] Blueprint health validated (`validate_blueprint_health`)
- [ ] snapshot saved before destructive changes (`create_blueprint_snapshot`)
- [ ] runtime behavior verified in gameplay (`run_quick_playtest`)
- [ ] no orphaned nodes or dangling connections

## Tool Selection Rules

- Do not mutate existing graphs without first grounding on editor context and exact node ids.
- Do not use `add_blueprint_event` as a substitute for full Blueprint architecture work.
- Do not treat compile success as sufficient verification for gameplay behavior.
- Do not reparent or rename structural elements without immediate compile and runtime checks.

## Key Takeaway

The Blueprint surface is strong when you respect its split:

- scaffolding tools for creation
- editor-native tools for exact inspection and mutation
- repair and verification tools for proving the asset still works
