# RiftbornAI — Design Systems (Deep Dive)

**Last updated**: 2026-02-21

Design Systems are domain-specific knowledge engines that encode professional
game design expertise. Each system understands one domain deeply enough to
translate natural language into concrete UE assets.

Important coverage boundary:

- This document describes internal design-system knowledge modules, not the guaranteed public callable tool surface.
- In particular, `GameDesignSystem` and `AchievementDesignSystem` are not currently grounded public authoring lanes. See [AI_School/COVERAGE_BOUNDARIES.md](AI_School/COVERAGE_BOUNDARIES.md) before treating those names as callable workflows.

---

## 1. Overview

This overview lists the current Design Systems and Knowledge Bases:

### Design Systems

| System | File | Domain |
|--------|------|--------|
| `GameDesignSystem` | `GameDesignSystem.h` | Full game planning — genre, loops, progression, scope, GDD |
| `LevelDesignSystem` | `LevelDesignSystem.h` | Rooms, lighting, PCG, layouts, environment themes |
| `VFXDesignSystem` | `VFXDesignSystem.h` | Niagara VFX — styles, categories, particles, emitter layers |
| `GASDesignSystem` | `GASDesignSystem.h` | Gameplay Ability System — abilities, effects, attributes, combos |
| `AnimationDesignSystem` | `AnimationDesignSystem.h` | Animation specs, montages, blend spaces, notify events |
| `AudioDesignSystem` | `AudioDesignSystem.h` | Sound cues, music states, spatial audio, frequency profiles |
| `PhysicsDesignSystem` | `PhysicsDesignSystem.h` | Physics material specs, constraints, ragdoll |
| `InputDesignSystem` | `InputDesignSystem.h` | Enhanced Input mappings, contexts, triggers |
| `UIDesignSystem` | `UIDesignSystem.h` | Widget specs, HUD elements, menus |
| `CinematicsDesignSystem` | `CinematicsDesignSystem.h` | Sequencer shots, cameras, cutscenes |
| `NetworkingDesignSystem` | `NetworkingDesignSystem.h` | Replication, RPCs, session management |
| `StreamingDesignSystem` | `StreamingDesignSystem.h` | Level streaming, world partition |
| `SaveLoadDesignSystem` | `SaveLoadDesignSystem.h` | Save system design, serialization |
| `LocalizationDesignSystem` | `LocalizationDesignSystem.h` | I18n, text tables, string IDs |
| `AchievementDesignSystem` | `AchievementDesignSystem.h` | Achievement/trophy definitions |

Current public-surface note:

- The presence of a design-system module in source does not mean RiftbornAI ships a matching public tool lane.
- Verify callable workflows against `Bridge/toolbook/public_surface.json`, the shipped contracts, and the live registry before promising a route.

### Knowledge Bases

| Knowledge Base | File | Domain |
|---------------|------|--------|
| `AIBehaviorKnowledgeBase` | `AIBehaviorKnowledgeBase.cpp` | AI patterns (BTs, EQS, perception) |
| `AudioKnowledgeBase` | `AudioKnowledgeBase.cpp` | Audio engineering rules |
| `GameDesignKnowledgeBase` | `GameDesignKnowledgeBase.cpp` | Game design theory |
| `GenreKnowledgeBase` | `GenreKnowledgeBase.cpp` (3 files) | Genre-specific patterns |
| `UEKnowledgeBase` | `UEKnowledgeBase.cpp` | UE API knowledge |

---

## 2. Game Design System (Detail)

**The most complex design system** — encodes professional game design from concept to implementation.

### Genre Classification (40+ genres)

`ActionRPG`, `FPS`, `TPS`, `Platformer`, `Roguelike`, `Roguelite`, `Metroidvania`, `Soulslike`, `OpenWorld`, `Survival`, `SurvivalHorror`, `Horror`, `Stealth`, `Racing`, `Fighting`, `HackAndSlash`, `TurnBasedRPG`, `TacticalRPG`, `RTS`, `TowerDefense`, `MOBA`, `BattleRoyale`, `Simulation`, `Management`, `Puzzle`, `Adventure`, `VisualNovel`, `Sandbox`, `MMO`, `CardGame`, `BoardGame`, `Sports`, `Rhythm`, `Educational`

### Design Pillars

Each game has pillars across categories:
- **CoreGameplay** — The fundamental feel
- **Progression** — Player growth
- **Narrative** — Story and world
- **Social** — Multiplayer/community
- **Aesthetic** — Visual/audio identity
- **Accessibility** — Inclusivity
- **Monetization** — Business model
- **Technical** — Performance targets

### Core Loop Phases

Every game's minute-to-minute loop is decomposed into:

```
Engagement → Challenge → Action → Feedback → Reward → Decision → Progression → Mastery
```

### Progression Types

`CharacterLevel`, `SkillTree`, `EquipmentBased`, `MasteryBased`, `UnlockBased`, `StoryBased`, `ResourceBased`, `RelationshipBased`, `TerritoryBased`, `TimeBased`, `PrestigeBased`, `HybridMultiple`

### Content Scope Categories

Content scope categories include: Characters, NPCs, Enemies, Bosses, Levels,
Zones, Quests, Items, Weapons, Armor, Abilities, Skills, Vehicles, Mounts,
Collectibles, Achievements, Cutscenes, Dialogues, Tutorials, UIScreens,
AudioTracks, VFXEffects

### Monetization Models

`PremiumFullPrice`, `PremiumBudget`, `PremiumIndie`, `FreeToPlay`, `Subscription`, `SeasonPass`, `LiveService`, `Freemium`, `AdSupported`, `DonationWare`, `OpenSource`

---

## 3. VFX Design System (Detail)

Encodes Niagara VFX knowledge — from "make a fire explosion" to actual emitter parameters.

### VFX Category Taxonomy (31 types)

| Domain | Effects |
|--------|---------|
| **Elements** | Fire, Water, Smoke, Lightning, Ice, Earth, Wind |
| **Combat** | Impact, Explosion, Projectile, Slash, Shield, Heal, Debuff, Blood |
| **Ambient** | Sparkle, Dust, Ambient_Glow, Weather |
| **UI/Feedback** | Pickup, LevelUp, Portal, Spawn |
| **Status** | Buff, Poison, Trail, Aura, Teleport, Death |

### VFX Style Variants

Each effect can be styled: `Realistic`, `Stylized`, `Anime`, `Pixel`, `Painterly`, `Sci_Fi`, `Fantasy`, `Horror`, `Toon`, `Minimalist`

### Parameterization

Every VFX spec includes:
- **Color Palette** — Primary, secondary, core (bright), edge (fade), emissive intensity
- **Timing** — Duration, warmup, fade in/out, looping, loop delay
- **Particle Behavior** — Spawn count/rate, burst mode, size range, size-over-life, velocity, gravity, drag, turbulence, rotation rate, lifetime
- **Emitter Layers** — Multiple layers composited (e.g., core glow + sparks + smoke)

---

## 4. GAS Design System (Detail)

Encodes Gameplay Ability System architecture for generating working abilities.

### Ability Types

`Instant`, `Duration`, `Toggle`, `Passive`, `Channeled`, `Combo`, `Ultimate`

### Ability Specification

Each ability spec includes:
- **Activation** — OnButtonPress, OnButtonRelease, OnButtonHeld, OnEvent, Passive, OnTargetConfirm
- **Targeting** — Self, SingleTarget, AOE_Point, AOE_Self, Cone, Line, Projectile, Hitscan
- **Cost** — Mana, Stamina, Health, Resource, Charges, Ammo, Custom
- **Cooldown** — Duration, shared groups, charge count, charge recovery
- **Effects** — Modifiers with operations (Additive, Multiplicative, Override), magnitude types (Fixed, ScalableFloat, AttributeBased, SetByCaller, CustomCalculation)

### Attribute Categories

`Core` (Health, Mana, Stamina), `Combat` (Attack, Defense, Speed), `Stats` (STR, DEX, etc.), `Resources` (Ammo, Charges), `Multipliers` (DamageBonus, CritChance)

---

## 5. Level Design System (Detail)

Encodes level layout, lighting, and PCG knowledge.

### Room Types

`Arena`, `Corridor`, `Chamber`, `Vault`, `Throne`, `Treasure`, `Spawn`, `Safe`, `Puzzle`, `Shop`, `Hub`, `Outdoor`

### Layout Patterns

`Linear`, `Branching`, `Hub`, `Grid`, `Organic`, `Circular`, `Layered`, `Maze`, `Open`

### Environment Themes

`Medieval`, `SciFi`, `Fantasy`, `Modern`, `Horror`, `Desert`, `Forest`, `Cave`, `Urban`, `Industrial`, `Temple`, `Castle`, `Laboratory`

### PCG Content Types

`Props`, `Foliage`, `Rocks`, `Debris`, `Scatter`, `Lights`, `Enemies`, `Pickups`, `Navigation`

### Lighting Specs

Full structured lighting setup:
- Sky light (color, intensity)
- Directional light (color, intensity, rotation)
- Point lights (count, color, intensity, radius)
- Volumetric fog (enabled, color, density)

---

## 6. Generators

Design systems are consumed by **Generators** — JSON-defined execution plans.

```
Generators/
├── arena_map/           → Arena level generator
├── cover_flow/          → Cover + navmesh flow pass
├── vfx_template/        → VFX preset → Niagara → spawn
├── cinematic_camera/    → Viewport framing shot
├── scene_delta/         → Highlight tagged actors
├── gameplay_loop/       → Full gameplay loop wiring
├── cpp_component/       → C++ component generation
├── layout_mutator/      → Room layout mutations
└── schema/              → JSON Schema for validation
```

### Execution Flow

```
1. User clicks "Generate Arena" in Slate panel
2. C++ GeneratorRunner::Execute("arena_map", params)
3. Runner loads Generators/arena_map/arena_map.v1.json
4. For each step:
   - Validate risk matches expectation
   - Check if needs_confirmation requires UI gate
   - Execute via tool registry
   - Emit proof/trajectory
5. Return result with job_id and proof pointer
```

### Risk Model

Steps declare expected risk tier:
- `safe` — Read-only, no side effects
- `mutation` — Modifies project (reversible)
- `dangerous` — Requires confirmation
- `destructive` — May lose data (always confirmed)

If actual tool risk exceeds declared expectation → plan fails.

---

## 7. Vertical Slice Generator

The broadest generator: produces a **cross-referenced vertical-slice spec bundle** from a single prompt.

"Make a fire mage champion with fireball, flame wall, and phoenix ultimate"

Generates:
- C++ ability classes (GAS)
- VFX for each ability (Niagara emitter layers)
- Audio specs (cast, impact, loop)
- Animation specs (cast montage, channel loop)
- HUD widgets (ability slots, mana bar, cooldown indicators)
- Network replication specs (RPCs, multicast)
- Data table rows (ability stats, scaling curves)

All cross-referenced and wired together.

---

## 8. Game Orchestrator

Understands "make a roguelike" and wires up the right systems.

### Genre → System Mapping

Each genre maps to required/optional systems:

```
Roguelike → [
    Required: GameMode, PlayerCharacter, ProceduralGeneration, 
              RoomSystem, Permadeath, Combat, Health
    Optional: MetaProgression, Abilities, Inventory, Crafting
]

Soulslike → [
    Required: GameMode, PlayerCharacter, Combat, Stamina,
              WeaponSystem, BonfireSystem, SoulsProgression
    Optional: Invasions, SummoningSigns, Covenants
]
```

### System Catalog (80+ systems)

Core, Gameplay (Combat, Abilities, Health, Stamina, Mana, XP, Leveling, Inventory, Equipment, Crafting), AI (EnemyAI, BT, Perception, Pathfinding), World (ProceduralGen, Rooms, Dungeons, Biomes, Spawning), Meta (Permadeath, MetaProgression, SaveSystem, Achievements), UI (HUD, Menus, InventoryUI, DialogueUI), Networking, Polish (VFX, Audio, Animation, Cinematics), plus genre-specific systems.

---

## 9. Automated Playtester

AI-driven game testing that plays like different human personas.

### Personas (14 types)

| Category | Personas |
|----------|---------|
| **Skill** | Newbie, Casual, Core, Hardcore, Speedrunner |
| **Style** | Explorer, Rusher, Completionist, Social, Creative |
| **QA** | QATester, BalanceTester, AccessibilityTester |

### Issue Classification

| Severity | Categories |
|----------|-----------|
| Critical, Major, Minor, Suggestion | Bug, Balance, UX, Performance, Accessibility, Polish |

### Each issue includes:
- Steps to reproduce
- Location (level + world position)
- Screenshot path
- Reproduction rate
- Auto-suggested fix with confidence score
