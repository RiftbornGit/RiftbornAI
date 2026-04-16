# 04 — Workflow

Blueprint work should follow a strict order so graph changes stay understandable and recoverable.

## 1. Define The Class Job First

Before using any Blueprint tool, answer:

- what is this class for?
- what state belongs here?
- what events should enter it?
- what should remain outside this class?

If those answers are fuzzy, the Blueprint will become fuzzy too.

## 2. Choose The Right Editing Lane

Use the open-Blueprint authoring lane when you are:

- creating a new Blueprint
- adding components, variables, or functions
- doing bounded graph scaffolding

Use the editor-native lane when you are:

- inspecting an existing Blueprint
- locating exact nodes or graphs
- repairing compile problems
- refactoring or replacing existing graph structure

Do not mix lanes casually without understanding why.

## 3. Ground On Editor Context Before Existing-Graph Mutation

For an existing Blueprint, start here:

1. `get_editor_focus_state`
2. `focus_asset_editor`
3. `get_blueprint_editor_context`
4. `list_blueprint_graphs`
5. `list_blueprint_nodes` or `find_blueprint_nodes`

Only after that should you remove, replace, or retarget exact nodes.

## 4. Scaffold Structure Before Wiring Logic

When creating or expanding a Blueprint:

1. create the class with `create_blueprint`
2. open it with `open_blueprint`
3. add required components with `add_blueprint_component`
4. add explicit variables with `add_blueprint_variable`
5. add named functions with `add_blueprint_function`

This keeps architecture ahead of graph spaghetti.

## 5. Keep Event Flow Thin

After structure exists:

1. add the event entry point with `add_blueprint_event` when the restricted event lane fits
2. add exact nodes with `add_blueprint_node` only for bounded logic
3. connect them with `connect_blueprint_nodes`
4. push repeated or meaningful behavior into functions instead of extending the Event Graph indefinitely

If the requested behavior exceeds the safe event lane, switch to the editor-native inspection/mutation path instead of forcing it.

## 6. Set Defaults Deliberately

Use explicit defaults instead of hoping the graph implies them.

Relevant tools include:

- `set_blueprint_pin_value`
- `set_blueprint_pin_default`
- `rename_blueprint_variable`

Defaults are part of behavior. Treat them like code, not incidental editor state.

## 7. Compile Early And Often

After each meaningful structural or graph edit:

- use `compile_blueprint` on the open authoring lane
- use `get_blueprint_compile_diagnostics` or `assert_blueprint_compiles` on the editor-native lane

Do not stack many unrelated edits before the first compile.

## 8. Repair Deterministically

If compile fails:

1. inspect diagnostics with `get_blueprint_compile_diagnostics`
2. repair targeted issues with `repair_blueprint_compile_errors`
3. if the asset is in a broader broken loop, use `validate_and_repair_blueprint_loop`
4. re-run compile assertions

Do not keep editing through a broken Blueprint hoping the next change fixes it.

## 9. Use Snapshots Before Risky Surgery

Before broader graph refactors, create a recovery point:

- `create_blueprint_snapshot`
- `export_blueprint_text`

Then compare later if needed:

- `compare_blueprint_snapshots`
- `list_blueprint_snapshots`

This is especially useful before node replacement, reparenting, or larger cleanup work.

## 10. Reparent Only With Intent

Use `reparent_blueprint` only when the class boundary is actually wrong.

After reparenting:

1. inspect inherited assumptions
2. compile immediately
3. run repair tools if needed
4. verify behavior in runtime context

Reparenting is an architectural change, not a convenience tweak.

## 11. Verify In Runtime Context

Compile success is not enough.

After Blueprint edits:

1. place or use the asset in context
2. exercise the expected interaction
3. run `run_quick_playtest`
4. if relevant, inspect surrounding world/UI behavior with the normal observation tools

The Blueprint is only healthy when its behavior still makes sense in the level.

## Recommended Sequences

### New Simple Gameplay Blueprint

`create_blueprint` -> `open_blueprint` -> `add_blueprint_component` -> `add_blueprint_variable` -> `add_blueprint_function` -> `add_blueprint_event` -> `add_blueprint_node` -> `connect_blueprint_nodes` -> `compile_blueprint` -> `validate_blueprint_health` -> `run_quick_playtest`

### Existing Blueprint Refactor

`get_editor_focus_state` -> `focus_asset_editor` -> `get_blueprint_editor_context` -> `list_blueprint_graphs` -> `find_blueprint_nodes` -> `create_blueprint_snapshot` -> `replace_blueprint_node` or `remove_blueprint_node` -> `get_blueprint_compile_diagnostics` -> `repair_blueprint_compile_errors` if needed -> `assert_blueprint_compiles` -> `run_quick_playtest`

### Structural Repair Pass

`get_blueprint_compile_diagnostics` -> `validate_blueprint_health` -> `repair_blueprint_compile_errors` or `validate_and_repair_blueprint_loop` -> `assert_blueprint_compiles` -> `export_blueprint_text` if review is needed -> `run_quick_playtest`

## Key Takeaway

The safe Blueprint order is:

architecture first, context second, exact edits third, compile fourth, runtime proof last.

If you invert that order, graph debt and repair cost rise immediately.
