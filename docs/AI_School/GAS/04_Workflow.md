# 04 — The Correct GAS Workflow

Build the system contract first, then the assets, then the runtime wiring.

## Phase 1: Define The Ability Contract

### Step 1.1 — Write The Design Brief

For each ability, decide:

- trigger
- targeting model
- state change
- cost
- cooldown or lockout rule
- relevant gameplay tags
- failure conditions

Do not create assets until this is explicit.

### Step 1.2 — Check System Interactions

Ask:

- what resource pool does this share?
- what tags block or enable it?
- what other abilities should it combo with?
- what states should cancel it or prevent it?

## Phase 2: Define Tags And Attributes

### Step 2.1 — Register Gameplay Tags

Use:

- `add_gameplay_tag`
- `get_gameplay_tags`
- `create_gameplay_tag_table`

Establish tags before wiring abilities or effects to them.

### Step 2.2 — Define Attributes Deliberately

If needed, create the attribute surface with:

- `create_attribute_set`

Remember: this scaffolds the asset. It does not replace broader gameplay integration work.

## Phase 3: Create Effects Before Final Ability Wiring

### Step 3.1 — Create Gameplay Effects

Use:

- `create_gameplay_effect`

Think in categories:

- cost effects
- cooldown effects
- damage or heal effects
- persistent buff/debuff effects

### Step 3.2 — Configure Duration And Ticking

Use:

- `configure_gameplay_effect`

Set:

- duration policy
- period
- chance to apply
- periodic execute-on-application behavior

### Step 3.3 — Configure Stacking

Use:

- `configure_ge_stacking`

Do this before multiple abilities or repeated applications depend on the same effect behavior.

## Phase 4: Create The Ability Asset

### Step 4.1 — Create Ability Blueprint

Use:

- `create_gameplay_ability`

This creates the asset scaffold. It is not the end of the job.

### Step 4.2 — Assign Cost, Cooldown, And Policies

Use:

- `set_ability_cost`
- `set_ability_cooldown`
- `set_ability_policies`

These steps turn an empty shell into a system participant.

## Phase 5: Validate Asset Health

### Step 5.1 — Check Compile State

Use:

- `assert_blueprint_compiles`
- `validate_blueprint_health`

Blueprint health is necessary before runtime verification.

## Phase 6: Discover And Assign

### Step 6.1 — Confirm Assets

Use:

- `get_gas_assets`

Make sure the intended assets exist in the expected paths.

### Step 6.2 — Grant To A Real Actor

Use:

- `add_ability_to_actor`
- `set_actor_gameplay_tags`

If the actor is not GAS-ready, stop and solve that integration problem instead of pretending the ability is playable.

## Phase 7: Inspect Runtime State

### Step 7.1 — Verify Asset Presence And Grant Path

Use:

- `get_gas_assets`

This proves the authored ability, effect, and attribute assets exist on the current public surface before runtime validation.

### Step 7.2 — Verify Runtime Behavior In Play

Use:

- `run_quick_playtest`
- `audit_net_replication` and `inspect_actor_replication` when multiplayer truth matters

The current default public lane does not expose actor-level GAS state getters. This is where you confirm gameplay behavior and replication truth instead of assuming the authored assets are enough.

## Phase 8: Playtest

### Step 8.1 — Validate Feel And Interaction

Use:

- `run_quick_playtest`

Check:

- activation timing
- cost consumption
- cooldown behavior
- repeatability
- interaction with nearby abilities or enemies

## Short Workflow Summary

`define contract` -> `register tags` -> `define attributes` -> `create effects` -> `configure effect duration/stacking` -> `create ability` -> `set cost/cooldown/policies` -> `validate blueprint health` -> `grant to actor` -> `inspect runtime state` -> `playtest`

If an ability fails at runtime, go back to the contract and effect configuration layers before adding more content around it.
