# 03 - Anti-Patterns

## 1. Creating Layers Before Defining What They Mean

If the difference between two layers cannot be explained clearly, the layer split is probably premature or arbitrary.

## 2. Mixing Spatial And State Concerns Into One Mess

Approach-based loading and world-state variation are different design problems.
When they are mixed carelessly, debugging the world becomes painful.

## 3. Assigning Actors To Layers Blind

Reassigning actors without first checking the current layer set and partition state is how worlds drift into contradictory loading behavior.

## 4. Treating Data Layers As Generic Buckets

Layers should encode meaning, not just file organization convenience.

## 5. Letting Important Content Load Too Late

If a landmark, route clue, or encounter setup appears only after the player already needs it, streaming has failed the experience.

## 6. Ignoring Texture Residency

Actor presence is not enough if important surfaces are still obviously under-resident or over-budget at the wrong moment.

## 7. Depending On Stubbed Or Non-Default Helpers As The Main Plan

If a helper is not on the safe default lane, do not make the whole workflow depend on it.

## 8. Solving Pop-In Only After The Layout Is Final

Streaming design should shape the world early enough that loading logic supports the route structure and state changes.

## Key Takeaway

Most bad streaming setups are not missing systems.

They are missing a clear world-state model, clear spatial logic, or explicit diagnostics.
