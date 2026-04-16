# 05 — Tool Guide

Use exact registered UI tool names. If a name is uncertain, verify with `describe_tool`.

## Widget Creation Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `create_widget` | Create a new Widget Blueprint shell | Default lane for new HUD, menu, prompt, or overlay scaffolds. Use when design judgment is still shaping the layout. |
| `create_widget_from_json` | Import a widget from structured spec | Use when the hierarchy is deterministic and fully defined. Good for batch-importing design specs. |

Design rules:

- one Widget Blueprint per screen or logical surface
- name every widget element meaningfully — names become binding references
- decide between `create_widget` and `create_widget_from_json` based on whether design exploration or spec fidelity matters more

## Widget Inspection Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `get_widget_editor_context` | Show which Widget Blueprint editor is open and which asset is active | Before any edit — confirms you are editing the right asset |
| `list_widget_tree` | Show the full parent/child hierarchy, slot types, and widget names | Before restructuring — reveals bindings, slot assumptions, and naming |

Design rules:

- always inspect before restructure
- if the tree has unexpected structure, investigate before changing it
- the inspection result is the honest baseline for safe edits

## Widget Structure Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `rename_widget` | Rename a widget element while preserving binding references | When widget identity matters to bindings, navigation rules, or code references. Never delete-and-recreate when rename would preserve references. |
| `set_widget_property` | Set a property on a widget element (visibility, text, size, etc.) | When changing authored properties on existing widgets |
| `set_widget_visibility` | Set widget visibility state | When toggling element visibility based on state |
| `add_widget_child` | Add a child widget to a parent container | When composing screens from smaller widget pieces |

Design rules:

- rename preserves references; delete-and-recreate breaks them
- visibility changes should match state logic (low health → show warning)
- child composition is preferred over monolithic Widget Blueprints

## Compile And Layout Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `compile_widget_blueprint` | Compile the Widget Blueprint asset | After any structural or logic change. Do not batch at end of session. |
| `verify_widget_blueprint_layout` | Audit anchors, hierarchy, slot structure, common authoring mistakes | After compile — catches responsive layout issues before PIE |

Design rules:

- compile after each meaningful change, not at the end
- layout verification catches: missing anchors, overlapping elements, empty containers, unbounded text
- if layout audit fails, fix before moving to PIE

## Navigation Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `set_widget_navigation_rule` | Define explicit directional focus movement (up/down/left/right/next/previous) | For every interactive screen that must work with gamepad or keyboard |
| `get_widget_navigation_state` | Inspect the current focus graph and find gaps | After setting navigation rules — verifies completeness |

Design rules:

- every interactive screen needs explicit navigation
- starting focus target must be defined
- directional movement must match visual layout
- back/cancel behavior must be explicit
- verify the focus graph with `get_widget_navigation_state` before trusting it

## PIE Verification Tools

| Tool | Purpose | When To Use |
|------|---------|-------------|
| `assert_widget_visible_in_pie` | Confirm widget appears on screen during gameplay | First runtime check — proves the widget is added to viewport and visible |
| `capture_ui_state` | Snapshot current focus state, text content, and visibility | After visibility check — captures runtime state for debugging |
| `simulate_widget_navigation_in_pie` | Send real Slate navigation input during PIE | For testing actual gamepad/keyboard navigation behavior |
| `run_ui_flow_test` | Run a multi-step interaction path (navigate, confirm, verify result) | For proving complete interaction flows: menu paths, prompt acceptance, inventory actions |

Design rules:

- PIE verification is not optional — the designer preview is not proof
- assert visibility first, then test navigation, then test full flows
- capture state for debugging when behavior does not match expectations
- test with real input, not assumptions

## Default Lanes

### Authoring And Structure Lane

Main tools: `create_widget`, `create_widget_from_json`, `get_widget_editor_context`, `list_widget_tree`, `rename_widget`, `add_widget_child`, `set_widget_property`

Use for: creating new screens, importing structured specs, inspecting and restructuring hierarchy, composing complex screens from child widgets.

### Compile And Layout Lane

Main tools: `compile_widget_blueprint`, `verify_widget_blueprint_layout`

Use for: proving the asset compiles, catching layout and anchor mistakes, validating responsive behavior.

### Runtime Interaction Lane

Main tools: `assert_widget_visible_in_pie`, `capture_ui_state`, `set_widget_navigation_rule`, `get_widget_navigation_state`, `simulate_widget_navigation_in_pie`, `run_ui_flow_test`

Use for: proving visibility, checking focus and text state, validating navigation, proving complete interaction flows.

## Verification Checklist

Before calling a UI surface done:

- [ ] widget compiles clean (`compile_widget_blueprint`)
- [ ] layout audit passes (`verify_widget_blueprint_layout`)
- [ ] widget visible in PIE (`assert_widget_visible_in_pie`)
- [ ] navigation graph complete (if interactive) (`get_widget_navigation_state`)
- [ ] navigation tested with real input (if interactive) (`simulate_widget_navigation_in_pie`)
- [ ] critical interaction flows proven (`run_ui_flow_test`)
- [ ] state captured for reference (`capture_ui_state`)

## Proven Sequences

### New HUD Surface

`create_widget` → `compile_widget_blueprint` → `verify_widget_blueprint_layout` → `assert_widget_visible_in_pie` → `capture_ui_state`

Use for: health bars, ammo counters, minimap shells, ability cooldown displays.

### Menu Navigation Pass

`create_widget` or `create_widget_from_json` → `set_widget_navigation_rule` → `get_widget_navigation_state` → `compile_widget_blueprint` → `simulate_widget_navigation_in_pie` → `run_ui_flow_test`

Use for: main menus, pause screens, settings panels, inventory grids.

### Existing Widget Diagnosis And Repair

`get_widget_editor_context` → `list_widget_tree` → targeted fix → `compile_widget_blueprint` → `verify_widget_blueprint_layout` → `capture_ui_state`

Use for: fixing broken navigation, repairing widget hierarchy, resolving visibility bugs.

### End-To-End UI Flow Proof

`assert_widget_visible_in_pie` → `run_ui_flow_test` → `capture_ui_state`

Use for: final proof of critical interaction paths — accept/cancel prompts, purchase flows, character creation steps.

### Structured Import And Validation

`create_widget_from_json` → `list_widget_tree` → `compile_widget_blueprint` → `verify_widget_blueprint_layout` → `assert_widget_visible_in_pie`

Use for: importing design specs, validating imported structure matches intent.

## Rules

1. Do not treat visual completion as interaction completion.
2. Do not trust mouse-only checks for screens that must support controller.
3. Do not restructure widgets blind — inspect the tree first.
4. Do not skip compile or layout audit after meaningful hierarchy changes.
5. Do not call a screen done until the intended flow works in PIE.
6. Do not build monolithic Widget Blueprints — compose from focused children.
7. Do not use absolute positioning when anchors and alignment would work.

## Key Takeaway

The UI surface is strongest when you separate: widget creation, hierarchy inspection, compile and layout audit, and live interaction proof. If you skip the last stage, the player is testing your assumptions for you.
