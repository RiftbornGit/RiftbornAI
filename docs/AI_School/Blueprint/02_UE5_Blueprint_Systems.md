# 02 — UE5 Blueprint Systems

UE5 Blueprint authoring has two different lanes in this repo:

- the open-Blueprint authoring lane
- the editor-native inspection and mutation-depth lane

You need to know which one you are in before editing.

## The Open-Blueprint Authoring Lane

This is the older direct authoring flow around an open Blueprint editor instance.

Key tools:

- `create_blueprint`
- `open_blueprint`
- `add_blueprint_component`
- `add_blueprint_variable`
- `add_blueprint_function`
- `add_blueprint_event`
- `add_blueprint_node`
- `connect_blueprint_nodes`
- `compile_blueprint`

Important rule:

- `open_blueprint` is required before component, variable, and function mutation on this lane

This lane is useful for:

- creating a new Blueprint class
- adding common class structure
- simple controlled graph construction
- restricted event wiring for gameplay causality

## The Editor-Native Inspection And Mutation-Depth Lane

This is the grounded UE 5.7 asset-editor surface documented in `docs/UE57_SUPER_SYSTEMS_TOOLS.md`.

Key tools:

- `get_editor_focus_state`
- `focus_asset_editor`
- `get_blueprint_editor_context`
- `list_blueprint_graphs`
- `list_blueprint_nodes`
- `find_blueprint_nodes`
- `get_blueprint_compile_diagnostics`
- `assert_blueprint_compiles`
- `focus_blueprint_node`
- `remove_blueprint_node`
- `replace_blueprint_node`
- `set_blueprint_pin_default`
- `rename_blueprint_variable`
- `reparent_blueprint`
- `batch_compile_blueprints`
- `repair_blueprint_compile_errors`

This lane is best when:

- you need to understand an existing Blueprint precisely
- you need exact node ids, graph names, or compile diagnostics
- you are repairing or refactoring existing graphs
- you need deterministic edits rather than blind graph-building

## Graph Types Matter

Not all Blueprint graphs have the same job.

### Event Graph

Use for external triggers and high-level causal flow.

### Function Graphs

Use for encapsulated logic with clear names and reusable behavior.

### Macro Or Specialized Graphs

Use only when their semantics are justified. Do not hide architecture problems inside macros just to reduce visible graph size.

## Restricted Event Wiring Exists For A Reason

`add_blueprint_event` is intentionally constrained.

Allowed event types and actions are limited because this tool is meant for safe gameplay causality patterns such as:

- overlap -> increment value -> destroy self
- begin play -> print debug
- overlap -> set variable

That makes it useful for well-bounded gameplay scripting, but it is not a substitute for all Blueprint authoring.

## Exact Node Work Requires Context First

Before mutating existing graphs, use:

1. `get_editor_focus_state`
2. `focus_asset_editor`
3. `get_blueprint_editor_context`
4. `list_blueprint_graphs`
5. `list_blueprint_nodes`
6. `focus_blueprint_node` if you need the editor on the exact node

Do not remove or replace nodes by guesswork.

## Compile State Is Not Optional

Blueprint work needs at least three levels of verification:

1. asset/editor context is correct
2. compile diagnostics are clean enough to proceed
3. runtime playtest still makes sense

Use:

- `get_blueprint_compile_diagnostics`
- `assert_blueprint_compiles`
- `validate_blueprint_health`
- `run_quick_playtest`

## Snapshots And Text Export

There are also supporting tools for safer review and recovery:

- `create_blueprint_snapshot`
- `compare_blueprint_snapshots`
- `export_blueprint_text`
- `list_blueprint_snapshots`

These are especially useful when doing broader graph surgery or review-oriented work.

## Reparenting And Repair Are First-Class Operations

The current surface explicitly supports:

- `reparent_blueprint`
- `repair_blueprint_compile_errors`
- `validate_and_repair_blueprint_loop`

That means large Blueprint edits should not rely on manual cleanup as the default strategy. The repo expects deterministic repair/compile loops where possible.

## Key Takeaway

Use the right lane for the job:

- open-Blueprint lane for creating and scaffolding
- editor-native lane for grounded inspection, exact mutation, and repairs

Then compile and verify in runtime context before claiming the Blueprint is healthy.
