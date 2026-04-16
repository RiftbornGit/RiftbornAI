# 02 — UE5 Widget And UI Verification Systems

RiftbornAI's UI surface covers Widget Blueprint creation, widget-tree inspection, compile and layout validation, navigation wiring, and live PIE verification.

## System Architecture

The UMG widget stack has clear layers:

```
Player Controller
  └── HUD Class
        └── Widget Blueprints (added to viewport)
              ├── Root Canvas Panel
              │     ├── Child Widgets (panels, text, images, buttons)
              │     │     ├── Slot properties (anchors, alignment, size)
              │     │     └── Visibility bindings
              │     └── Nested Widget Blueprints (composition)
              ├── Event Graph (interaction logic)
              └── Animations (state-driven visual feedback)
```

Widget Blueprints are the authoring containers. Each widget should have one job — compose complex screens from smaller, focused widgets.

## Widget Blueprints Are The Main Authoring Container

Use:

- `create_widget` — when the layout is still being shaped by design judgment. Good default for new HUD, menu, prompt, or overlay scaffolds.
- `create_widget_from_json` — when the structure is already well-specified and deterministic. Good for importing structured specs.

Key concepts:

- the **Canvas Panel** is the default root — children are positioned by anchors and alignment
- **anchors** pin elements to screen regions (top-left corner, center, stretch to fill)
- **alignment** offsets from the anchor point (0,0 = top-left of element at anchor; 0.5,0.5 = centered on anchor)
- **size-to-content** vs explicit size determines whether the widget grows with content or stays fixed

Design rules for Widget Blueprints:

- one Widget Blueprint per screen or logical surface (do not build monoliths)
- compose complex screens by nesting child Widget Blueprints
- name every widget meaningfully — names become binding references and navigation targets

## Inspect Before Editing

Use:

- `get_widget_editor_context` — confirms which Widget Blueprint editor is open and which asset is active
- `list_widget_tree` — shows the real parent-child hierarchy, slot arrangement, and widget types

These tools tell you the honest baseline before any structural changes. Without this step:

- you may edit the wrong widget
- you may break bindings by rearranging a hierarchy you did not understand
- you may duplicate structure that already exists

Always inspect before restructure.

## Compile And Audit Layout

Use:

- `compile_widget_blueprint` — the safe compile lane for Widget Blueprints. Run after any structural or logic change.
- `verify_widget_blueprint_layout` — audits anchors, hierarchy, slot structure, and common authoring mistakes. Catches responsive layout issues before PIE.
- `rename_widget` — the correct rename lane when widget identity matters to bindings, navigation rules, and code references.

Common layout problems this catches:

- elements with no anchor (absolute positioning that breaks at other resolutions)
- overlapping elements from incorrect alignment
- empty containers that add hierarchy complexity without content
- text blocks with no size constraints (content overflow)

## Navigation Is A First-Class System

Use:

- `set_widget_navigation_rule` — define explicit directional focus movement (up, down, left, right, next, previous)
- `get_widget_navigation_state` — inspect the current focus graph and identify gaps

Navigation must be authored for every interactive screen. This matters for:

- menus (main menu, pause, settings)
- inventory and shop screens
- confirmation prompts
- any UI expected to work with gamepad or keyboard

Navigation design rules:

- set a starting focus target for every screen (the most common action or first list item)
- define back/cancel behavior explicitly
- directional focus should match visual layout (up goes to the element visually above)
- wrap behavior must be intentional (does pressing down on the last item go back to top?)

If navigation is not specified, the interface is only partially built.

## Live Verification In PIE

Use:

- `assert_widget_visible_in_pie` — confirms the widget appears on screen during gameplay. Catches missing add-to-viewport, wrong visibility bindings, or z-order issues.
- `capture_ui_state` — snapshots current focus state, text content, and visibility. Useful for debugging state-dependent UI.
- `run_ui_flow_test` — runs a multi-step interaction path (navigate to option, confirm, verify result). Good for menus, inventory flows, and critical path testing.
- `simulate_widget_navigation_in_pie` — sends real Slate navigation input during PIE. Tests focus movement with actual gamepad/keyboard simulation.

PIE verification catches problems the designer preview cannot:

- visibility bindings that depend on runtime state (health-dependent HUD elements)
- navigation bugs that only appear with real input
- text overflow from actual localized strings
- z-order fights between multiple overlapping widgets
- timing issues with UI animation sequences

## Current Surface Strength

Strongest for:

- Widget Blueprint creation and structured import
- widget tree inspection and safe hierarchy edits
- compile and layout validation
- explicit navigation authoring
- runtime PIE verification of visibility, focus, and interaction

Not a substitute for:

- visual design judgment (the tools build structure, not taste)
- responsive layout testing across many resolutions (verify manually at target resolutions)
- complex animation sequencing (UMG animation tools are beyond the current authoring surface)

## Key Takeaway

The UI tool surface has four layers: create the structure, inspect before editing, compile and validate, verify in PIE. Skipping the inspection step before edits or the PIE step after edits is how clean-looking widgets break at runtime.

- `assert_widget_visible_in_pie`
- `capture_ui_state`
- `run_ui_flow_test`
- `simulate_widget_navigation_in_pie`

to verify the interface in runtime context.

This is the lane for proving:

- the widget actually appears
- the right text and focus state are present
- the expected input flow works
- the player can navigate and complete the task

## Current Strength Of The Surface

The UI tool surface is strongest for:

- creating widget shells
- inspecting real hierarchy
- compiling and auditing layout
- wiring and checking navigation
- running live UI proof in PIE

It is not a substitute for product judgment about:

- information priority
- art direction
- copywriting
- overall UX scope

Those decisions still need to come first.

## Key Takeaway

Treat Widget Blueprint work as a loop:

structure -> compile -> audit -> verify in PIE

If you skip the last two steps, the interface is still hypothetical.
