# 04 — Workflow

## Phase 1: Define The Player Task And Surface Requirements

Before touching any tool, answer:

- what decision, action, or confirmation does the player need to make
- what is the urgency level (split-second combat read vs. relaxed menu browse)
- what input contexts must work (mouse, gamepad, keyboard, touch)
- what state changes must the surface communicate (health drops, ability ready, inventory changes)
- what existing UI surfaces overlap with this task

This prevents building a widget that looks complete but serves the wrong purpose.

Example outcomes from this phase:

- "HUD health bar: must be readable in split-second combat, must flash red below 25%, must support gamepad-only play"
- "Inventory screen: comparison mode for two items side-by-side, must support controller navigation between slots, must show stat deltas"
- "Confirmation prompt: blocks gameplay, must have clear accept/cancel, must prevent accidental confirm on destructive actions"

## Phase 2: Define Information Hierarchy

Before building the shell, list:

- primary information (what the player scans first — health, selected item, main action)
- secondary support information (less urgent details — exact numbers, stat breakdowns, flavor text)
- current selection or focus cues (what is highlighted, what responds to input)
- success, error, and disabled states (how each element changes when state changes)

This hierarchy determines what is largest, brightest, most central, and most animated. Do not build the shell until this hierarchy is clear — otherwise elements compete for attention with no intentional priority.

## Phase 3: Observe Existing UI State

If the level already has UI, check what exists before adding more.

Use:

- `observe_ue_project` — see the current level state and any active widgets
- `get_widget_editor_context` — confirm which Widget Blueprint editor is open (if editing existing)
- `list_widget_tree` — inspect the current hierarchy, slot structure, and naming

This prevents:

- duplicating a surface that already exists
- editing the wrong widget asset
- breaking bindings or navigation rules by restructuring blindly

If building from scratch in an empty level, skip to Phase 4.

## Phase 4: Create Or Revise The Widget

### New Widget

Choose the creation lane:

- `create_widget` — when the layout is still being shaped by design judgment. Start with this for most new HUD, menu, prompt, or overlay scaffolds.
- `create_widget_from_json` — when the structure is already well-defined in a spec. Good for importing deterministic layouts.

Design rules during creation:

- one Widget Blueprint per screen or logical surface (HUD, inventory panel, pause overlay, dialog box)
- name every widget element meaningfully — names become binding references and navigation targets
- use anchors and alignment instead of absolute pixel positions
- compose complex screens from smaller child Widget Blueprints

### Existing Widget Revision

Start with:

- `get_widget_editor_context` — confirm the asset
- `list_widget_tree` — understand the current hierarchy

Then make targeted changes. Use `rename_widget` when widget identity matters to bindings and references — do not delete and recreate.

## Phase 5: Wire Navigation

For any interactive screen (menus, inventory, prompts, settings):

Use:

- `set_widget_navigation_rule` — define explicit directional focus movement
- `get_widget_navigation_state` — inspect the current focus graph for gaps

Navigation authoring checklist:

- starting focus target defined (which element gets focus when the screen opens)
- directional movement matches visual layout (up goes to the element visually above)
- back/cancel behavior defined (escape or B button returns to previous screen or closes)
- wrap behavior intentional (does the last item wrap to first, or stop)
- all interactive elements reachable by navigation (no orphaned buttons)

This is where the screen becomes usable, not just visible. Skip this phase only for non-interactive display-only widgets.

## Phase 6: Compile And Audit

Use:

- `compile_widget_blueprint` — compile after any structural or logic change
- `verify_widget_blueprint_layout` — audit anchors, hierarchy, slot structure, and common mistakes

Do not batch compile at the end of a large edit session. Compile after each meaningful change to catch problems early.

Common issues caught here:

- missing anchors (elements positioned absolutely)
- empty containers adding complexity without content
- text blocks with no size constraints
- overlapping elements from misaligned anchors

## Phase 7: Verify In PIE

The critical proof step. Use:

- `assert_widget_visible_in_pie` — confirm the widget actually appears on screen during gameplay
- `capture_ui_state` — snapshot current focus state, text content, and visibility
- `simulate_widget_navigation_in_pie` — send real Slate navigation input to test focus movement
- `run_ui_flow_test` — run a multi-step interaction path (navigate, confirm, verify result)

What PIE verification catches that the designer cannot:

- visibility bindings that depend on runtime state (health-dependent elements)
- navigation bugs from real input
- text overflow from actual localized strings
- z-order fights between overlapping widgets
- timing issues with UI animation sequences
- widget-not-added-to-viewport errors

## Phase 8: Iterate On Failures

If PIE verification reveals problems:

- return to inspection (`list_widget_tree`, `get_widget_navigation_state`)
- diagnose whether the problem is structural, navigation, or state-binding
- fix the specific issue
- re-compile and re-verify

Do not stack multiple fixes before recompiling. Fix one thing, verify, then fix the next.

## Recommended Sequences

### New HUD Shell

1. Define player task and urgency level
2. `create_widget` → scaffold the HUD
3. `compile_widget_blueprint` → verify it compiles clean
4. `verify_widget_blueprint_layout` → audit layout assumptions
5. `assert_widget_visible_in_pie` → confirm it appears in game
6. `capture_ui_state` → verify text and state are correct

### Menu With Controller Navigation

1. Define menu structure and navigation expectations
2. `create_widget` or `create_widget_from_json` → build the menu
3. `set_widget_navigation_rule` → wire all directional focus
4. `get_widget_navigation_state` → verify the focus graph is complete
5. `compile_widget_blueprint` → compile
6. `simulate_widget_navigation_in_pie` → test real navigation input
7. `run_ui_flow_test` → prove the full interaction path works

### Existing Widget Repair

1. `get_widget_editor_context` → confirm the asset
2. `list_widget_tree` → understand current hierarchy
3. Targeted fix (rename, restructure, rebind)
4. `compile_widget_blueprint` → compile
5. `verify_widget_blueprint_layout` → audit layout
6. `capture_ui_state` → verify state in PIE

### Confirmation Prompt Validation

1. `compile_widget_blueprint` → compile
2. `assert_widget_visible_in_pie` → confirm it appears when triggered
3. `run_ui_flow_test` → test accept and cancel paths
4. `capture_ui_state` → verify correct result state

### Full UI Surface End-To-End

1. Observe existing state
2. Define hierarchy and player task
3. Create or revise widget
4. Wire navigation
5. Compile and audit layout
6. Assert visibility in PIE
7. Run interaction flow test
8. Capture final state for verification

## Key Takeaway

The safe order is: define the job first, define hierarchy second, structure the widget third, wire navigation fourth, prove interaction fifth.

Skipping the last step is how a screen becomes visually complete but behaviorally wrong. Skipping the inspection step before edits is how clean-looking widgets get broken.
