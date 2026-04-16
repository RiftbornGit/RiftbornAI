# 03 — Anti-Patterns

Most Blueprint failures are not syntax failures. They are architecture failures that happen to compile.

This document is about the patterns that create graph debt and unstable gameplay behavior.

## Anti-Pattern: The God Blueprint

Symptoms:

- one class owns movement, UI, audio, combat, interaction, VFX, and save logic
- the Event Graph scrolls for screens
- every new feature adds another branch instead of another boundary

Why it is bad:

- nobody can safely reason about what a change will break
- compile success hides design failure
- refactors become risky because everything is coupled

Do instead:

- move reusable capability into components
- move repeated logic into named functions
- split unrelated behavior into separate classes or children only when specialization is real

## Anti-Pattern: Event Graph As The Only Graph

Symptoms:

- almost all logic is inline in Event Graph
- functions exist rarely or not at all
- understanding one feature requires tracing half the graph

Why it is bad:

- intent stays trapped in node layout rather than function names
- simple bug fixes become graph archaeology

Do instead:

- keep Event Graph focused on triggers and routing
- move calculations, checks, and sequences into functions with clear names

## Anti-Pattern: Tick As Default Control Flow

Symptoms:

- constant polling for state that changes infrequently
- per-frame checks for overlap, targeting, or state transitions that should be event-driven
- Tick exists because the correct trigger was never identified

Why it is bad:

- wastes frame budget
- makes causality harder to understand
- creates race-like bugs where one frame of order matters

Do instead:

- use overlap, input, timer, dispatcher, or explicit gameplay events when possible
- reserve Tick for truly continuous behavior

## Anti-Pattern: Hard References Everywhere

Symptoms:

- many class variables point directly at concrete Blueprint types
- logic assumes exact implementations instead of capabilities
- reuse requires copy-paste instead of contract-based interaction

Why it is bad:

- creates brittle dependency chains
- makes replacement and testing harder

Do instead:

- use direct references only when ownership is stable and obvious
- prefer interfaces or event-style communication when multiple classes may respond

## Anti-Pattern: Duplicate Node Islands

Symptoms:

- the same sequence of nodes appears in multiple places
- bug fixes require updating several graphs manually
- copy-paste is used instead of encapsulation

Why it is bad:

- behavior drifts over time
- fixes become incomplete

Do instead:

- extract the repeated logic into a function
- if the reuse crosses classes, reconsider component or parent-child structure

## Anti-Pattern: Mutation Without Context

Symptoms:

- removing or replacing nodes by name guesswork
- editing a Blueprint without checking which graph is focused
- changing defaults or variables without grounding on exact node ids and compile state

Why it is bad:

- easy to damage the wrong graph
- easy to introduce hidden compile or runtime errors

Do instead:

- inspect context first with `get_editor_focus_state`, `focus_asset_editor`, `get_blueprint_editor_context`, `list_blueprint_graphs`, and `list_blueprint_nodes`
- then perform exact mutation with node ids or graph names

## Anti-Pattern: Compiling Only At The End

Symptoms:

- large graph edits happen before any compile check
- failures are discovered after many unrelated changes
- runtime verification is skipped because “it compiled”

Why it is bad:

- error isolation becomes difficult
- compile clean does not guarantee gameplay correctness

Do instead:

- compile after meaningful edits with `compile_blueprint` or `assert_blueprint_compiles`
- use `get_blueprint_compile_diagnostics` when something is unclear
- verify behavior in PIE with `run_quick_playtest`

## Anti-Pattern: Using Restricted Event Wiring As Full Blueprint Authoring

Symptoms:

- trying to force all graph construction through `add_blueprint_event`
- assuming restricted event patterns can replace broader Blueprint architecture work

Why it is bad:

- the tool exists for safe bounded causality patterns, not as a universal graph builder
- pushing beyond that boundary creates brittle or partial authoring flows

Do instead:

- use the restricted event lane for simple event-response behavior
- use the editor-native lane when existing graph structure, exact nodes, or repairs matter

## Anti-Pattern: Renaming Or Reparenting Blindly

Symptoms:

- variables are renamed without checking graph references
- reparenting happens without compile/repair follow-up
- assumptions are made about inherited components and root structure

Why it is bad:

- breaks bindings and assumptions across the class
- turns one structural edit into many hidden downstream errors

Do instead:

- use `rename_blueprint_variable` and then compile immediately
- use `reparent_blueprint` only when the architecture boundary truly changes
- follow with `repair_blueprint_compile_errors` or `validate_and_repair_blueprint_loop` when needed

## Anti-Pattern: Declaring Victory Without Runtime Proof

Symptoms:

- the Blueprint compiles but nobody checks behavior on an actor in context
- UI, overlap, or interaction logic is considered done from compile state alone

Why it is bad:

- many Blueprint bugs are behavioral, not structural
- compile success says nothing about timing, ownership, input, or actor setup

Do instead:

- spawn or exercise the Blueprint in context
- use `run_quick_playtest`
- validate the expected state change, not just the absence of compiler errors

## Key Takeaway

Blueprint debt usually arrives as “just one more node.”

Fight that instinct by asking:

- is this class boundary still clean?
- should this be a function instead of another node island?
- am I editing with exact context or guessing?
- have I proven runtime behavior, not just compile status?
