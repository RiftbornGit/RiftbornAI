# 01 — Class Boundaries And Responsibilities

The hardest part of Unreal C++ is usually not syntax. It is choosing where behavior belongs.

## Start With Ownership

Before writing code, answer:

- who owns this behavior?
- how long should it live?
- does it need world presence?
- does it need transform data?
- should many classes reuse it?

Those questions usually determine the class type.

## Actor

Use an `Actor` when the thing is a world object with its own identity.

Good fit:

- destructible prop
- trigger volume actor
- interactable world object
- spawned environmental hazard

Bad fit:

- reusable health logic
- quest manager
- player-wide inventory rules shared across many classes

If the code is mostly a reusable behavior rather than a world entity, it probably does not belong in an Actor.

## Character

Use a `Character` when the class is fundamentally about a controllable or simulated pawn with locomotion.

Good fit:

- player pawn
- humanoid enemy
- NPC with capsule movement and movement-based gameplay

Do not choose `Character` just because the class might someday move. Character brings a lot of assumptions and weight.

## ActorComponent vs SceneComponent

Use a component when behavior should be reusable across owners.

### ActorComponent

Use when the behavior does not need its own transform.

Good fit:

- inventory
- health
- interaction state
- cooldown manager

### SceneComponent

Use when the behavior needs a transform or attachment point.

Good fit:

- aim origin
- sensor boom
- attachable interaction anchor

The rule is simple:

- no transform needed -> `ActorComponent`
- transform needed -> `SceneComponent`

## GameMode

Use `GameMode` for match or rules authority.

Good fit:

- win/loss logic
- round flow
- spawn rules
- score or mode-specific victory conditions

Bad fit:

- long-lived services unrelated to the current match
- systems that many maps should access regardless of mode

GameMode is about the current rules of play, not about being “the main place for code.”

## Subsystem

Use a subsystem for a scoped service with a clear engine lifetime.

The supported lane explicitly distinguishes:

- `GameInstance` subsystem
- `World` subsystem
- `LocalPlayer` subsystem

### GameInstance Subsystem

Use for services that should live across map loads for the current running game session.

### World Subsystem

Use for services that belong to one world instance.

### LocalPlayer Subsystem

Use for per-local-player services.

Do not call everything a subsystem. The point is to match engine lifetime, not to create abstract “manager” classes by reflex.

## GameMode vs Subsystem

A common confusion:

- GameMode = current match rules
- Subsystem = service lifetime tied to engine scope

If the code governs the match, it leans GameMode.
If the code provides a service across a scope, it leans Subsystem.

## Component vs Subsystem

Another common confusion:

- Component = behavior attached to an owner
- Subsystem = service without needing that actor ownership model

If many different actors need the behavior through attachment, prefer a component.
If the behavior is a shared scoped service, prefer a subsystem.

## Blueprint-To-C++ Is A Separate Decision

Do not choose class boundaries by looking at the current Blueprint shape alone.

Sometimes the right answer is:

- keep a Blueprint
- convert only part of it
- move reusable logic into a component
- move rule logic into GameMode or a subsystem

Blueprint structure is a clue, not a verdict.

## Key Takeaway

Choose class type by:

- ownership
- lifetime
- need for transform
- reuse
- authority

If you skip those questions, you will generate the wrong class efficiently.
