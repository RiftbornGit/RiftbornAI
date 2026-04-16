# 05 — RiftbornAI Tool Guide For GAS Work

Use exact registered tool names. If a tool name is uncertain, confirm with `describe_tool`.

## Tags And Shared Vocabulary

| Task | Tool | Notes |
|---|---|---|
| Add a project gameplay tag | `add_gameplay_tag` | Register tags before using them in logic |
| List registered project tags | `get_gameplay_tags` | In current surfaces this name is overloaded; use `describe_tool` if the active route is unclear |
| Create a tag data table | `create_gameplay_tag_table` | Useful for batch tag definition workflows |

Design rules:

- register tags before creating abilities, effects, or actor assignments that reference them
- treat tags as a stable shared vocabulary — do not invent ad hoc tag names per-asset
- use hierarchical tag structure (e.g., `Ability.Combat.Strike`) for filtering and category queries

## Core GAS Asset Creation

| Task | Tool | Notes |
|---|---|---|
| Create ability asset | `create_gameplay_ability` | Creates a `GA_*` Blueprint scaffold |
| Create gameplay effect | `create_gameplay_effect` | Creates a `GE_*` Blueprint scaffold with duration policy |
| Create attribute set | `create_attribute_set` | Creates an AttributeSet Blueprint scaffold |
| List GAS assets | `get_gas_assets` | Discover abilities, effects, and attributes in the project |

Design rules:

- check existing assets with `get_gas_assets` before creating new ones — prevents duplication
- effects come before abilities in authoring order because abilities reference their cost and cooldown effects
- attribute sets define the numeric contracts — create them before the effects that modify them

## Effect And Ability Configuration

| Task | Tool | Notes |
|---|---|---|
| Configure effect timing and chance | `configure_gameplay_effect` | Duration policy, period, execute-on-application, chance-to-apply |
| Configure effect stacking | `configure_ge_stacking` | Stack type, limits, overflow, expiration behavior |
| Assign cooldown effect to ability | `set_ability_cooldown` | Gameplay Effect must already exist |
| Assign cost effect to ability | `set_ability_cost` | Gameplay Effect must already exist |
| Configure ability runtime policies | `set_ability_policies` | Instancing, net execution, net security, replication policy |

Design rules:

- configure stacking intentionally — unlimited stacking of damage effects or buffs causes balance problems
- create cost and cooldown effects before referencing them from abilities
- set replication policies on abilities that will be used in multiplayer contexts

## Actor Assignment And Runtime Wiring

| Task | Tool | Notes |
|---|---|---|
| Grant ability to actor | `add_ability_to_actor` | Requires an actor with a usable ASC |
| Add gameplay tags to actor | `set_actor_gameplay_tags` | Actor-level tag application via ASC |

## Runtime Verification

| Task | Tool | Notes |
|---|---|---|
| Inspect granted abilities | `get_gas_abilities` | Verify the actor actually has the expected ability specs |
| Inspect attribute values | `get_gas_attributes` | Verify spawned attribute sets and values |
| Inspect active effects | `get_active_effects` | Useful after applying buffs, debuffs, or cooldowns |
| Quick interaction playtest | `run_quick_playtest` | Validate feel and interaction after GAS changes |

Design rules:

- verify on a real actor with an AbilitySystemComponent, not just the asset in isolation
- check attributes after effects apply to confirm values moved as expected
- playtest is the final gate — ability assets that compile but feel wrong during play are not done

## Verification Checklist

Before calling a GAS surface done:

- [ ] tags registered (`add_gameplay_tag`)
- [ ] attribute sets created with expected attributes (`create_attribute_set`)
- [ ] effects configured with correct duration, stacking, and modifiers (`configure_gameplay_effect`, `configure_ge_stacking`)
- [ ] abilities reference correct cost and cooldown effects (`set_ability_cost`, `set_ability_cooldown`)
- [ ] Blueprints compile clean (`assert_blueprint_compiles`)
- [ ] abilities granted to a real actor (`add_ability_to_actor`)
- [ ] runtime state verified (`get_gas_abilities`, `get_gas_attributes`, `get_active_effects`)
- [ ] interaction feels correct in gameplay (`run_quick_playtest`)
- [ ] stacking, cooldowns, and costs behave as designed under repeated use

## Blueprint Health

| Task | Tool | Notes |
|---|---|---|
| Prove Blueprint compiles | `assert_blueprint_compiles` | Use after creating or modifying GAS Blueprints |
| Validate Blueprint health | `validate_blueprint_health` | Broader health check for Blueprint assets |

## Recommended Default Flows

### New Active Ability

`add_gameplay_tag` -> `create_gameplay_effect` -> `configure_gameplay_effect` -> `create_gameplay_ability` -> `set_ability_cost` -> `set_ability_cooldown` -> `set_ability_policies` -> `assert_blueprint_compiles` -> `add_ability_to_actor` -> `get_gas_abilities` -> `run_quick_playtest`

Use for: combat strikes, spells, dodge rolls, interaction abilities — any discrete player action with cost and cooldown.

### New Persistent Buff Or Debuff

`add_gameplay_tag` -> `create_gameplay_effect` -> `configure_gameplay_effect` -> `configure_ge_stacking` -> `assert_blueprint_compiles` -> `add_ability_to_actor` or effect application path -> `get_active_effects` -> `run_quick_playtest`

Use for: poison DOT, armor buff, speed boost, stat debuffs — anything that persists over time with stacking rules.

### Resource-System Setup

`create_attribute_set` -> `assert_blueprint_compiles` -> `get_gas_assets` -> actor integration work -> `get_gas_attributes`

Use for: health/mana/stamina systems, hunger/thirst meters, crafting resource pools.

### Effect Tuning Pass

`get_gas_assets` -> `get_active_effects` -> `configure_gameplay_effect` -> `configure_ge_stacking` -> `assert_blueprint_compiles` -> `get_gas_attributes` -> `run_quick_playtest`

Use for: rebalancing existing effects, adjusting stack limits, changing duration policies.

## Rules

1. Do not confuse registry state with runtime state.
2. Do not assume a created asset is a complete system.
3. Verify on a real actor after assignment.
4. Use playtest after changing costs, cooldowns, stacking, or replication-related policies.
5. Create effects before abilities — abilities reference their cost and cooldown effects.
6. Define attribute sets early — effects that modify attributes need the target attributes to exist.
7. Treat tags as shared contracts, not throwaway labels.

## Key Takeaway

The GAS surface is strongest when you separate: tag vocabulary, attribute definition, effect configuration, ability wiring, and runtime verification. Skipping any layer produces abilities that look created but behave incorrectly under gameplay pressure.
