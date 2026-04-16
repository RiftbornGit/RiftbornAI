# 02 — UE5 GAS Systems

UE5's Gameplay Ability System is a runtime state framework. The important point is not that it has many classes. The important point is that each class has a clear responsibility.

## Core Runtime Pieces

### AbilitySystemComponent (ASC)

This is the authority holding abilities, effects, tags, and attributes for an actor.

From [docs/UE5_CPP_API_REFERENCE.md](../../UE5_CPP_API_REFERENCE.md):

- the actor should expose an ASC through `IAbilitySystemInterface`
- the ASC should be replicated when multiplayer behavior matters
- the ASC is the object that grants abilities and applies effects

If the actor has no ASC, GAS asset work will not become playable simply by existing.

### Gameplay Ability

A `UGameplayAbility` defines activation and execution behavior.

Typical jobs:

- activation gating
- orchestration
- choosing targets
- applying effects
- spawning related gameplay feedback

### Gameplay Effect

A `UGameplayEffect` is the data-driven state change surface.

Typical jobs:

- damage
- heal
- buff
- debuff
- periodic state change
- persistent aura or lockout state

Effects should carry real state changes whenever possible instead of burying them in one-off logic.

### Attribute Set

A `UAttributeSet` holds gameplay attributes such as Health, Mana, and Stamina.

Attribute sets are not "just data containers." They define which numbers the game treats as first-class system state.

### Gameplay Tags

Tags are the shared vocabulary of the system:

- ability identity
- requirements
- cooldown groups
- states
- status effects
- activation blockers

Good GAS architecture uses tags as reusable logic hooks, not as disconnected labels.

## What RiftbornAI Currently Supports Cleanly

The supported GAS surface in this repo is strong for runtime asset scaffolding and wiring, but it is not a full restored planning/codegen layer yet.

Current supported tools include:

- tag registry and tag-table creation
- ability, effect, and attribute-set asset creation
- gameplay-effect configuration
- gameplay-effect stacking configuration
- ability cost/cooldown/policy assignment
- granting an ability to an actor with an ASC
- runtime awareness of abilities, attributes, and active effects

The comment in `Source/RiftbornAI/Public/Tools/GASToolsModule.h` is explicit: planning, preset, and codegen helpers are intentionally omitted until backed by working implementations.

That means the agent must think clearly before asset creation. The system will not do the design thinking for you.

## Supported Authoring Tools

### Tags

- `add_gameplay_tag`
- `get_gameplay_tags`
- `create_gameplay_tag_table`

Use these before you depend on tags in ability or effect rules.

### Core Asset Creation

- `create_gameplay_ability`
- `create_gameplay_effect`
- `create_attribute_set`
- `get_gas_assets`

These create or discover assets. They do not magically invent balanced rules.

### Effect And Ability Configuration

- `configure_gameplay_effect`
- `configure_ge_stacking`
- `set_ability_cooldown`
- `set_ability_cost`
- `set_ability_policies`

These are where runtime behavior starts to become systemic instead of merely present.

### Runtime Assignment

- `add_ability_to_actor`
- `set_actor_gameplay_tags`

These are actor-level mutation steps. They only make sense on actors that are actually GAS-ready.

## Verification Tools

For runtime inspection, use:

- `get_gas_abilities`
- `get_gas_attributes`
- `get_active_effects`
- `run_quick_playtest`

For blueprint asset health around created GAS Blueprints, use:

- `assert_blueprint_compiles`
- `validate_blueprint_health`

## Important Limitation: Asset Creation Is Scaffolding

The current `create_gameplay_ability`, `create_gameplay_effect`, and `create_attribute_set` tools create Blueprint assets and basic defaults. They do not fully implement combat logic, target handling, prediction, montage timing, or UI feedback by themselves.

That matters because:

- a created ability asset may still do nothing meaningful at runtime
- an effect may exist but not be attached as cost or cooldown
- an attribute set may exist but not yet expose the right properties for the wider game

Treat creation as the start of the workflow, not the end.

## Verification Mindset

The minimum useful loop for GAS work is:

1. design the state change
2. create or update the relevant tags/effects/ability assets
3. grant the ability to a real actor with an ASC
4. inspect runtime state
5. playtest the interaction

Without actor-level verification, GAS work remains theoretical.

## Key Takeaway

Use UE5 GAS well by keeping responsibilities clean:

- ASC owns runtime state
- ability orchestrates
- effect modifies state
- attribute set stores numbers
- tags describe rules and state

Then use RiftbornAI's supported tool surface to scaffold, configure, assign, and verify each layer deliberately.
