# 01 — Ability Design And State Model

An ability is not a button animation. It is a contract between the game and the player.

When the player presses a button, the system needs unambiguous answers:

- Can this ability activate right now?
- What resource does it consume?
- What target model does it use?
- What state does it apply?
- What stops it from activating again immediately?
- How does it interact with other abilities, effects, and tags?

If you cannot answer those questions, you are not ready to author GAS assets.

## The Five Parts Of An Ability

### 1. Trigger

How the ability begins.

- button press
- button release
- held input
- passive condition
- event-driven activation

The trigger must match player expectation. A charge shot and an instant dash should not feel like the same activation model.

### 2. Targeting

Who or what the ability affects.

- self
- single target
- point in space
- area around self
- cone
- line
- projectile
- hitscan

Targeting is part of the ability's identity, not a later implementation detail.

### 3. Cost

What the player gives up to activate it.

- mana
- stamina
- health
- ammo
- charges
- a temporary opportunity cost such as locking out another state

Cost is how a system creates tradeoffs. Without cost or opportunity cost, many abilities collapse into spam.

### 4. Cooldown Or Availability Rule

Why the ability cannot be reused immediately.

- simple cooldown
- shared cooldown group
- charge recovery
- combo window
- tag-gated lockout

Cooldown is not just pacing. It shapes build choices, sequencing, and perceived power.

### 5. State Change

What the ability actually does to the game.

- damage
- heal
- buff
- debuff
- movement
- summoning
- crowd control
- tag application or removal

This is where Gameplay Effects and tags matter. The ability itself is often the trigger and orchestration layer, while effects and attributes carry the real state changes.

## Think In Interactions, Not Isolated Abilities

A single ability may look fine in isolation and still damage the overall system.

Ask:

- Does this share resources with other abilities?
- Does it create or consume tags another ability depends on?
- Can it bypass intended cooldown or combo rules?
- Does it punish the player for using related abilities at the wrong time?
- Does it create a dominant strategy?

GAS gets its power from systemic interactions. That also means mistakes compound quickly.

## Tags Are Gameplay Language

Gameplay tags should describe state and rules clearly.

Examples:

- `Ability.Skill.Fireball`
- `State.Burning`
- `Cooldown.Fire`
- `Status.Stunned`
- `Resource.ManaBlocked`

Good tags:

- are hierarchical
- are consistent
- describe intent
- can be reused by multiple systems

Bad tags:

- are one-off hacks
- mix unrelated meanings
- encode presentation instead of gameplay state

## Attributes Are Not Just Numbers

Attributes define the balance surface of the game.

Typical categories:

- survivability: Health, MaxHealth
- energy: Mana, Stamina, MaxMana, MaxStamina
- combat: Attack, Defense, CritChance
- progression: scaling stats or derived multipliers

Only create attributes that have a real systemic job. Every new attribute increases balance and verification cost.

## Abilities Need Failure States

A good ability definition includes why activation can fail:

- insufficient resource
- target invalid
- blocked by tag
- on cooldown
- wrong range
- wrong stance or state

Players feel bad when abilities fail unclearly. Designers feel pain when failure rules are implicit and spread across ad hoc logic.

## Persistent Effects Need Policy

When an effect lasts, you must define:

- duration policy: instant, duration, or infinite
- period if it ticks over time
- stacking rules
- overflow behavior
- expiration behavior

If you do not define these deliberately, persistent effects often become the source of bugs, exploit loops, or unreadable combat state.

## Replication Is Design, Not Just Networking

In multiplayer or replicated contexts, ability rules must stay authoritative and legible.

You should know:

- what can predict locally
- what must be server authoritative
- what state the player needs immediate feedback on

Even in early prototyping, ignoring replication often bakes bad assumptions into the ability contract.

## Key Takeaway

Before authoring an ability, write down:

- trigger
- target model
- cost
- cooldown or lockout rule
- state change
- tags involved
- failure conditions
- interactions with other abilities

If that design is weak, the GAS asset will only preserve the weakness more formally.
