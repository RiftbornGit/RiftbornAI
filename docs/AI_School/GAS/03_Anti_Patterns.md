# 03 — Anti-Patterns

These are the failure modes most likely to make a GAS-based system brittle, unreadable, or misleading.

## NEVER: Start By Creating A `GA_*` Asset Without A Design Contract

**What goes wrong:**

- the ability asset exists but its trigger, targeting, cost, and failure rules are undefined
- later systems bolt on ad hoc conditions and exceptions

**Do instead:** Define the ability contract first. Trigger, target model, cost, cooldown, state change, and tags should be clear before creation.

## NEVER: Assume Asset Creation Equals A Working Ability

**What goes wrong:**

- the repo now contains an ability asset, but no meaningful runtime behavior exists
- agents report progress based on files created rather than interactions verified

**Do instead:** Treat asset creation as scaffolding. Continue through effect configuration, actor assignment, runtime inspection, and playtest.

## NEVER: Invent Tags Ad Hoc During Wiring

**What goes wrong:**

- ability gating and effect rules depend on tags that were never registered
- naming becomes inconsistent across systems
- debugging becomes painful because tags stop acting like a shared language

**Do instead:** Register tags deliberately with a coherent naming scheme before using them.

## NEVER: Model Cooldowns And Costs Outside GAS When The System Should Own Them

**What goes wrong:**

- timers, booleans, and resource checks drift into scattered custom logic
- shared cooldown groups and status interactions become hard to reason about

**Do instead:** Use gameplay effects, tags, and explicit cooldown/cost assignment when the mechanic belongs in the ability system.

## NEVER: Create Attributes Without A System Job

**What goes wrong:**

- the project fills with numbers that no larger system needs
- balance surface grows faster than the game design justifies

**Do instead:** Only create attributes that support real combat, progression, or resource interactions.

## NEVER: Grant Abilities To Actors That Are Not GAS-Ready

**What goes wrong:**

- assignment fails or becomes meaningless
- later debugging focuses on the wrong layer

**Do instead:** Verify the actor has an AbilitySystemComponent and can actually host abilities.

## NEVER: Design An Ability In Isolation

**What goes wrong:**

- the new ability bypasses combo rules, cooldown pacing, or shared resources
- status effects stack in degenerate ways
- one ability becomes obviously dominant

**Do instead:** Evaluate every ability against the rest of the system, not just its own happy path.

## NEVER: Skip Runtime Inspection

**What goes wrong:**

- you assume granted abilities are present
- you assume attributes changed
- you assume effects are active or expiring correctly

**Do instead:** Use `get_gas_abilities`, `get_gas_attributes`, and `get_active_effects` after wiring.

## NEVER: Skip Playtest Because The Blueprints Compile

**What goes wrong:**

- compile-clean assets still feel wrong, activate late, mis-target, or create spammy loops
- interaction bugs only appear in motion and sequence

**Do instead:** Use `run_quick_playtest` after meaningful GAS changes, especially after new costs, cooldowns, or stacking behavior.

## NEVER: Forget Replication Implications

**What goes wrong:**

- locally it appears correct
- in networked or authority-sensitive contexts it diverges

**Do instead:** Treat replication policy as part of the ability contract, not an afterthought.

## Golden Rule

**A good GAS change is not “asset exists.” It is “runtime behavior is explicit, inspectable, and playtested.”**
