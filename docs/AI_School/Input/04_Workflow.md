# 04 - Workflow

Input work should move from verbs to contexts to mappings to runtime proof.

## Phase 1: Define The Action Taxonomy

Before touching an input tool, answer:

- what verbs exist (move, look, jump, interact, attack, dodge, confirm, cancel)
- which verbs are always available vs mode-dependent
- which verbs are digital (jump, confirm) vs analog (move, steer, throttle)

This is the design skeleton. If you skip it, the mapping will be arbitrary.

Write the verb list as a simple table:

```
Verb        | Value Type | Context        | Devices
Move        | Axis2D     | Gameplay       | WASD, Left Stick
Look        | Axis2D     | Gameplay       | Mouse, Right Stick
Jump        | Digital    | Gameplay       | Space, Face South
Interact    | Digital    | Gameplay       | E, Face West
Confirm     | Digital    | UI             | Enter, Face South
Cancel      | Digital    | UI             | Escape, Face East
```

This table becomes the source of truth for the rest of the workflow.

## Phase 2: Choose Value Types

For each action, choose the correct input value shape:

- Digital
- Axis1D
- Axis2D
- Axis3D

Do not leave this vague.

## Phase 3: Define Context Boundaries

Decide which mapping contexts you need.

Typical separation:

- gameplay
- UI
- vehicle
- build or placement

If the player changes mode, the control context should usually make that explicit.

## Phase 4: Create The Assets

Use:

- `create_input_action`
- `create_input_mapping_context`

to create the core assets.

## Phase 5: Add Concrete Bindings

Use:

- `add_input_mapping`

to connect hardware inputs to actions in the right context.

If the task depends on modifiers or triggers, check the live registry first before assuming those deeper helpers are available.

## Phase 6: Verify In Runtime

Use:

- `run_quick_playtest`

This is where the real quality check happens. Verify:

- the intended actions are reachable and responsive
- the context boundaries activate and deactivate at the right moment
- the controls feel coherent on the intended device (keyboard, gamepad, or both)
- analog range feels appropriate (movement feels smooth, not binary)
- no accidental conflicts between actions in the same context

If something feels wrong in play, the fix is in the verb model or mapping, not in more assets.

## Recommended Sequences

### New Basic Gameplay Controls

`define verbs and value types` -> `create_input_action` (Move, Look, Jump, Interact) -> `create_input_mapping_context` (Gameplay) -> `add_input_mapping` (keyboard + gamepad) -> `run_quick_playtest`

This is the foundation. Do not add complexity until this layer feels right.

### New Mode-Specific Control Layer

`define mode boundary` -> `create_input_mapping_context` (UI, Vehicle, etc.) -> `create_input_action` (mode-specific verbs) -> `add_input_mapping` (mode bindings) -> runtime verification in that mode

Verify both the mode's controls and the transition between modes.

### Existing Scheme Extension

`review the verb model and active contexts` -> add only the needed new actions with `create_input_action` -> `add_input_mapping` -> `run_quick_playtest`

Do not add new contexts or restructure the verb model unless the extension genuinely requires it.

### Multi-Device Mapping Pass

`review existing keyboard bindings` -> `add_input_mapping` (gamepad equivalents) -> `run_quick_playtest` on both device types

Verify that the gamepad layout has appropriate ergonomics, not just 1:1 key equivalence.

### Full Input Overhaul

`document current verb taxonomy` -> `identify context boundaries` -> `create_input_action` for all verbs -> `create_input_mapping_context` per mode -> `add_input_mapping` for all devices -> `run_quick_playtest` per mode

## Key Takeaway

The safe order is:

design the verbs first, map the hardware second, judge the result in play third.

If you reverse that order, the control scheme usually becomes arbitrary.
