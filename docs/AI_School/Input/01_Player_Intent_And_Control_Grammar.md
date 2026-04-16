# 01 - Player Intent And Control Grammar

Input is how the player speaks to the game.

That means a good input system should feel like a stable grammar, not a pile of disconnected button assignments.

## Start With Verbs

Before creating any input asset, define the player verbs:

- move
- look
- jump
- interact
- attack
- dodge
- confirm
- cancel

These verbs are the contract.

Keys, buttons, and sticks are only the transport layer.

## Intent Matters More Than Hardware

The player thinks:

- I want to move left
- I want to open the menu
- I want to use the selected item

The player does not think:

- I want the `W` key to mean two different things depending on hidden state

When control schemes are built around hardware instead of intent, they become hard to learn and hard to maintain.

A verb like `Interact` is stable across keyboards, gamepads, and touch. A key assignment like `KeyE` is hardware-specific and meaningless on a different device.

```
Player intent layer:    Move    Look    Jump    Interact    Attack
                         │       │       │         │          │
Value type contract:   Axis2D  Axis2D  Digital  Digital    Digital
                         │       │       │         │          │
Hardware transport:    WASD    Mouse   Space      E        LMouse
                       LStick  RStick  BtnSouth  BtnWest   RT
```

This layering is what makes the control scheme portable and predictable.

## Contexts Should Represent Modes

Good input systems separate meaning by context:

- moment-to-moment gameplay
- menu navigation
- dialogue choices
- vehicle control
- placement or build mode

Contexts keep the mapping grammar understandable.

If every mode shares one giant context, conflicts multiply and control meaning becomes opaque.

## Value Types Are Part Of The Contract

Different verbs need different input shapes:

- Digital for simple pressed or not (jump, confirm, interact)
- Axis1D for one-dimensional analog ranges (throttle, zoom, steering wheel)
- Axis2D for movement or look planes (character movement, camera look)
- Axis3D when the interaction truly needs three axes (rare in most games)

Choosing the wrong value type early creates friction later. If movement is modeled as four Digital actions instead of one Axis2D, the downstream consumption loses analog range, deadzone control, and stick normalization.

Value types are contracts between the input layer and the gameplay layer. Changing them later breaks every consumer.

## Consistency Beats Cleverness

Strong control schemes make similar actions feel similar.

If confirm, accept, interact, and advance all move around without reason, the player pays a tax every time the game changes state.

Do not make the player relearn the grammar in each screen or mode unless the fantasy truly changes.

## Feel Comes From Runtime, Not Asset Names

An input action asset can be perfectly named and still feel wrong in play because of:

- poor mapping choices
- context conflicts
- mismatch with animation or gameplay timing
- untested controller paths

That is why runtime proof matters.

## Key Takeaway

Good input design starts from player intent, separates contexts clearly, and uses runtime feel as the final judge.
