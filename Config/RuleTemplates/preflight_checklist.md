# Preflight Rules

Before any non-trivial mutating task, complete these checks.

## State the Real Mutation
Name exactly what will change: which asset, which world state, which behavior.
"Make combat feel better" is not a mutation. "Edit dash ability cooldown and
add impact VFX" is a mutation.

## Check Editor State
Before planning tool calls, verify the editor can accept them:
- Check for modal dialogs, restore prompts, save prompts
- Confirm the correct editor surface is open for asset-editor work
- Clear any blockers before continuing

## Pick the Implementation Lane
Choose deliberately:
- World-building (landscape, foliage, lighting)
- Blueprint (gameplay logic, UI binding)
- C++ (subsystems, components, performance-critical)
- Asset-editor (materials, animation, Niagara)
- Sequencer (cinematics, cutscenes)

Do not mix lanes without reason. A Blueprint task should not casually
create C++ classes, and a lighting task should not casually edit materials.
