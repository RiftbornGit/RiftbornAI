# 03 — Anti-Patterns

## 1. Building A Pretty Layout Before Defining The Player Task

Symptoms:

- the widget has fonts, colors, and spacing but no clarity about what decision it supports
- the team argues about visual polish while the player still cannot find the action that matters
- layout revisions break because the styling was built before the structure was stable

Why it is bad:

- visual investment makes structural changes expensive
- layout without purpose drifts into decoration
- the player task is the foundation; skipping it means rebuilding later

Do instead:

- define the player task, urgency, and input context first
- build hierarchy structure before styling
- test readability before polish

## 2. Giving Every Element Equal Weight

Symptoms:

- every panel, label, and button is the same size and contrast
- the player scans the whole screen instead of finding what matters instantly
- nothing stands out, so everything feels low priority

Why it is bad:

- hierarchy is the core craft of UI — without it the screen is noise
- equal weight means equal reading cost, which means slow decisions
- the HUD competes with itself

Do instead:

- make the most important element largest, most contrasty, or highest on screen
- reduce secondary elements: smaller, dimmer, or grouped apart
- use motion or color accent only for state changes that need attention

## 3. Treating HUDs Like Dashboards

Symptoms:

- health, mana, ammo, XP, quest, minimap, compass, status effects, ability cooldowns, and inventory all visible simultaneously
- the player's eyes dart between clusters with no clear priority
- adding a new system means adding another widget to the pile

Why it is bad:

- persistent clutter creates scanning tax during gameplay
- every always-on element competes for attention with gameplay-critical information
- screen real estate is finite — burning it on low-urgency data costs high-urgency readability

Do instead:

- show only what the player needs for the current gameplay state
- fade or hide secondary info until it becomes relevant (low health triggers health bar, ability becomes available triggers highlight)
- design for the gameplay camera view, not for a static mockup

## 4. Leaving Navigation Implicit

Symptoms:

- controller or keyboard focus jumps to unpredictable widgets
- tabbing through a menu visits elements in visual order rather than logical order
- there is no starting focus, no back behavior, no confirm path
- gamepad users cannot complete basic flows

Why it is bad:

- navigation IS the interface for non-mouse users
- unpredictable focus makes menus feel broken, not just inconvenient
- implicit navigation order depends on widget creation order, which is rarely the right order

Do instead:

- define navigation rules explicitly with `set_widget_navigation_rule`
- set a starting focus target for every screen
- define back/cancel behavior for every menu
- test with `simulate_widget_navigation_in_pie` before calling it done

## 5. Restructuring Widgets Blindly

Symptoms:

- widgets rearranged or renamed without checking the current hierarchy
- bindings, slot assumptions, or navigation rules break silently
- changes that looked safe in the editor fail in PIE

Why it is bad:

- UMG hierarchy has dependencies: bindings reference named children, navigation rules reference specific widgets, visibility logic depends on parent/child relationships
- changing structure without understanding it creates hard-to-trace runtime bugs
- the editor may not show the failure until PIE

Do instead:

- inspect with `get_widget_editor_context` and `list_widget_tree` before restructuring
- rename with `rename_widget` (preserves binding references) instead of delete-and-recreate
- compile and verify layout after any structural change

## 6. Trusting Designer Preview As Final Proof

Symptoms:

- the widget looks correct in the Widget Blueprint designer
- nobody tested it in PIE with real gameplay data
- runtime visibility, focus state, or binding values differ from preview

Why it is bad:

- the designer shows a static snapshot with default values
- runtime state (player health, inventory contents, quest state) can change layout, text overflow, and visibility
- timing-dependent UI (fade in, pulse, animation) only works at runtime

Do instead:

- use `assert_widget_visible_in_pie` to confirm the widget actually appears
- use `capture_ui_state` to check runtime text and focus state
- test with real gameplay data, not just designer defaults

## 7. Assuming JSON Import Means The Widget Is Finished

Symptoms:

- `create_widget_from_json` completed successfully
- nobody reviewed the result in the editor or PIE
- assumptions about slot types, anchors, or bindings were wrong in the spec

Why it is bad:

- JSON import creates structure, not necessarily correct structure
- anchor and alignment mistakes are invisible until the screen resizes
- missing visibility bindings, navigation rules, or state-change logic make the widget a shell

Do instead:

- inspect the imported hierarchy with `list_widget_tree`
- compile with `compile_widget_blueprint`
- audit layout with `verify_widget_blueprint_layout`
- wire navigation and test in PIE

## 8. One Widget Blueprint For Everything

Symptoms:

- HUD, inventory, pause menu, and dialog system are all in one Widget Blueprint
- the event graph is massive with unrelated logic interleaved
- editing one screen risks breaking another

Why it is bad:

- Widget Blueprints should have one job, like any other class
- mixed responsibility means mixed bugs and mixed testing
- reuse becomes impossible when everything is entangled

Do instead:

- one Widget Blueprint per screen or logical surface (HUD, inventory panel, pause overlay, dialog box)
- compose screens by adding child widgets, not by building monoliths
- separate data logic from presentation where possible

## 9. Ignoring Anchor And Alignment

Symptoms:

- widgets placed by absolute pixel position
- the UI breaks at different resolutions or aspect ratios
- elements overlap or disappear on wide/narrow screens

Why it is bad:

- games ship to multiple resolutions and aspect ratios
- absolute positioning only works for one screen size
- anchor and alignment are UMG's answer to responsive layout

Do instead:

- use anchors to pin elements to screen regions (top-left, center, stretch)
- use alignment to offset from anchor points instead of absolute coordinates
- test at multiple resolutions (at minimum: 1080p and 4K) using `verify_widget_blueprint_layout`

## 10. Skipping State Change Feedback

Symptoms:

- values change but nothing visually indicates the change
- health drops with no flash, pulse, or color shift
- buttons become disabled with no visual distinction

Why it is bad:

- state change is the most important thing UI communicates
- without feedback, the player does not notice changes or thinks the UI is broken
- feedback tells the player their actions had consequence

Do instead:

- every state change needs a visual signal: color, scale, opacity, motion, or sound
- critical changes (health low, ability ready, error state) need stronger signals than minor updates
- disabled states must look visibly different from active states

## Key Takeaway

Most bad game UI is not missing elements. It is missing hierarchy, missing navigation, missing state feedback, or missing runtime proof. The screen exists but does not serve the player's decisions.
- broken focus
- wrong text
- bad safe-area assumptions
- runtime overlap or clipping

## 7. Importing A JSON Spec And Assuming It Is Finished

`create_widget_from_json` is a strong scaffold lane, not a quality guarantee.
Every imported screen still needs hierarchy review, layout audit, and live verification.

## 8. Renaming Or Rewiring Widgets Casually

Widget identity can affect bindings and logic assumptions.
Use the supported rename lane and verify afterward.

## Key Takeaway

Most bad UI is not caused by missing widgets.

It is caused by weak priority, weak navigation, and weak runtime proof.
