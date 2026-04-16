# 02 - UE5 Enhanced Input Systems

RiftbornAI's current input surface is centered on Enhanced Input asset creation and base mapping authoring.

```
┌───────────────────────────────────────────────────────┐
│ Enhanced Input Pipeline                                │
│                                                        │
│ Input Action (verb)                                    │
│   └── value type: Digital | Axis1D | Axis2D | Axis3D  │
│                                                        │
│ Mapping Context (mode container)                       │
│   └── key/button/axis bindings per action              │
│   └── modifiers (deadzone, negate, swizzle) [beta]     │
│   └── triggers (pressed, held, released) [beta]        │
│                                                        │
│ Runtime                                                │
│   └── active contexts stack → resolve → fire delegates │
└───────────────────────────────────────────────────────┘
```

## The Production Lane Covers Three Core Jobs

Use:

- `create_input_action`
- `create_input_mapping_context`
- `add_input_mapping`

for the default visible lane.

These tools let you:

- define new input actions
- choose the action value type
- create mapping-context containers
- bind concrete keys or buttons into those contexts

## Input Actions Are The Verb Assets

Use `create_input_action` to define the gameplay verb.

Key decisions:

- the action name should describe the intent clearly (`IA_Jump`, `IA_Interact`, `IA_Move`)
- the value type must match the control shape the game actually needs

Value type reference:

| Value Type | Use For | Example |
|-----------|---------|---------|
| Digital | Boolean pressed/not | Jump, Confirm, Interact |
| Axis1D | One-dimensional range | Throttle, Zoom, Scroll |
| Axis2D | Two-axis plane | Movement, Camera Look |
| Axis3D | Three-axis space | Rare; 6DOF flight |

If those are wrong, the rest of the input stack is built on weak ground. Value types are contracts — changing them later breaks every downstream consumer.

## Mapping Contexts Are The Mode Containers

Use `create_input_mapping_context` to separate major control modes.

Examples:

- default gameplay
- UI navigation
- vehicle
- build mode

Contexts are how you prevent the same hardware input from becoming ambiguous everywhere.

## Key Mappings Are The Hardware Bindings

Use `add_input_mapping` to bind a concrete key or button to an action inside a context.

This is the safe production lane for:

- keyboard keys
- mouse buttons
- controller buttons
- simple direct mappings

## Deeper Modifier And Trigger Surfaces Exist, But Are Not The Default Lane

There are adjacent helper tools for modifiers and triggers, but they are beta-gated rather than part of the primary shipped lane.

That means:

- do not assume they are available in every session
- verify with `list_all_tools` or `describe_tool` before relying on them

The school should teach the control architecture first, then let exact deeper tooling follow the live registry.

## Runtime Verification Still Matters

The input surface does not replace runtime proof.

Use:

- `run_quick_playtest`

to verify that the authored assets actually produce understandable, usable controls.

If the task also changes menu navigation, read the UI track.
If the task also changes gameplay logic reacting to input, read the Blueprint or C++ tracks too.

## Current Strength Of The Surface

The input tool surface is strongest for:

- action creation with explicit value types
- mapping-context creation for mode separation
- concrete base key/button mappings
- runtime proof through playtest

It is weaker as a full introspection or tuning environment than some other asset domains.

What this means in practice:

- design decisions (verb taxonomy, context boundaries, value types) must be made clearly before tool calls
- the tools create assets reliably, but they cannot tell you whether the design is right
- runtime feel is the only real verification, so `run_quick_playtest` is non-negotiable

Adjacent domains:

- if the task also changes menu navigation or HUD prompts, read the UI track
- if the task also changes gameplay logic reacting to input, read the Blueprint or C++ tracks
- if the task involves gamepad-specific analog tuning beyond base mappings, check the live registry for deeper helpers

## Key Takeaway

Treat the current input lane as:

actions -> contexts -> mappings -> playtest proof

If you stop before the last step, you only know that assets exist, not that the controls are good.
