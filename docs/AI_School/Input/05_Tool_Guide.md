# 05 - Tool Guide

Use exact registered input tool names. If a name is uncertain, verify with `describe_tool`.

## Input Tool Map

### Action And Context Authoring

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create a new input verb asset | `create_input_action` | Choose a clear action name and the correct value type |
| Create a mode-specific mapping container | `create_input_mapping_context` | Separate gameplay, UI, vehicle, or other major contexts |

Design rules:

- name actions after player verbs, not hardware (`IA_Jump`, not `IA_Spacebar`)
- choose value type deliberately: Digital, Axis1D, Axis2D, Axis3D must match the interaction shape
- create separate contexts for distinct modes — one context per coherent control grammar

### Mapping

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Bind a key or button to an action | `add_input_mapping` | Core production lane for direct hardware mapping |

Design rules:

- map keyboard and gamepad bindings when both input paths are expected
- verify that the same physical key is not used for conflicting actions in the same context
- if the task requires modifiers or triggers, check the live registry before assuming helper tools exist

### Verification

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Verify the authored controls in runtime | `run_quick_playtest` | Required because input quality is a feel problem, not an asset problem |
| Check for deeper helper availability | `list_all_tools`, `describe_tool` | Use before relying on beta-gated trigger or modifier helpers |

Design rules:

- runtime proof is non-negotiable — asset existence does not equal good controls
- test on the intended device type (keyboard vs gamepad vs both)
- verify context switching if the game has multiple modes

## Default Lanes

### Action And Context Authoring Lane

Use for:

- creating new verbs
- defining value types
- creating mode containers

Main tools:

- `create_input_action`
- `create_input_mapping_context`

### Mapping Lane

Use for:

- connecting hardware inputs to actions

Main tools:

- `add_input_mapping`

### Verification Lane

Use for:

- proving the control scheme actually works in play

Main tools:

- `run_quick_playtest`

## Verification Checklist

Before calling an input surface done:

- [ ] all player verbs identified and named after intent, not hardware
- [ ] each action has the correct value type (Digital, Axis1D, Axis2D, Axis3D)
- [ ] mapping contexts separate distinct modes (gameplay, UI, vehicle, etc.)
- [ ] bindings added for all intended device types
- [ ] no conflicting bindings within the same context
- [ ] deeper modifier/trigger helpers verified against live registry if needed
- [ ] controls exercised in runtime playtest (`run_quick_playtest`)
- [ ] context switching tested if multiple modes exist

## Proven Sequences

### Core Gameplay Inputs

`create_input_action` -> `create_input_mapping_context` -> `add_input_mapping` -> `run_quick_playtest`

Use for: initial control setup — movement, look, jump, interact, attack. The foundational verb layer.

### Separate UI Or Mode Context

`create_input_mapping_context` -> `create_input_action` -> `add_input_mapping` -> verify the mode transition and runtime behavior

Use for: adding a distinct control grammar for menus, dialogue, inventory, build mode, or vehicles.

### Extension Of An Existing Scheme

`create_input_action` only if a new verb is truly needed -> `add_input_mapping` -> `run_quick_playtest`

Use for: adding a new ability or interaction to an already-stable control scheme.

### Device Profile Expansion

review existing keyboard bindings -> `add_input_mapping` for gamepad equivalents -> `run_quick_playtest` on both devices

Use for: ensuring the game feels coherent on keyboard+mouse and gamepad when both are expected.

## Rules

1. Do not design from keys outward — start from player verbs.
2. Do not stuff every control into one context — separate by mode.
3. Do not assume beta modifier or trigger helpers are present without checking the live registry.
4. Do not call an input task done until the controls are exercised in runtime.
5. Do not name actions after hardware when they represent verbs.
6. Do not add bindings for only one device type when both keyboard and gamepad are expected.
7. Value types are contracts — changing them later breaks downstream consumption.

## Key Takeaway

The input surface is strongest when you separate:

- verb definition
- context definition
- hardware mapping
- playtest proof

Trying to skip straight to key binding usually produces a weak control language.
