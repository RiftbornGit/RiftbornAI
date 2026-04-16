# 03 - Anti-Patterns

## 1. Starting From Keys Instead Of Verbs

Symptom: the first design conversation is about `SpaceBar` or `ButtonSouth` instead of `Jump` or `Interact`.

Why it fails: the control scheme is shaped by hardware, not by player intent. When you remap or add a gamepad, the names and structure stop making sense.

Do instead: define the verb taxonomy first. Map hardware to verbs, not the reverse.

## 2. One Giant Context For Everything

Symptom: gameplay, UI, dialogue, vehicles, and build mode all compete inside a single mapping context. Context conflicts surface as "pressing X does the wrong thing."

Why it fails: Enhanced Input resolves bindings per context. When one context holds everything, every action competes for the same key space. Adding a new mode creates silent conflicts.

Do instead: create a new mapping context for each distinct player mode. Contexts are cheap; debugging context conflicts is not.

## 3. Naming Actions After Hardware

Symptom: action names like `IA_LeftMouse`, `IA_ButtonSouth`, `IA_KeyQ`.

Why it fails: the action name describes transport, not intent. When the control scheme is remapped, read through a Blueprint, or displayed in UI prompts, the name carries no meaning.

Do instead: name actions after verbs: `IA_Attack`, `IA_Interact`, `IA_OpenInventory`.

## 4. Using The Wrong Value Type

Symptom: 2D movement modeled as four separate digital actions (MoveForward, MoveBack, MoveLeft, MoveRight) instead of one Axis2D.

Why it fails: the downstream consumption expects a vector. Reconstructing one from four booleans loses analog range, introduces deadzone problems, and makes the action harder to map to sticks.

Do instead: choose the value type that matches the interaction shape. Movement is Axis2D. Look is Axis2D. Jump is Digital. Throttle is Axis1D.

## 5. Overloading The Same Input Without Strong Context Boundaries

Symptom: `E` means interact, talk, pick up, open, and dismount depending on hidden state that the player cannot see.

Why it fails: the player cannot predict what will happen. One key doing many things is only safe when the active context is obvious.

Do instead: separate contexts make the meaning of each key unambiguous per mode. If the modes share a key, the player should be able to predict which mode they are in.

## 6. Copying Keyboard Logic To Controller Blindly

Symptom: direct 1:1 keyboard-to-gamepad mapping where the control layout technically works but the ergonomics are wrong — important actions on hard-to-reach buttons, no analog advantage used.

Why it fails: keyboard and gamepad have different ergonomic shapes. A layout that feels natural on keyboard can produce hand fatigue or missed inputs on a controller.

Do instead: design each device profile with device-appropriate ergonomics. Face buttons for frequent actions, triggers for analog, bumpers for modifiers.

## 7. Treating Asset Creation As Finished Input Design

Symptom: input actions and mapping contexts are created, but the controls are never tested in runtime. "The assets exist" is treated as "the controls are done."

Why it fails: input quality is a feel problem. Timing, analog curves, context switching, and controller dead zones cannot be assessed from assets alone.

Do instead: playtest the controls on the intended device. Runtime proof is the only real verification.

## 8. Relying On Beta Input Depth Without Checking Availability

Symptom: planning a workflow around modifier or trigger helpers that appear in documentation but are not present in the default visible tool lane.

Why it fails: if the tool is not registered, the entire plan stalls. Beta-gated tools require explicit verification.

Do instead: `list_all_tools` or `describe_tool` before building a workflow around any non-default input helper.

## 9. Ignoring Analog Range On Digital Actions

Symptom: modeling a trigger pull or steering as Digital when the gameplay expects analog expression.

Why it fails: the control flattens the input to on/off. Players lose the ability to half-press, feather, or modulate.

Do instead: if the verb benefits from analog expression, use Axis1D or Axis2D. Reserve Digital for true boolean intent (jump, confirm, cancel).

## Key Takeaway

Most bad input systems are not missing bindings.

They are missing a clear verb model, clear context boundaries, or runtime proof.
